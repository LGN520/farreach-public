#include "com_inswitchcache_core_SocketHelper.h"

#include <unistd.h> // close
#include <cstring> // memset
#include <cstdlib> // exit

#include <sys/socket.h> // socket API
#include <netinet/in.h> // struct sockaddr_in
#include <errno.h> // errno
#include <net/if.h> // struct ifreq; ifname -> ifidx
//#include <sys/ioctl.h> // ioctl
#include <netpacket/packet.h> // sockaddr_ll
#include <arpa/inet.h> // inetaddr conversion; endianess conversion
#include <netinet/tcp.h> // TCP_NODELAY

#include "../../../common/helper.h"
#include "../../../common/key.h"
#include "../../../common/dynamic_array.h"
#include "../../../common/socket_helper.h" // including IP_FRAGMENT_MAXSIZE, IP_FRAGMENT_RESERVESIZE

// JNI helper funcs

inline bool jboolean2bool(jboolean &jbool) {
	return (jbool == JNI_TRUE);
}

const char *jstring2charptr(JNIEnv *jenv, jstring &jstr, bool &iscopy) {
	jboolean iscopy_str;
	const char *result = jenv->GetStringUTFChars(jstr, &iscopy_str);
	iscopy = (iscopy_str == JNI_TRUE);
	return result;
}

inline int jint2int(jint &jintval) {
	return (int)jintval;
}

inline short jshort2short(jshort &jshortval) {
	return (short)jshortval;
}

inline jint int2jint(int &intval) {
	return (jint)intval;
}

inline jint int2jint(const int &intval) {
	return (jint)intval;
}

inline jshort short2jshort(short &shortval) {
	return (jshort)shortval;
}

char *jbytearray2charptr(JNIEnv *jenv, jbyteArray &jbuf, bool &iscopy) {
	jboolean iscopy_buf;
	char *buf = (char *)(jenv->GetByteArrayElements(jbuf, &iscopy_buf));
	iscopy = (iscopy_buf == JNI_TRUE);
	return buf;
}

jstring charptr2jstring(JNIEnv *jenv, const char *charptr) {
	return jenv->NewStringUTF(charptr);
}

// store data and free memory if necessary

void saf_jstring(JNIEnv *jenv, jstring &jstr, const char *charptr, const bool &iscopy) {
	// NOTE: charptr is const char *, so we never change the data of jstr -> NOT need to store data
	if (iscopy) { // need to free memory
		jenv->ReleaseStringUTFChars(jstr, charptr);
	}
}

void saf_jbytearray(JNIEnv *jenv, jbyteArray &jbuf, char *charptr, const bool &iscopy, bool needstore) {
	// NOTE: if needstore = false, we only need to release bytearray when iscopy = true
	if (needstore || iscopy) {
		// mode = 0 means copy back data from charptr into jbuf, and free memory of charptr if necessary
		jenv->ReleaseByteArrayElements(jbuf, (jbyte *)charptr, 0);
	}
}

static void copy_srcaddr_to_judpaddr(JNIEnv *jenv, struct sockaddr_in &srcaddr, jobject &judpaddr) {
	char srcipaddr[256];
	memset(srcipaddr, '\0', 256);
	inet_ntop(AF_INET, (void *)&(srcaddr.sin_addr), srcipaddr, 256);
	jstring jsrcipaddr = charptr2jstring(jenv, srcipaddr);

	short srcport = ntohs(srcaddr.sin_port);
	jshort jsrcport = short2jshort(srcport);

	jclass udpaddr_cls = jenv->GetObjectClass(judpaddr);
	jfieldID ipaddr_fieldid = jenv->GetFieldID(udpaddr_cls, "ipAddr", "Ljava/lang/String;");
	jenv->SetObjectField(judpaddr, ipaddr_fieldid, jsrcipaddr);
	jfieldID udpport_fieldid = jenv->GetFieldID(udpaddr_cls, "udpPort", "S");
	jenv->SetShortField(judpaddr, udpport_fieldid, jsrcport);

	// release jstring
	jenv->DeleteLocalRef(jsrcipaddr);
}

