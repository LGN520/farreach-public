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

// # of bytes before idx in inswitch_hdr
#define INSWITCH_PREV_BYTES 14

// # of bytes in clone_hdr
#define CLONE_BYTES 14

// # of bytes before cur_scanidx&max_scannum&cur_scanswitchidx&max_scanswitchnum in split_hdr
#define SPLIT_PREV_BYTES 4

// # of padding bytes in stat_hdr
#define STAT_PADDING_BYTES 1

// # of bytes in debug_hdr
//#define DEBUG_BYTES 1

// op_hdr -> scan_hdr -> split_hdr -> vallen_hdr -> val_hdr -> shadowtype_hdr -> seq_hdr -> inswitch_hdr -> stat_hdr -> clone_hdr

// (1) vallen&value: mask 0b0001; seq: mask 0b0010; inswitch_hdr: mask 0b0100; stat: mask 0b1000;
// (2) scan/split: specific value (X + 0b0000); not parsed optypes: X + 0b0000
enum class PacketType {
	//WARMUPREQ=0x0011, 
	//LOADREQ=0x0021, LOADREQ_SPINE=0x0031, 
	PUTREQ=0x0001, DISTNOCACHE_PUTREQ_SPINE=0x0041,
	PUTREQ_SEQ=0x0003, PUTREQ_POP_SEQ=0x0013, PUTREQ_SEQ_CASE3=0x0023, PUTREQ_POP_SEQ_CASE3=0x0033, NETCACHE_PUTREQ_SEQ_CACHED=0x0043,
	//CACHE_POP_INSWITCH=0x0007,
	GETRES_LATEST_SEQ_INSWITCH=0x000f, GETRES_DELETED_SEQ_INSWITCH=0x001f, GETRES_LATEST_SEQ_INSWITCH_CASE1=0x002f, GETRES_DELETED_SEQ_INSWITCH_CASE1=0x003f, PUTREQ_SEQ_INSWITCH_CASE1=0x004f, DELREQ_SEQ_INSWITCH_CASE1=0x005f, LOADSNAPSHOTDATA_INSWITCH_ACK=0x006f, CACHE_POP_INSWITCH=0x007f, NETCACHE_VALUEUPDATE_INSWITCH=0x008f, GETRES_LATEST_SEQ_SERVER_INSWITCH=0x009f, GETRES_DELETED_SEQ_SERVER_INSWITCH=0x010f, DISTCACHE_SPINE_VALUEUPDATE_INSWITCH=0x011f, DISTCACHE_LEAF_VALUEUPDATE_INSWITCH=0x012f, DISTCACHE_VALUEUPDATE_INSWITCH=0x013f, DISTCACHE_VALUEUPDATE_INSWITCH_ORIGIN=0x014f,
	GETRES_LATEST_SEQ=0x000b, GETRES_DELETED_SEQ=0x001b, CACHE_EVICT_LOADDATA_INSWITCH_ACK=0x002b, NETCACHE_VALUEUPDATE=0x003b, GETRES_LATEST_SEQ_SERVER=0x004b, GETRES_DELETED_SEQ_SERVER=0x005b,
	GETRES=0x0009, GETRES_SERVER=0x0019, DISTCACHE_GETRES_SPINE=0x0029,
	PUTREQ_INSWITCH=0x0005,
	DELREQ_SEQ_INSWITCH=0x0006, PUTREQ_LARGEVALUE_SEQ_INSWITCH=0x0016,
	PUTREQ_SEQ_INSWITCH=0x0007,
	GETREQ_INSWITCH=0x0004, DELREQ_INSWITCH=0x0014, CACHE_EVICT_LOADFREQ_INSWITCH=0x0024, CACHE_EVICT_LOADDATA_INSWITCH=0x0034, LOADSNAPSHOTDATA_INSWITCH=0x0044, SETVALID_INSWITCH=0x0054, NETCACHE_WARMUPREQ_INSWITCH=0x0064, NETCACHE_WARMUPREQ_INSWITCH_POP=0x0074, DISTCACHE_INVALIDATE_INSWITCH=0x0084, DISTCACHE_VALUEUPDATE_INSWITCH_ACK=0x0094, PUTREQ_LARGEVALUE_INSWITCH=0x00a4,
	DELREQ_SEQ=0x0002, DELREQ_SEQ_CASE3=0x0012, NETCACHE_DELREQ_SEQ_CACHED=0x0022, PUTREQ_LARGEVALUE_SEQ=0x0032,
	PUTRES=0x0008, DELRES=0x0018, PUTRES_SERVER=0x0028, DELRES_SERVER=0x0038,
	WARMUPREQ=0x0000, SCANREQ=0x0010, SCANREQ_SPLIT=0x0020, GETREQ=0x0030, DELREQ=0x0040, GETREQ_POP=0x0050, GETREQ_NLATEST=0x0060, CACHE_POP_INSWITCH_ACK=0x0070, SCANRES_SPLIT=0x0080, CACHE_POP=0x0090, CACHE_EVICT=0x00a0, CACHE_EVICT_ACK=0x00b0, CACHE_EVICT_CASE2=0x00c0, WARMUPACK=0x00d0, LOADACK=0x00e0, CACHE_POP_ACK=0x00f0, CACHE_EVICT_LOADFREQ_INSWITCH_ACK=0x0100, SETVALID_INSWITCH_ACK=0x0110, NETCACHE_GETREQ_POP=0x0120, NETCACHE_CACHE_POP=0x0130, NETCACHE_CACHE_POP_ACK=0x0140, NETCACHE_CACHE_POP_FINISH=0x0150, NETCACHE_CACHE_POP_FINISH_ACK=0x0160, NETCACHE_CACHE_EVICT=0x0170, NETCACHE_CACHE_EVICT_ACK=0x0180, NETCACHE_VALUEUPDATE_ACK=0x0190, GETREQ_SPINE=0x0200, SCANRES_SPLIT_SERVER=0x0210, WARMUPREQ_SPINE=0x0220, WARMUPACK_SERVER=0x0230, LOADACK_SERVER=0x0240, DISTCACHE_CACHE_EVICT_VICTIM=0x0250, DISTCACHE_CACHE_EVICT_VICTIM_ACK=0x0260, DISTNOCACHE_DELREQ_SPINE=0x0270, DISTCACHE_INVALIDATE=0x0280, DISTCACHE_INVALIDATE_ACK=0x0290, DISTCACHE_UPDATE_TRAFFICLOAD=0x02a0, DISTCACHE_SPINE_VALUEUPDATE_INSWITCH_ACK=0x02b0, DISTCACHE_LEAF_VALUEUPDATE_INSWITCH_ACK=0x02c0, PUTREQ_LARGEVALUE=0x02d0, DISTNOCACHE_PUTREQ_LARGEVALUE_SPINE=0x02e0, GETRES_LARGEVALUE_SERVER=0x02f0, GETRES_LARGEVALUE=0x0300, LOADREQ=0x0310, LOADREQ_SPINE=0x0320
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

// NOTE: update get_frag_hdrsize accordingly
static const uint32_t optype_for_udprecvlarge_large_num = 4;
static const packet_type_t optype_for_udprecvlarge_ipfrag_list[optype_for_recvlarge_num] = {packet_type_t::PUTREQ_LARGEVALUE, packet_type_t::PUTREQ_LARGEVALUE_SEQ, packet_type_t::GETRES_LARGEVALUE, packet_type_t::LOADREQ};

typedef uint16_t optype_t;
typedef uint16_t switchidx_t;

template<class key_t> class ScanRequestSplit;
template<class key_t, class val_t> class PutRequestSeq;

template<class key_t>
class Packet {
	public:
		Packet();
		Packet(packet_type_t type, switchidx_t globalswitchidx, key_t key);
		virtual ~Packet(){}

