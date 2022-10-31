#ifndef PACKET_FORMAT_H
#define PACKET_FORMAT_H

#include <array>
#include <cstring>
#include <vector>

#include "helper.h"
#include "snapshot_record.h"
#include "dynamic_array.h"

// mask for other_hdr
//#define VALID_MASK 0x01

// # of bytes in debug_hdr
//#define DEBUG_BYTES 1

// op_hdr -> scan_hdr -> split_hdr -> vallen_hdr -> val_hdr -> shadowtype_hdr -> seq_hdr -> inswitch_hdr -> stat_hdr -> clone_hdr

// (1) vallen&value: mask 0b0001; seq: mask 0b0010; inswitch_hdr: mask 0b0100; stat: mask 0b1000;
// (2) scan/split: specific value (X + 0b0000); not parsed optypes: X + 0b0000
enum class PacketType {
	//WARMUPREQ=0x0011, 
	//LOADREQ=0x0021, LOADREQ_SPINE=0x0031, 
	PUTREQ=0x0001, DISTNOCACHE_PUTREQ_SPINE=0x0041,
	PUTREQ_SEQ=0x0003, PUTREQ_POP_SEQ=0x0013, PUTREQ_SEQ_CASE3=0x0023, PUTREQ_POP_SEQ_CASE3=0x0033, NETCACHE_PUTREQ_SEQ_CACHED=0x0043, PUTREQ_SEQ_BEINGEVICTED=0x0053, PUTREQ_SEQ_CASE3_BEINGEVICTED=0x0063, PUTREQ_SEQ_BEINGEVICTED_SPINE=0x0073, PUTREQ_SEQ_CASE3_BEINGEVICTED_SPINE=0x0083,
	//CACHE_POP_INSWITCH=0x0007,
	GETRES_LATEST_SEQ_INSWITCH=0x000f, GETRES_DELETED_SEQ_INSWITCH=0x001f, GETRES_LATEST_SEQ_INSWITCH_CASE1=0x002f, GETRES_DELETED_SEQ_INSWITCH_CASE1=0x003f, PUTREQ_SEQ_INSWITCH_CASE1=0x004f, DELREQ_SEQ_INSWITCH_CASE1=0x005f, LOADSNAPSHOTDATA_INSWITCH_ACK=0x006f, CACHE_POP_INSWITCH=0x007f, NETCACHE_VALUEUPDATE_INSWITCH=0x008f, GETRES_LATEST_SEQ_SERVER_INSWITCH=0x009f, GETRES_DELETED_SEQ_SERVER_INSWITCH=0x010f, DISTCACHE_SPINE_VALUEUPDATE_INSWITCH=0x011f, DISTCACHE_LEAF_VALUEUPDATE_INSWITCH=0x012f, DISTCACHE_VALUEUPDATE_INSWITCH=0x013f, DISTCACHE_VALUEUPDATE_INSWITCH_ORIGIN=0x014f, NETCACHE_CACHE_POP_INSWITCH_NLATEST=0x015f,
	GETRES_LATEST_SEQ=0x000b, GETRES_DELETED_SEQ=0x001b, CACHE_EVICT_LOADDATA_INSWITCH_ACK=0x002b, NETCACHE_VALUEUPDATE=0x003b, GETRES_LATEST_SEQ_SERVER=0x004b, GETRES_DELETED_SEQ_SERVER=0x005b, GETRES_SEQ=0x006b,
	GETRES=0x0009, GETRES_SERVER=0x0019, DISTCACHE_GETRES_SPINE=0x0029,
	PUTREQ_INSWITCH=0x0005,
	DELREQ_SEQ_INSWITCH=0x0006, PUTREQ_LARGEVALUE_SEQ_INSWITCH=0x0016,
	PUTREQ_SEQ_INSWITCH=0x0007,
	GETREQ_INSWITCH=0x0004, DELREQ_INSWITCH=0x0014, CACHE_EVICT_LOADFREQ_INSWITCH=0x0024, CACHE_EVICT_LOADDATA_INSWITCH=0x0034, LOADSNAPSHOTDATA_INSWITCH=0x0044, SETVALID_INSWITCH=0x0054, NETCACHE_WARMUPREQ_INSWITCH=0x0064, NETCACHE_WARMUPREQ_INSWITCH_POP=0x0074, DISTCACHE_INVALIDATE_INSWITCH=0x0084, DISTCACHE_VALUEUPDATE_INSWITCH_ACK=0x0094, PUTREQ_LARGEVALUE_INSWITCH=0x00a4,
	DELREQ_SEQ=0x0002, DELREQ_SEQ_CASE3=0x0012, NETCACHE_DELREQ_SEQ_CACHED=0x0022, PUTREQ_LARGEVALUE_SEQ=0x0032, PUTREQ_LARGEVALUE_SEQ_CACHED=0x0042, PUTREQ_LARGEVALUE_SEQ_CASE3=0x0052, DELREQ_SEQ_BEINGEVICTED=0x0062, DELREQ_SEQ_CASE3_BEINGEVICTED=0x0072, PUTREQ_LARGEVALUE_SEQ_BEINGEVICTED=0x0082, PUTREQ_LARGEVALUE_SEQ_CASE3_BEINGEVICTED=0x0092, DELREQ_SEQ_BEINGEVICTED_SPINE=0x00a2, DELREQ_SEQ_CASE3_BEINGEVICTED_SPINE=0x00b2, PUTREQ_LARGEVALUE_SEQ_BEINGEVICTED_SPINE=0x00c2, PUTREQ_LARGEVALUE_SEQ_CASE3_BEINGEVICTED_SPINE=0x00d2, GETREQ_LARGEVALUEBLOCK_SEQ=0x00e2,
	PUTRES=0x0008, DELRES=0x0018, PUTRES_SERVER=0x0028, DELRES_SERVER=0x0038,
	PUTRES_SEQ=0x000a, DELRES_SEQ=0x001a,
	WARMUPREQ=0x0000, SCANREQ=0x0010, SCANREQ_SPLIT=0x0020, GETREQ=0x0030, DELREQ=0x0040, GETREQ_POP=0x0050, GETREQ_NLATEST=0x0060, CACHE_POP_INSWITCH_ACK=0x0070, SCANRES_SPLIT=0x0080, CACHE_POP=0x0090, CACHE_EVICT=0x00a0, CACHE_EVICT_ACK=0x00b0, CACHE_EVICT_CASE2=0x00c0, WARMUPACK=0x00d0, LOADACK=0x00e0, CACHE_POP_ACK=0x00f0, CACHE_EVICT_LOADFREQ_INSWITCH_ACK=0x0100, SETVALID_INSWITCH_ACK=0x0110, NETCACHE_GETREQ_POP=0x0120, NETCACHE_CACHE_POP=0x0130, NETCACHE_CACHE_POP_ACK=0x0140, NETCACHE_CACHE_POP_FINISH=0x0150, NETCACHE_CACHE_POP_FINISH_ACK=0x0160, NETCACHE_CACHE_EVICT=0x0170, NETCACHE_CACHE_EVICT_ACK=0x0180, NETCACHE_VALUEUPDATE_ACK=0x0190, GETREQ_SPINE=0x0200, SCANRES_SPLIT_SERVER=0x0210, WARMUPREQ_SPINE=0x0220, WARMUPACK_SERVER=0x0230, LOADACK_SERVER=0x0240, DISTCACHE_CACHE_EVICT_VICTIM=0x0250, DISTCACHE_CACHE_EVICT_VICTIM_ACK=0x0260, DISTNOCACHE_DELREQ_SPINE=0x0270, DISTCACHE_INVALIDATE=0x0280, DISTCACHE_INVALIDATE_ACK=0x0290, DISTCACHE_UPDATE_TRAFFICLOAD=0x02a0, DISTCACHE_SPINE_VALUEUPDATE_INSWITCH_ACK=0x02b0, DISTCACHE_LEAF_VALUEUPDATE_INSWITCH_ACK=0x02c0, PUTREQ_LARGEVALUE=0x02d0, DISTNOCACHE_PUTREQ_LARGEVALUE_SPINE=0x02e0, GETRES_LARGEVALUE_SERVER=0x02f0, GETRES_LARGEVALUE=0x0300, LOADREQ=0x0310, LOADREQ_SPINE=0x0320, NETCACHE_CACHE_POP_ACK_NLATEST=0x0330, GETREQ_BEINGEVICTED=0x0340, GETRES_LARGEVALUE_SEQ=0x0350
};
/*enum class PacketType {
	GETREQ, PUTREQ, DELREQ, SCANREQ, GETRES, PUTRES, DELRES, SCANRES_SPLIT, GETREQ_INSWITCH, GETREQ_POP, GETREQ_NLATEST, 
	GETRES_LATEST_SEQ, GETRES_LATEST_SEQ_INSWITCH, GETRES_LATEST_SEQ_INSWITCH_CASE1, 
	GETRES_DELETED_SEQ, GETRES_DELETED_SEQ_INSWITCH, GETRES_DELETED_SEQ_INSWITCH_CASE1, 
	PUTREQ_INSWITCH, PUTREQ_SEQ, PUTREQ_POP_SEQ, PUTREQ_SEQ_INSWITCH_CASE1, PUTREQ_SEQ_CASE3, PUTREQ_POP_SEQ_CASE3,
	DELREQ_INSWITCH, DELREQ_SEQ, DELREQ_SEQ_INSWITCH_CASE1, DELREQ_SEQ_CASE3, SCANREQ_SPLIT,

	CACHE_POP, CACHE_POP_INSWITCH, CACHE_POP_INSWITCH_ACK, CACHE_EVICT, CACHE_EVICT_ACK, CACHE_EVICT_CASE2,
};*/
typedef PacketType packet_type_t;