static void copy_dynamicbuf_to_jdynamicbuf(JNIEnv *jenv, DynamicArray &dynamicbuf, jobject &jdynamicbuf) {
	int bufsize = dynamicbuf.size();
	jint jtmp_off = 0;
	jint jtmp_len = int2jint(bufsize);
	jbyteArray jtmp_dynamicbuf = jenv->NewByteArray(bufsize);
	jenv->SetByteArrayRegion(jtmp_dynamicbuf, 0, bufsize, (jbyte*)dynamicbuf.array());

	jclass dynamicbuf_class = jenv->GetObjectClass(jdynamicbuf);
	jmethodID dynamicmemcpy_method = jenv->GetMethodID(dynamicbuf_class, "dynamicMemcpy", "(I[BI)V");
	jenv->CallVoidMethod(jdynamicbuf, dynamicmemcpy_method, jtmp_off, jtmp_dynamicbuf, jtmp_len);

	// release jbyteArray
	jenv->DeleteLocalRef(jtmp_dynamicbuf);
}

static netreach_key_t jkey2key(JNIEnv *jenv, jobject &jkey) {
    jclass key_class = jenv->GetObjectClass(jkey);
    jmethodID serialize_method = jenv->GetMethodID(key_class, "serialize", "([BII)I");

    const int tmpoff = 0;
    const int keysize = 16;
    jbyteArray jtmp_keybytes = jenv->NewByteArray(keysize);
    jint jtmpoff = int2jint(tmpoff);
    jint jkeysize = int2jint(keysize);
    jenv->CallIntMethod(jkey, serialize_method, jtmp_keybytes, jtmpoff, jkeysize);

    bool iscopy_buf = false;
    char* tmp_keybytes = jbytearray2charptr(jenv, jtmp_keybytes, iscopy_buf);

    netreach_key_t key;
    key.deserialize(tmp_keybytes, keysize);

    // release const char* if necessary
    saf_jbytearray(jenv, jtmp_keybytes, tmp_keybytes, iscopy_buf, false);
    // release jbyteArray
    jenv->DeleteLocalRef(jtmp_keybytes);
    return key;
}

// Range query helper

static void encodeDynamicbufs(DynamicArray* dynamicbufs, int bufnum, DynamicArray &dst_dynamicbuf) {
	// FORMAT: <int bufnum> + <int buf0_size> + <byte[] buf0_bytes> + <int buf1_size> + <byte[] buf1_bytes> + ...
	int tmpoff = 0;
	dst_dynamicbuf.dynamic_memcpy(tmpoff, (char *)&bufnum, sizeof(int));
	tmpoff += sizeof(int);
	for (int i = 0; i < bufnum; i++) {
	    int tmpsize = dynamicbufs[i].size();
	    dst_dynamicbuf.dynamic_memcpy(tmpoff, (char *)&tmpsize, sizeof(int));
	    tmpoff += sizeof(int);

	    dst_dynamicbuf.dynamic_memcpy(tmpoff, dynamicbufs[i].array(), tmpsize);
	    tmpoff += tmpsize;
	}
	return;
}

static void encodeDynamicbufs(std::vector<std::vector<DynamicArray>> &dynamicbufs, DynamicArray &dst_dynamicbuf) {
    // FORMAT: <int bufnum> + <int buf0_size> + <byte[] buf0_bytes> + <int buf1_size> + <byte[] buf1_bytes> + ...
    int bufnum = 0;
    for (int i = 0; i < dynamicbufs.size(); i++) {
        for (int j = 0; j < dynamicbufs[i].size(); j++) {
            bufnum += 1;
        }
    }

    int tmpoff = 0;
    dst_dynamicbuf.dynamic_memcpy(tmpoff, (char *)&bufnum, sizeof(int));
    tmpoff += sizeof(int);
    for (int i = 0; i < dynamicbufs.size(); i++) {
        for (int j = 0; j < dynamicbufs[i].size(); j++) {
            int tmpsize = dynamicbufs[i][j].size();
            dst_dynamicbuf.dynamic_memcpy(tmpoff, (char *)&tmpsize, sizeof(int));
            tmpoff += sizeof(int);

            dst_dynamicbuf.dynamic_memcpy(tmpoff, dynamicbufs[i][j].array(), tmpsize);
            tmpoff += tmpsize;
        }
    }
    return;
}


// JNI-based socket

