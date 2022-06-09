#ifndef PACKET_FORMAT_H
#define PACKET_FORMAT_H

#include <array>
#include <cstring>
#include <vector>

#include "helper.h"
#include "snapshot_record.h"

// mask for other_hdr
#define VALID_MASK 0x01

// # of bytes before idx in inswitch_hdr
#define INSWITCH_PREV_BYTES 14

// # of bytes in clone_hdr
#define CLONE_BYTES 3

// # of bytes before cur_scanidx&max_scannum in split_hdr
#define SPLIT_PREV_BYTES 1

// # of bytes in debug_hdr
//#define DEBUG_BYTES 1

// op_hdr -> scan_hdr -> split_hdr -> vallen_hdr -> val_hdr -> shadowtype_hdr -> seq_hdr -> inswitch_hdr -> stat_hdr -> clone_hdr

// (1) vallen&value: mask 0b0001; seq: mask 0b0010; inswitch_hdr: mask 0b0100; stat: mask 0b1000;
// (2) scan/split: specific value (X + 0b0000); not parsed optypes: X + 0b0000
enum class PacketType {
	PUTREQ=0x01, WARMUPREQ=0x11, LOADREQ=0x21,
	GETRES_LATEST_SEQ=0x03, GETRES_DELETED_SEQ=0x13, PUTREQ_SEQ=0x23, PUTREQ_POP_SEQ=0x33, PUTREQ_SEQ_CASE3=0x43, PUTREQ_POP_SEQ_CASE3=0x53,
	GETRES_LATEST_SEQ_INSWITCH=0x07, GETRES_DELETED_SEQ_INSWITCH=0x17, CACHE_POP_INSWITCH=0x27,
	GETRES_LATEST_SEQ_INSWITCH_CASE1=0x0f, GETRES_DELETED_SEQ_INSWITCH_CASE1=0x1f, PUTREQ_SEQ_INSWITCH_CASE1=0x2f, DELREQ_SEQ_INSWITCH_CASE1=0x3f,
	GETRES=0x09,
	PUTREQ_INSWITCH=0x05,
	GETREQ_INSWITCH=0x04, DELREQ_INSWITCH=0x14,
	DELREQ_SEQ=0x02, DELREQ_SEQ_CASE3=0x12,
	PUTRES=0x08, DELRES=0x18,
	SCANREQ=0x10, SCANREQ_SPLIT=0x20, GETREQ=0x30, DELREQ=0x40, GETREQ_POP=0x50, GETREQ_NLATEST=0x60, CACHE_POP_INSWITCH_ACK=0x70, SCANRES_SPLIT=0x80, CACHE_POP=0x90, CACHE_EVICT=0xa0, CACHE_EVICT_ACK=0xb0, CACHE_EVICT_CASE2=0xc0, WARMUPACK=0xd0, LOADACK=0xe0, CACHE_POP_ACK=0xf0
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

template<class key_t> class ScanRequestSplit;

template<class key_t>
class Packet {
	public:
		Packet();
		Packet(packet_type_t type, key_t key);
		virtual ~Packet(){}

		packet_type_t type() const;
		key_t key() const;

		virtual uint32_t serialize(char * const data, uint32_t max_size) = 0;
	protected:
		uint8_t _type;
		key_t _key;

		virtual uint32_t size() = 0;
		virtual void deserialize(const char * data, uint32_t recv_size) = 0;
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

template<class key_t, class val_t>
class ScanResponseSplit : public ScanRequestSplit<key_t> { // ophdr + scanhdr(endkey) + splithdr(isclone+curscanidx+maxscannum) + nodeidx_foreval(not processed by switch) + snapshotid(not processed by switch) + pairs
	public: 
		//ScanResponseSplit(key_t key, key_t endkey, uint32_t num, uint16_t cur_scanidx, uint16_t max_scannum, int32_t pairnum, std::vector<std::pair<key_t, val_t>> pairs);
		ScanResponseSplit(key_t key, key_t endkey, uint16_t cur_scanidx, uint16_t max_scannum, uint16_t nodeidx_foreval, int snapshotid, int32_t parinum, std::vector<std::pair<key_t, snapshot_record_t>> pairs);
		ScanResponseSplit(const char * data, uint32_t recv_size);

		uint16_t nodeidx_foreval() const;
		int snapshotid() const;
		int32_t pairnum() const;
		std::vector<std::pair<key_t, snapshot_record_t>> pairs() const;

		virtual uint32_t serialize(char * const data, uint32_t max_size);

		static size_t get_frag_hdrsize();
		static size_t get_srcnum_off();
		static size_t get_srcnum_len();
		static bool get_srcnum_conversion();
		static size_t get_srcid_off();
		static size_t get_srcid_len();
		static bool get_srcid_conversion();
	protected:
		virtual uint32_t size();
		virtual void deserialize(const char * data, uint32_t recv_size);
		uint16_t _nodeidx_foreval;
		int _snapshotid;
		int32_t _pairnum;
		std::vector<std::pair<key_t, snapshot_record_t>> _pairs;
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
class GetResponseLatestSeq : public Packet<key_t> { // ophdr + val + shadowtype + seq
	public: 
		GetResponseLatestSeq();
		GetResponseLatestSeq(key_t key, val_t val, uint32_t seq);
		virtual ~GetResponseLatestSeq(){}