// For large value -> NOTE: update the util APIs for large value (at the bottom of packet_format.h)
// (1) server/client does NOT know whether the request or get response has large value -> use optype_for_udprecvlarge_ipfrag_list to juduge if it needs to return from udprecvlarge_ipfrag in advance
static const uint32_t optype_for_udprecvlarge_ipfrag_num = 9;
// NOTE: NOT including SCANRES_SPLIT which is processed by udprecvlarge_multisrc_ipfrag instead of udprecvlarge_ipfrag
static const packet_type_t optype_for_udprecvlarge_ipfrag_list[optype_for_udprecvlarge_ipfrag_num] = {packet_type_t::PUTREQ_LARGEVALUE, packet_type_t::PUTREQ_LARGEVALUE_SEQ, packet_type_t::PUTREQ_LARGEVALUE_SEQ_CACHED, packet_type_t::PUTREQ_LARGEVALUE_SEQ_CASE3, packet_type_t::PUTREQ_LARGEVALUE_SEQ_BEINGEVICTED, packet_type_t::PUTREQ_LARGEVALUE_SEQ_CASE3_BEINGEVICTED, packet_type_t::LOADREQ, packet_type_t::GETRES_LARGEVALUE, packet_type_t::GETRES_LARGEVALUE_SEQ};
// (2) server may receive other requests when waiting for a large request -> use optype_with_clientlogicalidx_list to judge whether server needs to extract clientlogicalidx from the current packet to push/update PktRingBuffer
static const uint32_t optype_with_clientlogicalidx_num = 7;
static const packet_type_t optype_with_clientlogicalidx_list[optype_with_clientlogicalidx_num] = {packet_type_t::PUTREQ_LARGEVALUE, packet_type_t::PUTREQ_LARGEVALUE_SEQ, packet_type_t::PUTREQ_LARGEVALUE_SEQ_CACHED, packet_type_t::PUTREQ_LARGEVALUE_SEQ_CASE3, packet_type_t::PUTREQ_LARGEVALUE_SEQ_BEINGEVICTED, packet_type_t::PUTREQ_LARGEVALUE_SEQ_CASE3_BEINGEVICTED, packet_type_t::LOADREQ};

typedef uint16_t optype_t;
typedef uint16_t switchidx_t;

template<class key_t> class ScanRequestSplit;
template<class key_t, class val_t> class PutRequestSeq;
template<class key_t> class WarmupRequest;

template<class key_t>
class Packet { // ophdr
	public:
		static uint32_t get_ophdrsize(method_t methodid);

		// Utils for method ID
		static bool is_singleswitch(method_t methodid);
		static int get_inswitch_prev_bytes(method_t methodid); // # of bytes before idx in inswitch_hdr
		static int get_clone_bytes(method_t methoid); // # of bytes in clone_hdr
		static int get_split_prev_bytes(method_t methodid); // # of bytes before cur_scanidx&max_scannum&cur_scanswitchidx&max_scanswitchnum in split_hdr
		static int get_stat_padding_bytes(method_t methodid); // # of padding bytes in stat_hdr

		Packet();
		Packet(method_t methodid, packet_type_t type, key_t key);
		Packet(method_t methodid, packet_type_t type, switchidx_t globalswitchidx, key_t key);
		Packet(method_t methodid, packet_type_t type, switchidx_t spineswitchidx, switchidx_t leafswitchidx, key_t key);
		virtual ~Packet(){}

		packet_type_t type() const;
		key_t key() const;
		switchidx_t globalswitchidx() const;
		switchidx_t spineswitchidx() const;
		switchidx_t leafswitchidx() const;

		virtual uint32_t serialize(char * const data, uint32_t max_size) = 0;
	protected:
		method_t _methodid = INVALID_ID;
		// for single switch (optype + key)
		optype_t _type;
		key_t _key;
		// for distnocache and distfarreach (optype + globalswitchidx + key)
		switchidx_t _globalswitchidx;
		// for distcache (optype + spineswitchidx + leafswitchidx + key)
		switchidx_t _spineswitchidx;
		switchidx_t _leafswitchidx;

		virtual uint32_t size() = 0;
		virtual void deserialize(const char * data, uint32_t recv_size) = 0;

		uint32_t serialize_ophdr(char * const data, uint32_t max_size);
		uint32_t dynamic_serialize_ophdr(dynamic_array_t &dynamic_data);
		uint32_t deserialize_ophdr(const char * data, uint32_t recv_size);
};

template<class key_t>
class GetRequest : public Packet<key_t> { // ophdr [+ shadowtypehdr + switchload_hdr]
	public: 
		GetRequest();
		GetRequest(method_t methodid, key_t key);
		GetRequest(method_t methodid, switchidx_t spineswitchidx, switchidx_t leafswitchidx, key_t key);
		GetRequest(method_t methodid, const char * data, uint32_t recv_size);
		virtual ~GetRequest(){}

		uint32_t spineload() const;
		uint32_t leafload() const;

		virtual uint32_t serialize(char * const data, uint32_t max_size);
	protected:
		virtual uint32_t size();
		virtual void deserialize(const char * data, uint32_t recv_size);

		// for distcache (ophdr + shadowtypehdr + switchload_hdr)
		uint32_t _spineload;
		uint32_t _leafload;
};

template<class key_t, class val_t>
class PutRequest : public Packet<key_t> { // ophdr + val + shadowtype
	public:
		PutRequest();
		PutRequest(method_t methodid, key_t key, val_t val);
		PutRequest(method_t methodid, const char * data, uint32_t recv_size);

		val_t val() const;

		virtual uint32_t serialize(char * const data, uint32_t max_size);
	protected:
		virtual uint32_t size();
		virtual void deserialize(const char * data, uint32_t recv_size);
		val_t _val;
};

template<class key_t>
class DelRequest : public Packet<key_t> { // ophdr
	public: 
		DelRequest();
		DelRequest(method_t methodid, key_t key);
		DelRequest(method_t methodid, const char * data, uint32_t recv_size);

		virtual uint32_t serialize(char * const data, uint32_t max_size);
	protected:
		virtual uint32_t size();
		virtual void deserialize(const char * data, uint32_t recv_size);
};

template<class key_t>
class ScanRequest : public Packet<key_t> { // ophdr + scanhdr
	public: 
		ScanRequest();
		//ScanRequest(key_t key, key_t endkey, uint32_t num);
		ScanRequest(method_t methodid, key_t key, key_t endkey);
		ScanRequest(method_t methodid, const char * data, uint32_t recv_size);

		key_t endkey() const;
		//uint32_t num() const;

		virtual uint32_t serialize(char * const data, uint32_t max_size);
	protected:
		virtual uint32_t size();
		virtual void deserialize(const char * data, uint32_t recv_size);
		key_t _endkey;
		//uint32_t _num;
};

