package com.inswitchcache.core;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.ArrayList;
import java.util.Arrays;

/**
 * Farreach socket helper (UDP) based on JNI for fine-grained socket options.
 */
public final class SocketHelper {

  private SocketHelper() {
    throw new IllegalStateException("SocketHelper is a utility class!");
  }

  public static int createUdpsock(boolean needTimeout, String role) {
    return createUdpsock(needTimeout, role, GlobalConfig.DEFAULT_SOCKET_TIMEOUT_SECS, 0,
        GlobalConfig.UDP_DEFAULT_RCVBUFSIZE);
  }

  public static native int createUdpsock(boolean needTimeout, String role, int timeoutSec, int timeoutUsec,
      int udpRcvbufsize);

  // udpsendto.len means the number of bytes you want to send in buf
  public static native void udpsendto(int sowsckfd, byte[] buf, int len, String dstip, short dstport, String role);

  // udprecvfrom returns the number of received bytes; return value = -1 means
  // timeout;
  // udprecvfrom.len means the maximum number of bytes you can receive in buf
  public static native int udprecvfrom(int sockfd, byte[] buf, int len, String role);

  // NOTE: we use UdpAddr to store srcip/srcport if necessary (JNI cannot change
  // String/int parameters)
  public static native int udprecvfrom(int sockfd, byte[] buf, int len, UdpAddr udpaddr, String role);

  // udpsendlargeipfrag.len means the number of bytes you want to send in buf (for
  // PUT w/ value > 128B)
  // udpsendlargeipfrag.fraghdrsize means the number of bytes of packet header you
  // want to embed into each fragment
  public static native void udpsendlargeipfrag(int sockfd, byte[] buf, int len, String dstip, short dstport,
      String role, int fraghdrsize);

  // udprecvlargeipfrag returns the number of received bytes; return value = -1
  // means timeout;
  // udprecvlargeudpfrag returns the number of received bytes; return value = -1
  // means timeout (used by socket receiving packets not through programmable
  // switch);
  public static native int udprecvlargeipfrag(int methodid, int sockfd, DynamicArray dynamicbuf, String role);

  // NOTE: we use UdpAddr to store srcip/srcport if necessary (JNI cannot change
  // String/int parameters)
  public static native int udprecvlargeipfrag(int methodid, int sockfd, DynamicArray dynamicbuf, UdpAddr udpaddr,
      String role);

  public static native int udprecvlargeudpfrag(int methodid, int sockfd, DynamicArray dynamicbuf, String role);

  public static native int udprecvlargeudpfrag(int methodid, int sockfd, DynamicArray dynamicbuf, UdpAddr udpaddr,
      String role);

  public static native int prepareUdpserver(boolean needTimeout, short listenport, String role, int timeoutSec,
      int timeoutUsec, int udpRcvbufsize);

  public static native void close(int sockfd);

  // support range query
  // NOTE: JNI encodes per-server [per-switch] dynamicbuf of ScanResponseSplit
  // into a single dynamicbuf
  // FORMAT: <int bufnum> + <int buf0_size> + <byte[] buf0_bytes> + <int
  // buf1_size> + <byte[] buf1_bytes> + ...
  public static native int udprecvlargeipfragMultisrc(int methodid, int sockfd, DynamicArray dynamicbuf, String role,
      Key key);

  public static native int udprecvlargeipfragMultisrcDist(int methodid, int sockfd, DynamicArray dynamicbuf,
      String role, Key key);

  // NOTE: use decodeDynamicbufs to decode multiple ScanResponseSplits from the
  // single dynamicbuf
  public static ArrayList<DynamicArray> decodeDynamicbufs(DynamicArray dynamicbuf) {
    return decodeDynamicbufs(dynamicbuf.array());
  }

  public static ArrayList<DynamicArray> decodeDynamicbufs(byte[] dynamicbuf) {
    ArrayList<DynamicArray> result = new ArrayList<>();

    int tmpoff = 0;
    int tmpmaxsize = dynamicbuf.length;

    // bufnum
    MyUtils.myAssert(tmpoff + 4 <= tmpmaxsize);
    byte[] bufnumBytes = new byte[4];
    System.arraycopy(dynamicbuf, tmpoff, bufnumBytes, 0, 4);
    int bufnum = ByteBuffer.wrap(bufnumBytes).order(ByteOrder.LITTLE_ENDIAN).getInt();
    tmpoff += 4;

    for (int i = 0; i < bufnum; i++) {
      // bufsize
      MyUtils.myAssert(tmpoff + 4 <= tmpmaxsize);
      byte[] tmpsizeBytes = new byte[4];
      System.arraycopy(dynamicbuf, tmpoff, tmpsizeBytes, 0, 4);
      int tmpsize = ByteBuffer.wrap(tmpsizeBytes).order(ByteOrder.LITTLE_ENDIAN).getInt();
      tmpoff += 4;

      // bufdata
      MyUtils.myAssert(tmpoff + tmpsize <= tmpmaxsize);
      byte[] tmpdataBytes = new byte[tmpsize];
      System.arraycopy(dynamicbuf, tmpoff, tmpdataBytes, 0, tmpsize);
      DynamicArray tmpbuf = new DynamicArray(GlobalConfig.MAX_BUFFER_SIZE, GlobalConfig.MAX_LARGE_BUFFER_SIZE);
      tmpbuf.dynamicMemcpy(0, tmpdataBytes, tmpsize);
      tmpoff += tmpsize;

      result.add(tmpbuf);
    }

    return result;
  }

  // Load JNI libraries
  static {
    // NOTE: we need to ensure that "./jnilib/" is one of the following path
    String property = System.getProperty("java.library.path");
    if (property == null) {
      throw new RuntimeException("Path isn't set.");
    }
    String workingDir = System.getProperty("user.dir");
    String jnipath = workingDir + "/jnilib";
    boolean hasjnipath = false;
    for (String str : Arrays.asList(property.split(":"))) {
      if (str == jnipath) {
        hasjnipath = true;
        break;
      }
    }
    if (!hasjnipath) {
      property = jnipath + ":" + property;
      System.setProperty("java.library.path", property);
      // System.out.println(System.getProperty("java.library.path"));
    }

    System.load(workingDir + "/jnilib/libSocketJNI.so"); // libSocketJNI.so
  }
}