JNIEXPORT jint JNICALL Java_com_inswitchcache_core_SocketHelper_createUdpsock
(JNIEnv * jenv, jclass jcz, jboolean jneed_timeout, jstring jrole, jint jtimeout_sec, jint jtimeout_usec, jint judp_rcvbufsize) {
	// Convert Java types into C/C++ types
	bool need_timeout = jboolean2bool(jneed_timeout);
	bool iscopy_role;
	const char *role = jstring2charptr(jenv, jrole, iscopy_role);
	int timeout_sec = jint2int(jtimeout_sec);
	int timeout_usec = jint2int(jtimeout_sec);
	int udp_rcvbufsize = jint2int(judp_rcvbufsize);
	// int udp_serverport = jint2int(udp_serverport);
	int sockfd = -1;
	create_udpsock(sockfd, need_timeout, role, timeout_sec, timeout_usec, udp_rcvbufsize);
	INVARIANT(sockfd != -1);

	saf_jstring(jenv, jrole, role, iscopy_role);

	return int2jint(sockfd);
}
// JNI-based socket

JNIEXPORT jint JNICALL Java_com_inswitchcache_core_SocketHelper_createUdpsockWithport
(JNIEnv * jenv, jclass jcz, jboolean jneed_timeout, jstring jrole, jint jtimeout_sec, jint jtimeout_usec, jint judp_rcvbufsize, jint judp_serverport) {
	// Convert Java types into C/C++ types
	bool need_timeout = jboolean2bool(jneed_timeout);
	bool iscopy_role;
	const char *role = jstring2charptr(jenv, jrole, iscopy_role);
	int timeout_sec = jint2int(jtimeout_sec);
	int timeout_usec = jint2int(jtimeout_sec);
	int udp_rcvbufsize = jint2int(judp_rcvbufsize);
	int udp_serverport = jint2int(udp_serverport);
	int sockfd = -1;
	create_udpsock(sockfd, need_timeout, role, timeout_sec, timeout_usec, udp_rcvbufsize,udp_serverport);
	INVARIANT(sockfd != -1);

	saf_jstring(jenv, jrole, role, iscopy_role);

	return int2jint(sockfd);
}

JNIEXPORT void JNICALL Java_com_inswitchcache_core_SocketHelper_udpsendto
  (JNIEnv * jenv, jclass jcz, jint jsockfd, jbyteArray jbuf, jint jlen, jstring jdstip, jshort jdstport, jstring jrole) {
	// Convert Java types into C/C++ types
	int sockfd = jint2int(jsockfd);
	bool iscopy_buf;
	const char *buf = (const char *)jbytearray2charptr(jenv, jbuf, iscopy_buf); // NOTE: we never change buf in udpsendto
	int len = jint2int(jlen);
	bool iscopy_dstip;
	const char *dstip = jstring2charptr(jenv, jdstip, iscopy_dstip);
	short dstport = jshort2short(jdstport);
	bool iscopy_role;
	const char *role = jstring2charptr(jenv, jrole, iscopy_role);

	struct sockaddr_in dstaddr;
	socklen_t dstaddrlen = sizeof(struct sockaddr_in);
	set_sockaddr(dstaddr, inet_addr(dstip), dstport);

    int flags = 0;
	udpsendto(sockfd, buf, len, flags, &dstaddr, dstaddrlen, role);

	saf_jstring(jenv, jrole, role, iscopy_role);
	saf_jbytearray(jenv, jbuf, (char *)buf, iscopy_buf, false);
}

JNIEXPORT jint JNICALL Java_com_inswitchcache_core_SocketHelper_udprecvfrom__I_3BILjava_lang_String_2
(JNIEnv * jenv, jclass jcz, jint jsockfd, jbyteArray jbuf, jint jlen, jstring jrole) {
	// Convert Java types into C/C++ types
	int sockfd = jint2int(jsockfd);
	bool iscopy_buf;
	char *buf = (char *)jbytearray2charptr(jenv, jbuf, iscopy_buf); // NOTE: we change buf in udprecvfrom
	int len = jint2int(jlen);
	bool iscopy_role;
	const char *role = jstring2charptr(jenv, jrole, iscopy_role);

    int flags = 0;
	int recvsize = -1;
	bool is_timeout = udprecvfrom(sockfd, buf, len, flags, NULL, NULL, recvsize, role);
	if (is_timeout) {
	    recvsize = -1;
	}

	saf_jstring(jenv, jrole, role, iscopy_role);
	saf_jbytearray(jenv, jbuf, buf, iscopy_buf, true);
	return int2jint(recvsize);
}