		packet_type_t type() const;
		switchidx_t globalswitchidx() const;
		key_t key() const;

		virtual uint32_t serialize(char * const data, uint32_t max_size) = 0;
	protected:
		optype_t _type;
		switchidx_t _globalswitchidx;
		key_t _key;

		virtual uint32_t size() = 0;
		virtual void deserialize(const char * data, uint32_t recv_size) = 0;

		uint32_t serialize_ophdr(char * const data, uint32_t max_size);
		uint32_t dynamic_serialize_ophdr(dynamic_array_t &dynamic_data);
		uint32_t deserialize_ophdr(const char * data, uint32_t recv_size);
};

template<class key_t>
class GetRequest : public Packet<key_t> { // ophdr
	public: 
		GetRequest();
		GetRequest(key_t key);
		GetRequest(const char * data, uint32_t recv_size);
		virtual ~GetRequest(){}

		virtual uint32_t serialize(char * const data, uint32_t max_size);
	protected:
		virtual uint32_t size();
		virtual void deserialize(const char * data, uint32_t recv_size);
};

template<class key_t, class val_t>
class PutRequest : public Packet<key_t> { // ophdr + val + shadowtype
	public:
		PutRequest();
		PutRequest(key_t key, val_t val);
		PutRequest(const char * data, uint32_t recv_size);

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
		DelRequest(key_t key);
		DelRequest(const char * data, uint32_t recv_size);

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
		ScanRequest(key_t key, key_t endkey);
		ScanRequest(const char * data, uint32_t recv_size);

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
class GetResponse : public Packet<key_t> { // ophdr + val + shadowtype + stat_hdr
	public:
		GetResponse();
		GetResponse(key_t key, val_t val, bool stat, uint16_t nodeidx_foreval);
		GetResponse(const char * data, uint32_t recv_size);