template<class key_t, class val_t>
class GetResponse : public Packet<key_t> { // ophdr + val + shadowtype + stat_hdr [+ switchload_hdr]
	public:
		GetResponse();
		GetResponse(method_t methodid, key_t key, val_t val, bool stat, uint16_t nodeidx_foreval);
		GetResponse(method_t methodid, const char * data, uint32_t recv_size);

		val_t val() const;
		bool stat() const;
		uint16_t nodeidx_foreval() const;
		uint32_t spineload() const;
		uint32_t leafload() const;

		virtual uint32_t serialize(char * const data, uint32_t max_size);
	protected:
		virtual uint32_t size();
		virtual void deserialize(const char * data, uint32_t recv_size);

		val_t _val;
		bool _stat;
		uint16_t _nodeidx_foreval;
		// for distcache
		uint32_t _spineload;
		uint32_t _leafload;
};

template<class key_t, class val_t>
class GetResponseServer : public GetResponse<key_t, val_t> { // ophdr + val + shadowtype + stat_hdr [+ switchload_hdr]
	public:
		GetResponseServer(method_t methodid, key_t key, val_t val, bool stat, uint16_t nodeidx_foreval);
		GetResponseServer(method_t methodid, switchidx_t spineswitchidx, switchidx_t leafswitchidx, key_t key, val_t val, bool stat, uint16_t nodeidx_foreval, uint32_t spineload, uint32_t leafload);
};


template<class key_t, class val_t>
class GetResponseSeq : public GetResponse<key_t, val_t> { // ophdr + val + shadowtype + seq + stat_hdr
	public: 
		GetResponseSeq();
		GetResponseSeq(method_t methodid, key_t key, val_t val, uint32_t seq, bool stat, uint16_t nodeidx_foreval);
		GetResponseSeq(method_t methodid, const char *data, uint32_t recv_size);
		virtual ~GetResponseSeq(){}

		uint32_t seq() const;

		virtual uint32_t serialize(char * const data, uint32_t max_size);

	protected:
		virtual uint32_t size();
		virtual void deserialize(const char * data, uint32_t recv_size);
		uint32_t _seq;
};

template<class key_t>
class PutResponse : public Packet<key_t> { // ophdr + shadowtype + stat_hdr
	public: 
		PutResponse(method_t methodid, key_t key, bool stat, uint16_t nodeidx_foreval);
		PutResponse(method_t methodid, const char * data, uint32_t recv_size);

		bool stat() const;
		uint16_t nodeidx_foreval() const;

		virtual uint32_t serialize(char * const data, uint32_t max_size);
	protected:
		virtual uint32_t size();
		virtual void deserialize(const char * data, uint32_t recv_size);
		bool _stat;
		uint16_t _nodeidx_foreval;
};

template<class key_t>
class PutResponseSeq : public PutResponse<key_t> { // ophdr + shadowtype + seq + stat_hdr
	public:
		PutResponseSeq(method_t methodid, key_t key, uint32_t seq, bool stat, uint16_t nodeidx_foreval);
		PutResponseSeq(method_t methodid, const char * data, uint32_t recv_size);

		uint32_t seq() const;

		virtual uint32_t serialize(char * const data, uint32_t max_size);
	protected:
		virtual uint32_t size();
		virtual void deserialize(const char * data, uint32_t recv_size);
		uint32_t _seq;
};

template<class key_t>
class PutResponseServer : public PutResponse<key_t> { // ophdr + shadowtype + stat_hdr
	public:
		PutResponseServer(method_t methodid, key_t key, bool stat, uint16_t nodeidx_foreval);
};

template<class key_t>
class DelResponse : public Packet<key_t> { // ophdr + shadowtype + stat_hdr
	public: 
		DelResponse(method_t methodid, key_t key, bool stat, uint16_t nodeidx_foreval);
		DelResponse(method_t methodid, const char * data, uint32_t recv_size);

		bool stat() const;
		uint16_t nodeidx_foreval() const;

		virtual uint32_t serialize(char * const data, uint32_t max_size);
	protected:
		virtual uint32_t size();
		virtual void deserialize(const char * data, uint32_t recv_size);
	private:
		bool _stat;
		uint16_t _nodeidx_foreval;
};

template<class key_t>
class DelResponseSeq : public PutResponseSeq<key_t> { // ophdr + shadowtype + seq + stat_hdr
	public:
		DelResponseSeq(method_t methodid, key_t key, uint32_t seq, bool stat, uint16_t nodeidx_foreval);
		DelResponseSeq(method_t methodid, const char * data, uint32_t recv_size);
};

template<class key_t>
class DelResponseServer : public DelResponse<key_t> { // ophdr + shadowtype + stat_hdr
	public:
		DelResponseServer(method_t methodid, key_t key, bool stat, uint16_t nodeidx_foreval);
};

template<class key_t, class val_t>
class ScanResponseSplit : public ScanRequestSplit<key_t> { // ophdr + scanhdr(endkey) + splithdr(isclone+curscanidx+maxscannum [+curscanswitchidx+maxscanswitchnum] ) + nodeidx_foreval(not processed by switch) + snapshotid(not processed by switch) + pairs
	public: 
		ScanResponseSplit(method_t methodid, key_t key, key_t endkey, uint16_t cur_scanidx, uint16_t max_scannum, uint16_t nodeidx_foreval, int snapshotid, int32_t parinum, std::vector<std::pair<key_t, snapshot_record_t>> pairs);
		ScanResponseSplit(method_t methodid, key_t key, key_t endkey, uint16_t cur_scanidx, uint16_t max_scannum, uint16_t cur_scanswitchidx, uint16_t max_scanswitchnum, uint16_t nodeidx_foreval, int snapshotid, int32_t parinum, std::vector<std::pair<key_t, snapshot_record_t>> pairs);
		ScanResponseSplit(method_t methodid, const char * data, uint32_t recv_size);

		uint16_t nodeidx_foreval() const;
		int snapshotid() const;
		int32_t pairnum() const;
		std::vector<std::pair<key_t, snapshot_record_t>> pairs() const;

		virtual uint32_t serialize(char * const data, uint32_t max_size);
		uint32_t dynamic_serialize(dynamic_array_t &dynamic_data);

		static size_t get_frag_hdrsize(method_t methodid);

		// cur_scanidx and max_scannum of server for a given leaf switch
		static size_t get_srcnum_off(method_t methodid);
		static size_t get_srcnum_len();
		static bool get_srcnum_conversion();
		static size_t get_srcid_off(method_t methodid);
		static size_t get_srcid_len();
		static bool get_srcid_conversion();

		// cur_scanswitchidx and max_scanswitchnum of leaf switch
		static size_t get_srcswitchnum_off(method_t methodid);
		static size_t get_srcswitchnum_len();
		static bool get_srcswitchnum_conversion();
		static size_t get_srcswitchid_off(method_t methodid);
		static size_t get_srcswitchid_len();
		static bool get_srcswitchid_conversion();
	protected:
		virtual uint32_t size();
		virtual void deserialize(const char * data, uint32_t recv_size);
		uint16_t _nodeidx_foreval;
		int _snapshotid;
		int32_t _pairnum;
		std::vector<std::pair<key_t, snapshot_record_t>> _pairs;
};

template<class key_t, class val_t>
class ScanResponseSplitServer : public ScanResponseSplit<key_t, val_t> { // ophdr + scanhdr(endkey) + splithdr(isclone+curscanidx+maxscannum+curscanswitchidx+maxscanswitchnum) + nodeidx_foreval(not processed by switch) + snapshotid(not processed by switch) + pairs
	public:
		ScanResponseSplitServer(method_t methodid, key_t key, key_t endkey, uint16_t cur_scanidx, uint16_t max_scannum, uint16_t cur_scanswitchidx, uint16_t max_scanswitchnum, uint16_t nodeidx_foreval, int snapshotid, int32_t parinum, std::vector<std::pair<key_t, snapshot_record_t>> pairs);
};

// NOT used in distcache
template<class key_t>
class GetRequestPOP : public WarmupRequest<key_t> { // ophdr
	public: 
		GetRequestPOP(method_t methodid, const char * data, uint32_t recv_size);

		virtual uint32_t serialize(char * const data, uint32_t max_size);
};

// NOT used in distcache
template<class key_t>
class GetRequestNLatest : public WarmupRequest<key_t> { // ophdr
	public: 
		GetRequestNLatest(method_t methodid, const char * data, uint32_t recv_size);

		virtual uint32_t serialize(char * const data, uint32_t max_size);
};

