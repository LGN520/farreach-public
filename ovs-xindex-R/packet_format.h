#ifndef PACKET_FORMAT_H
#define PACKET_FORMAT_H

#include <array>
#include <cstring>

#include "helper.h"

enum class PacketType {GET_REQ, PUT_REQ, DEL_REQ, SCAN_REQ, 
	GET_RES, PUT_RES, DEL_RES, SCAN_RES};
typedef PacketType packet_type_t;

template<class key_t>
class Packet {
	public:
		Packet();
		Packet(packet_type_t type, uint32_t thread_id, key_t key);

		packet_type_t type() const;
		uint32_t thread_id() const;
		key_t key() const;

		virtual uint32_t serialize(char * const data, uint32_t max_size) = 0;
	protected:
		packet_type_t _type;
		uint32_t _thread_id;
		key_t _key;

		virtual uint32_t size() = 0;
		virtual void deserialize(const char * data, uint32_t recv_size) = 0;
};

template<class key_t>
class GetRequest : public Packet<key_t> {
	public: 
		GetRequest(uint32_t thread_id, key_t key);
		GetRequest(const char * data, uint32_t recv_size);

		virtual uint32_t serialize(char * const data, uint32_t max_size);
	protected:
		virtual uint32_t size();
		virtual void deserialize(const char * data, uint32_t recv_size);
};

template<class key_t, class val_t>
class GetResponse : public Packet<key_t> {
	public:
		GetResponse(uint32_t thread_id, key_t key, val_t val);
		GetResponse(const char * data, uint32_t recv_size);

		val_t val() const;

		virtual uint32_t serialize(char * const data, uint32_t max_size);
	protected:
		virtual uint32_t size();
		virtual void deserialize(const char * data, uint32_t recv_size);
	private:
		val_t _val;
};

// APIs
packet_type_t get_packet_type(const char * data, uint32_t recv_size);

#endif
