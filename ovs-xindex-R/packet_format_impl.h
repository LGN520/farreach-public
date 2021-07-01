#include "packet_format.h"

// Packet
template<class key_t>
Packet<key_t>::Packet() 
	: _type(packet_type_t(0)), _thread_id(0), _key(key_t(0))
{
}

template<class key_t>
Packet<key_t>::Packet(packet_type_t type, uint32_t thread_id, key_t key)
	: _type(type), _thread_id(thread_id), _key(key)
{
}

template<class key_t>
packet_type_t Packet<key_t>::type() const {
	return _type;
}

template<class key_t>
uint32_t Packet<key_t>::thread_id() const {
	return _thread_id;
}

template<class key_t>
key_t Packet<key_t>::key() const {
	return _key;
}


// GetRequest
template<class key_t>
GetRequest<key_t>::GetRequest(uint32_t thread_id, key_t key)
	: Packet<key_t>(packet_type_t::GET_REQ, thread_id, key)
{
}

template<class key_t>
GetRequest<key_t>::GetRequest(const char * data, uint32_t recv_size) {
	this->_type = packet_type_t::GET_REQ;
	this->deserialize(data, recv_size);
}

template<class key_t>
uint32_t GetRequest<key_t>::size() {
	return sizeof(packet_type_t) + sizeof(uint32_t) + sizeof(key_t);
}

template<class key_t>
uint32_t GetRequest<key_t>::serialize(char * const data, uint32_t max_size) {
	uint32_t my_size = this->size();
	INVARIANT(max_size >= my_size);
	char *begin = data;
	memcpy(begin, (void *)&this->_type, sizeof(packet_type_t));
	begin += sizeof(packet_type_t);
	memcpy(begin, (void *)&this->_thread_id, sizeof(uint32_t));
	begin += sizeof(uint32_t);
	memcpy(begin, (void *)&this->_key, sizeof(key_t));
	return my_size;
}

template<class key_t>
void GetRequest<key_t>::deserialize(const char * data, uint32_t recv_size) {
	uint32_t my_size = this->size();
	INVARIANT(my_size == recv_size);
	const char *begin = data;
	memcpy((void *)&this->_type, begin, sizeof(packet_type_t));
	begin += sizeof(packet_type_t);
	memcpy((void *)&this->_thread_id, begin, sizeof(uint32_t));
	begin += sizeof(uint32_t);
	memcpy((void *)&this->_key, begin, sizeof(key_t));
}

// GetResponse

template<class key_t, class val_t>
GetResponse<key_t, val_t>::GetResponse(uint32_t thread_id, key_t key, val_t val) 
	: Packet<key_t>(PacketType::GET_RES, thread_id, key), _val(val)
{	
}

template<class key_t, class val_t>
GetResponse<key_t, val_t>::GetResponse(const char * data, uint32_t recv_size) {
	this->_type = PacketType::GET_RES;
	this->deserialize(data, recv_size);
}

template<class key_t, class val_t>
val_t GetResponse<key_t, val_t>::val() const {
	return _val;
}

template<class key_t, class val_t>
uint32_t GetResponse<key_t, val_t>::size() {
	return sizeof(packet_type_t) + sizeof(uint32_t) + sizeof(key_t) + sizeof(val_t);
}

template<class key_t, class val_t>
uint32_t GetResponse<key_t, val_t>::serialize(char * const data, uint32_t max_size) {
	uint32_t my_size = this->size();
	INVARIANT(max_size >= my_size);
	char *begin = data;
	memcpy(begin, (void *)&this->_type, sizeof(packet_type_t));
	begin += sizeof(packet_type_t);
	memcpy(begin, (void *)&this->_thread_id, sizeof(uint32_t));
	begin += sizeof(uint32_t);
	memcpy(begin, (void *)&this->_key, sizeof(key_t));
	begin += sizeof(val_t);
	memcpy(begin, (void *)&this->_val, sizeof(val_t));
	return my_size;
}

template<class key_t, class val_t>
void GetResponse<key_t, val_t>::deserialize(const char * data, uint32_t recv_size) {
	uint32_t my_size = this->size();
	INVARIANT(my_size == recv_size);
	const char *begin = data;
	memcpy((void *)&this->_type, begin, sizeof(packet_type_t));
	begin += sizeof(packet_type_t);
	memcpy((void *)&this->_thread_id, begin, sizeof(uint32_t));
	begin += sizeof(uint32_t);
	memcpy((void *)&this->_key, begin, sizeof(key_t));
	begin += sizeof(val_t);
	memcpy((void *)&this->_val, begin, sizeof(val_t));
}

// APIs
packet_type_t get_packet_type(const char * data, uint32_t recv_size) {
	INVARIANT(recv_size >= sizeof(packet_type_t));
	packet_type_t tmp;
	memcpy((void *)&tmp, data, sizeof(packet_type_t));
	return tmp;
}