template<class key_t, class val_t>
class GetResponseLatestSeq : public PutRequestSeq<key_t, val_t> { // ophdr + val + shadowtype + seq + stat_hdr (stat=true)
	public: 
		GetResponseLatestSeq();
		GetResponseLatestSeq(method_t methodid, key_t key, val_t val, uint32_t seq, uint16_t nodeidx_foreval);
		virtual ~GetResponseLatestSeq(){}

		virtual uint32_t serialize(char * const data, uint32_t max_size);

		bool stat() const;
		uint16_t nodeidx_foreval() const;

	protected:
		virtual uint32_t size();
		virtual void deserialize(const char * data, uint32_t recv_size);
		bool _stat; // must be true for GetResponseLatestSeq
		uint16_t _nodeidx_foreval;
};

template<class key_t, class val_t>
class GetResponseLatestSeqServer : public GetResponseLatestSeq<key_t, val_t> { // ophdr + val + shadowtype + seq + stat_hdr (stat=true)
	public: 
		GetResponseLatestSeqServer(method_t methodid, switchidx_t switchidx, key_t key, val_t val, uint32_t seq, uint16_t nodeidx_foreval);
};

template<class key_t, class val_t>
class GetResponseLatestSeqInswitchCase1 : public GetResponseLatestSeq<key_t, val_t> { // ophdr + val + shadowtype + seq + inswitch.idx + stat_hdr (nodeidx_foreval=0) + clone_hdr
	public: 
		GetResponseLatestSeqInswitchCase1();
		GetResponseLatestSeqInswitchCase1(method_t methodid, key_t key, val_t val, uint32_t seq, uint16_t idx, bool stat);
		GetResponseLatestSeqInswitchCase1(method_t methodid, const char * data, uint32_t recv_size);

		uint16_t idx() const;
		bool stat() const;

		uint32_t bwcost();

		virtual uint32_t serialize(char * const data, uint32_t max_size);
	protected:
		virtual uint32_t size();
		virtual void deserialize(const char * data, uint32_t recv_size);
		uint16_t _idx;
		bool _stat;
		uint16_t _nodeidx_foreval;
};

template<class key_t, class val_t>
class GetResponseDeletedSeq : public GetResponseLatestSeq<key_t, val_t> { // ophdr + val + shadowtype + seq + stat_hdr (stat=false)
	public: 
		GetResponseDeletedSeq(method_t methodid, key_t key, val_t val, uint32_t seq, uint16_t nodeidx_foreval);

	protected:
		virtual void deserialize(const char * data, uint32_t recv_size);
		// NOTE: _stat must be false for GetResponseDeletedSeq
};

template<class key_t, class val_t>
class GetResponseDeletedSeqServer : public GetResponseDeletedSeq<key_t, val_t> { // ophdr + val + shadowtype + seq + stat_hdr (stat=false)
	public: 
		GetResponseDeletedSeqServer(method_t methodid, switchidx_t switchidx, key_t key, val_t val, uint32_t seq, uint16_t nodeidx_foreval);
};

template<class key_t, class val_t>
class GetResponseDeletedSeqInswitchCase1 : public GetResponseLatestSeqInswitchCase1<key_t, val_t> { // ophdr + val + shadowtype + seq + inswitch.idx + stat_hdr + clone_hdr
	public: 
		GetResponseDeletedSeqInswitchCase1(method_t methodid, key_t key, val_t val, uint32_t seq, uint16_t idx, bool stat);
		GetResponseDeletedSeqInswitchCase1(method_t methodid, const char * data, uint32_t recv_size);
};

template<class key_t, class val_t>
class PutRequestSeq : public Packet<key_t> { // ophdr + val + shadowtype + seq
	public: 
		PutRequestSeq();
		PutRequestSeq(method_t methodid, key_t key, val_t val, uint32_t seq);
		PutRequestSeq(method_t methodid, const char * data, uint32_t recv_size);

		val_t val() const;
		uint32_t seq() const;

		virtual uint32_t serialize(char * const data, uint32_t max_size);

	protected:
		virtual uint32_t size();
		virtual void deserialize(const char * data, uint32_t recv_size);
		val_t _val;
		uint32_t _seq;
};

template<class key_t, class val_t>
class PutRequestPopSeq : public PutRequestSeq<key_t, val_t> { // ophdr + val + shadowtype + seq
	public: 
		PutRequestPopSeq(method_t methodid, const char * data, uint32_t recv_size);

		virtual uint32_t serialize(char * const data, uint32_t max_size);
};

template<class key_t, class val_t>
class PutRequestSeqInswitchCase1 : public GetResponseLatestSeqInswitchCase1<key_t, val_t> { // ophdr + val + shadowtype + seq + inswitch.idx + stat_hdr + clone_hdr
	public: 
		PutRequestSeqInswitchCase1(method_t methodid, key_t key, val_t val, uint32_t seq, uint16_t idx, bool stat);
		PutRequestSeqInswitchCase1(method_t methodid, const char * data, uint32_t recv_size);
};

template<class key_t, class val_t>
class PutRequestSeqCase3 : public PutRequestSeq<key_t, val_t> { // ophdr + val + shadowtype + seq
	public: 
		PutRequestSeqCase3(method_t methodid, const char * data, uint32_t recv_size);

		virtual uint32_t serialize(char * const data, uint32_t max_size);
};

template<class key_t, class val_t>
class PutRequestPopSeqCase3 : public PutRequestSeq<key_t, val_t> { // ophdr + val + shadowtype + seq
	public: 
		PutRequestPopSeqCase3(method_t methodid, const char * data, uint32_t recv_size);

		virtual uint32_t serialize(char * const data, uint32_t max_size);
};

template<class key_t>
class DelRequestSeq : public Packet<key_t> { // ophdr + shadowtype + seq
	public: 
		DelRequestSeq();
		DelRequestSeq(method_t methodid, const char * data, uint32_t recv_size);

		virtual uint32_t serialize(char * const data, uint32_t max_size);

		uint32_t seq() const;

	protected:
		virtual uint32_t size();
		virtual void deserialize(const char * data, uint32_t recv_size);
		uint32_t _seq;
};

template<class key_t>
class GetRequestLargevalueblockSeq : public DelRequestSeq<key_t> { // ophdr + shadowtype + seq
	public:
		GetRequestLargevalueblockSeq();
		GetRequestLargevalueblockSeq(method_t methodid, const char * data, uint32_t recv_size);
};

template<class key_t, class val_t>
class DelRequestSeqInswitchCase1 : public GetResponseLatestSeqInswitchCase1<key_t, val_t> { // ophdr + val + shadowtype + seq + inswitch.idx + stat_hdr + clone_hdr
	public: 
		DelRequestSeqInswitchCase1(method_t methodid, key_t key, val_t val, uint32_t seq, uint16_t idx, bool stat);
		DelRequestSeqInswitchCase1(method_t methodid, const char * data, uint32_t recv_size);
};

template<class key_t>
class DelRequestSeqCase3 : public DelRequestSeq<key_t> { // ophdr + shadowtype + seq
	public: 
		DelRequestSeqCase3(method_t methodid, const char * data, uint32_t recv_size);

		virtual uint32_t serialize(char * const data, uint32_t max_size);
};

template<class key_t>
class ScanRequestSplit : public ScanRequest<key_t> { // ophdr + scanhdr + splithdr
	public: 
		ScanRequestSplit();
		ScanRequestSplit(method_t methodid, key_t key, key_t endkey, uint16_t cur_scanidx, uint16_t max_scannum);
		ScanRequestSplit(method_t methodid, key_t key, key_t endkey, uint16_t cur_scanidx, uint16_t max_scannum, uint16_t cur_scanswitchidx, uint16_t max_scanswitchnum);
		ScanRequestSplit(method_t methodid, const char * data, uint32_t recv_size);

		uint16_t cur_scanidx() const;
		uint16_t max_scannum() const;
		// for distributed scenario
		uint16_t cur_scanswitchidx() const;
		uint16_t max_scanswitchnum() const;

		virtual uint32_t serialize(char * const data, uint32_t max_size);
	protected:
		virtual uint32_t size();
		virtual void deserialize(const char * data, uint32_t recv_size);
		uint16_t _cur_scanidx;
		uint16_t _max_scannum;
		uint16_t _cur_scanswitchidx;
		uint16_t _max_scanswitchnum;
};

