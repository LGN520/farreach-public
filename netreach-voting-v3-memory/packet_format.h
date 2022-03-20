#ifndef PACKET_FORMAT_H
#define PACKET_FORMAT_H

#include <array>
#include <cstring>
#include <vector>

#include "helper.h"

// mask for other_hdr
#define VALID_MASK = 0x01

enum class PacketType {GET_REQ, PUT_REQ, DEL_REQ, SCAN_REQ, GET_RES, PUT_RES, DEL_RES, SCAN_RES, 
	GET_REQ_POP, GET_RES_POP, GET_RES_NPOP, GET_RES_POP_LARGE, GET_RES_POP_EVICT,
	PUT_REQ_SEQ, PUT_REQ_POP, PUT_REQ_RECIR, PUT_REQ_POP_EVICT, 
	PUT_REQ_LARGE, PUT_REQ_LARGE_SEQ, PUT_REQ_LARGE_RECIR, PUT_REQ_LARGE_EVICT,
	DEL_REQ_SEQ, DEL_REQ_RECIR, PUT_REQ_CASE1, DEL_REQ_CASE1, GET_RES_POP_EVICT_CASE2, PUT_REQ_POP_EVICT_CASE2, 
	PUT_REQ_LARGE_EVICT_CASE2, PUT_REQ_MAY_CASE3, PUT_REQ_CASE3, DEL_REQ_MAY_CASE3, DEL_REQ_CASE3,
	GET_RES_POP_EVICT_SWITCH, GET_RES_POP_EVICT_CASE2_SWITCH, PUT_REQ_POP_EVICT_SWITCH, PUT_REQ_POP_EVICT_CASE2_SWITCH,
	PUT_REQ_LARGE_POP_EVICT_SWITCH, PUT_REQ_LARGE_POP_EVICT_CASE2_SWITCH};
typedef PacketType packet_type_t;

template<class key_t>
class Packet {
	public:
		Packet();
		Packet(packet_type_t type, uint16_t hashidx, key_t key);

		packet_type_t type() const;
		uint16_t hashidx() const;
		key_t key() const;

		virtual uint32_t serialize(char * const data, uint32_t max_size) = 0;
	protected:
		uint8_t _type;
		uint16_t _hashidx;
		key_t _key;

		virtual uint32_t size() = 0;
		virtual void deserialize(const char * data, uint32_t recv_size) = 0;
};

template<class key_t>
class GetRequest : public Packet<key_t> {
	public: 
		GetRequest();
		GetRequest(uint16_t hashidx, key_t key);
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
		PutRequest(uint16_t hashidx, key_t key, val_t val);
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
		DelRequest(uint16_t hashidx, key_t key);
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
		GetResponse(uint16_t hashidx, key_t key, val_t val);
		GetResponse(const char * data, uint32_t recv_size);

		val_t val() const;

		virtual uint32_t serialize(char * const data, uint32_t max_size);
	protected:
		virtual uint32_t size();
		virtual void deserialize(const char * data, uint32_t recv_size);
	private:
		val_t _val;
};

template<class key_t>
class PutResponse : public Packet<key_t> {
	public: 
		PutResponse(uint16_t hashidx, key_t key, bool stat);
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
		DelResponse(uint16_t hashidx, key_t key, bool stat);
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
class PutRequestLargeSeq : public PutRequestSeq<key_t, val_t> { // seq
	public:
		PutRequestLargeSeq(const char * data, uint32_t recv_size);

		virtual uint32_t serialize(char * const data, uint32_t max_size);
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
class PutRequestCase3 : public PutRequest<key_t, val_t> {
	public:
		PutRequestCase3(const char * data, uint32_t recv_size);

		virtual uint32_t serialize(char * const data, uint32_t max_size);
};

template<class key_t>
class DelRequestCase3 : public DelRequest<key_t> {
	public:
		DelRequestCase3(const char * data, uint32_t recv_size);

		virtual uint32_t serialize(char * const data, uint32_t max_size);
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