JNIEXPORT jint JNICALL Java_com_inswitchcache_core_SocketHelper_udprecvfrom__I_3BILcom_inswitchcache_core_UdpAddr_2Ljava_lang_String_2
(JNIEnv * jenv, jclass jcz, jint jsockfd, jbyteArray jbuf, jint jlen, jobject judpaddr, jstring jrole) {
	// Convert Java types into C/C++ types
	int sockfd = jint2int(jsockfd);
	bool iscopy_buf;
	char *buf = (char *)jbytearray2charptr(jenv, jbuf, iscopy_buf); // NOTE: we change buf in udprecvfrom
	int len = jint2int(jlen);
	bool iscopy_role;
	const char *role = jstring2charptr(jenv, jrole, iscopy_role);

	struct sockaddr_in srcaddr;
	socklen_t srcaddrlen = sizeof(struct sockaddr_in);
	int flags = 0;
	int recvsize = -1;
	bool is_timeout = udprecvfrom(sockfd, buf, len, flags, &srcaddr, &srcaddrlen, recvsize, role);
	if (is_timeout) {
	    recvsize = -1;
	}
	if (recvsize >= 0) {
		copy_srcaddr_to_judpaddr(jenv, srcaddr, judpaddr);
	}

	saf_jstring(jenv, jrole, role, iscopy_role);
	saf_jbytearray(jenv, jbuf, buf, iscopy_buf, true);
	return int2jint(recvsize);
}

JNIEXPORT void JNICALL Java_com_inswitchcache_core_SocketHelper_udpsendlargeipfrag
(JNIEnv *jenv, jclass jcz, jint jsockfd, jbyteArray jbuf, jint jlen, jstring jdstip, jshort jdstport, jstring jrole, jint jfraghdrsize)
{
	// Convert Java types into C/C++ types
	int sockfd = jint2int(jsockfd);
	bool iscopy_buf;
	const char *buf = (const char *)jbytearray2charptr(jenv, jbuf, iscopy_buf); // NOTE: we never change buf in udpsendto
	int len = jint2int(jlen);
	bool iscopy_dstip;
	const char *dstip = jstring2charptr(jenv, jdstip, iscopy_dstip);
	short dstport = jshort2short(jdstport);
	bool iscopy_role;
	const char *role = jstring2charptr(jenv, jrole, iscopy_role);
	int frag_hdrsize = jint2int(jfraghdrsize);

	struct sockaddr_in dstaddr;
	socklen_t dstaddrlen = sizeof(struct sockaddr_in);
	set_sockaddr(dstaddr, inet_addr(dstip), dstport);

	int flags = 0;
	udpsendlarge_ipfrag(sockfd, buf, len, flags, &dstaddr, dstaddrlen, role, frag_hdrsize);

	saf_jstring(jenv, jrole, role, iscopy_role);
	saf_jbytearray(jenv, jbuf, (char *)buf, iscopy_buf, false);
}

JNIEXPORT jint JNICALL Java_com_inswitchcache_core_SocketHelper_udprecvlargeipfrag__IILcom_inswitchcache_core_DynamicArray_2Ljava_lang_String_2
(JNIEnv * jenv, jclass jcz, jint jmethodid, jint jsockfd, jobject jdynamicbuf, jstring jrole) {
	// Convert Java types into C/C++ types
	int methodid = jint2int(jmethodid);
	int sockfd = jint2int(jsockfd);
	bool iscopy_buf;
	bool iscopy_role;
	const char *role = jstring2charptr(jenv, jrole, iscopy_role);

	DynamicArray dynamicbuf(MAX_BUFSIZE, MAX_LARGE_BUFSIZE); // avoid from invoking java method frequently
	int flags = 0;
	bool is_timeout = udprecvlarge_ipfrag(methodid, sockfd, dynamicbuf, flags, NULL, NULL, role, NULL);

    int recvsize = -1;
	if (is_timeout) { // clear jdynamicbuf if timeout
	    recvsize = -1;
		jclass dynamicbuf_class = jenv->GetObjectClass(jdynamicbuf);
		jmethodID clear_method = jenv->GetMethodID(dynamicbuf_class, "clear", "()V");
		jenv->CallVoidMethod(jdynamicbuf, clear_method);
	}
	else { // copy dynamicbuf into jdynamicbuf
	    recvsize = dynamicbuf.size();
		copy_dynamicbuf_to_jdynamicbuf(jenv, dynamicbuf, jdynamicbuf);
	}

	saf_jstring(jenv, jrole, role, iscopy_role);
	return int2jint(recvsize);
}

