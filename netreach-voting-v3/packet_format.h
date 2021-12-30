#ifndef PACKET_FORMAT_H
#define PACKET_FORMAT_H

#include <array>
#include <cstring>
#include <vector>

#include "helper.h"

enum class PacketType {GET_REQ, PUT_REQ, DEL_REQ, SCAN_REQ, 
	GET_RES, PUT_RES, DEL_RES, SCAN_RES, 
	GET_REQ_S, PUT_REQ_GS, PUT_REQ_PS, 
	DEL_REQ_S, GET_RES_S, GET_RES_NS,
	PUT_REQ_CASE1, DEL_REQ_CASE1,
	PUT_REQ_GS_CASE2, PUT_REQ_PS_CASE2,
	PUT_REQ_CASE3, DEL_REQ_CASE3};
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

		uint32_t seq() const;
		uint8_t is_assigned() const;

		virtual uint32_t serialize(char * const data, uint32_t max_size);
	protected:
		virtual uint32_t size();
		virtual void deserialize(const char * data, uint32_t recv_size);
		val_t _val;
		// Unused in server
		uint32_t _seq;
		uint8_t _is_assigned;
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
		// Unused in server
		uint32_t _seq;
		uint8_t _is_assigned;
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
class GetRequestS : public GetRequest<key_t> {
	public: 
		GetRequestS(uint8_t thread_id, key_t key);
		GetRequestS(const char * data, uint32_t recv_size);

		virtual uint32_t serialize(char * const data, uint32_t max_size);
};

// For hash-table-based eviction
/*template<class key_t, class val_t>
class PutRequestS : public PutRequest<key_t, val_t> {
	public:
		PutRequestS(uint8_t thread_id, key_t key, val_t val);
		PutRequestS(const char * data, uint32_t recv_size);

		virtual uint32_t serialize(char * const data, uint32_t max_size);
};*/

template<class key_t, class val_t>
class GetResponseS : public GetResponse<key_t, val_t> {
	public:
		GetResponseS(uint8_t thread_id, key_t key, val_t val);
		GetResponseS(const char * data, uint32_t recv_size);

		virtual void deserialize(const char * data, uint32_t recv_size);
};

template<class key_t, class val_t>
class GetResponseNS : public GetResponse<key_t, val_t> {
	public:
		GetResponseNS(uint8_t thread_id, key_t key, val_t val);
		GetResponseNS(const char * data, uint32_t recv_size);

		virtual void deserialize(const char * data, uint32_t recv_size);
};

template<class key_t, class val_t>
class PutRequestGS : public PutRequest<key_t, val_t> {
	public:
		PutRequestGS(uint8_t thread_id, key_t key, val_t val);
		PutRequestGS(const char * data, uint32_t recv_size);

		virtual uint32_t serialize(char * const data, uint32_t max_size);
};

template<class key_t, class val_t>
class PutRequestPS : public PutRequest<key_t, val_t> {
	public:
		PutRequestPS(uint8_t thread_id, key_t key, val_t val);
		PutRequestPS(const char * data, uint32_t recv_size);

		virtual uint32_t serialize(char * const data, uint32_t max_size);
};

template<class key_t>
class DelRequestS : public DelRequest<key_t> {
	public:
		DelRequestS(uint8_t thread_id, key_t key);
		DelRequestS(const char * data, uint32_t recv_size);

		virtual uint32_t serialize(char * const data, uint32_t max_size);
};

template<class key_t, class val_t>
class PutRequestCase1 : public PutRequest<key_t, val_t> {
	public:
		PutRequestCase1(uint8_t thread_id, key_t key, val_t val);
		PutRequestCase1(const char * data, uint32_t recv_size);

		virtual uint32_t serialize(char * const data, uint32_t max_size);
};

template<class key_t, class val_t>
class DelRequestCase1 : public PutRequest<key_t, val_t> {
	public:
		DelRequestCase1(uint8_t thread_id, key_t key, val_t val);
		DelRequestCase1(const char * data, uint32_t recv_size);

		virtual uint32_t serialize(char * const data, uint32_t max_size);
};

template<class key_t, class val_t>
class PutRequestGSCase2 : public PutRequest<key_t, val_t> {
	public:
		PutRequestGSCase2(uint8_t thread_id, key_t key, val_t val);
		PutRequestGSCase2(const char * data, uint32_t recv_size);

		virtual uint32_t serialize(char * const data, uint32_t max_size);
};

template<class key_t, class val_t>
class PutRequestPSCase2 : public PutRequest<key_t, val_t> {
	public:
		PutRequestPSCase2(uint8_t thread_id, key_t key, val_t val);
		PutRequestPSCase2(const char * data, uint32_t recv_size);

		virtual uint32_t serialize(char * const data, uint32_t max_size);
};

template<class key_t, class val_t>
class PutRequestCase3 : public PutRequest<key_t, val_t> {
	public:
		PutRequestCase3(uint8_t thread_id, key_t key, val_t val);
		PutRequestCase3(const char * data, uint32_t recv_size);

		virtual uint32_t serialize(char * const data, uint32_t max_size);
};

template<class key_t>
class DelRequestCase3 : public DelRequest<key_t> {
	public:
		DelRequestCase3(uint8_t thread_id, key_t key);
		DelRequestCase3(const char * data, uint32_t recv_size);

		virtual uint32_t serialize(char * const data, uint32_t max_size);
};

// APIs
packet_type_t get_packet_type(const char * data, uint32_t recv_size);

#endif
