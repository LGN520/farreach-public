#ifndef PACKET_FORMAT_H
#define PACKET_FORMAT_H

#include <array>
#include <cstring>
#include <vector>

#include "helper.h"

// mask for other_hdr
#define VALID_MASK 0x01

// # of bytes before idx in inswitch_hdr
#define INSWITCH_PREV_BYTES 7

enum class PacketType {
	GETREQ, PUTREQ, DELREQ, SCANREQ, GETRES, PUTRES, DELRES, SCANRES, GETREQ_INSWITCH, GETREQ_POP, GETREQ_NLATEST, 
	GETRES_LATEST_SEQ, GETRES_LATEST_SEQ_INSWITCH, GETRES_LATEST_SEQ_INSWITCH_CASE1, 
	GETRES_DELETED_SEQ, GETRES_DELETED_SEQ_INSWITCH, GETRES_DELETED_SEQ_INSWITCH_CASE1, 
	PUTREQ_INSWITCH, PUTREQ_SEQ, PUTREQ_POP_SEQ, PUTREQ_SEQ_INSWITCH_CASE1, PUTREQ_SEQ_CASE3, PUTREQ_POP_SEQ_CASE3,
	DELREQ_INSWITCH, DELREQ_SEQ, DELREQ_SEQ_INSWITCH_CASE1, DELREQ_SEQ_CASE3,

	CACHE_POP, CACHE_POP_INSWITCH, CACHE_POP_INSWITCH_ACK, CACHE_EVICT, CACHE_EVICT_ACK
};
typedef PacketType packet_type_t;

template<class key_t>
class Packet {
	public:
		Packet();
		Packet(packet_type_t type, key_t key);

		packet_type_t type() const;
		key_t key() const;

		virtual uint32_t serialize(char * const data, uint32_t max_size) = 0;
	protected:
		int8_t _type;
		key_t _key;

		virtual uint32_t size() = 0;
		virtual void deserialize(const char * data, uint32_t recv_size) = 0;
};

template<class key_t>
class GetRequest : public Packet<key_t> {
	public: 
		GetRequest();
		GetRequest(key_t key);
		GetRequest(const char * data, uint32_t recv_size);

		virtual uint32_t serialize(char * const data, uint32_t max_size);
	protected:
		virtual uint32_t size();
		virtual void deserialize(const char * data, uint32_t recv_size);
};

template<class key_t, class val_t>
class PutRequest : public Packet<key_t> {
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
class DelRequest : public Packet<key_t> {
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
class ScanRequest : public Packet<key_t> {
	public: 
		ScanRequest(uint16_t hashidx, key_t key, key_t endkey, uint32_t num);
		ScanRequest(const char * data, uint32_t recv_size);

		key_t endkey() const;
		uint32_t num() const;

		virtual uint32_t serialize(char * const data, uint32_t max_size);
	protected:
		virtual uint32_t size();
		virtual void deserialize(const char * data, uint32_t recv_size);
	private:
		key_t _endkey;
		uint32_t _num;
};

template<class key_t, class val_t>
class GetResponse : public Packet<key_t> {
	public:
		GetResponse();
		GetResponse(key_t key, val_t val, bool stat);
		GetResponse(const char * data, uint32_t recv_size);

		val_t val() const;
		bool stat() const;

		virtual uint32_t serialize(char * const data, uint32_t max_size);
	protected:
		virtual uint32_t size();
		virtual void deserialize(const char * data, uint32_t recv_size);
	private:
		val_t _val;
		bool _stat;
};

template<class key_t>
class PutResponse : public Packet<key_t> {
	public: 
		PutResponse(key_t key, bool stat);
		PutResponse(const char * data, uint32_t recv_size);

		bool stat() const;

		virtual uint32_t serialize(char * const data, uint32_t max_size);
	protected:
		virtual uint32_t size();
		virtual void deserialize(const char * data, uint32_t recv_size);
	private:
		bool _stat;
};

template<class key_t>
class DelResponse : public Packet<key_t> {
	public: 
		DelResponse(key_t key, bool stat);
		DelResponse(const char * data, uint32_t recv_size);

		bool stat() const;