JNIEXPORT jint JNICALL Java_com_inswitchcache_core_SocketHelper_udprecvlargeipfrag__IILcom_inswitchcache_core_DynamicArray_2Lcom_inswitchcache_core_UdpAddr_2Ljava_lang_String_2
(JNIEnv * jenv, jclass jcz, jint jmethodid, jint jsockfd, jobject jdynamicbuf, jobject judpaddr, jstring jrole) {
	// Convert Java types into C/C++ types
	int methodid = jint2int(jmethodid);
	int sockfd = jint2int(jsockfd);
	bool iscopy_buf;
	bool iscopy_role;
	const char *role = jstring2charptr(jenv, jrole, iscopy_role);

	struct sockaddr_in srcaddr;
	socklen_t srcaddrlen = sizeof(struct sockaddr_in);
	DynamicArray dynamicbuf(MAX_BUFSIZE, MAX_LARGE_BUFSIZE); // avoid from invoking java method frequently
    int flags = 0;
	bool is_timeout = udprecvlarge_ipfrag(methodid, sockfd, dynamicbuf, flags, &srcaddr, &srcaddrlen, role, NULL);

    int recvsize = -1;
	if (is_timeout) { // clear jdynamicbuf if timeout
	    recvsize = -1;
		jclass dynamicbuf_class = jenv->GetObjectClass(jdynamicbuf);
		jmethodID clear_method = jenv->GetMethodID(dynamicbuf_class, "clear", "()V");
		jenv->CallVoidMethod(jdynamicbuf, clear_method);
	}
	else { // copy dynamicbuf into jdynamicbuf
	    recvsize = dynamicbuf.size();
		copy_dynamicbuf_to_jdynamicbuf(jenv, dynamicbuf, jdynamicbuf);

		// Copy src address of the first packet for both large and not-large packet
		copy_srcaddr_to_judpaddr(jenv, srcaddr, judpaddr);
	}

	saf_jstring(jenv, jrole, role, iscopy_role);
	return int2jint(recvsize);
}

JNIEXPORT jint JNICALL Java_com_inswitchcache_core_SocketHelper_udprecvlargeudpfrag__IILcom_inswitchcache_core_DynamicArray_2Ljava_lang_String_2
(JNIEnv * jenv, jclass jcz, jint jmethodid, jint jsockfd, jobject jdynamicbuf, jstring jrole) {
	// Convert Java types into C/C++ types
	int methodid = jint2int(jmethodid);
	int sockfd = jint2int(jsockfd);
	bool iscopy_buf;
	bool iscopy_role;
	const char *role = jstring2charptr(jenv, jrole, iscopy_role);

	DynamicArray dynamicbuf(MAX_BUFSIZE, MAX_LARGE_BUFSIZE); // avoid from invoking java method frequently
	int flags = 0;
	bool is_timeout = udprecvlarge_udpfrag(methodid, sockfd, dynamicbuf, flags, NULL, NULL, role);

    int recvsize = -1;
	if (is_timeout) { // clear jdynamicbuf if timeout
	    recvsize = -1;
		jclass dynamicbuf_class = jenv->GetObjectClass(jdynamicbuf);
		jmethodID clear_method = jenv->GetMethodID(dynamicbuf_class, "clear", "()V");
		jenv->CallVoidMethod(jdynamicbuf, clear_method);
	}
	else { // copy dynamicbuf into jdynamicbuf
	    recvsize = dynamicbuf.size();
		copy_dynamicbuf_to_jdynamicbuf(jenv, dynamicbuf, jdynamicbuf);
	}

	saf_jstring(jenv, jrole, role, iscopy_role);
	return int2jint(recvsize);
}

