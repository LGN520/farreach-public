#ifndef PACKET_FORMAT_H
#define PACKET_FORMAT_H

#include <array>
#include <cstring>
#include <vector>

#include "../common/helper.h"

enum class PacketType {GET_REQ, PUT_REQ, DEL_REQ, SCAN_REQ, 
	GET_RES, PUT_RES, DEL_RES, SCAN_RES, 
	GET_REQ_S, PUT_REQ_GS, PUT_REQ_N, PUT_REQ_PS, 
	DEL_REQ_S, GET_RES_S, GET_RES_NS};
typedef PacketType packet_type_t;

template<class key_t>
class Packet {
	public:
		Packet();
		Packet(packet_type_t type, uint8_t thread_id, key_t key);

		packet_type_t type() const;
		uint8_t thread_id() const;
		key_t key() const;

		virtual uint32_t serialize(char * const data, uint32_t max_size) = 0;
	protected:
		uint8_t _type;
		uint8_t _thread_id;
		key_t _key;

		virtual uint32_t size() = 0;
		virtual void deserialize(const char * data, uint32_t recv_size) = 0;
};

template<class key_t>
class GetRequest : public Packet<key_t> {
	public: 
		GetRequest();
		GetRequest(uint8_t thread_id, key_t key);
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
		PutRequest(uint8_t thread_id, key_t key, val_t val);
		PutRequest(const char * data, uint32_t recv_size);

		val_t val() const;

		virtual uint32_t serialize(char * const data, uint32_t max_size);
	protected:
		virtual uint32_t size();
		virtual void deserialize(const char * data, uint32_t recv_size);
		val_t _val;
		// Useless in server-side
		uint32_t _seq;
		uint8_t _is_assigned;
};

template<class key_t>
class DelRequest : public Packet<key_t> {
	public: 
		DelRequest();
		DelRequest(uint8_t thread_id, key_t key);
		DelRequest(const char * data, uint32_t recv_size);

		virtual uint32_t serialize(char * const data, uint32_t max_size);
	protected:
		virtual uint32_t size();
		virtual void deserialize(const char * data, uint32_t recv_size);
};

template<class key_t>
class ScanRequest : public Packet<key_t> {
	public: 
		ScanRequest(uint8_t thread_id, key_t key, uint32_t num);
		ScanRequest(const char * data, uint32_t recv_size);

		uint32_t num() const;

		virtual uint32_t serialize(char * const data, uint32_t max_size);
	protected:
		virtual uint32_t size();
		virtual void deserialize(const char * data, uint32_t recv_size);
	private:
		uint32_t _num;
};

template<class key_t, class val_t>
class GetResponse : public Packet<key_t> {
	public:
		GetResponse();
		GetResponse(uint8_t thread_id, key_t key, val_t val);
		GetResponse(const char * data, uint32_t recv_size);

		val_t val() const;

		virtual uint32_t serialize(char * const data, uint32_t max_size);
	protected:
		virtual uint32_t size();
		virtual void deserialize(const char * data, uint32_t recv_size);
	private:
		val_t _val;
		// Useless in client-side
		uint32_t _seq;
		uint8_t _is_assigned;
};

template<class key_t>
class PutResponse : public Packet<key_t> {
	public: 
		PutResponse(uint8_t thread_id, key_t key, bool stat);
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
		DelResponse(uint8_t thread_id, key_t key, bool stat);
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
		ScanResponse(uint8_t thread_id, key_t key, uint32_t num, std::vector<std::pair<key_t, val_t>> pairs);
		ScanResponse(const char * data, uint32_t recv_size);

		uint32_t num() const;
		std::vector<std::pair<key_t, val_t>> pairs() const;

		virtual uint32_t serialize(char * const data, uint32_t max_size);
	protected:
		virtual uint32_t size();
		virtual void deserialize(const char * data, uint32_t recv_size);
	private:
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
class PutRequestN : public PutRequest<key_t, val_t> {
	public:
		PutRequestN(uint8_t thread_id, key_t key, val_t val);
		PutRequestN(const char * data, uint32_t recv_size);

		virtual uint32_t serialize(char * const data, uint32_t max_size);
};

template<class key_t, class val_t>
class PutRequestPS : public PutRequest<key_t, val_t> {
	public:
		PutRequestPS(uint8_t thread_id, key_t key, val_t val, key_t evicted_key);
		PutRequestPS(const char * data, uint32_t recv_size);

		key_t evicted_key();

		virtual uint32_t serialize(char * const data, uint32_t max_size);
	protected:
		virtual uint32_t size();
		virtual void deserialize(const char * data, uint32_t recv_size);
	private:
		key_t _evicted_key;
};

template<class key_t>
class DelRequestS : public DelRequest<key_t> {
	public:
		DelRequestS(uint8_t thread_id, key_t key);
		DelRequestS(const char * data, uint32_t recv_size);

		virtual uint32_t serialize(char * const data, uint32_t max_size);
};

// APIs
packet_type_t get_packet_type(const char * data, uint32_t recv_size);

#endif