		virtual uint32_t serialize(char * const data, uint32_t max_size);
	protected:
		virtual uint32_t size();
		virtual void deserialize(const char * data, uint32_t recv_size);
	private:
		bool _stat;
};

template<class key_t, class val_t>
class ScanResponse : public Packet<key_t> {
	public: 
		ScanResponse(uint16_t hashidx, key_t key, key_t endkey, uint32_t num, std::vector<std::pair<key_t, val_t>> pairs);
		ScanResponse(const char * data, uint32_t recv_size);

		key_t endkey() const;
		uint32_t num() const;
		std::vector<std::pair<key_t, val_t>> pairs() const;

		virtual uint32_t serialize(char * const data, uint32_t max_size);
	protected:
		virtual uint32_t size();
		virtual void deserialize(const char * data, uint32_t recv_size);
	private:
		key_t _endkey;
		uint32_t _num;
		std::vector<std::pair<key_t, val_t>> _pairs;
};

template<class key_t>
class GetRequestPOP : public GetRequest<key_t> {
	public: 
		GetRequestPOP(const char * data, uint32_t recv_size);

		virtual uint32_t serialize(char * const data, uint32_t max_size);
};

template<class key_t>
class GetRequestNLatest : public GetRequest<key_t> {
	public: 
		GetRequestNLatest(const char * data, uint32_t recv_size);

		virtual uint32_t serialize(char * const data, uint32_t max_size);
};

template<class key_t, class val_t>
class GetResponseLatestSeq : public Packet<key_t> { // seq (w/o stat)
	public: 
		GetResponseLatestSeq(key_t key, val_t val, int32_t seq);

		virtual uint32_t serialize(char * const data, uint32_t max_size);

		val_t val() const;
		int32_t seq() const;

	protected:
		virtual uint32_t size();
		virtual void deserialize(const char * data, uint32_t recv_size);
		val_t _val;
		int32_t _seq;
};

template<class key_t, class val_t>
class GetResponseLatestSeqInswitchCase1 : public GetResponseLatestSeq<key_t, val_t> { // seq + idx + stat
	public: 
		GetResponseLatestSeqInswitchCase1(key_t key, val_t val, int32_t seq, int16_t idx, bool stat);
		GetResponseLatestSeqInswitchCase1(const char * data, uint32_t recv_size);

		int16_t idx() const;
		bool stat() const;

		virtual uint32_t serialize(char * const data, uint32_t max_size);
	protected:
		virtual uint32_t size();
		virtual void deserialize(const char * data, uint32_t recv_size);
		int16_t _idx;
		bool _stat;
};

template<class key_t, class val_t>
class GetResponseDeletedSeq : public GetResponseLatestSeq<key_t, val_t> { // seq (w/o stat)
	public: 
		GetResponseLatestSeq(key_t key, val_t val, int32_t seq);

	protected:
		virtual void deserialize(const char * data, uint32_t recv_size);
};

template<class key_t, class val_t>
class GetResponseDeletedSeqInswitchCase1 : public GetResponseLatestSeqInswitchCase1<key_t, val_t> { // seq + idx + stat
	public: 
		GetResponseDeletedSeqInswitchCase1(key_t key, val_t val, int32_t seq, int16_t idx, bool stat);
		GetResponseDeletedSeqInswitchCase1(const char * data, uint32_t recv_size);
};

template<class key_t, class val_t>
class PutRequestSeq : public GetResponseLatestSeq<key_t, val_t> { // seq (w/o stat)
	public: 
		PutRequestSeq(const char * data, uint32_t recv_size);

		virtual uint32_t serialize(char * const data, uint32_t max_size);

	protected:
		virtual void deserialize(const char * data, uint32_t recv_size);
};

template<class key_t, class val_t>
class PutRequestPopSeq : public PutRequestSeq<key_t, val_t> { // seq (w/o stat)
	public: 
		PutRequestPopSeq(const char * data, uint32_t recv_size);