// NOTE: only used in end-hosts
template<class key_t, class val_t>
class CachePop : public PutRequestSeq<key_t, val_t> { // ophdr + val + seq + stat (not stat_hdr) + serveridx
	public: 
		CachePop();
		CachePop(method_t methodid, key_t key, val_t val, uint32_t seq, bool stat, uint16_t serveridx);
		CachePop(method_t methodid, const char * data, uint32_t recv_size);

		virtual uint32_t serialize(char * const data, uint32_t max_size);

		bool stat() const;
		uint16_t serveridx() const;

		uint32_t bwcost();

	protected:
		virtual uint32_t size();
		virtual void deserialize(const char * data, uint32_t recv_size);
		bool _stat;
		uint16_t _serveridx;
};

template<class key_t, class val_t>
class CachePopInswitch : public PutRequestSeq<key_t, val_t> { // ophdr + val + shadowtype + seq + inswitch_hdr + stat_hdr
	public: 
		CachePopInswitch(method_t methodid, key_t key, val_t val, uint32_t seq, uint16_t freeidx, bool stat);
		CachePopInswitch(method_t methodid, switchidx_t switchidx, key_t key, val_t val, uint32_t seq, uint16_t freeidx, bool stat);
		CachePopInswitch(method_t methodid, switchidx_t spineswitchidx, switchidx_t leafswitchidx, key_t key, val_t val, uint32_t seq, uint16_t freeidx, bool stat);

		virtual uint32_t serialize(char * const data, uint32_t max_size);

		uint16_t freeidx() const;
		bool stat() const;

	protected:
		virtual uint32_t size();
		virtual void deserialize(const char * data, uint32_t recv_size);
		uint16_t _freeidx;
		bool _stat;
};

template<class key_t>
class CachePopInswitchAck : public WarmupRequest<key_t> { // ophdr
	public: 
		CachePopInswitchAck(method_t methodid, const char * data, uint32_t recv_size);

		virtual uint32_t serialize(char * const data, uint32_t max_size);
};

// NOTE: only used in end-hosts
template<class key_t, class val_t>
class CacheEvict : public GetResponseLatestSeq<key_t, val_t> { // ophdr + val + seq + stat + serveridx
	public: 
		CacheEvict();
		CacheEvict(method_t methodid, key_t key, val_t val, uint32_t seq, bool stat, uint16_t serveridx);
		CacheEvict(method_t methodid, const char * data, uint32_t recv_size);
		virtual ~CacheEvict(){}

		uint16_t serveridx() const;

		uint32_t bwcost();

		virtual uint32_t serialize(char * const data, uint32_t max_size);
	protected:
		virtual uint32_t size();
		virtual void deserialize(const char * data, uint32_t recv_size);
		uint16_t _serveridx;
};

// NOTE: only used in end-hosts
template<class key_t>
class CacheEvictAck : public WarmupRequest<key_t> { // ophdr
	public: 
		CacheEvictAck(method_t methodid, key_t key);
		CacheEvictAck(method_t methodid, const char * data, uint32_t recv_size);
		virtual ~CacheEvictAck(){}

		uint32_t bwcost();
};

// NOTE: only used in end-hosts
template<class key_t, class val_t>
class CacheEvictCase2 : public CacheEvict<key_t, val_t> { // ophdr + val + seq + stat + serveridx
	public: 
		CacheEvictCase2(method_t methodid, key_t key, val_t val, uint32_t seq, bool stat, uint16_t serveridx);
		CacheEvictCase2(method_t methodid, const char * data, uint32_t recv_size);
};

/*template<class key_t, class val_t>
class WarmupRequest : public PutRequest<key_t, val_t> { // ophdr + val + shadowtype
	public: 
		WarmupRequest(key_t key, val_t val);
		WarmupRequest(const char * data, uint32_t recv_size);
};*/

template<class key_t>
class WarmupRequest : public Packet<key_t> { // ophdr
	public: 
		WarmupRequest();
		WarmupRequest(method_t methodid, key_t key);
		WarmupRequest(method_t methodid, const char * data, uint32_t recv_size);
		virtual ~WarmupRequest(){}

		virtual uint32_t serialize(char * const data, uint32_t max_size);
	protected:
		virtual uint32_t size();
		virtual void deserialize(const char * data, uint32_t recv_size);
};

template<class key_t>
class WarmupAck : public WarmupRequest<key_t> { // ophdr
	public: 
		WarmupAck(method_t methodid, key_t key);
		WarmupAck(method_t methodid, const char * data, uint32_t recv_size);
};

template<class key_t, class val_t>
class LoadRequest : public Packet<key_t> { // ophdr + client_logical_idx + fragseq + val in payload (NOT parsed by switch -> NOT need shadowtype)
	public: 
		LoadRequest(method_t methodid, key_t key, val_t val, uint16_t client_logical_idx, uint32_t fragseq);
		LoadRequest(method_t methodid, const char * data, uint32_t recv_size);

		static size_t get_frag_hdrsize(method_t methodid);
		uint32_t dynamic_serialize(dynamic_array_t &dynamic_data);

		uint16_t client_logical_idx() const; // parsed into frag_hdr.padding by switch
		uint32_t fragseq() const; // parsed into frag_hdr.padding2 by switch
		val_t val() const;

		virtual uint32_t serialize(char * const data, uint32_t max_size);
	protected:
		virtual uint32_t size();
		virtual void deserialize(const char * data, uint32_t recv_size);
		uint16_t _client_logical_idx; // parsed into frag_hdr.padding by switch
		uint32_t _fragseq;
		val_t _val;
};

template<class key_t>
class LoadAck : public WarmupRequest<key_t> { // ophdr
	public: 
		LoadAck(method_t methodid, key_t key);
		LoadAck(method_t methodid, const char * data, uint32_t recv_size);
};

// NOTE: only used in end-hosts
template<class key_t>
class CachePopAck : public WarmupRequest<key_t> { // ophdr
	public: 
		CachePopAck(method_t methodid, key_t key);
		CachePopAck(method_t methodid, const char * data, uint32_t recv_size);

		uint32_t bwcost();
};

template<class key_t>
class CacheEvictLoadfreqInswitch : public Packet<key_t> { // ophdr + shadowtype + inswitch_hdr
	public: 
		CacheEvictLoadfreqInswitch();
		CacheEvictLoadfreqInswitch(method_t methodid, key_t key, uint16_t evictidx);
		CacheEvictLoadfreqInswitch(method_t methodid, switchidx_t switchidx, key_t key, uint16_t evictidx);
		CacheEvictLoadfreqInswitch(method_t methodid, switchidx_t spineswitchidx, switchidx_t leafswitchidx, key_t key, uint16_t evictidx);

		uint16_t evictidx() const;

		virtual uint32_t serialize(char * const data, uint32_t max_size);
	private:
		virtual uint32_t size();
		virtual void deserialize(const char * data, uint32_t recv_size);
		uint16_t _evictidx;
};

template<class key_t>
class CacheEvictLoadfreqInswitchAck : public Packet<key_t> { // ophdr + frequency_hdr
	public: 
		CacheEvictLoadfreqInswitchAck(method_t methodid, const char * data, uint32_t recv_size);

		uint32_t frequency() const;

		virtual uint32_t serialize(char * const data, uint32_t max_size);
	private:
		virtual uint32_t size();
		virtual void deserialize(const char * data, uint32_t recv_size);
		uint32_t _frequency;
};


template<class key_t>
class CacheEvictLoaddataInswitch : public CacheEvictLoadfreqInswitch<key_t> { // ophdr + shadowtype + inswitch_hdr
	public: 
		CacheEvictLoaddataInswitch(method_t methodid, key_t key, uint16_t evictidx);
		CacheEvictLoaddataInswitch(method_t methodid, switchidx_t switchidx, key_t key, uint16_t evictidx);
		CacheEvictLoaddataInswitch(method_t methodid, switchidx_t spineswitchidx, switchidx_t leafswitchidx_t, key_t key, uint16_t evictidx);
};

template<class key_t, class val_t>
class CacheEvictLoaddataInswitchAck : public Packet<key_t> { // ophdr + val + shadowtype + seq + stat_hdr
	public: 
		CacheEvictLoaddataInswitchAck(method_t methodid, const char * data, uint32_t recv_size);

		val_t val() const;
		uint32_t seq() const;
		bool stat() const;