		val_t val() const;
		bool stat() const;
		uint16_t nodeidx_foreval() const;

		virtual uint32_t serialize(char * const data, uint32_t max_size);
	protected:
		virtual uint32_t size();
		virtual void deserialize(const char * data, uint32_t recv_size);
	private:
		val_t _val;
		bool _stat;
		uint16_t _nodeidx_foreval;
};

template<class key_t, class val_t>
class GetResponseServer : public GetResponse<key_t, val_t> { // ophdr + val + shadowtype + stat_hdr
	public:
		GetResponseServer(key_t key, val_t val, bool stat, uint16_t nodeidx_foreval);
};

template<class key_t>
class PutResponse : public Packet<key_t> { // ophdr + shadowtype + stat_hdr
	public: 
		PutResponse(key_t key, bool stat, uint16_t nodeidx_foreval);
		PutResponse(const char * data, uint32_t recv_size);

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
class PutResponseServer : public PutResponse<key_t> { // ophdr + shadowtype + stat_hdr
	public:
		PutResponseServer(key_t key, bool stat, uint16_t nodeidx_foreval);
};

template<class key_t>
class DelResponse : public Packet<key_t> { // ophdr + shadowtype + stat_hdr
	public: 
		DelResponse(key_t key, bool stat, uint16_t nodeidx_foreval);
		DelResponse(const char * data, uint32_t recv_size);

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
class DelResponseServer : public DelResponse<key_t> { // ophdr + shadowtype + stat_hdr
	public:
		DelResponseServer(key_t key, bool stat, uint16_t nodeidx_foreval);
};

template<class key_t, class val_t>
class ScanResponseSplit : public ScanRequestSplit<key_t> { // ophdr + scanhdr(endkey) + splithdr(isclone+curscanidx+maxscannum+curscanswitchidx+maxscanswitchnum) + nodeidx_foreval(not processed by switch) + snapshotid(not processed by switch) + pairs
	public: 
		//ScanResponseSplit(key_t key, key_t endkey, uint32_t num, uint16_t cur_scanidx, uint16_t max_scannum, int32_t pairnum, std::vector<std::pair<key_t, val_t>> pairs);
		ScanResponseSplit(key_t key, key_t endkey, uint16_t cur_scanidx, uint16_t max_scannum, uint16_t cur_scanswitchidx, uint16_t max_scanswitchnum, uint16_t nodeidx_foreval, int snapshotid, int32_t parinum, std::vector<std::pair<key_t, snapshot_record_t>> pairs);
		ScanResponseSplit(const char * data, uint32_t recv_size);

		uint16_t nodeidx_foreval() const;
		int snapshotid() const;
		int32_t pairnum() const;
		std::vector<std::pair<key_t, snapshot_record_t>> pairs() const;

		virtual uint32_t serialize(char * const data, uint32_t max_size);
		uint32_t dynamic_serialize(dynamic_array_t &dynamic_data);

		static size_t get_frag_hdrsize();