		virtual uint32_t serialize(char * const data, uint32_t max_size);
};

template<class key_t, class val_t>
class PutRequestSeqInswitchCase1 : public GetResponseLatestSeqInswitchCase1<key_t, val_t> { // seq + idx + stat
	public: 
		PutRequestSeqInswitchCase1(key_t key, val_t val, int32_t seq, int16_t idx, bool stat);
		PutRequestSeqInswitchCase1(const char * data, uint32_t recv_size);
};

template<class key_t, class val_t>
class PutRequestSeqCase3 : public PutRequestSeq<key_t, val_t> { // seq (w/o stat)
	public: 
		PutRequestSeqCase3(const char * data, uint32_t recv_size);

		virtual uint32_t serialize(char * const data, uint32_t max_size);
};

template<class key_t, class val_t>
class PutRequestPopSeqCase3 : public PutRequestSeq<key_t, val_t> { // seq (w/o stat)
	public: 
		PutRequestPopSeqCase3(const char * data, uint32_t recv_size);

		virtual uint32_t serialize(char * const data, uint32_t max_size);
};

template<class key_t>
class DelRequestSeq : public Packet<key_t> { // seq (w/o stat)
	public: 
		DelRequestSeq(const char * data, uint23_t recv_size);

		virtual uint32_t serialize(char * const data, uint32_t max_size);

		int32_t seq() const;

	protected:
		virtual uint32_t size();
		virtual void deserialize(const char * data, uint32_t recv_size);
		int32_t _seq;
};

template<class key_t, class val_t>
class DelRequestSeqInswitchCase1 : public GetResponseLatestSeqInswitchCase1<key_t, val_t> { // seq + idx + stat
	public: 
		DelRequestSeqInswitchCase1(key_t key, val_t val, int32_t seq, int16_t idx, bool stat);
		DelRequestSeqInswitchCase1(const char * data, uint32_t recv_size);
};

// NOTE: only used in end-hosts
template<class key_t, class val_t>
class CachePop : public GetResponseLatestSeq<key_t, val_t> { // seq (w/o stat) + serveridx
	public: 
		CachePop(key_t key, val_t val, int32_t seq, int16_t serveridx);
		CachePop(const char * data, uint32_t recv_size);

		virtual uint32_t serialize(char * const data, uint32_t max_size);

		int16_t serveridx() const;

	protected:
		virtual uint32_t size();
		virtual void deserialize(const char * data, uint32_t recv_size);
		int16_t _serveridx;
};

template<class key_t, class val_t>
class CachePopInSwitch : public GetResponseLatestSeq<key_t, val_t> { // seq (w/o stat) + inswitch_hdr
	public: 
		CachePopInSwitch(key_t key, val_t val, int32_t seq, int16_t freeidx);

		virtual uint32_t serialize(char * const data, uint32_t max_size);

		int16_t freeidx() const;

	protected:
		virtual uint32_t size();
		virtual void deserialize(const char * data, uint32_t recv_size);
		int16_t _freeidx;
};

template<class key_t>
class CachePopInSwitchAck : public GetRequest<key_t> {
	public: 
		CachePopInswitchAck(const char * data, uint32_t recv_size);

		virtual uint32_t serialize(char * const data, uint32_t max_size);
};

// NOTE: only used in end-hosts
template<class key_t, class val_t>
class CacheEvict : public GetResponseLatestSeq<key_t, val_t> { // seq + stat + serveridx
	public: 
		CacheEvict(key_t key, val_t val, int32_t seq, bool stat, int16_t serveridx);
		CacheEvict(const char * data, uint32_t recv_size);

		bool stat() const;
		int16_t serveridx() const;

		virtual uint32_t serialize(char * const data, uint32_t max_size);
	protected:
		virtual uint32_t size();
		virtual void deserialize(const char * data, uint32_t recv_size);
		bool _stat;
		int16_t _serveridx;
};

// NOTE: only used in end-hosts
template<class key_t>
class CacheEvictAck : public GetRequest<key_t> {
	public: 
		CacheEvictAck(key_t key);
		CacheEvictAck(const char * data, uint32_t recv_size);
};

















template<class key_t, class val_t>
class GetResponsePOP : public GetResponse<key_t, val_t> { // seq
	public: 
		GetResponsePOP(uint16_t hashidx, key_t key, val_t val, int32_t seq);

		virtual uint32_t serialize(char * const data, uint32_t max_size);

		int32_t seq() const;