		virtual uint32_t serialize(char * const data, uint32_t max_size);
	private:
		virtual uint32_t size();
		virtual void deserialize(const char * data, uint32_t recv_size);
		val_t _val;
		uint32_t _seq;
		bool _stat;
};

template<class key_t>
class LoadsnapshotdataInswitch : public CacheEvictLoadfreqInswitch<key_t> { // ophdr + shadowtype + inswitch_hdr
	public: 
		LoadsnapshotdataInswitch(method_t methodid, key_t key, uint16_t loadidx);
		LoadsnapshotdataInswitch(method_t methodid, switchidx_t switchidx, key_t key, uint16_t loadidx);
		LoadsnapshotdataInswitch(method_t methodid, switchidx_t spineswitchidx, switchidx_t leafswitchidx, key_t key, uint16_t loadidx);

		uint16_t loadidx() const;
};

template<class key_t, class val_t>
class LoadsnapshotdataInswitchAck : public GetResponseLatestSeqInswitchCase1<key_t, val_t> { // ophdr + val + shadowtype + seq + inswitch_hdr.idx + stat_hdr
	public: 
		LoadsnapshotdataInswitchAck(method_t methodid, const char * data, uint32_t recv_size);
	private:
		virtual uint32_t size();
		virtual void deserialize(const char * data, uint32_t recv_size);
};

template<class key_t>
class SetvalidInswitch : public Packet<key_t> { // ophdr + shadowtype + inswitch_hdr + validvalue_hdr
	public: 
		SetvalidInswitch(method_t methodid, key_t key, uint16_t idx, uint8_t validvalue);
		SetvalidInswitch(method_t methodid, switchidx_t switchidx, key_t key, uint16_t idx, uint8_t validvalue);
		SetvalidInswitch(method_t methodid, switchidx_t spineswitchidx, switchidx_t leafswitchidx, key_t key, uint16_t idx, uint8_t validvalue);

		uint16_t idx() const; // freeidx or evictidx
		uint8_t validvalue() const;

		virtual uint32_t serialize(char * const data, uint32_t max_size);
	private:
		virtual uint32_t size();
		virtual void deserialize(const char * data, uint32_t recv_size);
		uint16_t _idx;
		uint8_t _validvalue;
};

template<class key_t>
class SetvalidInswitchAck : public WarmupRequest<key_t> { // ophdr
	public: 
		SetvalidInswitchAck(method_t methodid, const char * data, uint32_t recv_size);

		virtual uint32_t serialize(char * const data, uint32_t max_size);
};

template<class key_t>
class NetcacheGetRequestPop : public GetRequest<key_t> { // ophdr [+ shadowtypehdr + switchload_hdr] + clonehdr
	public:
		NetcacheGetRequestPop(method_t methodid, key_t key);
		NetcacheGetRequestPop(method_t methodid, const char * data, uint32_t recvsize);

		virtual uint32_t serialize(char * const data, uint32_t max_size);
	private:
		virtual uint32_t size();
		virtual void deserialize(const char * data, uint32_t recv_size);
};

// NOTE: only used in end-hosts
template<class key_t>
class NetcacheCachePop : public WarmupRequest<key_t> { // ophdr + serveridx
	public: 
		NetcacheCachePop();
		NetcacheCachePop(method_t methodid, key_t key, uint16_t serveridx);
		NetcacheCachePop(method_t methodid, const char * data, uint32_t recv_size);

		virtual uint32_t serialize(char * const data, uint32_t max_size);

		uint16_t serveridx() const;

	protected:
		virtual uint32_t size();
		virtual void deserialize(const char * data, uint32_t recv_size);
		uint16_t _serveridx;
};

// NOTE: only used in end-hosts
template<class key_t, class val_t>
class NetcacheCachePopAck : public CachePop<key_t, val_t> { // ophdr + val + seq + stat (not stat_hdr) + serveridx
	public: 
		NetcacheCachePopAck();
		NetcacheCachePopAck(method_t methodid, key_t key, val_t val, uint32_t seq, bool stat, uint16_t serveridx);
		NetcacheCachePopAck(method_t methodid, const char * data, uint32_t recv_size);
};

// NOTE: only used in end-hosts
template<class key_t>
class NetcacheCachePopFinish : public NetcacheCachePop<key_t> { // ophdr + serveridx
	public: 
		NetcacheCachePopFinish(method_t methodid, key_t key, uint16_t serveridx);
		NetcacheCachePopFinish(method_t methodid, key_t key, uint16_t serveridx, uint16_t kvidx);
		NetcacheCachePopFinish(method_t methodid, const char * data, uint32_t recv_size);

		virtual uint32_t serialize(char * const data, uint32_t max_size);
	
		uint16_t kvidx() const;
	protected:
		virtual uint32_t size();
		virtual void deserialize(const char * data, uint32_t recv_size);
		uint16_t _kvidx;
};

// NOTE: only used in end-hosts
template<class key_t>
class NetcacheCachePopFinishAck : public NetcacheCachePop<key_t> { // ophdr + serveridx
	public: 
		NetcacheCachePopFinishAck(method_t methodid, key_t key, uint16_t serveridx);
		NetcacheCachePopFinishAck(method_t methodid, const char * data, uint32_t recv_size);
};

template<class key_t>
class NetcacheWarmupRequestInswitchPop : public CacheEvictLoadfreqInswitch<key_t> { // ophdr + shadowtype + inswitch_hdr + clone_hdr
	public: 
		NetcacheWarmupRequestInswitchPop(method_t methodid, key_t key);
		NetcacheWarmupRequestInswitchPop(method_t methodid, const char * data, uint32_t recv_size);

		virtual uint32_t serialize(char * const data, uint32_t max_size);
	protected:
		virtual uint32_t size();
		virtual void deserialize(const char * data, uint32_t recv_size);
};

// NOTE: only used in end-hosts
template<class key_t>
class NetcacheCacheEvict : public NetcacheCachePop<key_t> { // ophdr + serveridx
	public: 
		NetcacheCacheEvict(method_t methodid, key_t key, uint16_t serveridx);
		NetcacheCacheEvict(method_t methodid, const char * data, uint32_t recv_size);
};

// NOTE: only used in end-hosts
template<class key_t>
class NetcacheCacheEvictAck : public NetcacheCachePop<key_t> { // ophdr + serveridx
	public: 
		NetcacheCacheEvictAck(method_t methodid, key_t key, uint16_t serveridx);
		NetcacheCacheEvictAck(method_t methodid, const char * data, uint32_t recv_size);
};

template<class key_t, class val_t>
class NetcachePutRequestSeqCached : public PutRequestSeq<key_t, val_t> { // ophdr + val + shadowtype + seq
	public: 
		NetcachePutRequestSeqCached(method_t methodid, const char * data, uint32_t recv_size);
};

template<class key_t>
class NetcacheDelRequestSeqCached : public DelRequestSeq<key_t> { // ophdr + shadowtype + seq
	public: 
		NetcacheDelRequestSeqCached(method_t methodid, const char * data, uint32_t recv_size);
};

template<class key_t, class val_t>
class NetcacheValueupdate : public GetResponseLatestSeq<key_t, val_t> { // ophdr + val + shadowtype + seq + stat_hdr
	public: 
		NetcacheValueupdate(method_t methodid, key_t key, val_t val, uint32_t seq, bool stat);
		NetcacheValueupdate(method_t methodid, switchidx_t spineswitchidx, switchidx_t leafswitchidx, key_t key, val_t val, uint32_t seq, bool stat);
};

template<class key_t>
class NetcacheValueupdateAck : public WarmupRequest<key_t> { // ophdr
	public: 
		NetcacheValueupdateAck(method_t methodid, const char * data, uint32_t recv_size);
};

template<class key_t>
class WarmupAckServer : public WarmupAck<key_t> { // ophdr
	public: 
		WarmupAckServer(method_t methodid, key_t key);
};

template<class key_t>
class LoadAckServer : public LoadAck<key_t> { // ophdr
	public: 
		LoadAckServer(method_t methodid, key_t key);
};

// only used by end-hosts
template<class key_t>
class DistcacheCacheEvictVictim : public Packet<key_t> { // ophdr + victimkey + victimidx
	public: 
		DistcacheCacheEvictVictim();
		DistcacheCacheEvictVictim(method_t methodid, key_t key, key_t victimkey, uint16_t victimidx);
		DistcacheCacheEvictVictim(method_t methodid, const char * data, uint32_t recv_size);
		virtual ~DistcacheCacheEvictVictim(){}