		virtual uint32_t serialize(char * const data, uint32_t max_size);

		val_t val() const;
		uint32_t seq() const;

	protected:
		virtual uint32_t size();
		virtual void deserialize(const char * data, uint32_t recv_size);
		val_t _val;
		uint32_t _seq;
};

template<class key_t, class val_t>
class GetResponseLatestSeqInswitchCase1 : public GetResponseLatestSeq<key_t, val_t> { // ophdr + val + shadowtype + seq + inswitch.idx + stat_hdr + clone_hdr
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
};

template<class key_t, class val_t>
class GetResponseDeletedSeq : public GetResponseLatestSeq<key_t, val_t> { // ophdr + val + shadowtype + seq
	public: 
		GetResponseDeletedSeq(key_t key, val_t val, uint32_t seq);

	protected:
		virtual void deserialize(const char * data, uint32_t recv_size);
};

template<class key_t, class val_t>
class GetResponseDeletedSeqInswitchCase1 : public GetResponseLatestSeqInswitchCase1<key_t, val_t> { // ophdr + val + shadowtype + seq + inswitch.idx + stat_hdr + clone_hdr
	public: 
		GetResponseDeletedSeqInswitchCase1(key_t key, val_t val, uint32_t seq, uint16_t idx, bool stat);
		GetResponseDeletedSeqInswitchCase1(const char * data, uint32_t recv_size);
};

template<class key_t, class val_t>
class PutRequestSeq : public GetResponseLatestSeq<key_t, val_t> { // ophdr + val + shadowtype + seq
	public: 
		PutRequestSeq();
		PutRequestSeq(const char * data, uint32_t recv_size);

		virtual uint32_t serialize(char * const data, uint32_t max_size);

	protected:
		virtual void deserialize(const char * data, uint32_t recv_size);
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
		ScanRequestSplit(key_t key, key_t endkey, uint16_t cur_scanidx, uint16_t max_scannum);
		ScanRequestSplit(const char * data, uint32_t recv_size);

		uint16_t cur_scanidx() const;
		uint16_t max_scannum() const;

		virtual uint32_t serialize(char * const data, uint32_t max_size);
	protected:
		virtual uint32_t size();
		virtual void deserialize(const char * data, uint32_t recv_size);
		uint16_t _cur_scanidx;
		uint16_t _max_scannum;
};

// NOTE: only used in end-hosts
template<class key_t, class val_t>
class CachePop : public GetResponseLatestSeq<key_t, val_t> { // ophdr + val + seq + serveridx
	public: 
		CachePop(key_t key, val_t val, uint32_t seq, uint16_t serveridx);
		CachePop(const char * data, uint32_t recv_size);

		virtual uint32_t serialize(char * const data, uint32_t max_size);

		uint16_t serveridx() const;

	protected:
		virtual uint32_t size();
		virtual void deserialize(const char * data, uint32_t recv_size);
		uint16_t _serveridx;
};

template<class key_t, class val_t>
class CachePopInswitch : public GetResponseLatestSeq<key_t, val_t> { // ophdr + val + shadowtype + seq + inswitch_hdr
	public: 
		CachePopInswitch(key_t key, val_t val, uint32_t seq, uint16_t freeidx);

		virtual uint32_t serialize(char * const data, uint32_t max_size);

		uint16_t freeidx() const;

	protected:
		virtual uint32_t size();
		virtual void deserialize(const char * data, uint32_t recv_size);
		uint16_t _freeidx;
};

template<class key_t>
class CachePopInswitchAck : public GetRequest<key_t> { // ophdr + clone_hdr
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

		bool stat() const;
		uint16_t serveridx() const;

		virtual uint32_t serialize(char * const data, uint32_t max_size);
	protected:
		virtual uint32_t size();
		virtual void deserialize(const char * data, uint32_t recv_size);
		bool _stat;
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

template<class key_t, class val_t>
class WarmupRequest : public PutRequest<key_t, val_t> { // ophdr + val + shadowtype
	public: 
		WarmupRequest(key_t key, val_t val);
		WarmupRequest(const char * data, uint32_t recv_size);
};

template<class key_t>
class WarmupAck : public GetRequest<key_t> { // ophdr
	public: 
		WarmupAck(key_t key);
		WarmupAck(const char * data, uint32_t recv_size);
};

template<class key_t, class val_t>
class LoadRequest : public PutRequest<key_t, val_t> { // ophdr + val + shadowtype
	public: 
		LoadRequest(key_t key, val_t val);
		LoadRequest(const char * data, uint32_t recv_size);
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

// APIs
packet_type_t get_packet_type(const char * data, uint32_t recv_size);

#endif