	protected:
		virtual uint32_t size();
		virtual void deserialize(const char * data, uint32_t recv_size);
		int32_t _seq;
};

template<class key_t, class val_t>
class GetResponseNPOP : public GetResponse<key_t, val_t> {
	public: 
		GetResponseNPOP(uint16_t hashidx, key_t key, val_t val);

	protected:
		virtual void deserialize(const char * data, uint32_t recv_size);
};

template<class key_t, class val_t>
class GetResponsePOPLarge : public GetResponse<key_t, val_t> {
	public: 
		GetResponsePOPLarge(uint16_t hashidx, key_t key, val_t val);

	protected:
		virtual void deserialize(const char * data, uint32_t recv_size);
};

template<class key_t, class val_t>
class GetResponsePOPEvict : public PutRequestSeq<key_t, val_t> { // seq
	public:
		GetResponsePOPEvict(const char * data, uint32_t recv_size);

		virtual uint32_t serialize(char * const data, uint32_t max_size);
};

template<class key_t, class val_t>
class PutRequestSeq : public PutRequest<key_t, val_t> { // seq
	public:
		PutRequestSeq(const char * data, uint32_t recv_size);

		virtual uint32_t serialize(char * const data, uint32_t max_size);

		int32_t seq() const;
	protected:
		virtual uint32_t size();
		virtual void deserialize(const char * data, uint32_t recv_size);
		int32_t _seq;
};

template<class key_t, class val_t>
class PutRequestPOPEvict : public PutRequestSeq<key_t, val_t> { // seq
	public:
		PutRequestPOPEvict(const char * data, uint32_t recv_size);

		virtual uint32_t serialize(char * const data, uint32_t max_size);
};

template<class key_t, class val_t>
class PutRequestLarge : public PutRequest<key_t, val_t> {
	public:
		PutRequestLarge(uint16_t hashidx, key_t key, val_t val, int32_t seq);
		PutRequestLarge(const char * data, uint32_t recv_size);
};

template<class key_t, class val_t>
class PutRequestLargeSeq : public PutRequestSeq<key_t, val_t> { // ophdr + vallen + seq + value (in payload)
	public:
		PutRequestLargeSeq(const char * data, uint32_t recv_size);

	protected:
		virtual void deserialize(const char * data, uint32_t recv_size); // NOTE: deserialize seq before value
		virtual uint32_t serialize(char * const data, uint32_t max_size); // not support serialize
};

template<class key_t, class val_t>
class PutRequestLargeEvict : public PutRequestSeq<key_t, val_t> { // seq
	public:
		PutRequestLargeEvict(const char * data, uint32_t recv_size);

		virtual uint32_t serialize(char * const data, uint32_t max_size);
};

template<class key_t, class val_t>
class DelRequestSeq : public DelRequest<key_t, val_t> { // seq
	public:
		DelRequestSeq(const char * data, uint32_t recv_size);

		virtual uint32_t serialize(char * const data, uint32_t max_size);

		int32_t seq() const;
	protected:
		virtual uint32_t size();
		virtual void deserialize(const char * data, uint32_t recv_size);
		int32_t _seq;
};

template<class key_t, class val_t>
class PutRequestCase1 : public PutRequest<key_t, val_t> {
	public:
		PutRequestCase1(const char * data, uint32_t recv_size);

		virtual uint32_t serialize(char * const data, uint32_t max_size);
};

template<class key_t, class val_t>
class DelRequestCase1 : public PutRequest<key_t, val_t> {
	public:
		DelRequestCase1(const char * data, uint32_t recv_size);

		virtual uint32_t serialize(char * const data, uint32_t max_size);
};

template<class key_t, class val_t>
class GetResponsePOPEvictCase2 : public PutRequestPOPEvictCase2<key_t, val_t> { // seq + valid
	public:
		GetResponsePOPEvictCase2(const char * data, uint32_t recv_size);

		virtual uint32_t serialize(char * const data, uint32_t max_size);
};

template<class key_t, class val_t>
class PutRequestPOPEvictCase2 : public PutRequestPOPEvict<key_t, val_t> { // seq + valid
	public:
		PutRequestPOPEvictCase2(const char * data, uint32_t recv_size);