		key_t victimkey() const;
		uint16_t victimidx() const;

		virtual uint32_t serialize(char * const data, uint32_t max_size);
	protected:
		virtual uint32_t size();
		virtual void deserialize(const char * data, uint32_t recv_size);
		key_t _victimkey;
		uint16_t _victimidx;
};

template<class key_t>
class DistcacheCacheEvictVictimAck : public WarmupRequest<key_t> { // ophdr
	public: 
		DistcacheCacheEvictVictimAck(method_t methodid, key_t key);
		DistcacheCacheEvictVictimAck(method_t methodid, const char * data, uint32_t recv_size);
};

template<class key_t>
class DistcacheInvalidate : public WarmupRequest<key_t> { // ophdr
	public: 
		DistcacheInvalidate(method_t methodid, switchidx_t spineswitchidx, switchidx_t leafswitchidx, key_t key);
		DistcacheInvalidate(method_t methodid, const char * data, uint32_t recv_size);
};

template<class key_t>
class DistcacheInvalidateAck : public WarmupRequest<key_t> { // ophdr
	public: 
		DistcacheInvalidateAck(method_t methodid, key_t key);
		DistcacheInvalidateAck(method_t methodid, const char * data, uint32_t recv_size);
};

template<class key_t>
class DistcacheUpdateTrafficload : public GetRequest<key_t> { // ophdr + shadowtypehdr + switchloadhdr
	public: 
		DistcacheUpdateTrafficload(method_t methodid, switchidx_t spineswitchidx, switchidx_t leafswitchidx, key_t key, uint32_t spineload, uint32_t leafload);
};

template<class key_t, class val_t>
class DistcacheSpineValueupdateInswitch : public GetResponseLatestSeq<key_t, val_t> { // ophdr + val + shadowtype + seq + inswitch_hdr.idx + stat_hdr
	public: 
		DistcacheSpineValueupdateInswitch();
		DistcacheSpineValueupdateInswitch(method_t methodid, switchidx_t spineswitchidx, switchidx_t leafswitchidx, key_t key, val_t val, uint32_t seq, bool stat, uint16_t kvidx);

		virtual uint32_t serialize(char * const data, uint32_t max_size);

		uint16_t kvidx() const;
	protected:
		virtual uint32_t size();
		uint16_t _kvidx;
};

template<class key_t, class val_t>
class DistcacheLeafValueupdateInswitch : public DistcacheSpineValueupdateInswitch<key_t, val_t> { // ophdr + val + shadowtype + seq + inswitch_hdr.idx + stat_hdr
	public: 
		DistcacheLeafValueupdateInswitch(method_t methodid, switchidx_t spineswitchidx, switchidx_t leafswitchidx, key_t key, val_t val, uint32_t seq, bool stat, uint16_t kvidx);
};

template<class key_t>
class DistcacheSpineValueupdateInswitchAck : public WarmupRequest<key_t> { // ophdr
	public: 
		DistcacheSpineValueupdateInswitchAck(method_t methodid, const char * data, uint32_t recv_size);
};

template<class key_t>
class DistcacheLeafValueupdateInswitchAck : public WarmupRequest<key_t> { // ophdr
	public: 
		DistcacheLeafValueupdateInswitchAck(method_t methodid, const char * data, uint32_t recv_size);
};

template<class key_t, class val_t>
class DistcacheValueupdateInswitch : public DistcacheSpineValueupdateInswitch<key_t, val_t> { // ophdr + val + shadowtype + seq + inswitch_hdr.idx + stat_hdr
	public: 
		DistcacheValueupdateInswitch(method_t methodid, switchidx_t spineswitchidx, switchidx_t leafswitchidx, key_t key, val_t val, uint32_t seq, bool stat, uint16_t kvidx);
};

template<class key_t>
class DistcacheValueupdateInswitchAck : public WarmupRequest<key_t> { // ophdr + shadowtype + inswitch_hdr
	public: 
		DistcacheValueupdateInswitchAck(method_t methodid, const char * data, uint32_t recv_size);

		virtual uint32_t size();
		virtual void deserialize(const char * data, uint32_t recv_size);
};

// For large value

template<class key_t, class val_t>
class PutRequestLargevalue : public Packet<key_t> { // ophdr + client_logical_idx + fragseq + val in payload (NOT parsed by switch -> NOT need shadowtype_hdr)
	public:
		PutRequestLargevalue();
		PutRequestLargevalue(method_t methodid, key_t key, val_t val, uint16_t client_logical_idx, uint32_t fragseq);
		PutRequestLargevalue(method_t methodid, const char * data, uint32_t recv_size);

		static size_t get_frag_hdrsize(method_t methodid);
		uint32_t dynamic_serialize(dynamic_array_t &dynamic_data);

		uint16_t client_logical_idx() const; // parsed into frag_hdr.padding by switch
		uint32_t fragseq() const; // parsed into frag_hdr.padding2 by switch
		val_t val() const;

		virtual uint32_t serialize(char * const data, uint32_t max_size);
	protected:
		virtual uint32_t size();
		virtual void deserialize(const char * data, uint32_t recv_size);
		uint16_t _client_logical_idx; // parsed into frag_hdr.padding by switch
		uint32_t _fragseq;
		val_t _val;
};

template<class key_t, class val_t>
class PutRequestLargevalueSeq : public PutRequestLargevalue<key_t, val_t> { // ophdr + shadowtype + seq + client_logical_idx + fragseq + val in payload (NOT parsed by switch)
	public:
		PutRequestLargevalueSeq();
		PutRequestLargevalueSeq(method_t methodid, key_t key, val_t val, uint32_t seq, uint16_t client_logical_idx, uint32_t fragseq);
		PutRequestLargevalueSeq(method_t methodid, const char * data, uint32_t recv_size);

		static size_t get_frag_hdrsize(method_t methodid);

		uint32_t seq() const;

		virtual uint32_t serialize(char * const data, uint32_t max_size);
	protected:
		virtual uint32_t size();
		virtual void deserialize(const char * data, uint32_t recv_size);
		uint32_t _seq;
};

template<class key_t, class val_t>
class PutRequestLargevalueSeqCached : public PutRequestLargevalueSeq<key_t, val_t> { // ophdr + shadowtype + seq + client_logical_idx + fragseq + val in payload (NOT parsed by switch)
	public:
		PutRequestLargevalueSeqCached(method_t methodid, key_t key, val_t val, uint32_t seq, uint16_t client_logical_idx, uint32_t fragseq);
		PutRequestLargevalueSeqCached(method_t methodid, const char * data, uint32_t recv_size);

		static size_t get_frag_hdrsize(method_t methodid);
};

template<class key_t, class val_t>
class PutRequestLargevalueSeqCase3 : public PutRequestLargevalueSeq<key_t, val_t> { // ophdr + shadowtype + seq + client_logical_idx + fragseq + val in payload (NOT parsed by switch)
	public:
		PutRequestLargevalueSeqCase3(method_t methodid, key_t key, val_t val, uint32_t seq, uint16_t client_logical_idx, uint32_t fragseq);
		PutRequestLargevalueSeqCase3(method_t methodid, const char * data, uint32_t recv_size);

		static size_t get_frag_hdrsize(method_t methodid);
};

template<class key_t, class val_t>
class GetResponseLargevalue : public Packet<key_t> { // ophdr + val&stat_hdr&switchload_hdr in payload (NOT parsed by switch -> NOT need shadowtype_hdr)
	public:
		GetResponseLargevalue();
		GetResponseLargevalue(method_t methodid, key_t key, val_t val, bool stat, uint16_t nodeidx_foreval); // NOTE: DistCache will NOT serialize GETRES_LARGEVALUE
		GetResponseLargevalue(method_t methodid, const char * data, uint32_t recv_size);

		static size_t get_frag_hdrsize(method_t methodid);
		uint32_t dynamic_serialize(dynamic_array_t &dynamic_data);

		val_t val() const;
		bool stat() const;
		uint16_t nodeidx_foreval() const;
		uint32_t spineload() const;
		uint32_t leafload() const;