JNIEXPORT jint JNICALL Java_com_inswitchcache_core_SocketHelper_udprecvlargeudpfrag__IILcom_inswitchcache_core_DynamicArray_2Lcom_inswitchcache_core_UdpAddr_2Ljava_lang_String_2
(JNIEnv * jenv, jclass jcz, jint jmethodid, jint jsockfd, jobject jdynamicbuf, jobject judpaddr, jstring jrole) {
	// Convert Java types into C/C++ types
	int methodid = jint2int(jmethodid);
	int sockfd = jint2int(jsockfd);
	bool iscopy_buf;
	bool iscopy_role;
	const char *role = jstring2charptr(jenv, jrole, iscopy_role);

	struct sockaddr_in srcaddr;
	socklen_t srcaddrlen = sizeof(struct sockaddr_in);
	DynamicArray dynamicbuf(MAX_BUFSIZE, MAX_LARGE_BUFSIZE); // avoid from invoking java method frequently
    int flags = 0;
	bool is_timeout = udprecvlarge_udpfrag(methodid, sockfd, dynamicbuf, flags, &srcaddr, &srcaddrlen, role);

    int recvsize = -1;
	if (is_timeout) { // clear jdynamicbuf if timeout
	    recvsize = -1;
		jclass dynamicbuf_class = jenv->GetObjectClass(jdynamicbuf);
		jmethodID clear_method = jenv->GetMethodID(dynamicbuf_class, "clear", "()V");
		jenv->CallVoidMethod(jdynamicbuf, clear_method);
	}
	else { // copy dynamicbuf into jdynamicbuf
	    recvsize = dynamicbuf.size();
		copy_dynamicbuf_to_jdynamicbuf(jenv, dynamicbuf, jdynamicbuf);

		// Copy src address of the first packet for both large and not-large packet
		copy_srcaddr_to_judpaddr(jenv, srcaddr, judpaddr);
	}

	saf_jstring(jenv, jrole, role, iscopy_role);
	return int2jint(recvsize);
}

JNIEXPORT jint JNICALL Java_com_inswitchcache_core_SocketHelper_prepareUdpserver
(JNIEnv * jenv, jclass jcz, jboolean jneed_timeout, jshort jlistenport, jstring jrole, jint jtimeout_sec, jint jtimeout_usec, jint judp_rcvbufsize) {
	// Convert Java types into C/C++ types
	bool need_timeout = jboolean2bool(jneed_timeout);
	short listenport = jshort2short(jlistenport);
	bool iscopy_role;
	const char *role = jstring2charptr(jenv, jrole, iscopy_role);
	int timeout_sec = jint2int(jtimeout_sec);
	int timeout_usec = jint2int(jtimeout_sec);
	int udp_rcvbufsize = jint2int(judp_rcvbufsize);

	int sockfd = -1;
	prepare_udpserver(sockfd, need_timeout, listenport, role, timeout_sec, timeout_usec, udp_rcvbufsize);
	INVARIANT(sockfd != -1);

	saf_jstring(jenv, jrole, role, iscopy_role);
	return int2jint(sockfd);
}

JNIEXPORT void JNICALL Java_com_inswitchcache_core_SocketHelper_close
(JNIEnv * jenv, jclass jcz, jint jsockfd) {
	// Convert Java types into C/C++ types
	int sockfd = jint2int(jsockfd);

	close(sockfd);
}