		virtual uint32_t serialize(char * const data, uint32_t max_size);

		bool valid() const;
	protected:
		virtual uint32_t size();
		virtual void deserialize(const char * data, uint32_t recv_size);
		bool _valid;
};

template<class key_t, class val_t>
class PutRequestLargeEvictCase2 : public PutRequestSeq<key_t, val_t> { // seq
	public:
		PutRequestLargeEvictCase2(const char * data, uint32_t recv_size);

		virtual uint32_t serialize(char * const data, uint32_t max_size);
};

template<class key_t, class val_t>
class PutRequestCase3 : public PutRequestSeq<key_t, val_t> { // seq
	public:
		PutRequestCase3(const char * data, uint32_t recv_size);

		virtual uint32_t serialize(char * const data, uint32_t max_size);
};

template<class key_t>
class DelRequestCase3 : public DelRequestSeq<key_t> { // seq
	public:
		DelRequestCase3(const char * data, uint32_t recv_size);

		virtual uint32_t serialize(char * const data, uint32_t max_size);
};

template<class key_t, class val_t>
class PutRequestLargeCase3 : public PutRequestLargeSeq<key_t, val_t> { // ophdr + vallen + seq + value (in payload)
	public:
		PutRequestLargeCase3(const char * data, uint32_t recv_size);

		virtual uint32_t serialize(char * const data, uint32_t max_size);
};

template<class key_t>
class PutResponseCase3 : public PutResponse<key_t> { // serveridx
	public: 
		PutResponseCase3(uint16_t hashidx, key_t key, bool stat);

		int16_t serveridx() const;
		virtual uint32_t serialize(char * const data, uint32_t max_size);
	protected:
		int16_t _serveridx();
		virtual uint32_t size();
		virtual void deserialize(const char * data, uint32_t recv_size);
};


template<class key_t>
class DelResponseCase3 : public DelResponse<key_t> { // serveridx
	public: 
		DelResponseCase3(uint16_t hashidx, key_t key, bool stat);

		int16_t serveridx() const;
		virtual uint32_t serialize(char * const data, uint32_t max_size);
	protected:
		int16_t _serveridx();
		virtual uint32_t size();
		virtual void deserialize(const char * data, uint32_t recv_size);
};


template<class key_t, class val_t>
class GetResponsePOPEvictSwitch : public PutRequest<key_t, val_t> {
	public:
		GetResponsePOPEvictSwitch(const char * data, uint32_t recv_size);

		virtual uint32_t serialize(char * const data, uint32_t max_size);
};

template<class key_t, class val_t>
class GetResponsePOPEvictCase2Switch : public PutRequest<key_t, val_t> {
	public:
		GetResponsePOPEvictCase2Switch(const char * data, uint32_t recv_size);

		virtual uint32_t serialize(char * const data, uint32_t max_size);
};

template<class key_t, class val_t>
class PutRequestPOPEvictSwitch : public PutRequestSeq<key_t, val_t> { // seq
	public:
		PutRequestPOPEvictSwitch(const char * data, uint32_t recv_size);

		virtual uint32_t serialize(char * const data, uint32_t max_size);
};

template<class key_t, class val_t>
class PutRequestPOPEvictCase2Switch : public PutRequestEvictCase2<key_t, val_t> { // seq + valid
	public:
		PutRequestPOPEvictCase2Switch(const char * data, uint32_t recv_size);

		virtual uint32_t serialize(char * const data, uint32_t max_size);
};

template<class key_t, class val_t>
class PutRequestLargeEvictSwitch : public PutRequestSeq<key_t, val_t> { // seq
	public:
		PutRequestLargeEvictSwitch(const char * data, uint32_t recv_size);

		virtual uint32_t serialize(char * const data, uint32_t max_size);
};

template<class key_t, class val_t>
class PutRequestLargeEvictCase2Switch : public PutRequestSeq<key_t, val_t> { // seq
	public:
		PutRequestLargeEvictCase2Switch(const char * data, uint32_t recv_size);

		virtual uint32_t serialize(char * const data, uint32_t max_size);
};

// APIs
packet_type_t get_packet_type(const char * data, uint32_t recv_size);

#endif