		// cur_scanidx and max_scannum of server for a given leaf switch
		static size_t get_srcnum_off();
		static size_t get_srcnum_len();
		static bool get_srcnum_conversion();
		static size_t get_srcid_off();
		static size_t get_srcid_len();
		static bool get_srcid_conversion();

		// cur_scanswitchidx and max_scanswitchnum of leaf switch
		static size_t get_srcswitchnum_off();
		static size_t get_srcswitchnum_len();
		static bool get_srcswitchnum_conversion();
		static size_t get_srcswitchid_off();
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
		ScanResponseSplitServer(key_t key, key_t endkey, uint16_t cur_scanidx, uint16_t max_scannum, uint16_t cur_scanswitchidx, uint16_t max_scanswitchnum, uint16_t nodeidx_foreval, int snapshotid, int32_t parinum, std::vector<std::pair<key_t, snapshot_record_t>> pairs);
};

template<class key_t>
class GetRequestPOP : public GetRequest<key_t> { // ophdr
	public: 
		GetRequestPOP(const char * data, uint32_t recv_size);

		virtual uint32_t serialize(char * const data, uint32_t max_size);
};

template<class key_t>
class GetRequestNLatest : public GetRequest<key_t> { // ophdr
	public: 
		GetRequestNLatest(const char * data, uint32_t recv_size);

		virtual uint32_t serialize(char * const data, uint32_t max_size);
};

template<class key_t, class val_t>
class GetResponseLatestSeq : public PutRequestSeq<key_t, val_t> { // ophdr + val + shadowtype + seq + stat_hdr (stat=true)
	public: 
		GetResponseLatestSeq();
		GetResponseLatestSeq(key_t key, val_t val, uint32_t seq, uint16_t nodeidx_foreval);
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
		GetResponseLatestSeqServer(switchidx_t switchidx, key_t key, val_t val, uint32_t seq, uint16_t nodeidx_foreval);
};

template<class key_t, class val_t>
class GetResponseLatestSeqInswitchCase1 : public GetResponseLatestSeq<key_t, val_t> { // ophdr + val + shadowtype + seq + inswitch.idx + stat_hdr (nodeidx_foreval=0) + clone_hdr
	public: 
		GetResponseLatestSeqInswitchCase1();
		GetResponseLatestSeqInswitchCase1(key_t key, val_t val, uint32_t seq, uint16_t idx, bool stat);
		GetResponseLatestSeqInswitchCase1(const char * data, uint32_t recv_size);

		uint16_t idx() const;
		bool stat() const;

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
		GetResponseDeletedSeq(key_t key, val_t val, uint32_t seq, uint16_t nodeidx_foreval);

	protected:
		virtual void deserialize(const char * data, uint32_t recv_size);
		// NOTE: _stat must be false for GetResponseDeletedSeq
};

template<class key_t, class val_t>
class GetResponseDeletedSeqServer : public GetResponseDeletedSeq<key_t, val_t> { // ophdr + val + shadowtype + seq + stat_hdr (stat=false)
	public: 
		GetResponseDeletedSeqServer(switchidx_t switchidx, key_t key, val_t val, uint32_t seq, uint16_t nodeidx_foreval);
};

template<class key_t, class val_t>
class GetResponseDeletedSeqInswitchCase1 : public GetResponseLatestSeqInswitchCase1<key_t, val_t> { // ophdr + val + shadowtype + seq + inswitch.idx + stat_hdr + clone_hdr
	public: 
		GetResponseDeletedSeqInswitchCase1(key_t key, val_t val, uint32_t seq, uint16_t idx, bool stat);
		GetResponseDeletedSeqInswitchCase1(const char * data, uint32_t recv_size);
};

template<class key_t, class val_t>
class PutRequestSeq : public Packet<key_t> { // ophdr + val + shadowtype + seq
	public: 
		PutRequestSeq();
		PutRequestSeq(key_t key, val_t val, uint32_t seq);
		PutRequestSeq(const char * data, uint32_t recv_size);

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
		PutRequestPopSeq(const char * data, uint32_t recv_size);