		virtual uint32_t serialize(char * const data, uint32_t max_size);
	protected:
		virtual uint32_t size();
		virtual void deserialize(const char * data, uint32_t recv_size);
		val_t _val;
		bool _stat;
		uint16_t _nodeidx_foreval;
		uint32_t _spineload;
		uint32_t _leafload;
};

template<class key_t, class val_t>
class GetResponseLargevalueSeq : public GetResponseLargevalue<key_t, val_t> { // ophdr + val&seq&stat_hdr in payload (NOT parsed by switch -> NOT need shadowtype_hdr)
	public:
		GetResponseLargevalueSeq(method_t methodid, key_t key, val_t val, uint32_t seq, bool stat, uint16_t nodeidx_foreval);
		//GetResponseLargevalueSeq(method_t methodid, switchidx_t spineswitchidx, switchidx_t leafswitchidx, key_t key, val_t val, uint32_t seq, bool stat, uint16_t nodeidx_foreval, uint32_t spineload, uint32_t leafload);
		GetResponseLargevalueSeq(method_t methodid, const char * data, uint32_t recv_size);

		uint32_t seq() const;

		static size_t get_frag_hdrsize(method_t methodid);

		virtual uint32_t serialize(char * const data, uint32_t max_size);
		uint32_t dynamic_serialize(dynamic_array_t &dynamic_data);
	protected:
		virtual uint32_t size();
		virtual void deserialize(const char * data, uint32_t recv_size);
		uint32_t _seq;
};

template<class key_t, class val_t>
class GetResponseLargevalueServer : public GetResponseLargevalue<key_t, val_t> { // ophdr + val&stat_hdr in payload (NOT parsed by switch -> NOT need shadowtype_hdr)
	public:
		GetResponseLargevalueServer(method_t methodid, key_t key, val_t val, bool stat, uint16_t nodeidx_foreval);
		GetResponseLargevalueServer(method_t methodid, switchidx_t spineswitchidx, switchidx_t leafswitchidx, key_t key, val_t val, bool stat, uint16_t nodeidx_foreval, uint32_t spineload, uint32_t leafload);
		GetResponseLargevalueServer(method_t methodid, const char * data, uint32_t recv_size);

		static size_t get_frag_hdrsize(method_t methodid);
};

// For key being evicted

template<class key_t>
class GetRequestBeingevicted : public GetRequest<key_t> { // ophdr
	public: 
		GetRequestBeingevicted();
		GetRequestBeingevicted(method_t methodid, key_t key);
		GetRequestBeingevicted(method_t methodid, const char * data, uint32_t recv_size);

		virtual uint32_t serialize(char * const data, uint32_t max_size);
};

template<class key_t, class val_t>
class PutRequestSeqBeingevicted : public PutRequestSeq<key_t, val_t> { // ophdr + val + shadowtype + seq
	public: 
		PutRequestSeqBeingevicted(method_t methodid, const char * data, uint32_t recv_size);

		virtual uint32_t serialize(char * const data, uint32_t max_size);
};

template<class key_t, class val_t>
class PutRequestSeqCase3Beingevicted : public PutRequestSeq<key_t, val_t> { // ophdr + val + shadowtype + seq
	public: 
		PutRequestSeqCase3Beingevicted(method_t methodid, const char * data, uint32_t recv_size);

		virtual uint32_t serialize(char * const data, uint32_t max_size);
};

template<class key_t>
class DelRequestSeqBeingevicted : public DelRequestSeq<key_t> { // ophdr + shadowtype + seq
	public: 
		DelRequestSeqBeingevicted(method_t methodid, const char * data, uint32_t recv_size);

		virtual uint32_t serialize(char * const data, uint32_t max_size);
};

template<class key_t>
class DelRequestSeqCase3Beingevicted : public DelRequestSeq<key_t> { // ophdr + shadowtype + seq
	public: 
		DelRequestSeqCase3Beingevicted(method_t methodid, const char * data, uint32_t recv_size);

		virtual uint32_t serialize(char * const data, uint32_t max_size);
};

template<class key_t, class val_t>
class PutRequestLargevalueSeqBeingevicted : public PutRequestLargevalueSeq<key_t, val_t> { // ophdr + shadowtype + seq + client_logical_idx + fragseq + val in payload (NOT parsed by switch)
	public:
		PutRequestLargevalueSeqBeingevicted(method_t methodid, key_t key, val_t val, uint32_t seq, uint16_t client_logical_idx, uint32_t fragseq);
		PutRequestLargevalueSeqBeingevicted(method_t methodid, const char * data, uint32_t recv_size);

		static size_t get_frag_hdrsize(method_t methodid);
};

template<class key_t, class val_t>
class PutRequestLargevalueSeqCase3Beingevicted : public PutRequestLargevalueSeq<key_t, val_t> { // ophdr + shadowtype + seq + client_logical_idx + fragseq + val in payload (NOT parsed by switch)
	public:
		PutRequestLargevalueSeqCase3Beingevicted(method_t methodid, key_t key, val_t val, uint32_t seq, uint16_t client_logical_idx, uint32_t fragseq);
		PutRequestLargevalueSeqCase3Beingevicted(method_t methodid, const char * data, uint32_t recv_size);

		static size_t get_frag_hdrsize(method_t methodid);
};

// Fix Tofino bugs for NetCache

// NOTE: only used in end-hosts
template<class key_t, class val_t>
class NetcacheCachePopAckNLatest : public NetcacheCachePopAck<key_t, val_t> { // ophdr + default val (vallen=0) + seq + stat (not stat_hdr) + serveridx
	public: 
		NetcacheCachePopAckNLatest(method_t methodid, key_t key, uint32_t seq, bool stat, uint16_t serveridx);
		NetcacheCachePopAckNLatest(method_t methodid, const char * data, uint32_t recv_size);
};

template<class key_t, class val_t>
class NetcacheCachePopInswitchNLatest : public CachePopInswitch<key_t, val_t> { // ophdr + default val (vallen=0) + shadowtype + seq + inswitch_hdr + stat_hdr
	public:
		NetcacheCachePopInswitchNLatest(method_t methodid, key_t key, uint32_t seq, uint16_t freeidx, bool stat);
};

// APIs
static uint32_t serialize_packet_type(optype_t type, char * data, uint32_t maxsize);
static uint32_t dynamic_serialize_packet_type(optype_t type, dynamic_array_t &dynamic_data, int off);
static packet_type_t get_packet_type(const char * data, uint32_t recv_size);
static uint32_t deserialize_packet_type(optype_t &type, const char * data, uint32_t recvsize);
static uint32_t serialize_switchidx(switchidx_t switchidx, char *data, uint32_t maxsize);
static uint32_t dynamic_serialize_switchidx(switchidx_t switchidx, dynamic_array_t &dynamic_data, int off);
static uint32_t deserialize_switchidx(switchidx_t &switchidx, const char *data, uint32_t recvsize);

static netreach_key_t get_packet_key(method_t methodid, const char * data, uint32_t recvsize);
static bool is_same_optype(packet_type_t type1, packet_type_t type2);
// Util APIs for large value
static size_t get_frag_hdrsize(method_t methodid, packet_type_t type);
static size_t get_frag_totalsize(method_t methodid, packet_type_t type, size_t frag_maxsize);
static uint16_t get_packet_clientlogicalidx(method_t methodid, const char * data, uint32_t recvsize);
static uint32_t get_packet_fragseq(method_t methodid, const char * data, uint32_t recvsize);
static bool is_packet_with_largevalue(packet_type_t type); // whether the packet is large to be processed by udprecvlarge_ipfrag
static bool is_packet_with_clientlogicalidx(packet_type_t type); // whether the large packet is sent to server
// Utils for range query
static size_t get_srcnum_off(method_t methodid, packet_type_t type);
static size_t get_srcnum_len(packet_type_t type);
static bool get_srcnum_conversion(packet_type_t type);
static size_t get_srcid_off(method_t methodid, packet_type_t type);
static size_t get_srcid_len(packet_type_t type);
static bool get_srcid_conversion(packet_type_t type);
static size_t get_srcswitchnum_off(method_t methodid, packet_type_t type);
static size_t get_srcswitchnum_len(packet_type_t type);
static bool get_srcswitchnum_conversion(packet_type_t type);
static size_t get_srcswitchid_off(method_t methodid, packet_type_t type);
static size_t get_srcswitchid_len(packet_type_t type);
static bool get_srcswitchid_conversion(packet_type_t type);

#endif