JNIEXPORT jint JNICALL Java_com_inswitchcache_core_SocketHelper_udprecvlargeipfragMultisrc
(JNIEnv *jenv, jclass jcz, jint jmethodid, jint jsockfd, jobject jdynamicbuf, jstring jrole, jobject jkey) {
    int methodid = jint2int(jmethodid);
	int sockfd = jint2int(jsockfd);
	bool iscopy_role;
	const char *role = jstring2charptr(jenv, jrole, iscopy_role);
	netreach_key_t key = jkey2key(jenv, jkey);

    size_t received_scannum = 0;
    DynamicArray *scanbufs = NULL;
    //set_recvtimeout(sockfd, CLIENT_SCAN_SOCKET_TIMEOUT_SECS, 0); // 10s for SCAN
    bool is_timeout = udprecvlarge_multisrc_ipfrag(methodid, sockfd, &scanbufs, received_scannum, 0, NULL, NULL, role, true, optype_t(packet_type_t::SCANRES_SPLIT), key);
    //set_recvtimeout(sockfd, CLIENT_SOCKET_TIMEOUT_SECS, 0); // 5s for others

	int recvsize = -1;
	if (is_timeout) { // clear jdynamicbuf if timeout
        recvsize = -1;
        jclass dynamicbuf_class = jenv->GetObjectClass(jdynamicbuf);
        jmethodID clear_method = jenv->GetMethodID(dynamicbuf_class, "clear", "()V");
        jenv->CallVoidMethod(jdynamicbuf, clear_method);
    }
    else { // encode multiple DynamicArrays into a single DynamicArray
        INVARIANT(scanbufs != NULL);
        INVARIANT(received_scannum > 0);

        DynamicArray dynamicbuf(MAX_BUFSIZE, MAX_LARGE_BUFSIZE);
        encodeDynamicbufs(scanbufs, received_scannum, dynamicbuf);

        recvsize = dynamicbuf.size();
        copy_dynamicbuf_to_jdynamicbuf(jenv, dynamicbuf, jdynamicbuf);
    }

	if (scanbufs != NULL) {
    	delete [] scanbufs;
    	scanbufs = NULL;
    }

	saf_jstring(jenv, jrole, role, iscopy_role);
    return int2jint(recvsize);
}

JNIEXPORT jint JNICALL Java_com_inswitchcache_core_SocketHelper_udprecvlargeipfragMultisrcDist
(JNIEnv *jenv, jclass jcz, jint jmethodid, jint jsockfd, jobject jdynamicbuf, jstring jrole, jobject jkey) {
    int methodid = jint2int(jmethodid);
	int sockfd = jint2int(jsockfd);
	bool iscopy_role;
	const char *role = jstring2charptr(jenv, jrole, iscopy_role);
	netreach_key_t key = jkey2key(jenv, jkey);

    size_t received_scannum = 0;
    DynamicArray *scanbufs = NULL;
    bool is_timeout = udprecvlarge_multisrc_ipfrag(methodid, sockfd, &scanbufs, received_scannum, 0, NULL, NULL, role, true, optype_t(packet_type_t::SCANRES_SPLIT), key);

    std::vector<std::vector<dynamic_array_t>> perswitch_perserver_scanbufs;
    std::vector<std::vector<struct sockaddr_in>> perswitch_perserver_addrs;
    std::vector<std::vector<socklen_t>> perswitch_perserver_addrlens;
    //set_recvtimeout(sockfd, CLIENT_SCAN_SOCKET_TIMEOUT_SECS, 0); // 10s for SCAN
    is_timeout = udprecvlarge_multisrc_ipfrag_dist(methodid, sockfd, perswitch_perserver_scanbufs, 0, perswitch_perserver_addrs, perswitch_perserver_addrlens, role, true, optype_t(packet_type_t::SCANRES_SPLIT), key);
    //set_recvtimeout(sockfd, CLIENT_SOCKET_TIMEOUT_SECS, 0); // 5s for others

	int recvsize = -1;
	if (is_timeout) { // clear jdynamicbuf if timeout
        recvsize = -1;
        jclass dynamicbuf_class = jenv->GetObjectClass(jdynamicbuf);
        jmethodID clear_method = jenv->GetMethodID(dynamicbuf_class, "clear", "()V");
        jenv->CallVoidMethod(jdynamicbuf, clear_method);
    }
    else { // encode multiple DynamicArrays into a single DynamicArray
        DynamicArray dynamicbuf(MAX_BUFSIZE, MAX_LARGE_BUFSIZE);
        encodeDynamicbufs(perswitch_perserver_scanbufs, dynamicbuf);

        recvsize = dynamicbuf.size();
        copy_dynamicbuf_to_jdynamicbuf(jenv, dynamicbuf, jdynamicbuf);
    }

	if (scanbufs != NULL) {
    	delete [] scanbufs;
    	scanbufs = NULL;
    }

	saf_jstring(jenv, jrole, role, iscopy_role);
    return int2jint(recvsize);
}