		virtual uint32_t serialize(char * const data, uint32_t max_size);
};

template<class key_t, class val_t>
class PutRequestSeqInswitchCase1 : public GetResponseLatestSeqInswitchCase1<key_t, val_t> { // ophdr + val + shadowtype + seq + inswitch.idx + stat_hdr + clone_hdr
	public: 
		PutRequestSeqInswitchCase1(key_t key, val_t val, uint32_t seq, uint16_t idx, bool stat);
		PutRequestSeqInswitchCase1(const char * data, uint32_t recv_size);
};

template<class key_t, class val_t>
class PutRequestSeqCase3 : public PutRequestSeq<key_t, val_t> { // ophdr + val + shadowtype + seq
	public: 
		PutRequestSeqCase3(const char * data, uint32_t recv_size);

		virtual uint32_t serialize(char * const data, uint32_t max_size);
};

template<class key_t, class val_t>
class PutRequestPopSeqCase3 : public PutRequestSeq<key_t, val_t> { // ophdr + val + shadowtype + seq
	public: 
		PutRequestPopSeqCase3(const char * data, uint32_t recv_size);

		virtual uint32_t serialize(char * const data, uint32_t max_size);
};

template<class key_t>
class DelRequestSeq : public Packet<key_t> { // ophdr + shadowtype + seq
	public: 
		DelRequestSeq();
		DelRequestSeq(const char * data, uint32_t recv_size);

		virtual uint32_t serialize(char * const data, uint32_t max_size);

		uint32_t seq() const;

	protected:
		virtual uint32_t size();
		virtual void deserialize(const char * data, uint32_t recv_size);
		uint32_t _seq;
};

template<class key_t, class val_t>
class DelRequestSeqInswitchCase1 : public GetResponseLatestSeqInswitchCase1<key_t, val_t> { // ophdr + val + shadowtype + seq + inswitch.idx + stat_hdr + clone_hdr
	public: 
		DelRequestSeqInswitchCase1(key_t key, val_t val, uint32_t seq, uint16_t idx, bool stat);
		DelRequestSeqInswitchCase1(const char * data, uint32_t recv_size);
};

template<class key_t>
class DelRequestSeqCase3 : public DelRequestSeq<key_t> { // ophdr + shadowtype + seq
	public: 
		DelRequestSeqCase3(const char * data, uint32_t recv_size);

		virtual uint32_t serialize(char * const data, uint32_t max_size);
};

template<class key_t>
class ScanRequestSplit : public ScanRequest<key_t> { // ophdr + scanhdr + splithdr
	public: 
		ScanRequestSplit();
		ScanRequestSplit(key_t key, key_t endkey, uint16_t cur_scanidx, uint16_t max_scannum, uint16_t cur_scanswitchidx, uint16_t max_scanswitchnum);
		ScanRequestSplit(const char * data, uint32_t recv_size);

		uint16_t cur_scanidx() const;
		uint16_t max_scannum() const;
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
		CachePop(key_t key, val_t val, uint32_t seq, bool stat, uint16_t serveridx);
		CachePop(const char * data, uint32_t recv_size);

		virtual uint32_t serialize(char * const data, uint32_t max_size);

		bool stat() const;
		uint16_t serveridx() const;

	protected:
		virtual uint32_t size();
		virtual void deserialize(const char * data, uint32_t recv_size);
		bool _stat;
		uint16_t _serveridx;
};

template<class key_t, class val_t>
class CachePopInswitch : public PutRequestSeq<key_t, val_t> { // ophdr + val + shadowtype + seq + inswitch_hdr + stat_hdr
	public: 
		CachePopInswitch(switchidx_t switchidx, key_t key, val_t val, uint32_t seq, uint16_t freeidx, bool stat);

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
class CachePopInswitchAck : public GetRequest<key_t> { // ophdr
	public: 
		CachePopInswitchAck(const char * data, uint32_t recv_size);

		virtual uint32_t serialize(char * const data, uint32_t max_size);
};

// NOTE: only used in end-hosts
template<class key_t, class val_t>
class CacheEvict : public GetResponseLatestSeq<key_t, val_t> { // ophdr + val + seq + stat + serveridx
	public: 
		CacheEvict();
		CacheEvict(key_t key, val_t val, uint32_t seq, bool stat, uint16_t serveridx);
		CacheEvict(const char * data, uint32_t recv_size);
		virtual ~CacheEvict(){}

		uint16_t serveridx() const;

		virtual uint32_t serialize(char * const data, uint32_t max_size);
	protected:
		virtual uint32_t size();
		virtual void deserialize(const char * data, uint32_t recv_size);
		uint16_t _serveridx;
};

// NOTE: only used in end-hosts
template<class key_t>
class CacheEvictAck : public GetRequest<key_t> { // ophdr
	public: 
		CacheEvictAck(key_t key);
		CacheEvictAck(const char * data, uint32_t recv_size);
		virtual ~CacheEvictAck(){}
};

// NOTE: only used in end-hosts
template<class key_t, class val_t>
class CacheEvictCase2 : public CacheEvict<key_t, val_t> { // ophdr + val + seq + stat + serveridx
	public: 
		CacheEvictCase2(key_t key, val_t val, uint32_t seq, bool stat, uint16_t serveridx);
		CacheEvictCase2(const char * data, uint32_t recv_size);
};

/*template<class key_t, class val_t>
class WarmupRequest : public PutRequest<key_t, val_t> { // ophdr + val + shadowtype
	public: 
		WarmupRequest(key_t key, val_t val);
		WarmupRequest(const char * data, uint32_t recv_size);
};*/

template<class key_t>
class WarmupRequest : public GetRequest<key_t> { // ophdr
	public: 
		WarmupRequest(key_t key);
		WarmupRequest(const char * data, uint32_t recv_size);
};

template<class key_t>
class WarmupAck : public GetRequest<key_t> { // ophdr
	public: 
		WarmupAck(key_t key);
		WarmupAck(const char * data, uint32_t recv_size);
};

template<class key_t, class val_t>
class LoadRequest : public Packet<key_t> { // ophdr + val in payload (NOT parsed by switch -> NOT need shadowtype)
	public: 
		LoadRequest(key_t key, val_t val);
		LoadRequest(const char * data, uint32_t recv_size);

		static size_t get_frag_hdrsize();
		uint32_t dynamic_serialize(dynamic_array_t &dynamic_data);

		val_t val() const;

		virtual uint32_t serialize(char * const data, uint32_t max_size);
	protected:
		virtual uint32_t size();
		virtual void deserialize(const char * data, uint32_t recv_size);
		val_t _val;
};

template<class key_t>
class LoadAck : public GetRequest<key_t> { // ophdr
	public: 
		LoadAck(key_t key);
		LoadAck(const char * data, uint32_t recv_size);
};

// NOTE: only used in end-hosts
template<class key_t>
class CachePopAck : public GetRequest<key_t> { // ophdr
	public: 
		CachePopAck(key_t key);
		CachePopAck(const char * data, uint32_t recv_size);
};

template<class key_t>
class CacheEvictLoadfreqInswitch : public Packet<key_t> { // ophdr + shadowtype + inswitch_hdr
	public: 
		CacheEvictLoadfreqInswitch(switchidx_t switchidx, key_t key, uint16_t evictidx);

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
		CacheEvictLoadfreqInswitchAck(const char * data, uint32_t recv_size);

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
		CacheEvictLoaddataInswitch(switchidx_t switchidx, key_t key, uint16_t evictidx);
};

template<class key_t, class val_t>
class CacheEvictLoaddataInswitchAck : public Packet<key_t> { // ophdr + val + shadowtype + seq + stat_hdr
	public: 
		CacheEvictLoaddataInswitchAck(const char * data, uint32_t recv_size);

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
		LoadsnapshotdataInswitch(switchidx_t switchidx, key_t key, uint16_t loadidx);

		uint16_t loadidx() const;
};

template<class key_t, class val_t>
class LoadsnapshotdataInswitchAck : public GetResponseLatestSeqInswitchCase1<key_t, val_t> { // ophdr + val + shadowtype + seq + inswitch_hdr.idx + stat_hdr
	public: 
		LoadsnapshotdataInswitchAck(const char * data, uint32_t recv_size);
	private:
		virtual uint32_t size();
		virtual void deserialize(const char * data, uint32_t recv_size);
};

template<class key_t>
class SetvalidInswitch : public Packet<key_t> { // ophdr + shadowtype + inswitch_hdr + validvalue_hdr
	public: 
		SetvalidInswitch(switchidx_t switchidx, key_t key, uint16_t idx, uint8_t validvalue);

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
class SetvalidInswitchAck : public GetRequest<key_t> { // ophdr
	public: 
		SetvalidInswitchAck(const char * data, uint32_t recv_size);

		virtual uint32_t serialize(char * const data, uint32_t max_size);
};

template<class key_t>
class WarmupAckServer : public WarmupAck<key_t> { // ophdr
	public: 
		WarmupAckServer(key_t key);
};

template<class key_t>
class LoadAckServer : public LoadAck<key_t> { // ophdr
	public: 
		LoadAckServer(key_t key);
};

// For large value

template<class key_t, class val_t>
class PutRequestLargevalue : public Packet<key_t> { // ophdr + val in payload (NOT parsed by switch -> NOT need shadowtype_hdr)
	public:
		PutRequestLargevalue();
		PutRequestLargevalue(key_t key, val_t val);
		PutRequestLargevalue(const char * data, uint32_t recv_size);

		static size_t get_frag_hdrsize();
		uint32_t dynamic_serialize(dynamic_array_t &dynamic_data);

		val_t val() const;

		virtual uint32_t serialize(char * const data, uint32_t max_size);
	protected:
		virtual uint32_t size();
		virtual void deserialize(const char * data, uint32_t recv_size);
		val_t _val;
};

template<class key_t, class val_t>
class PutRequestLargevalueSeq : public PutRequestLargevalue<key_t, val_t> { // ophdr + shadowtype + seq + val in payload (NOT parsed by switch)
	public:
		PutRequestLargevalueSeq(key_t key, val_t val, uint32_t seq);
		PutRequestLargevalueSeq(const char * data, uint32_t recv_size);

		static size_t get_frag_hdrsize();

		uint32_t seq() const;

		virtual uint32_t serialize(char * const data, uint32_t max_size);
	protected:
		virtual uint32_t size();
		virtual void deserialize(const char * data, uint32_t recv_size);
		uint32_t _seq;
};

template<class key_t, class val_t>
class GetResponseLargevalue : public Packet<key_t> { // ophdr + val&stat_hdr in payload (NOT parsed by switch -> NOT need shadowtype_hdr)
	public:
		GetResponseLargevalue();
		GetResponseLargevalue(key_t key, val_t val, bool stat, uint16_t nodeidx_foreval);
		GetResponseLargevalue(const char * data, uint32_t recv_size);

		static size_t get_frag_hdrsize();
		uint32_t dynamic_serialize(dynamic_array_t &dynamic_data);

		val_t val() const;
		bool stat() const;
		uint16_t nodeidx_foreval() const;

		virtual uint32_t serialize(char * const data, uint32_t max_size);
	protected:
		virtual uint32_t size();
		virtual void deserialize(const char * data, uint32_t recv_size);
		val_t _val;
		bool _stat;
		uint16_t _nodeidx_foreval;
};

template<class key_t, class val_t>
class GetResponseLargevalueServer : public GetResponseLargevalue<key_t, val_t> { // ophdr + val&stat_hdr in payload (NOT parsed by switch -> NOT need shadowtype_hdr)
	public:
		GetResponseLargevalueServer(key_t key, val_t val, bool stat, uint16_t nodeidx_foreval);
		GetResponseLargevalueServer(const char * data, uint32_t recv_size);

		static size_t get_frag_hdrsize();
};

// APIs
static uint32_t serialize_packet_type(optype_t type, char * data, uint32_t maxsize);
static uint32_t dynamic_serialize_packet_type(optype_t type, dynamic_array_t &dynamic_data, int off);
static packet_type_t get_packet_type(const char * data, uint32_t recv_size);
static uint32_t deserialize_packet_type(optype_t &type, const char * data, uint32_t recvsize);
static uint32_t serialize_switchidx(switchidx_t switchidx, char *data, uint32_t maxsize);
static uint32_t dynamic_serialize_switchidx(switchidx_t switchidx, dynamic_array_t &dynamic_data, int off);
static uint32_t deserialize_switchidx(switchidx_t &switchidx, const char *data, uint32_t recvsize);
static size_t get_frag_hdrsize(optype_t type);

#endif
