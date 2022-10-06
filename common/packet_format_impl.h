#ifndef PACKET_FORMAT_IMPL_H
#define PACKET_FORMAT_IMPL_H

#include "packet_format.h"
#include <arpa/inet.h>

#include "key.h"
#include "val.h"

// Packet

template<class key_t>
uint32_t Packet<key_t>::get_ophdrsize(method_t methodid) {
	int result = 0;
	switch (methodid) {
		case FARREACH_ID:
			result = sizeof(optype_t) + sizeof(key_t);
			break;
		case NOCACHE_ID:
			result = sizeof(optype_t) + sizeof(key_t);
			break;
		case NETCACHE_ID:
			result = sizeof(optype_t) + sizeof(key_t);
			break;
		case DISTFARREACH_ID:
			result = sizeof(optype_t) + sizeof(switchidx_t) + sizeof(key_t);
			break;
		case DISTNOCACHE_ID:
			result = sizeof(optype_t) + sizeof(switchidx_t) + sizeof(key_t);
			break;
		case DISTCACHE_ID:
			result = sizeof(optype_t) + sizeof(switchidx_t) + sizeof(switchidx_t) + sizeof(key_t);
			break;
		default:
			printf("Invalid method id: %d\n", methodid);
			exit(-1);
	}
	return result;
}

template<class key_t>
bool Packet<key_t>::is_singleswitch(method_t methodid) {
	if (methodid == FARREACH_ID || methodid == NOCACHE_ID || methodid == NETCACHE_ID) {
		return true;
	}
	return false;
}

template<class key_t>
int Packet<key_t>::get_inswitch_prev_bytes(method_t methodid) {
	int result = 0;
	switch (methodid) {
		case FARREACH_ID:
			result = 14;
			break;
		case NOCACHE_ID:
			result = 14;
			break;
		case NETCACHE_ID:
			result = 26;
			break;
		case DISTFARREACH_ID:
			result = 14;
			break;
		case DISTNOCACHE_ID:
			result = 14;
			break;
		case DISTCACHE_ID:
			result = 26;
			break;
		default:
			printf("Invalid method id: %d\n", methodid);
			exit(-1);
	}
	return result;
}

template<class key_t>
int Packet<key_t>::get_clone_bytes(method_t methodid) {
	int result = 0;
	switch (methodid) {
		case FARREACH_ID:
			result = 4;
			break;
		case NOCACHE_ID:
			result = 4;
			break;
		case NETCACHE_ID:
			result = 8;
			break;
		case DISTFARREACH_ID:
			result = 14;
			break;
		case DISTNOCACHE_ID:
			result = 14;
			break;
		case DISTCACHE_ID:
			result = 18;
			break;
		default:
			printf("Invalid method id: %d\n", methodid);
			exit(-1);
	}
	return result;
}

template<class key_t>
int Packet<key_t>::get_split_prev_bytes(method_t methodid) {
	int result = 0;
	switch (methodid) {
		case FARREACH_ID:
			result = 3;
			break;
		case NOCACHE_ID:
			result = 3;
			break;
		case NETCACHE_ID:
			result = 3;
			break;
		case DISTFARREACH_ID:
			result = 4;
			break;
		case DISTNOCACHE_ID:
			result = 4;
			break;
		case DISTCACHE_ID:
			result = 4;
			break;
		default:
			printf("Invalid method id: %d\n", methodid);
			exit(-1);
	}
	return result;
}

template<class key_t>
int Packet<key_t>::get_stat_padding_bytes(method_t methodid) {
	int result = 0;
	switch (methodid) {
		case FARREACH_ID:
			result = 1;
			break;
		case NOCACHE_ID:
			result = 1;
			break;
		case NETCACHE_ID:
			result = 1;
			break;
		case DISTFARREACH_ID:
			result = 1;
			break;
		case DISTNOCACHE_ID:
			result = 1;
			break;
		case DISTCACHE_ID:
			result = 1;
			break;
		default:
			printf("Invalid method id: %d\n", methodid);
			exit(-1);
	}
	return result;
}

template<class key_t>
Packet<key_t>::Packet() 
	: _methodid(INVALID_ID), _type(static_cast<optype_t>(0)), _key(key_t::min()), _globalswitchidx(0), _spineswitchidx(0), _leafswitchidx(0)
{
}

template<class key_t>
Packet<key_t>::Packet(method_t methodid, packet_type_t type, key_t key)
	: _methodid(methodid), _type(static_cast<optype_t>(type)), _key(key), _globalswitchidx(0), _spineswitchidx(0), _leafswitchidx(0)
{
}

template<class key_t>
Packet<key_t>::Packet(method_t methodid, packet_type_t type, switchidx_t globalswitchidx, key_t key)
	: _methodid(methodid), _type(static_cast<optype_t>(type)), _globalswitchidx(globalswitchidx), _key(key), _spineswitchidx(0), _leafswitchidx(0)
{
}

template<class key_t>
Packet<key_t>::Packet(method_t methodid, packet_type_t type, switchidx_t spineswitchidx, switchidx_t leafswitchidx, key_t key)
	: _methodid(methodid), _type(static_cast<optype_t>(type)), _spineswitchidx(spineswitchidx), _leafswitchidx(leafswitchidx), _key(key)
{
}

template<class key_t>
packet_type_t Packet<key_t>::type() const {
	return packet_type_t(_type);
}

template<class key_t>
key_t Packet<key_t>::key() const {
	return _key;
}

template<class key_t>
switchidx_t Packet<key_t>::globalswitchidx() const {
	return _globalswitchidx;
}

template<class key_t>
switchidx_t Packet<key_t>::spineswitchidx() const {
	return _spineswitchidx;
}

template<class key_t>
switchidx_t Packet<key_t>::leafswitchidx() const {
	return _leafswitchidx;
}

template<class key_t>
uint32_t Packet<key_t>::serialize_ophdr(char * const data, uint32_t max_size) {
	INVARIANT(this->_methodid != INVALID_ID);
	uint32_t ophdr_size = Packet<key_t>::get_ophdrsize(this->_methodid);
	INVARIANT(max_size >= ophdr_size);

	char *begin = data;
	uint32_t tmp_typesize = serialize_packet_type(this->_type, begin, max_size);
	begin += tmp_typesize;
	if (!Packet<key_t>::is_singleswitch(this->_methodid)) {
		if (this->_methodid == DISTCACHE_ID) {
			uint32_t tmp_spineswitchidxsize = serialize_switchidx(this->_spineswitchidx, begin, max_size - uint32_t(begin - data));
			begin += tmp_spineswitchidxsize;
			uint32_t tmp_leafswitchidxsize = serialize_switchidx(this->_leafswitchidx, begin, max_size - uint32_t(begin - data));
			begin += tmp_leafswitchidxsize;
		}
		else {
			uint32_t tmp_switchidxsize = serialize_switchidx(this->_globalswitchidx, begin, max_size - uint32_t(begin - data));
			begin += tmp_switchidxsize;
		}
	}
	uint32_t tmp_keysize = this->_key.serialize(begin, max_size - uint32_t(begin - data));
	begin += tmp_keysize;
	return uint32_t(begin - data);
}

template<class key_t>
uint32_t Packet<key_t>::dynamic_serialize_ophdr(dynamic_array_t &dynamic_data) {
	INVARIANT(this->_methodid != INVALID_ID);
	int tmpoff = 0;
	uint32_t tmp_typesize = dynamic_serialize_packet_type(this->_type, dynamic_data, tmpoff);
	tmpoff += tmp_typesize;
	if (!Packet<key_t>::is_singleswitch(this->_methodid)) {
		if (this->_methodid == DISTCACHE_ID) {
			uint32_t tmp_spineswitchidxsize = dynamic_serialize_switchidx(this->_spineswitchidx, dynamic_data, tmpoff);
			tmpoff += tmp_spineswitchidxsize;
			uint32_t tmp_leafswitchidxsize = dynamic_serialize_switchidx(this->_leafswitchidx, dynamic_data, tmpoff);
			tmpoff += tmp_leafswitchidxsize;
		}
		else {
			uint32_t tmp_switchidxsize = dynamic_serialize_switchidx(this->_globalswitchidx, dynamic_data, tmpoff);
			tmpoff += tmp_switchidxsize;
		}
	}
	uint32_t tmp_keysize = this->_key.dynamic_serialize(dynamic_data, tmpoff);
	tmpoff += tmp_keysize;
	return tmpoff;
}

template<class key_t>
uint32_t Packet<key_t>::deserialize_ophdr(const char * data, uint32_t recv_size) {
	INVARIANT(this->_methodid != INVALID_ID);
	uint32_t ophdr_size = Packet<key_t>::get_ophdrsize(this->_methodid);
	INVARIANT(recv_size >= ophdr_size);

	const char *begin = data;
	uint32_t tmp_typesize = deserialize_packet_type(this->_type, begin, recv_size);
	begin += tmp_typesize;
	if (!Packet<key_t>::is_singleswitch(this->_methodid)) {
		if (this->_methodid == DISTCACHE_ID) {
			uint32_t tmp_spineswitchidxsize = deserialize_switchidx(this->_spineswitchidx, begin, recv_size - uint32_t(begin - data));
			begin += tmp_spineswitchidxsize;
			uint32_t tmp_leafswitchidxsize = deserialize_switchidx(this->_leafswitchidx, begin, recv_size - uint32_t(begin - data));
			begin += tmp_leafswitchidxsize;
		}
		else {
			uint32_t tmp_switchidxsize = deserialize_switchidx(this->_globalswitchidx, begin, recv_size - uint32_t(begin - data));
			begin += tmp_switchidxsize;
		}
	}
	uint32_t tmp_keysize = this->_key.deserialize(begin, recv_size - uint32_t(begin - data));
	begin += tmp_keysize;
	return uint32_t(begin - data);
}

// GetRequest

template<class key_t>
GetRequest<key_t>::GetRequest()
	: Packet<key_t>(), _spineload(0), _leafload(0)
{
}

template<class key_t>
GetRequest<key_t>::GetRequest(method_t methodid, key_t key)
	: Packet<key_t>(methodid, packet_type_t::GETREQ, key), _spineload(0), _leafload(0)
{
	INVARIANT(methodid != DISTCACHE_ID);
}

template<class key_t>
GetRequest<key_t>::GetRequest(method_t methodid, switchidx_t spineswitchidx, switchidx_t leafswitchidx, key_t key)
	: Packet<key_t>(methodid, packet_type_t::GETREQ, spineswitchidx, leafswitchidx, key), _spineload(0), _leafload(0)
{
	INVARIANT(methodid == DISTCACHE_ID);
}

template<class key_t>
GetRequest<key_t>::GetRequest(method_t methodid, const char * data, uint32_t recv_size) {
	this->_methodid = methodid;
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == packet_type_t::GETREQ);
}

template<class key_t>
uint32_t GetRequest<key_t>::spineload() const {
	return _spineload;
}

template<class key_t>
uint32_t GetRequest<key_t>::leafload() const {
	return _leafload;
}

template<class key_t>
uint32_t GetRequest<key_t>::size() {
	uint32_t size = Packet<key_t>::get_ophdrsize(this->_methodid);
	if (this->_methodid == DISTCACHE_ID) {
		size += (sizeof(optype_t) + sizeof(uint32_t) + sizeof(uint32_t));
	}
	return size;
}

template<class key_t>
uint32_t GetRequest<key_t>::serialize(char * const data, uint32_t max_size) {
	uint32_t my_size = this->size();
	INVARIANT(max_size >= my_size);
	char *begin = data;
	uint32_t tmp_ophdrsize = this->serialize_ophdr(begin, max_size);
	begin += tmp_ophdrsize;
	if (this->_methodid == DISTCACHE_ID) {
		uint32_t tmp_shadowtypesize = serialize_packet_type(this->_type, begin, max_size - uint32_t(begin - data)); // shadowtype
		begin += tmp_shadowtypesize;
		uint32_t bigendian_spineload = htonl(this->_spineload);
		memcpy(begin, (void *)&bigendian_spineload, sizeof(uint32_t));
		begin += sizeof(uint32_t);
		uint32_t bigendian_leafload = htonl(this->_leafload);
		memcpy(begin, (void *)&bigendian_leafload, sizeof(uint32_t));
		begin += sizeof(uint32_t);
	}
	return uint32_t(begin - data);
}

template<class key_t>
void GetRequest<key_t>::deserialize(const char * data, uint32_t recv_size) {
	uint32_t my_size = this->size();
	INVARIANT(my_size <= recv_size);
	const char *begin = data;
	uint32_t tmp_ophdrsize = this->deserialize_ophdr(begin, recv_size);
	begin += tmp_ophdrsize;
	if (this->_methodid == DISTCACHE_ID) {
		begin += sizeof(optype_t); // shadowtype
		memcpy(&this->_spineload, begin, sizeof(uint32_t));
		this->_spineload = ntohl(this->_spineload);
		begin += sizeof(uint32_t);
		memcpy(&this->_leafload, begin, sizeof(uint32_t));
		this->_leafload = ntohl(this->_leafload);
		begin += sizeof(uint32_t);
	}
}

// PutRequest (value must <= 128B)

template<class key_t, class val_t>
PutRequest<key_t, val_t>::PutRequest()
	: Packet<key_t>(), _val()
{
}

template<class key_t, class val_t>
PutRequest<key_t, val_t>::PutRequest(method_t methodid, key_t key, val_t val) 
	: Packet<key_t>(methodid, PacketType::PUTREQ, key), _val(val)
{	
	INVARIANT(this->_val.val_length <= val_t::SWITCH_MAX_VALLEN);
}

template<class key_t, class val_t>
PutRequest<key_t, val_t>::PutRequest(method_t methodid, const char * data, uint32_t recv_size) {
	this->_methodid = methodid;
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::PUTREQ);
	INVARIANT(this->_val.val_length <= val_t::SWITCH_MAX_VALLEN);
}

template<class key_t, class val_t>
val_t PutRequest<key_t, val_t>::val() const {
	return _val;
}

template<class key_t, class val_t>
uint32_t PutRequest<key_t, val_t>::size() { // not used
	return Packet<key_t>::get_ophdrsize(this->_methodid) + sizeof(uint16_t) + val_t::SWITCH_MAX_VALLEN + sizeof(optype_t);
}

template<class key_t, class val_t>
uint32_t PutRequest<key_t, val_t>::serialize(char * const data, uint32_t max_size) {
	//uint32_t my_size = this->size();
	//INVARIANT(max_size >= my_size);
	char *begin = data;
	uint32_t tmp_ophdrsize = this->serialize_ophdr(begin, max_size);
	begin += tmp_ophdrsize;
	uint32_t tmp_valsize = this->_val.serialize(begin, max_size - uint32_t(begin - data));
	begin += tmp_valsize;
	uint32_t tmp_shadowtypesize = serialize_packet_type(this->_type, begin, max_size - uint32_t(begin - data)); // shadowtype
	begin += tmp_shadowtypesize;
	return uint32_t(begin - data);
}

template<class key_t, class val_t>
void PutRequest<key_t, val_t>::deserialize(const char * data, uint32_t recv_size) {
	//uint32_t my_size = this->size();
	//INVARIANT(my_size == recv_size);
	const char *begin = data;
	uint32_t tmp_ophdrsize = this->deserialize_ophdr(begin, recv_size);
	begin += tmp_ophdrsize;
	uint32_t tmp_valsize = this->_val.deserialize(begin, recv_size - uint32_t(begin - data));
	begin += tmp_valsize;
	begin += sizeof(optype_t); // deserialize shadowtype
}

// DelRequest

template<class key_t>
DelRequest<key_t>::DelRequest()
	: Packet<key_t>()
{
}

template<class key_t>
DelRequest<key_t>::DelRequest(method_t methodid, key_t key)
	: Packet<key_t>(methodid, packet_type_t::DELREQ, key)
{
}

template<class key_t>
DelRequest<key_t>::DelRequest(method_t methodid, const char * data, uint32_t recv_size) {
	this->_methodid = methodid;
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == packet_type_t::DELREQ);
}

template<class key_t>
uint32_t DelRequest<key_t>::size() {
	return Packet<key_t>::get_ophdrsize(this->_methodid);
}

template<class key_t>
uint32_t DelRequest<key_t>::serialize(char * const data, uint32_t max_size) {
	uint32_t my_size = this->size();
	INVARIANT(max_size >= my_size);
	char *begin = data;
	uint32_t tmp_ophdrsize = this->serialize_ophdr(begin, max_size);
	begin += tmp_ophdrsize;
	return uint32_t(begin - data);
}

template<class key_t>
void DelRequest<key_t>::deserialize(const char * data, uint32_t recv_size) {
	uint32_t my_size = this->size();
	INVARIANT(my_size == recv_size);
	const char *begin = data;
	uint32_t tmp_ophdrsize = this->deserialize_ophdr(begin, recv_size);
	begin += tmp_ophdrsize;
}

// ScanRequest

template<class key_t>
ScanRequest<key_t>::ScanRequest()
	: Packet<key_t>(), _endkey(key_t::min())
{
}

/*template<class key_t>
ScanRequest<key_t>::ScanRequest(key_t key, key_t endkey, uint32_t num)
	: Packet<key_t>(packet_type_t::SCAN_REQ, hashidx, key), _endkey(endkey), _num(num)
{
}*/
template<class key_t>
ScanRequest<key_t>::ScanRequest(method_t methodid, key_t key, key_t endkey)
	: Packet<key_t>(methodid, packet_type_t::SCANREQ, key), _endkey(endkey)
{
}

template<class key_t>
ScanRequest<key_t>::ScanRequest(method_t methodid, const char * data, uint32_t recv_size) {
	this->_methodid = methodid;
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == packet_type_t::SCANREQ);
}

template<class key_t>
key_t ScanRequest<key_t>::endkey() const {
	return this->_endkey;
}

/*template<class key_t>
uint32_t ScanRequest<key_t>::num() const {
	return this->_num;
}*/

template<class key_t>
uint32_t ScanRequest<key_t>::size() {
	return Packet<key_t>::get_ophdrsize(this->_methodid) + sizeof(key_t);// + sizeof(uint32_t);
}

template<class key_t>
uint32_t ScanRequest<key_t>::serialize(char * const data, uint32_t max_size) {
	uint32_t my_size = this->size();
	INVARIANT(max_size >= my_size);
	char *begin = data;
	uint32_t tmp_ophdrsize = this->serialize_ophdr(begin, max_size);
	begin += tmp_ophdrsize;
	uint32_t tmp_endkeysize = this->_endkey.serialize(begin, max_size - uint32_t(begin - data));
	begin += tmp_endkeysize;
	//memcpy(begin, (void *)&this->_num, sizeof(uint32_t));
	return uint32_t(begin - data); // + sizeof(uint32_t);
}

template<class key_t>
void ScanRequest<key_t>::deserialize(const char * data, uint32_t recv_size) {
	uint32_t my_size = this->size();
	INVARIANT(my_size == recv_size);
	const char *begin = data;
	uint32_t tmp_ophdrsize = this->deserialize_ophdr(begin, recv_size);
	begin += tmp_ophdrsize;
	uint32_t tmp_endkeysize = this->_endkey.deserialize(begin, recv_size - uint32_t(begin - data));
	begin += tmp_endkeysize;
	//memcpy((void *)&this->_num, begin, sizeof(uint32_t));
}


// GetResponse (value must <= 128B)

template<class key_t, class val_t>
GetResponse<key_t, val_t>::GetResponse()
	: Packet<key_t>(), _val(), _stat(false), _nodeidx_foreval(0), _spineload(0), _leafload(0)
{
}

template<class key_t, class val_t>
GetResponse<key_t, val_t>::GetResponse(method_t methodid, key_t key, val_t val, bool stat, uint16_t nodeidx_foreval) 
	: Packet<key_t>(methodid, PacketType::GETRES, key), _val(val), _stat(stat), _nodeidx_foreval(nodeidx_foreval), _spineload(0), _leafload(0)
{	
	INVARIANT(this->_val.val_length <= val_t::SWITCH_MAX_VALLEN);
}

template<class key_t, class val_t>
GetResponse<key_t, val_t>::GetResponse(method_t methodid, const char * data, uint32_t recv_size) {
	this->_methodid = methodid;
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::GETRES);
	INVARIANT(this->_val.val_length <= val_t::SWITCH_MAX_VALLEN);
}

template<class key_t, class val_t>
val_t GetResponse<key_t, val_t>::val() const {
	return _val;
}

template<class key_t, class val_t>
bool GetResponse<key_t, val_t>::stat() const {
	return _stat;
}

template<class key_t, class val_t>
uint16_t GetResponse<key_t, val_t>::nodeidx_foreval() const {
	return _nodeidx_foreval;
}

template<class key_t, class val_t>
uint32_t GetResponse<key_t, val_t>::spineload() const {
	return _spineload;
}

template<class key_t, class val_t>
uint32_t GetResponse<key_t, val_t>::leafload() const {
	return _leafload;
}

template<class key_t, class val_t>
uint32_t GetResponse<key_t, val_t>::size() { // unused
	uint32_t size = Packet<key_t>::get_ophdrsize(this->_methodid) + sizeof(uint16_t) + val_t::SWITCH_MAX_VALLEN + sizeof(optype_t) + sizeof(bool) + sizeof(uint16_t) + Packet<key_t>::get_stat_padding_bytes(this->_methodid);
	if (this->_methodid == DISTCACHE_ID) {
		size += (sizeof(uint32_t) + sizeof(uint32_t));
	}
	return size;
}

template<class key_t, class val_t>
uint32_t GetResponse<key_t, val_t>::serialize(char * const data, uint32_t max_size) {
	//uint32_t my_size = this->size();
	//INVARIANT(max_size >= my_size);
	char *begin = data;
	uint32_t tmp_ophdrsize = this->serialize_ophdr(begin, max_size);
	begin += tmp_ophdrsize;
	uint32_t tmp_valsize = this->_val.serialize(begin, max_size - uint32_t(begin - data));
	begin += tmp_valsize;
	uint32_t tmp_shadowtypesize = serialize_packet_type(this->_type, begin, max_size - uint32_t(begin - data)); // shadowtype
	begin += tmp_shadowtypesize;
	memcpy(begin, (void *)&this->_stat, sizeof(bool));
	begin += sizeof(bool);
	uint16_t bigendian_nodeidx_foreval = htons(this->_nodeidx_foreval);
	memcpy(begin, (void *)&bigendian_nodeidx_foreval, sizeof(uint16_t));
	begin += sizeof(uint16_t);
	begin += Packet<key_t>::get_stat_padding_bytes(this->_methodid);
	if (this->_methodid == DISTCACHE_ID) {
		uint32_t bigendian_spineload = htonl(this->_spineload);
		memcpy(begin, (void *)&bigendian_spineload, sizeof(uint32_t));
		begin += sizeof(uint32_t);
		uint32_t bigendian_leafload = htonl(this->_leafload);
		memcpy(begin, (void *)&bigendian_leafload, sizeof(uint32_t));
		begin += sizeof(uint32_t);
	}
	return uint32_t(begin - data);
}

template<class key_t, class val_t>
void GetResponse<key_t, val_t>::deserialize(const char * data, uint32_t recv_size) {
	//uint32_t my_size = this->size();
	//INVARIANT(my_size == recv_size);
	const char *begin = data;
	uint32_t tmp_ophdrsize = this->deserialize_ophdr(begin, recv_size);
	begin += tmp_ophdrsize;
	uint32_t tmp_valsize = this->_val.deserialize(begin, recv_size - uint32_t(begin - data));
	begin += tmp_valsize;
	begin += sizeof(optype_t); // deserialize shadowtype
	memcpy((void *)&this->_stat, begin, sizeof(bool));
	begin += sizeof(bool);
	memcpy(&this->_nodeidx_foreval, begin, sizeof(uint16_t));
	this->_nodeidx_foreval = ntohs(this->_nodeidx_foreval);
	begin += sizeof(uint16_t);
	begin += Packet<key_t>::get_stat_padding_bytes(this->_methodid);
	if (this->_methodid == DISTCACHE_ID) {
		memcpy(&this->_spineload, begin, sizeof(uint32_t));
		this->_spineload = ntohl(this->_spineload);
		begin += sizeof(uint32_t);
		memcpy(&this->_leafload, begin, sizeof(uint32_t));
		this->_leafload = ntohl(this->_leafload);
		begin += sizeof(uint32_t);
	}
}

// GetResponseServer (value must <= 128B)

template<class key_t, class val_t>
GetResponseServer<key_t, val_t>::GetResponseServer(method_t methodid, key_t key, val_t val, bool stat, uint16_t nodeidx_foreval) 
	: GetResponse<key_t, val_t>(methodid, key, val, stat, nodeidx_foreval)
{	
	this->_type = optype_t(packet_type_t::GETRES_SERVER);
	INVARIANT(this->_methodid == DISTFARREACH_ID || this->_methodid == DISTNOCACHE_ID);
}

template<class key_t, class val_t>
GetResponseServer<key_t, val_t>::GetResponseServer(method_t methodid, switchidx_t spineswitchidx, switchidx_t leafswitchidx, key_t key, val_t val, bool stat, uint16_t nodeidx_foreval, uint32_t spineload, uint32_t leafload) 
	: GetResponse<key_t, val_t>(methodid, key, val, stat, nodeidx_foreval)
{	
	this->_type = optype_t(packet_type_t::GETRES_SERVER);
	this->_spineswitchidx = spineswitchidx;
	this->_leafswitchidx = leafswitchidx;
	this->_spineload = spineload;
	this->_leafload = leafload;
	INVARIANT(this->_methodid == DISTCACHE_ID);
}

// PutResponse

template<class key_t>
PutResponse<key_t>::PutResponse(method_t methodid, key_t key, bool stat, uint16_t nodeidx_foreval) 
	: Packet<key_t>(methodid, PacketType::PUTRES, key), _stat(stat), _nodeidx_foreval(nodeidx_foreval)
{	
}

template<class key_t>
PutResponse<key_t>::PutResponse(method_t methodid, const char * data, uint32_t recv_size) {
	this->_methodid = methodid;
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::PUTRES);
}

template<class key_t>
bool PutResponse<key_t>::stat() const {
	return _stat;
}

template<class key_t>
uint16_t PutResponse<key_t>::nodeidx_foreval() const {
	return _nodeidx_foreval;
}

template<class key_t>
uint32_t PutResponse<key_t>::size() {
	return Packet<key_t>::get_ophdrsize(this->_methodid) + sizeof(optype_t) + sizeof(bool) + sizeof(uint16_t) + Packet<key_t>::get_stat_padding_bytes(this->_methodid);
}

template<class key_t>
uint32_t PutResponse<key_t>::serialize(char * const data, uint32_t max_size) {
	uint32_t my_size = this->size();
	INVARIANT(max_size >= my_size);
	char *begin = data;
	uint32_t tmp_ophdrsize = this->serialize_ophdr(begin, max_size);
	begin += tmp_ophdrsize;
	uint32_t tmp_shadowtypesize = serialize_packet_type(this->_type, begin, max_size - uint32_t(begin - data)); // shadowtype
	begin += tmp_shadowtypesize;
	memcpy(begin, (void *)&this->_stat, sizeof(bool));
	begin += sizeof(bool);
	uint16_t bigendian_nodeidx_foreval = htons(this->_nodeidx_foreval);
	memcpy(begin, (void *)&bigendian_nodeidx_foreval, sizeof(uint16_t));
	begin += sizeof(uint16_t);
	begin += Packet<key_t>::get_stat_padding_bytes(this->_methodid);
	return uint32_t(begin - data);
}

template<class key_t>
void PutResponse<key_t>::deserialize(const char * data, uint32_t recv_size) {
	uint32_t my_size = this->size();
	INVARIANT(my_size <= recv_size);
	const char *begin = data;
	uint32_t tmp_ophdrsize = this->deserialize_ophdr(begin, recv_size);
	begin += tmp_ophdrsize;
	begin += sizeof(optype_t); // deserialize shadowtype
	memcpy((void *)&this->_stat, begin, sizeof(bool));
	begin += sizeof(bool);
	memcpy(&this->_nodeidx_foreval, begin, sizeof(uint16_t));
	this->_nodeidx_foreval = ntohs(this->_nodeidx_foreval);
	begin += sizeof(uint16_t);
	begin += Packet<key_t>::get_stat_padding_bytes(this->_methodid);
}

// PutResponseServer

template<class key_t>
PutResponseServer<key_t>::PutResponseServer(method_t methodid, key_t key, bool stat, uint16_t nodeidx_foreval) 
	: PutResponse<key_t>(methodid, key, stat, nodeidx_foreval)
{	
	this->_type = optype_t(packet_type_t::PUTRES_SERVER);
	INVARIANT(!Packet<key_t>::is_singleswitch(methodid));
}

// DelResponse

template<class key_t>
DelResponse<key_t>::DelResponse(method_t methodid, key_t key, bool stat, uint16_t nodeidx_foreval) 
	: Packet<key_t>(methodid, PacketType::DELRES, key), _stat(stat), _nodeidx_foreval(nodeidx_foreval)
{	
}

template<class key_t>
DelResponse<key_t>::DelResponse(method_t methodid, const char * data, uint32_t recv_size) {
	this->_methodid = methodid;
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::DELRES);
}

template<class key_t>
bool DelResponse<key_t>::stat() const {
	return _stat;
}

template<class key_t>
uint16_t DelResponse<key_t>::nodeidx_foreval() const {
	return _nodeidx_foreval;
}

template<class key_t>
uint32_t DelResponse<key_t>::size() {
	return Packet<key_t>::get_ophdrsize(this->_methodid) + sizeof(optype_t) + sizeof(bool) + sizeof(uint16_t) + Packet<key_t>::get_stat_padding_bytes(this->_methodid);
}

template<class key_t>
uint32_t DelResponse<key_t>::serialize(char * const data, uint32_t max_size) {
	uint32_t my_size = this->size();
	INVARIANT(max_size >= my_size);
	char *begin = data;
	uint32_t tmp_ophdrsize = this->serialize_ophdr(begin, max_size);
	begin += tmp_ophdrsize;
	uint32_t tmp_shadowtypesize = serialize_packet_type(this->_type, begin, max_size - uint32_t(begin - data)); // shadowtype
	begin += tmp_shadowtypesize;
	memcpy(begin, (void *)&this->_stat, sizeof(bool));
	begin += sizeof(bool);
	uint16_t bigendian_nodeidx_foreval = htons(this->_nodeidx_foreval);
	memcpy(begin, (void *)&bigendian_nodeidx_foreval, sizeof(uint16_t));
	begin += sizeof(uint16_t);
	begin += Packet<key_t>::get_stat_padding_bytes(this->_methodid);
	return uint32_t(begin - data);
}

template<class key_t>
void DelResponse<key_t>::deserialize(const char * data, uint32_t recv_size) {
	uint32_t my_size = this->size();
	INVARIANT(my_size <= recv_size);
	const char *begin = data;
	uint32_t tmp_ophdrsize = this->deserialize_ophdr(begin, recv_size);
	begin += tmp_ophdrsize;
	begin += sizeof(optype_t); // deserialize shadowtype
	memcpy((void *)&this->_stat, begin, sizeof(bool));
	begin += sizeof(bool);
	memcpy(&this->_nodeidx_foreval, begin, sizeof(uint16_t));
	this->_nodeidx_foreval = ntohs(this->_nodeidx_foreval);
	begin += sizeof(uint16_t);
	begin += Packet<key_t>::get_stat_padding_bytes(this->_methodid);
}

// DelResponseServer

template<class key_t>
DelResponseServer<key_t>::DelResponseServer(method_t methodid, key_t key, bool stat, uint16_t nodeidx_foreval) 
	: DelResponse<key_t>(methodid, key, stat, nodeidx_foreval)
{	
	this->_type = optype_t(packet_type_t::DELRES_SERVER);
	INVARIANT(!Packet<key_t>::is_singleswitch(methodid));
}

// ScanResponseSplit

template<class key_t, class val_t>
ScanResponseSplit<key_t, val_t>::ScanResponseSplit(method_t methodid, key_t key, key_t endkey, uint16_t cur_scanidx, uint16_t max_scannum, uint16_t nodeidx_foreval, int snapshotid, int32_t pairnum, std::vector<std::pair<key_t, snapshot_record_t>> pairs) 
	: ScanRequestSplit<key_t>(methodid, key, endkey, cur_scanidx, max_scannum), _nodeidx_foreval(nodeidx_foreval), _snapshotid(snapshotid), _pairnum(pairnum)
{	
	this->_type = static_cast<optype_t>(PacketType::SCANRES_SPLIT);
	INVARIANT(snapshotid >= 0);
	INVARIANT(pairnum == int32_t(pairs.size()));
	this->_pairs.assign(pairs.begin(), pairs.end());
}

template<class key_t, class val_t>
ScanResponseSplit<key_t, val_t>::ScanResponseSplit(method_t methodid, key_t key, key_t endkey, uint16_t cur_scanidx, uint16_t max_scannum, uint16_t cur_scanswitchidx, uint16_t max_scanswitchnum, uint16_t nodeidx_foreval, int snapshotid, int32_t pairnum, std::vector<std::pair<key_t, snapshot_record_t>> pairs) 
	: ScanRequestSplit<key_t>(methodid, key, endkey, cur_scanidx, max_scannum, cur_scanswitchidx, max_scanswitchnum), _nodeidx_foreval(nodeidx_foreval), _snapshotid(snapshotid), _pairnum(pairnum)
{	
	this->_type = static_cast<optype_t>(PacketType::SCANRES_SPLIT);
	INVARIANT(snapshotid >= 0);
	INVARIANT(pairnum == int32_t(pairs.size()));
	this->_pairs.assign(pairs.begin(), pairs.end());
}

template<class key_t, class val_t>
ScanResponseSplit<key_t, val_t>::ScanResponseSplit(method_t methodid, const char * data, uint32_t recv_size) {
	this->_methodid = methodid;
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::SCANRES_SPLIT);
	INVARIANT(this->_snapshotid >= 0);
}

template<class key_t, class val_t>
uint16_t ScanResponseSplit<key_t, val_t>::nodeidx_foreval() const {
	return this->_nodeidx_foreval;
}

template<class key_t, class val_t>
int ScanResponseSplit<key_t, val_t>::snapshotid() const {
	return this->_snapshotid;
}

template<class key_t, class val_t>
int32_t ScanResponseSplit<key_t, val_t>::pairnum() const {
	return this->_pairnum;
}

template<class key_t, class val_t>
std::vector<std::pair<key_t, snapshot_record_t>> ScanResponseSplit<key_t, val_t>::pairs() const {
	return this->_pairs;
}

template<class key_t, class val_t>
uint32_t ScanResponseSplit<key_t, val_t>::size() {
	uint32_t size = Packet<key_t>::get_ophdrsize(this->_methodid) + sizeof(key_t) + Packet<key_t>::get_split_prev_bytes(this->_methodid) + sizeof(uint16_t) + sizeof(uint16_t) + sizeof(uint16_t) + sizeof(int) + sizeof(int32_t); // ophdr + scanhdr.endkey + splithdr (isclone + globalserveridx + cur_scanidx, max_scannum, [cur_scanswitchidx, max_scanswitchnum]) + nodeidx_foreval + snapshotid + pairnum
	if (!Packet<key_t>::is_singleswitch(this->_methodid)) {
		size += (sizeof(uint16_t) + sizeof(uint16_t)); // cur_scanswitchidx + max_scanswitchnum
	}
	return size;
}

template<class key_t, class val_t>
uint32_t ScanResponseSplit<key_t, val_t>::serialize(char * const data, uint32_t max_size) {
	uint32_t my_size = this->size();
	INVARIANT(max_size >= my_size);
	char *begin = data;
	uint32_t tmp_ophdrsize = this->serialize_ophdr(begin, max_size);
	begin += tmp_ophdrsize;
	uint32_t tmp_endkeysize = this->_endkey.serialize(begin, max_size - uint32_t(begin - data));
	begin += tmp_endkeysize;
	//memcpy(begin, (void *)&this->_num, sizeof(uint32_t));
	//begin += sizeof(uint32_t);
	int tmp_split_prev_bytes = Packet<key_t>::get_split_prev_bytes(this->_methodid);
	memset(begin, 0, tmp_split_prev_bytes);
	begin += tmp_split_prev_bytes;
	uint16_t bigendian_cur_scanidx = htons(uint16_t(this->_cur_scanidx));
	memcpy(begin, (void *)&bigendian_cur_scanidx, sizeof(uint16_t));
	begin += sizeof(uint16_t);
	uint16_t bigendian_max_scannum = htons(uint16_t(this->_max_scannum));
	memcpy(begin, (void *)&bigendian_max_scannum, sizeof(uint16_t));
	begin += sizeof(uint16_t);
	if (!Packet<key_t>::is_singleswitch(this->_methodid)) {
		uint16_t bigendian_cur_scanswitchidx = htons(uint16_t(this->_cur_scanswitchidx));
		memcpy(begin, (void *)&bigendian_cur_scanswitchidx, sizeof(uint16_t));
		begin += sizeof(uint16_t);
		uint16_t bigendian_max_scanswitchnum = htons(uint16_t(this->_max_scanswitchnum));
		memcpy(begin, (void *)&bigendian_max_scanswitchnum, sizeof(uint16_t));
		begin += sizeof(uint16_t);
	}
	uint16_t bigendian_nodeidx_foreval = htons(this->_nodeidx_foreval);
	memcpy(begin, (void *)&bigendian_nodeidx_foreval, sizeof(uint16_t));
	begin += sizeof(uint16_t);
	memcpy(begin, (void *)&this->_snapshotid, sizeof(int)); // directly use little-endian
	begin += sizeof(int);

	uint32_t bigendian_pairnum = htonl(uint32_t(this->_pairnum));
	memcpy(begin, (void *)&bigendian_pairnum, sizeof(int32_t));
	begin += sizeof(int32_t);
	uint32_t totalsize = uint32_t(begin - data);
	for (uint32_t pair_i = 0; pair_i < this->_pairs.size(); pair_i++) {
		uint32_t tmp_pair_keysize = this->_pairs[pair_i].first.serialize(begin, max_size - totalsize);
		begin += tmp_pair_keysize;
		totalsize += tmp_pair_keysize;
		uint32_t tmp_pair_valsize = this->_pairs[pair_i].second.val.serialize_large(begin, max_size - totalsize);
		begin += tmp_pair_valsize;
		totalsize += tmp_pair_valsize;
	}
	return totalsize;
}

template<class key_t, class val_t>
uint32_t ScanResponseSplit<key_t, val_t>::dynamic_serialize(dynamic_array_t &dynamic_data) {
	//uint32_t my_size = this->size();
	//INVARIANT(max_size >= my_size);
	int tmpoff = 0;
	uint32_t tmp_ophdrsize = this->dynamic_serialize_ophdr(dynamic_data);
	tmpoff += tmp_ophdrsize;
	uint32_t tmp_endkeysize = this->_endkey.dynamic_serialize(dynamic_data, tmpoff);
	tmpoff += tmp_endkeysize;
	//dynamic_data.dynamic_memcpy(tmpoff, (char *)&this->_num, sizeof(uint32_t));
	//tmpoff += sizeof(uint32_t);
	int tmp_split_prev_bytes = Packet<key_t>::get_split_prev_bytes(this->_methodid);
	dynamic_data.dynamic_memset(tmpoff, 0, tmp_split_prev_bytes);
	tmpoff += tmp_split_prev_bytes;
	uint16_t bigendian_cur_scanidx = htons(uint16_t(this->_cur_scanidx));
	dynamic_data.dynamic_memcpy(tmpoff, (char *)&bigendian_cur_scanidx, sizeof(uint16_t));
	tmpoff += sizeof(uint16_t);
	uint16_t bigendian_max_scannum = htons(uint16_t(this->_max_scannum));
	dynamic_data.dynamic_memcpy(tmpoff, (char *)&bigendian_max_scannum, sizeof(uint16_t));
	tmpoff += sizeof(uint16_t);
	if (!Packet<key_t>::is_singleswitch(this->_methodid)) {
		uint16_t bigendian_cur_scanswitchidx = htons(uint16_t(this->_cur_scanswitchidx));
		dynamic_data.dynamic_memcpy(tmpoff, (char *)&bigendian_cur_scanswitchidx, sizeof(uint16_t));
		tmpoff += sizeof(uint16_t);
		uint16_t bigendian_max_scanswitchnum = htons(uint16_t(this->_max_scanswitchnum));
		dynamic_data.dynamic_memcpy(tmpoff, (char *)&bigendian_max_scanswitchnum, sizeof(uint16_t));
		tmpoff += sizeof(uint16_t);
	}
	uint16_t bigendian_nodeidx_foreval = htons(this->_nodeidx_foreval);
	dynamic_data.dynamic_memcpy(tmpoff, (char *)&bigendian_nodeidx_foreval, sizeof(uint16_t));
	tmpoff += sizeof(uint16_t);
	dynamic_data.dynamic_memcpy(tmpoff, (char *)&this->_snapshotid, sizeof(int)); // directly use little-endian
	tmpoff += sizeof(int);

	uint32_t bigendian_pairnum = htonl(uint32_t(this->_pairnum));
	dynamic_data.dynamic_memcpy(tmpoff, (char *)&bigendian_pairnum, sizeof(int32_t));
	tmpoff += sizeof(int32_t);
	uint32_t totalsize = tmpoff;
	for (uint32_t pair_i = 0; pair_i < this->_pairs.size(); pair_i++) {
		uint32_t tmp_pair_keysize = this->_pairs[pair_i].first.dynamic_serialize(dynamic_data, tmpoff);
		tmpoff += tmp_pair_keysize;
		totalsize += tmp_pair_keysize;
		uint32_t tmp_pair_valsize = this->_pairs[pair_i].second.val.dynamic_serialize_large(dynamic_data, tmpoff);
		tmpoff += tmp_pair_valsize;
		totalsize += tmp_pair_valsize;
	}
	return totalsize;
}

template<class key_t, class val_t>
void ScanResponseSplit<key_t, val_t>::deserialize(const char * data, uint32_t recv_size) {
	uint32_t my_size = this->size();
	INVARIANT(recv_size >= my_size);
	const char *begin = data;
	uint32_t tmp_ophdrsize = this->deserialize_ophdr(begin, recv_size);
	begin += tmp_ophdrsize;
	uint32_t tmp_endkeysize = this->_endkey.deserialize(begin, recv_size - uint32_t(begin - data));
	begin += tmp_endkeysize;
	//memcpy((void *)&this->_num, begin, sizeof(uint32_t));
	//begin += sizeof(uint32_t);
	int tmp_split_prev_bytes = Packet<key_t>::get_split_prev_bytes(this->_methodid);
	begin += tmp_split_prev_bytes;
	memcpy((void *)&this->_cur_scanidx, begin, sizeof(uint16_t));
	this->_cur_scanidx = ntohs(this->_cur_scanidx);
	begin += sizeof(uint16_t);
	memcpy((void *)&this->_max_scannum, begin, sizeof(uint16_t));
	this->_max_scannum = ntohs(this->_max_scannum);
	begin += sizeof(uint16_t);
	if (!Packet<key_t>::is_singleswitch(this->_methodid)) {
		memcpy((void *)&this->_cur_scanswitchidx, begin, sizeof(uint16_t));
		this->_cur_scanswitchidx = ntohs(this->_cur_scanswitchidx);
		begin += sizeof(uint16_t);
		memcpy((void *)&this->_max_scanswitchnum, begin, sizeof(uint16_t));
		this->_max_scanswitchnum = ntohs(this->_max_scanswitchnum);
		begin += sizeof(uint16_t);
	}
	memcpy(&this->_nodeidx_foreval, begin, sizeof(uint16_t));
	this->_nodeidx_foreval = ntohs(this->_nodeidx_foreval);
	begin += sizeof(uint16_t);
	memcpy(&this->_snapshotid, begin, sizeof(int));
	begin += sizeof(int);

	memcpy((void *)&this->_pairnum, begin, sizeof(int32_t));
	this->_pairnum = int32_t(ntohl(uint32_t(this->_pairnum)));
	begin += sizeof(int32_t);
	uint32_t totalsize = uint32_t(begin - data);
	this->_pairs.resize(this->_pairnum); // change size to this->_pairnum (not just reserve)
	for (int32_t pair_i = 0; pair_i < this->_pairnum; pair_i++) {
		uint32_t tmp_pair_keysize = this->_pairs[pair_i].first.deserialize(begin, recv_size - totalsize);
		begin += tmp_pair_keysize;
		totalsize += tmp_pair_keysize;
		uint32_t tmp_pair_valsize = this->_pairs[pair_i].second.val.deserialize_large(begin, recv_size - totalsize);
		begin += tmp_pair_valsize;
		totalsize += tmp_pair_valsize;
	}
}

template<class key_t, class val_t>
size_t ScanResponseSplit<key_t, val_t>::get_frag_hdrsize(method_t methodid) {
	//return sizeof(optype_t) + sizeof(key_t) + sizeof(key_t) + SPLIT_PREV_BYTES + sizeof(uint16_t) + sizeof(uint16_t) + sizeof(uint16_t); // op_hdr + scan_hdr + split_hdr + nodeidx_foreval (used to identify fragments from different servers)
	// NOTE: we only need nodeidx_foreval for entire split packet instead of each fragment; so we can place it in fragment body isntead of fragment header in udpsendlarge_ipfrag (see socket_helper.c)
	size_t result = Packet<key_t>::get_ophdrsize(methodid) + sizeof(key_t) + Packet<key_t>::get_split_prev_bytes(methodid) + sizeof(uint16_t) + sizeof(uint16_t); // op_hdr + scan_hdr + split_hdr (isclone + globalserveridx + cur_scanidx + max_scannum [+ cur_scanswitchidx + max_scanswitchnum])
	if (!Packet<key_t>::is_singleswitch(methodid)) {
		result += (sizeof(uint16_t) + sizeof(uint16_t));
	}
	return result;
}

template<class key_t, class val_t>
size_t ScanResponseSplit<key_t, val_t>::get_srcnum_off(method_t methodid) {
	return Packet<key_t>::get_ophdrsize(methodid) + sizeof(key_t) + Packet<key_t>::get_split_prev_bytes(methodid) + sizeof(uint16_t); // offset of split_hdr.max_scannum
}

template<class key_t, class val_t>
size_t ScanResponseSplit<key_t, val_t>::get_srcnum_len() {
	return sizeof(uint16_t);
}

template<class key_t, class val_t>
bool ScanResponseSplit<key_t, val_t>::get_srcnum_conversion() {
	return true;
}

template<class key_t, class val_t>
size_t ScanResponseSplit<key_t, val_t>::get_srcid_off(method_t methodid) {
	return Packet<key_t>::get_ophdrsize(methodid) + sizeof(key_t) + Packet<key_t>::get_split_prev_bytes(methodid); // offset of split_hdr.cur_scanidx
}

template<class key_t, class val_t>
size_t ScanResponseSplit<key_t, val_t>::get_srcid_len() {
	return sizeof(uint16_t);
}

template<class key_t, class val_t>
bool ScanResponseSplit<key_t, val_t>::get_srcid_conversion() {
	return true;
}

template<class key_t, class val_t>
size_t ScanResponseSplit<key_t, val_t>::get_srcswitchnum_off(method_t methodid) {
	INVARIANT(!Packet<key_t>::is_singleswitch(methodid));
	return Packet<key_t>::get_ophdrsize(methodid) + sizeof(key_t) + Packet<key_t>::get_split_prev_bytes(methodid) + sizeof(uint16_t) + sizeof(uint16_t) + sizeof(uint16_t); // offset of split_hdr.max_scanswitchnum
}

template<class key_t, class val_t>
size_t ScanResponseSplit<key_t, val_t>::get_srcswitchnum_len() {
	return sizeof(uint16_t);
}

template<class key_t, class val_t>
bool ScanResponseSplit<key_t, val_t>::get_srcswitchnum_conversion() {
	return true;
}

template<class key_t, class val_t>
size_t ScanResponseSplit<key_t, val_t>::get_srcswitchid_off(method_t methodid) {
	INVARIANT(!Packet<key_t>::is_singleswitch(methodid));
	return Packet<key_t>::get_ophdrsize(methodid) + sizeof(key_t) + Packet<key_t>::get_ophdrsize(methodid) + sizeof(uint16_t) + sizeof(uint16_t); // offset of split_hdr.cur_scanswitchidx
}

template<class key_t, class val_t>
size_t ScanResponseSplit<key_t, val_t>::get_srcswitchid_len() {
	return sizeof(uint16_t);
}

template<class key_t, class val_t>
bool ScanResponseSplit<key_t, val_t>::get_srcswitchid_conversion() {
	return true;
}

// ScanResponseSplitServer

template<class key_t, class val_t>
ScanResponseSplitServer<key_t, val_t>::ScanResponseSplitServer(method_t methodid, key_t key, key_t endkey, uint16_t cur_scanidx, uint16_t max_scannum, uint16_t cur_scanswitchidx, uint16_t max_scanswitchnum, uint16_t nodeidx_foreval, int snapshotid, int32_t pairnum, std::vector<std::pair<key_t, snapshot_record_t>> pairs) 
	: ScanResponseSplit<key_t, val_t>(methodid, key, endkey, cur_scanidx, max_scannum, cur_scanswitchidx, max_scanswitchnum, nodeidx_foreval, snapshotid, pairnum, pairs)
{	
	this->_type = static_cast<optype_t>(PacketType::SCANRES_SPLIT_SERVER);
}

// GetRequestPOP

template<class key_t>
GetRequestPOP<key_t>::GetRequestPOP(method_t methodid, const char *data, uint32_t recv_size)
{
	INVARIANT(methodid == FARREACH_ID || methodid == DISTFARREACH_ID);
	this->_methodid = methodid;
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::GETREQ_POP);
}

template<class key_t>
uint32_t GetRequestPOP<key_t>::serialize(char * const data, uint32_t max_size)
{
	COUT_N_EXIT("Invalid invoke of serialize for GetRequestPOP");
}

// GetRequestNLatest

template<class key_t>
GetRequestNLatest<key_t>::GetRequestNLatest(method_t methodid, const char *data, uint32_t recv_size)
{
	INVARIANT(methodid == FARREACH_ID || methodid == DISTFARREACH_ID);
	this->_methodid = methodid;
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::GETREQ_NLATEST);
}

template<class key_t>
uint32_t GetRequestNLatest<key_t>::serialize(char * const data, uint32_t max_size)
{
	COUT_N_EXIT("Invalid invoke of serialize for GetRequestNLatest");
}

// GetResponseLatestSeq (value must <= 128B)

template<class key_t, class val_t>
GetResponseLatestSeq<key_t, val_t>::GetResponseLatestSeq()
	: PutRequestSeq<key_t, val_t>(), _stat(true), _nodeidx_foreval(0)
{}

template<class key_t, class val_t>
GetResponseLatestSeq<key_t, val_t>::GetResponseLatestSeq(method_t methodid, key_t key, val_t val, uint32_t seq, uint16_t nodeidx_foreval)
	: PutRequestSeq<key_t, val_t>(methodid, key, val, seq), _stat(true), _nodeidx_foreval(nodeidx_foreval)
{
	//INVARIANT(methodid == FARREACH_ID || methodid == DISTFARREACH_ID); // NOTE: we do NOT check methodid here due to CACHE_EVICT/NETCACHE_VALUEUPDATE/DISTCACHE_SPINE_VALUEUPDATE_INSWITCH inheriting from GETRES_LATEST_SEQ
	this->_type = optype_t(packet_type_t::GETRES_LATEST_SEQ);
	INVARIANT(this->_val.val_length <= val_t::SWITCH_MAX_VALLEN);
	INVARIANT(seq >= 0);
}

template<class key_t, class val_t>
uint32_t GetResponseLatestSeq<key_t, val_t>::serialize(char * const data, uint32_t max_size) {
	//uint32_t my_size = this->size();
	//INVARIANT(max_size >= my_size);
	char *begin = data;
	uint32_t tmp_ophdrsize = this->serialize_ophdr(begin, max_size);
	begin += tmp_ophdrsize;
	uint32_t tmp_valsize = this->_val.serialize(begin, max_size - uint32_t(begin - data));
	begin += tmp_valsize;
	uint32_t tmp_shadowtypesize = serialize_packet_type(this->_type, begin, max_size - uint32_t(begin - data)); // shadowtype
	begin += tmp_shadowtypesize;
	uint32_t bigendian_seq = htonl(this->_seq);
	memcpy(begin, (void *)&bigendian_seq, sizeof(uint32_t)); // little-endian to big-endian
	begin += sizeof(uint32_t);
	memcpy(begin, (void *)&this->_stat, sizeof(bool));
	begin += sizeof(bool);
	uint16_t bigendian_nodeidx_foreval = htons(this->_nodeidx_foreval);
	memcpy(begin, (void *)&bigendian_nodeidx_foreval, sizeof(uint16_t));
	begin += sizeof(uint16_t);
	begin += Packet<key_t>::get_stat_padding_bytes(this->_methodid);
	return uint32_t(begin - data);
}

template<class key_t, class val_t>
bool GetResponseLatestSeq<key_t, val_t>::stat() const {
	return this->_stat;
}

template<class key_t, class val_t>
uint16_t GetResponseLatestSeq<key_t, val_t>::nodeidx_foreval() const {
	return this->_nodeidx_foreval;
}

template<class key_t, class val_t>
uint32_t GetResponseLatestSeq<key_t, val_t>::size() { // unused
	return Packet<key_t>::get_ophdrsize(this->_methodid) + sizeof(uint16_t) + val_t::SWITCH_MAX_VALLEN + sizeof(optype_t) + sizeof(uint32_t) + sizeof(bool) + sizeof(uint16_t) + Packet<key_t>::get_stat_padding_bytes(this->_methodid);
}

template<class key_t, class val_t>
void GetResponseLatestSeq<key_t, val_t>::deserialize(const char * data, uint32_t recv_size)
{
	COUT_N_EXIT("Invalid invoke of deserialize for GetResponseLatestSeq");
}

// GetResponseLatestSeqServer (value must <= 128B)

template<class key_t, class val_t>
GetResponseLatestSeqServer<key_t, val_t>::GetResponseLatestSeqServer(method_t methodid, switchidx_t switchidx, key_t key, val_t val, uint32_t seq, uint16_t nodeidx_foreval)
	: GetResponseLatestSeq<key_t, val_t>(methodid, key, val, seq, nodeidx_foreval)
{
	INVARIANT(methodid == DISTFARREACH_ID);
	this->_type = optype_t(packet_type_t::GETRES_LATEST_SEQ_SERVER);
	this->_leafswitchidx = switchidx;
}

// GetResponseLatestSeqInswitchCase1 (value must <= 128B)

template<class key_t, class val_t>
GetResponseLatestSeqInswitchCase1<key_t, val_t>::GetResponseLatestSeqInswitchCase1()
	: GetResponseLatestSeq<key_t, val_t>(), _idx(0), _stat(false), _nodeidx_foreval(0)
{
}

template<class key_t, class val_t>
GetResponseLatestSeqInswitchCase1<key_t, val_t>::GetResponseLatestSeqInswitchCase1(method_t methodid, key_t key, val_t val, uint32_t seq, uint16_t idx, bool stat) 
	: GetResponseLatestSeq<key_t, val_t>(methodid, key, val, seq, 0), _idx(idx)
{
	INVARIANT(methodid == FARREACH_ID || methodid == DISTFARREACH_ID);
	this->_stat = stat;
	this->_type = static_cast<optype_t>(PacketType::GETRES_LATEST_SEQ_INSWITCH_CASE1);
	INVARIANT(this->_val.val_length <= val_t::SWITCH_MAX_VALLEN);
	INVARIANT(seq >= 0);
	INVARIANT(idx >= 0);
}

template<class key_t, class val_t>
GetResponseLatestSeqInswitchCase1<key_t, val_t>::GetResponseLatestSeqInswitchCase1(method_t methodid, const char * data, uint32_t recv_size) {
	INVARIANT(methodid == FARREACH_ID || methodid == DISTFARREACH_ID);
	this->_methodid = methodid;
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::GETRES_LATEST_SEQ_INSWITCH_CASE1);
	INVARIANT(this->_val.val_length <= val_t::SWITCH_MAX_VALLEN);
	INVARIANT(this->_seq >= 0);
	INVARIANT(this->_idx >= 0);
}

template<class key_t, class val_t>
uint16_t GetResponseLatestSeqInswitchCase1<key_t, val_t>::idx() const {
	return _idx;
}

template<class key_t, class val_t>
bool GetResponseLatestSeqInswitchCase1<key_t, val_t>::stat() const {
	return _stat;
}

template<class key_t, class val_t>
uint32_t GetResponseLatestSeqInswitchCase1<key_t, val_t>::size() { // unused
	return Packet<key_t>::get_ophdrsize(this->_methodid) + sizeof(uint16_t) + val_t::SWITCH_MAX_VALLEN + sizeof(optype_t) + sizeof(uint32_t) + Packet<key_t>::get_inswitch_prev_bytes(this->_methodid) + sizeof(uint16_t) + sizeof(bool) + sizeof(uint16_t) + Packet<key_t>::get_stat_padding_bytes(this->_methodid) + Packet<key_t>::get_clone_bytes(this->_methodid);
}

template<class key_t, class val_t>
uint32_t GetResponseLatestSeqInswitchCase1<key_t, val_t>::serialize(char * const data, uint32_t max_size) {
	//uint32_t my_size = this->size();
	//INVARIANT(max_size >= my_size);
	char *begin = data;
	uint32_t tmp_ophdrsize = this->serialize_ophdr(begin, max_size);
	begin += tmp_ophdrsize;
	uint32_t tmp_valsize = this->_val.serialize(begin, max_size - uint32_t(begin - data));
	begin += tmp_valsize;
	uint32_t tmp_shadowtypesize = serialize_packet_type(this->_type, begin, max_size - uint32_t(begin - data)); // shadowtype
	begin += tmp_shadowtypesize;
	uint32_t bigendian_seq = htonl(this->_seq);
	memcpy(begin, (void *)&bigendian_seq, sizeof(uint32_t));
	begin += sizeof(uint32_t);
	int tmp_inswitch_prev_bytes = Packet<key_t>::get_inswitch_prev_bytes(this->_methodid);
	memset(begin, 0, tmp_inswitch_prev_bytes); // the first bytes of inswitch_hdr
	begin += tmp_inswitch_prev_bytes;
	uint16_t bigendian_idx = htons(this->_idx);
	memcpy(begin, (void *)&bigendian_idx, sizeof(uint16_t)); // little-endian to big-endian
	begin += sizeof(uint16_t);
	memcpy(begin, (void *)&this->_stat, sizeof(bool));
	begin += sizeof(bool); // stat_hdr.stat
	memset(begin, 0, sizeof(uint16_t)); // stat_hdr.nodeidx_foreval
	begin += sizeof(uint16_t);
	begin += Packet<key_t>::get_stat_padding_bytes(this->_methodid);
	int tmp_clone_bytes = Packet<key_t>::get_clone_bytes(this->_methodid);
	memset(begin, 0, tmp_clone_bytes); // clone_hdr
	begin += tmp_clone_bytes;
	return uint32_t(begin - data);
}

template<class key_t, class val_t>
void GetResponseLatestSeqInswitchCase1<key_t, val_t>::deserialize(const char * data, uint32_t recv_size) {
	//uint32_t my_size = this->size();
	//INVARIANT(my_size == recv_size);
	const char *begin = data;
	uint32_t tmp_ophdrsize = this->deserialize_ophdr(begin, recv_size);
	begin += tmp_ophdrsize;
	uint32_t tmp_valsize = this->_val.deserialize(begin, recv_size - uint32_t(begin - data));
	begin += tmp_valsize;
	begin += sizeof(optype_t); // deserialize shadowtype
	memcpy((void *)&this->_seq, begin, sizeof(uint32_t));
	this->_seq = ntohl(this->_seq);
	begin += sizeof(uint32_t);
	begin += Packet<key_t>::get_inswitch_prev_bytes(this->_methodid); // the first bytes of inswitch_hdr
	memcpy((void *)&this->_idx, begin, sizeof(uint16_t));
	this->_idx = ntohs(this->_idx); // big-endian to little-endian
	begin += sizeof(uint16_t);
	memcpy((void *)&this->_stat, begin, sizeof(bool));
	begin += sizeof(bool); // stat_hdr.stat
	//begin += sizeof(uint16_t); // stat_hdr.nodeidx_foreval
	//begin += STAT_PADDING_BYTES; // stat_hdr.padding
	//begin += CLONE_BYTES; // clone_hdr
}

// GetResponseDeletedSeq (value must = 0B)

template<class key_t, class val_t>
GetResponseDeletedSeq<key_t, val_t>::GetResponseDeletedSeq(method_t methodid, key_t key, val_t val, uint32_t seq, uint16_t nodeidx_foreval)
	: GetResponseLatestSeq<key_t, val_t>(methodid, key, val, seq, nodeidx_foreval)
{
	INVARIANT(methodid == FARREACH_ID || methodid == DISTFARREACH_ID);
	this->_stat = false;
	this->_type = static_cast<optype_t>(PacketType::GETRES_DELETED_SEQ);
	INVARIANT(this->_val.val_length == 0);
	INVARIANT(seq >= 0);
}

template<class key_t, class val_t>
void GetResponseDeletedSeq<key_t, val_t>::deserialize(const char * data, uint32_t recv_size)
{
	COUT_N_EXIT("Invalid invoke of deserialize for GetResponseDeletedSeq");
}

// GetResponseDeletedSeqServer (value must = 0B)

template<class key_t, class val_t>
GetResponseDeletedSeqServer<key_t, val_t>::GetResponseDeletedSeqServer(method_t methodid, switchidx_t switchidx, key_t key, val_t val, uint32_t seq, uint16_t nodeidx_foreval)
	: GetResponseDeletedSeq<key_t, val_t>(methodid, key, val, seq, nodeidx_foreval)
{
	INVARIANT(methodid == DISTFARREACH_ID);
	this->_type = static_cast<optype_t>(PacketType::GETRES_DELETED_SEQ_SERVER);
	this->_leafswitchidx = switchidx;
}

// GetResponseDeletedSeqInswitchCase1 (value must <= 128B)

template<class key_t, class val_t>
GetResponseDeletedSeqInswitchCase1<key_t, val_t>::GetResponseDeletedSeqInswitchCase1(method_t methodid, key_t key, val_t val, uint32_t seq, uint16_t idx, bool stat) 
	: GetResponseLatestSeqInswitchCase1<key_t, val_t>(methodid, key, val, seq, idx, stat)
{
	this->_type = static_cast<optype_t>(PacketType::GETRES_DELETED_SEQ_INSWITCH_CASE1);
	INVARIANT(this->_val.val_length <= val_t::SWITCH_MAX_VALLEN);
	INVARIANT(seq >= 0);
	INVARIANT(idx >= 0);
}

template<class key_t, class val_t>
GetResponseDeletedSeqInswitchCase1<key_t, val_t>::GetResponseDeletedSeqInswitchCase1(method_t methodid, const char * data, uint32_t recv_size) {
	INVARIANT(methodid == FARREACH_ID || methodid == DISTFARREACH_ID);
	this->_methodid = methodid;
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::GETRES_DELETED_SEQ_INSWITCH_CASE1);
	INVARIANT(this->_val.val_length <= val_t::SWITCH_MAX_VALLEN);
	INVARIANT(this->_seq >= 0);
	INVARIANT(this->_idx >= 0);
}

// PutRequestSeq (value must <= 128B)

template<class key_t, class val_t>
PutRequestSeq<key_t, val_t>::PutRequestSeq()
	: Packet<key_t>(), _val(), _seq(0)
{
	this->_type = optype_t(packet_type_t::PUTREQ_SEQ);
}

template<class key_t, class val_t>
PutRequestSeq<key_t, val_t>::PutRequestSeq(method_t methodid, key_t key, val_t val, uint32_t seq)
	: Packet<key_t>(methodid, packet_type_t::PUTREQ_SEQ, 0, 0, key), _val(val), _seq(seq)
{
}

template<class key_t, class val_t>
PutRequestSeq<key_t, val_t>::PutRequestSeq(method_t methodid, const char * data, uint32_t recv_size)
{
	this->_methodid = methodid;
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::PUTREQ_SEQ);
	INVARIANT(this->_val.val_length <= val_t::SWITCH_MAX_VALLEN)
	INVARIANT(this->_seq >= 0);
}

template<class key_t, class val_t>
val_t PutRequestSeq<key_t, val_t>::val() const {
	return this->_val;
}

template<class key_t, class val_t>
uint32_t PutRequestSeq<key_t, val_t>::seq() const {
	return this->_seq;
}

template<class key_t, class val_t>
uint32_t PutRequestSeq<key_t, val_t>::size() {
	return Packet<key_t>::get_ophdrsize(this->_methodid) + sizeof(uint16_t) + val_t::SWITCH_MAX_VALLEN + sizeof(optype_t) + sizeof(uint32_t);
}

template<class key_t, class val_t>
void PutRequestSeq<key_t, val_t>::deserialize(const char * data, uint32_t recv_size)
{
	//uint32_t my_size = this->size();
	//INVARIANT(my_size == recv_size);
	const char *begin = data;
	uint32_t tmp_ophdrsize = this->deserialize_ophdr(begin, recv_size);
	begin += tmp_ophdrsize;
	uint32_t tmp_valsize = this->_val.deserialize(begin, recv_size - uint32_t(begin - data));
	begin += tmp_valsize;
	begin += sizeof(optype_t); // deserialize shadowtype
	memcpy((void *)&this->_seq, begin, sizeof(uint32_t));
	this->_seq = ntohl(this->_seq); // Big-endian to little-endian
	begin += sizeof(uint32_t);
}

template<class key_t, class val_t>
uint32_t PutRequestSeq<key_t, val_t>::serialize(char * const data, uint32_t max_size)
{
	COUT_N_EXIT("Invalid invoke of serialize for PutRequestSeq");
}

// PutRequestPopSeq (value must <= 128B)

template<class key_t, class val_t>
PutRequestPopSeq<key_t, val_t>::PutRequestPopSeq(method_t methodid, const char * data, uint32_t recv_size)
{
	INVARIANT(methodid == FARREACH_ID || methodid == DISTFARREACH_ID);
	this->_methodid = methodid;
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::PUTREQ_POP_SEQ);
	INVARIANT(this->_val.val_length <= val_t::SWITCH_MAX_VALLEN)
	INVARIANT(this->_seq >= 0);
}

template<class key_t, class val_t>
uint32_t PutRequestPopSeq<key_t, val_t>::serialize(char * const data, uint32_t max_size)
{
	COUT_N_EXIT("Invalid invoke of serialize for PutRequestPopSeq");
}

// PutRequestSeqInswitchCase1 (value must <= 128B)

template<class key_t, class val_t>
PutRequestSeqInswitchCase1<key_t, val_t>::PutRequestSeqInswitchCase1(method_t methodid, key_t key, val_t val, uint32_t seq, uint16_t idx, bool stat) 
	: GetResponseLatestSeqInswitchCase1<key_t, val_t>(methodid, key, val, seq, idx, stat)
{
	this->_type = static_cast<optype_t>(PacketType::PUTREQ_SEQ_INSWITCH_CASE1);
	INVARIANT(this->_val.val_length <= val_t::SWITCH_MAX_VALLEN);
	INVARIANT(seq >= 0);
	INVARIANT(idx >= 0);
}

template<class key_t, class val_t>
PutRequestSeqInswitchCase1<key_t, val_t>::PutRequestSeqInswitchCase1(method_t methodid, const char * data, uint32_t recv_size) {
	INVARIANT(methodid == FARREACH_ID || methodid == DISTFARREACH_ID);
	this->_methodid = methodid;
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::PUTREQ_SEQ_INSWITCH_CASE1);
	INVARIANT(this->_val.val_length <= val_t::SWITCH_MAX_VALLEN);
	INVARIANT(this->_seq >= 0);
	INVARIANT(this->_idx >= 0);
}

// PutRequestSeqCase3 (value must <= 128B)

template<class key_t, class val_t>
PutRequestSeqCase3<key_t, val_t>::PutRequestSeqCase3(method_t methodid, const char * data, uint32_t recv_size)
{
	INVARIANT(methodid == FARREACH_ID || methodid == DISTFARREACH_ID);
	this->_methodid = methodid;
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::PUTREQ_SEQ_CASE3);
	INVARIANT(this->_val.val_length <= val_t::SWITCH_MAX_VALLEN)
	INVARIANT(this->_seq >= 0);
}

template<class key_t, class val_t>
uint32_t PutRequestSeqCase3<key_t, val_t>::serialize(char * const data, uint32_t max_size)
{
	COUT_N_EXIT("Invalid invoke of serialize for PutRequestSeqCase3");
}

// PutRequestPopSeqCase3 (value must <= 128B)

template<class key_t, class val_t>
PutRequestPopSeqCase3<key_t, val_t>::PutRequestPopSeqCase3(method_t methodid, const char * data, uint32_t recv_size)
{
	INVARIANT(methodid == FARREACH_ID || methodid == DISTFARREACH_ID);
	this->_methodid = methodid;
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::PUTREQ_POP_SEQ_CASE3);
	INVARIANT(this->_val.val_length <= val_t::SWITCH_MAX_VALLEN)
	INVARIANT(this->_seq >= 0);
}

template<class key_t, class val_t>
uint32_t PutRequestPopSeqCase3<key_t, val_t>::serialize(char * const data, uint32_t max_size)
{
	COUT_N_EXIT("Invalid invoke of serialize for PutRequestPopSeqCase3");
}

// DelRequestSeq

template<class key_t>
DelRequestSeq<key_t>::DelRequestSeq() 
	: Packet<key_t>(), _seq(0)
{
}

template<class key_t>
DelRequestSeq<key_t>::DelRequestSeq(method_t methodid, const char * data, uint32_t recv_size) {
	this->_methodid = methodid;
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == packet_type_t::DELREQ_SEQ);
}

template<class key_t>
uint32_t DelRequestSeq<key_t>::seq() const {
	return this->_seq;
}

template<class key_t>
uint32_t DelRequestSeq<key_t>::size() {
	//return sizeof(optype_t) + sizeof(key_t) + sizeof(optype_t) + sizeof(uint32_t) + DEBUG_BYTES;
	return Packet<key_t>::get_ophdrsize(this->_methodid) + sizeof(optype_t) + sizeof(uint32_t);
}

template<class key_t>
uint32_t DelRequestSeq<key_t>::serialize(char * const data, uint32_t max_size) {
	COUT_N_EXIT("Invalid invoke of serialize for DelRequestSeq");
}

template<class key_t>
void DelRequestSeq<key_t>::deserialize(const char * data, uint32_t recv_size) {
	uint32_t my_size = this->size();
	INVARIANT(my_size <= recv_size);
	const char *begin = data;
	uint32_t tmp_ophdrsize = this->deserialize_ophdr(begin, recv_size);
	begin += tmp_ophdrsize;
	begin += sizeof(optype_t); // deserialize shadowtype
	memcpy((void *)&this->_seq, begin, sizeof(uint32_t));
	this->_seq = ntohl(this->_seq);
	begin += sizeof(uint32_t);
}

// DelRequestSeqInswitchCase1 (value must <= 128B)

template<class key_t, class val_t>
DelRequestSeqInswitchCase1<key_t, val_t>::DelRequestSeqInswitchCase1(method_t methodid, key_t key, val_t val, uint32_t seq, uint16_t idx, bool stat) 
	: GetResponseLatestSeqInswitchCase1<key_t, val_t>(methodid, key, val, seq, idx, stat)
{
	this->_type = static_cast<optype_t>(PacketType::DELREQ_SEQ_INSWITCH_CASE1);
	INVARIANT(this->_val.val_length <= val_t::SWITCH_MAX_VALLEN);
	INVARIANT(seq >= 0);
	INVARIANT(idx >= 0);
}

template<class key_t, class val_t>
DelRequestSeqInswitchCase1<key_t, val_t>::DelRequestSeqInswitchCase1(method_t methodid, const char * data, uint32_t recv_size) {
	INVARIANT(methodid == FARREACH_ID || methodid == DISTFARREACH_ID);
	this->_methodid = methodid;
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::DELREQ_SEQ_INSWITCH_CASE1);
	INVARIANT(this->_val.val_length <= val_t::SWITCH_MAX_VALLEN);
	INVARIANT(this->_seq >= 0);
	INVARIANT(this->_idx >= 0);
}

// DelRequestSeqCase3

template<class key_t>
DelRequestSeqCase3<key_t>::DelRequestSeqCase3(method_t methodid, const char * data, uint32_t recv_size)
{
	INVARIANT(methodid == FARREACH_ID || methodid == DISTFARREACH_ID);
	this->_methodid = methodid;
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::DELREQ_SEQ_CASE3);
	INVARIANT(this->_seq >= 0);
}

template<class key_t>
uint32_t DelRequestSeqCase3<key_t>::serialize(char * const data, uint32_t max_size)
{
	COUT_N_EXIT("Invalid invoke of serialize for DelRequestSeqCase3");
}

// ScanRequestSplit

template<class key_t>
ScanRequestSplit<key_t>::ScanRequestSplit() 
	: ScanRequest<key_t>(), _cur_scanidx(0), _max_scannum(0), _cur_scanswitchidx(0), _max_scanswitchnum(0)
{
}

template<class key_t>
ScanRequestSplit<key_t>::ScanRequestSplit(method_t methodid, key_t key, key_t endkey, uint16_t cur_scanidx, uint16_t max_scannum)
	: ScanRequest<key_t>(methodid, key, endkey), _cur_scanidx(cur_scanidx), _max_scannum(max_scannum), _cur_scanswitchidx(0), _max_scanswitchnum(0)
{
	INVARIANT(Packet<key_t>::is_singleswitch(methodid));
	this->_type = static_cast<optype_t>(packet_type_t::SCANREQ_SPLIT);
}

template<class key_t>
ScanRequestSplit<key_t>::ScanRequestSplit(method_t methodid, key_t key, key_t endkey, uint16_t cur_scanidx, uint16_t max_scannum, uint16_t cur_scanswitchidx, uint16_t max_scanswitchnum)
	: ScanRequest<key_t>(methodid, key, endkey), _cur_scanidx(cur_scanidx), _max_scannum(max_scannum), _cur_scanswitchidx(cur_scanswitchidx), _max_scanswitchnum(max_scanswitchnum)
{
	INVARIANT(!Packet<key_t>::is_singleswitch(methodid));
	this->_type = static_cast<optype_t>(packet_type_t::SCANREQ_SPLIT);
}

template<class key_t>
ScanRequestSplit<key_t>::ScanRequestSplit(method_t methodid, const char * data, uint32_t recv_size) {
	this->_methodid = methodid;
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == packet_type_t::SCANREQ_SPLIT);
}

template<class key_t>
uint16_t ScanRequestSplit<key_t>::cur_scanidx() const {
	return this->_cur_scanidx;
}

template<class key_t>
uint16_t ScanRequestSplit<key_t>::max_scannum() const {
	return this->_max_scannum;
}

template<class key_t>
uint16_t ScanRequestSplit<key_t>::cur_scanswitchidx() const {
	return this->_cur_scanswitchidx;
}

template<class key_t>
uint16_t ScanRequestSplit<key_t>::max_scanswitchnum() const {
	return this->_max_scanswitchnum;
}

template<class key_t>
uint32_t ScanRequestSplit<key_t>::size() {
	uint32_t size = Packet<key_t>::get_ophdrsize(this->_methodid) + sizeof(key_t) + Packet<key_t>::get_split_prev_bytes(this->_methodid) + sizeof(uint16_t) + sizeof(uint16_t);// + sizeof(uint32_t);
	if (!Packet<key_t>::is_singleswitch(this->_methodid)) {
		size += (sizeof(uint16_t) + sizeof(uint16_t));
	}
	return size;
}

template<class key_t>
uint32_t ScanRequestSplit<key_t>::serialize(char * const data, uint32_t max_size) {
	COUT_N_EXIT("Invalid invoke of serialize for ScanRequestSplit");
}

template<class key_t>
void ScanRequestSplit<key_t>::deserialize(const char * data, uint32_t recv_size) {
	uint32_t my_size = this->size();
	INVARIANT(my_size == recv_size);
	const char *begin = data;
	uint32_t tmp_ophdrsize = this->deserialize_ophdr(begin, recv_size);
	begin += tmp_ophdrsize;
	uint32_t tmp_endkeysize = this->_endkey.deserialize(begin, recv_size - uint32_t(begin - data));
	begin += tmp_endkeysize;
	//memcpy((void *)&this->_num, begin, sizeof(uint32_t));
	//begin += sizeof(uint32_t);
	begin += Packet<key_t>::get_split_prev_bytes(this->_methodid);
	memcpy((void *)&this->_cur_scanidx, begin, sizeof(uint16_t));
	this->_cur_scanidx = ntohs(this->_cur_scanidx);
	begin += sizeof(uint16_t);
	memcpy((void *)&this->_max_scannum, begin, sizeof(uint16_t));
	this->_max_scannum = ntohs(this->_max_scannum);
	begin += sizeof(uint16_t);
	if (!Packet<key_t>::is_singleswitch(this->_methodid)) {
		memcpy((void *)&this->_cur_scanswitchidx, begin, sizeof(uint16_t));
		this->_cur_scanswitchidx = ntohs(this->_cur_scanswitchidx);
		begin += sizeof(uint16_t);
		memcpy((void *)&this->_max_scanswitchnum, begin, sizeof(uint16_t));
		this->_max_scanswitchnum = ntohs(this->_max_scanswitchnum);
		begin += sizeof(uint16_t);
	}
}

// CachePop (value must <= 128B; only used in end-hosts)

template<class key_t, class val_t>
CachePop<key_t, val_t>::CachePop()
	: PutRequestSeq<key_t, val_t>(), _stat(false), _serveridx(0)
{
	this->_type = static_cast<optype_t>(PacketType::CACHE_POP);
	INVARIANT(this->_val.val_length <= val_t::SWITCH_MAX_VALLEN);
	INVARIANT(_serveridx >= 0);
}

template<class key_t, class val_t>
CachePop<key_t, val_t>::CachePop(method_t methodid, key_t key, val_t val, uint32_t seq, bool stat, uint16_t serveridx)
	: PutRequestSeq<key_t, val_t>(methodid, key, val, seq), _stat(stat), _serveridx(serveridx)
{
	this->_type = static_cast<optype_t>(PacketType::CACHE_POP);
	INVARIANT(this->_val.val_length <= val_t::SWITCH_MAX_VALLEN);
	INVARIANT(seq >= 0);
	INVARIANT(serveridx >= 0);
}

template<class key_t, class val_t>
CachePop<key_t, val_t>::CachePop(method_t methodid, const char * data, uint32_t recv_size) {
	this->_methodid = methodid;
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::CACHE_POP);
	INVARIANT(this->_val.val_length <= val_t::SWITCH_MAX_VALLEN)
	INVARIANT(this->_seq >= 0);
	INVARIANT(this->_serveridx >= 0);
}

template<class key_t, class val_t>
uint32_t CachePop<key_t, val_t>::serialize(char * const data, uint32_t max_size) {
	//uint32_t my_size = this->size();
	//INVARIANT(max_size >= my_size);
	char *begin = data;
	uint32_t tmp_ophdrsize = this->serialize_ophdr(begin, max_size);
	begin += tmp_ophdrsize;
	uint32_t tmp_valsize = this->_val.serialize(begin, max_size - uint32_t(begin - data));
	begin += tmp_valsize;
	uint32_t bigendian_seq = htonl(this->_seq);
	memcpy(begin, (void *)&bigendian_seq, sizeof(uint32_t)); // little-endian to big-endian
	begin += sizeof(uint32_t);
	memcpy(begin, (void *)&this->_stat, sizeof(bool));
	begin += sizeof(bool);
	uint16_t bigendian_serveridx = htons(uint16_t(this->_serveridx));
	memcpy(begin, (void *)&bigendian_serveridx, sizeof(uint16_t)); // little-endian to big-endian
	begin += sizeof(uint16_t);
	return uint32_t(begin - data);
}

template<class key_t, class val_t>
bool CachePop<key_t, val_t>::stat() const {
	return _stat;
}

template<class key_t, class val_t>
uint16_t CachePop<key_t, val_t>::serveridx() const {
	return _serveridx;
}

template<class key_t, class val_t>
uint32_t CachePop<key_t, val_t>::size() { // unused
	return Packet<key_t>::get_ophdrsize(this->_methodid) + sizeof(uint16_t) + val_t::SWITCH_MAX_VALLEN + sizeof(uint32_t) + sizeof(bool) + sizeof(uint16_t);
}

template<class key_t, class val_t>
void CachePop<key_t, val_t>::deserialize(const char * data, uint32_t recv_size)
{
	//uint32_t my_size = this->size();
	//INVARIANT(my_size == recv_size);
	const char *begin = data;
	uint32_t tmp_ophdrsize = this->deserialize_ophdr(begin, recv_size);
	begin += tmp_ophdrsize;
	uint32_t tmp_valsize = this->_val.deserialize(begin, recv_size - uint32_t(begin - data));
	begin += tmp_valsize;
	memcpy((void *)&this->_seq, begin, sizeof(uint32_t));
	this->_seq = ntohl(this->_seq); // Big-endian to little-endian
	begin += sizeof(uint32_t);
	memcpy((void *)&this->_stat, begin, sizeof(bool));
	begin += sizeof(bool);
	memcpy((void *)&this->_serveridx, begin, sizeof(uint16_t));
	this->_serveridx = ntohs(this->_serveridx); // Big-endian to little-endian
	begin += sizeof(uint16_t);
}

// CachePopInswitch (value must <= 128B)

template<class key_t, class val_t>
CachePopInswitch<key_t, val_t>::CachePopInswitch(method_t methodid, key_t key, val_t val, uint32_t seq, uint16_t freeidx, bool stat)
	: PutRequestSeq<key_t, val_t>(methodid, key, val, seq), _freeidx(freeidx), _stat(stat)
{
	this->_type = static_cast<optype_t>(PacketType::CACHE_POP_INSWITCH);
	INVARIANT(this->_val.val_length <= val_t::SWITCH_MAX_VALLEN);
	INVARIANT(seq >= 0);
	INVARIANT(freeidx >= 0);
}

template<class key_t, class val_t>
CachePopInswitch<key_t, val_t>::CachePopInswitch(method_t methodid, switchidx_t switchidx, key_t key, val_t val, uint32_t seq, uint16_t freeidx, bool stat)
	: PutRequestSeq<key_t, val_t>(methodid, key, val, seq), _freeidx(freeidx), _stat(stat)
{
	this->_type = static_cast<optype_t>(PacketType::CACHE_POP_INSWITCH);
	this->_globalswitchidx = switchidx;
	INVARIANT(this->_val.val_length <= val_t::SWITCH_MAX_VALLEN);
	INVARIANT(seq >= 0);
	INVARIANT(freeidx >= 0);
}

template<class key_t, class val_t>
CachePopInswitch<key_t, val_t>::CachePopInswitch(method_t methodid, switchidx_t spineswitchidx, switchidx_t leafswitchidx, key_t key, val_t val, uint32_t seq, uint16_t freeidx, bool stat)
	: PutRequestSeq<key_t, val_t>(methodid, key, val, seq), _freeidx(freeidx), _stat(stat)
{
	this->_type = static_cast<optype_t>(PacketType::CACHE_POP_INSWITCH);
	this->_spineswitchidx = spineswitchidx;
	this->_leafswitchidx = leafswitchidx;
	INVARIANT(this->_val.val_length <= val_t::SWITCH_MAX_VALLEN);
	INVARIANT(seq >= 0);
	INVARIANT(freeidx >= 0);
}

template<class key_t, class val_t>
uint32_t CachePopInswitch<key_t, val_t>::serialize(char * const data, uint32_t max_size) {
	//uint32_t my_size = this->size();
	//INVARIANT(max_size >= my_size);
	char *begin = data;
	uint32_t tmp_ophdrsize = this->serialize_ophdr(begin, max_size);
	begin += tmp_ophdrsize;
	uint32_t tmp_valsize = this->_val.serialize(begin, max_size - uint32_t(begin - data));
	begin += tmp_valsize;
	uint32_t tmp_shadowtypesize = serialize_packet_type(this->_type, begin, max_size - uint32_t(begin - data)); // shadowtype
	begin += tmp_shadowtypesize;
	uint32_t bigendian_seq = htonl(this->_seq);
	memcpy(begin, (void *)&bigendian_seq, sizeof(uint32_t)); // little-endian to big-endian
	begin += sizeof(uint32_t);
	int tmp_inswitch_prev_bytes = Packet<key_t>::get_inswitch_prev_bytes(this->_methodid);
	memset(begin, 0, tmp_inswitch_prev_bytes); // the first bytes of inswitch_hdr
	begin += tmp_inswitch_prev_bytes;
	uint16_t bigendian_freeidx = htons(uint16_t(this->_freeidx));
	memcpy(begin, (void *)&bigendian_freeidx, sizeof(uint16_t)); // little-endian to big-endian
	begin += sizeof(uint16_t);
	memcpy(begin, (void *)&this->_stat, sizeof(bool));
	begin += sizeof(bool); // stat_hdr.stat
	memset(begin, 0, sizeof(uint16_t)); // stat_hdr.nodeidx_foreval
	begin += sizeof(uint16_t);
	begin += Packet<key_t>::get_stat_padding_bytes(this->_methodid);
	return uint32_t(begin - data);
}

template<class key_t, class val_t>
uint16_t CachePopInswitch<key_t, val_t>::freeidx() const {
	return _freeidx;
}

template<class key_t, class val_t>
bool CachePopInswitch<key_t, val_t>::stat() const {
	return _stat;
}

template<class key_t, class val_t>
uint32_t CachePopInswitch<key_t, val_t>::size() { // unused
	//return sizeof(optype_t) + sizeof(key_t) + sizeof(uint16_t) + val_t::MAX_VALLEN + sizeof(optype_t) + sizeof(uint32_t) + INSWITCH_PREV_BYTES + sizeof(uint16_t) + DEBUG_BYTES;
	return Packet<key_t>::get_ophdrsize(this->_methodid) + sizeof(uint16_t) + val_t::SWITCH_MAX_VALLEN + sizeof(optype_t) + sizeof(uint32_t) + Packet<key_t>::get_inswitch_prev_bytes(this->_methodid) + sizeof(uint16_t) + sizeof(bool) + sizeof(uint16_t) + Packet<key_t>::get_stat_padding_bytes(this->_methodid);
}

template<class key_t, class val_t>
void CachePopInswitch<key_t, val_t>::deserialize(const char * data, uint32_t recv_size)
{
	COUT_N_EXIT("Invalid invoke of deserialize for CachePopInswitch");
}

// CachePopInswitchAck (no value)

template<class key_t>
CachePopInswitchAck<key_t>::CachePopInswitchAck(method_t methodid, const char *data, uint32_t recv_size)
{
	this->_methodid = methodid;
	this->deserialize(data, recv_size);
	//INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::CACHE_POP_INSWITCH_ACK);
	if (unlikely(static_cast<packet_type_t>(this->_type) != PacketType::CACHE_POP_INSWITCH_ACK)) {
		printf("[CachePopInswitchAck] invalid packet type: %x\n", this->_type);
		exit(-1);
	}
}

template<class key_t>
uint32_t CachePopInswitchAck<key_t>::serialize(char * const data, uint32_t max_size)
{
	COUT_N_EXIT("Invalid invoke of serialize for CachePopInswitchAck");
}

// CacheEvict (value must <= 128B; only used in end-hosts)

template<class key_t, class val_t>
CacheEvict<key_t, val_t>::CacheEvict() 
	: GetResponseLatestSeq<key_t, val_t>(), _serveridx(0)
{
}

template<class key_t, class val_t>
CacheEvict<key_t, val_t>::CacheEvict(method_t methodid, key_t key, val_t val, uint32_t seq, bool stat, uint16_t serveridx) 
	: GetResponseLatestSeq<key_t, val_t>(methodid, key, val, seq, 0), _serveridx(serveridx)
{
	this->_stat = stat;
	this->_type = static_cast<optype_t>(PacketType::CACHE_EVICT);
	INVARIANT(this->_val.val_length <= val_t::SWITCH_MAX_VALLEN);
	INVARIANT(seq >= 0);
	INVARIANT(serveridx >= 0);
}

template<class key_t, class val_t>
CacheEvict<key_t, val_t>::CacheEvict(method_t methodid, const char * data, uint32_t recv_size) {
	this->_methodid = methodid;
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::CACHE_EVICT);
	INVARIANT(this->_seq >= 0);
	INVARIANT(this->_serveridx >= 0);
}

template<class key_t, class val_t>
uint16_t CacheEvict<key_t, val_t>::serveridx() const {
	return _serveridx;
}

template<class key_t, class val_t>
uint32_t CacheEvict<key_t, val_t>::size() { // unused
	return Packet<key_t>::get_ophdrsize(this->_methodid) + sizeof(uint16_t) + val_t::SWITCH_MAX_VALLEN + sizeof(uint32_t) + sizeof(bool) + sizeof(uint16_t);
}

template<class key_t, class val_t>
uint32_t CacheEvict<key_t, val_t>::serialize(char * const data, uint32_t max_size) {
	//uint32_t my_size = this->size();
	//INVARIANT(max_size >= my_size);
	char *begin = data;
	uint32_t tmp_ophdrsize = this->serialize_ophdr(begin, max_size);
	begin += tmp_ophdrsize;
	uint32_t tmp_valsize = this->_val.serialize(begin, max_size - uint32_t(begin - data));
	begin += tmp_valsize;
	uint32_t bigendian_seq = htonl(this->_seq);
	memcpy(begin, (void *)&bigendian_seq, sizeof(uint32_t));
	begin += sizeof(uint32_t);
	memcpy(begin, (void *)&this->_stat, sizeof(bool));
	begin += sizeof(bool);
	uint16_t bigendian_serveridx = htons(uint16_t(this->_serveridx));
	memcpy(begin, (void *)&bigendian_serveridx, sizeof(uint16_t));
	begin += sizeof(uint16_t);
	return uint32_t(begin - data);
}

template<class key_t, class val_t>
void CacheEvict<key_t, val_t>::deserialize(const char * data, uint32_t recv_size) {
	//uint32_t my_size = this->size();
	//INVARIANT(my_size == recv_size);
	const char *begin = data;
	uint32_t tmp_ophdrsize = this->deserialize_ophdr(begin, recv_size);
	begin += tmp_ophdrsize;
	uint32_t tmp_valsize = this->_val.deserialize(begin, recv_size - uint32_t(begin - data));
	begin += tmp_valsize;
	memcpy((void *)&this->_seq, begin, sizeof(uint32_t));
	this->_seq = ntohl(this->_seq);
	begin += sizeof(uint32_t);
	memcpy((void *)&this->_stat, begin, sizeof(bool));
	begin += sizeof(bool);
	memcpy((void *)&this->_serveridx, begin, sizeof(uint16_t));
	this->_serveridx = ntohs(this->_serveridx);
	begin += sizeof(uint16_t);
}

// CacheEvictAck (only used in end-hosts)

template<class key_t>
CacheEvictAck<key_t>::CacheEvictAck(method_t methodid, key_t key) 
	: WarmupRequest<key_t>(methodid, key) // CACHE_EVICT_ACK does NOT need spine/leafswitchidx for power-of-two-choices which is ONLY for GETREQ
{
	this->_type = static_cast<optype_t>(PacketType::CACHE_EVICT_ACK);
}

template<class key_t>
CacheEvictAck<key_t>::CacheEvictAck(method_t methodid, const char * data, uint32_t recv_size) {
	this->_methodid = methodid;
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::CACHE_EVICT_ACK);
}

// CacheEvictCase2 (value must <= 128B; only used in end-hosts)

template<class key_t, class val_t>
CacheEvictCase2<key_t, val_t>::CacheEvictCase2(method_t methodid, key_t key, val_t val, uint32_t seq, bool stat, uint16_t serveridx) 
	: CacheEvict<key_t, val_t>(methodid, key, val, seq, stat, serveridx)
{
	INVARIANT(methodid == FARREACH_ID || methodid == DISTFARREACH_ID);
	this->_type = static_cast<optype_t>(PacketType::CACHE_EVICT_CASE2);
	INVARIANT(this->_val.val_length <= val_t::SWITCH_MAX_VALLEN);
	INVARIANT(seq >= 0);
	INVARIANT(serveridx >= 0);
}

template<class key_t, class val_t>
CacheEvictCase2<key_t, val_t>::CacheEvictCase2(method_t methodid, const char * data, uint32_t recv_size) {
	INVARIANT(methodid == FARREACH_ID || methodid == DISTFARREACH_ID);
	this->_methodid = methodid;
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::CACHE_EVICT_CASE2);
	INVARIANT(this->_seq >= 0);
	INVARIANT(this->_serveridx >= 0);
}

// WarmupRequest

/*template<class key_t, class val_t>
WarmupRequest<key_t, val_t>::WarmupRequest(key_t key, val_t val) 
	: PutRequest<key_t, val_t>(key, val)
{
	this->_type = static_cast<optype_t>(PacketType::WARMUPREQ);
}

template<class key_t, class val_t>
WarmupRequest<key_t, val_t>::WarmupRequest(const char * data, uint32_t recv_size) {
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::WARMUPREQ);
}*/

template<class key_t>
WarmupRequest<key_t>::WarmupRequest()
	: Packet<key_t>()
{
}

template<class key_t>
WarmupRequest<key_t>::WarmupRequest(method_t methodid, key_t key)
	: Packet<key_t>(methodid, packet_type_t::WARMUPREQ, key) // WARMUPREQ does NOT need spine/leafswitchidx for power-of-two-choices which is ONLY for GETREQ
{
}

template<class key_t>
WarmupRequest<key_t>::WarmupRequest(method_t methodid, const char * data, uint32_t recv_size) {
	this->_methodid = methodid;
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::WARMUPREQ);
}

template<class key_t>
uint32_t WarmupRequest<key_t>::size() {
	return Packet<key_t>::get_ophdrsize(this->_methodid);
}

template<class key_t>
uint32_t WarmupRequest<key_t>::serialize(char * const data, uint32_t max_size) {
	uint32_t my_size = this->size();
	INVARIANT(max_size >= my_size);
	char *begin = data;
	uint32_t tmp_ophdrsize = this->serialize_ophdr(begin, max_size);
	begin += tmp_ophdrsize;
	return uint32_t(begin - data);
}

template<class key_t>
void WarmupRequest<key_t>::deserialize(const char * data, uint32_t recv_size) {
	uint32_t my_size = this->size();
	INVARIANT(my_size <= recv_size);
	const char *begin = data;
	uint32_t tmp_ophdrsize = this->deserialize_ophdr(begin, recv_size);
	begin += tmp_ophdrsize;
}

// WarmupAck

template<class key_t>
WarmupAck<key_t>::WarmupAck(method_t methodid, key_t key) 
	: WarmupRequest<key_t>(methodid, key) // WARMUPACK does NOT need spine/leafswitchidx for power-of-two-choices which is ONLY for GETREQ
{
	this->_type = static_cast<optype_t>(PacketType::WARMUPACK);
}

template<class key_t>
WarmupAck<key_t>::WarmupAck(method_t methodid, const char * data, uint32_t recv_size) {
	this->_methodid = methodid;
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::WARMUPACK);
}

// LoadRequest (value can be any size)

template<class key_t, class val_t>
LoadRequest<key_t, val_t>::LoadRequest(method_t methodid, key_t key, val_t val, uint16_t client_logical_idx, uint32_t fragseq) 
	: Packet<key_t>(methodid, PacketType::LOADREQ, key), _val(val), _client_logical_idx(client_logical_idx), _fragseq(fragseq)
{
	// NOTE: val can be any size for LOADREQ
}

template<class key_t, class val_t>
LoadRequest<key_t, val_t>::LoadRequest(method_t methodid, const char * data, uint32_t recv_size) {
	this->_methodid = methodid;
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::LOADREQ);
	// NOTE: val can be any size for LOADREQ
}

template<class key_t, class val_t>
val_t LoadRequest<key_t, val_t>::val() const {
	return _val;
}

template<class key_t, class val_t>
uint16_t LoadRequest<key_t, val_t>::client_logical_idx() const {
	return _client_logical_idx;
}

template<class key_t, class val_t>
uint32_t LoadRequest<key_t, val_t>::fragseq() const {
	return _fragseq;
}

template<class key_t, class val_t>
uint32_t LoadRequest<key_t, val_t>::size() { // not used
	return Packet<key_t>::get_ophdrsize(this->_methodid) + sizeof(uint16_t) + sizeof(uint32_t) + sizeof(uint32_t) + val_t::SWITCH_MAX_VALLEN;
}

template<class key_t, class val_t>
size_t LoadRequest<key_t, val_t>::get_frag_hdrsize(method_t methodid) {
	return Packet<key_t>::get_ophdrsize(methodid) + sizeof(uint16_t) + sizeof(uint32_t); // op_hdr + client_logical_idx + fragseq
}

template<class key_t, class val_t>
uint32_t LoadRequest<key_t, val_t>::dynamic_serialize(dynamic_array_t &dynamic_data) {
	int tmpoff = 0;
	uint32_t tmp_ophdrsize = this->dynamic_serialize_ophdr(dynamic_data);
	tmpoff += tmp_ophdrsize;
	uint16_t bigendian_client_logical_idx = htons(this->_client_logical_idx);
	dynamic_data.dynamic_memcpy(tmpoff, (char *)&bigendian_client_logical_idx, sizeof(uint16_t));
	tmpoff += sizeof(uint16_t);
	uint32_t bigendian_fragseq = htonl(this->_fragseq);
	dynamic_data.dynamic_memcpy(tmpoff, (char *)&bigendian_fragseq, sizeof(uint32_t));
	tmpoff += sizeof(uint32_t);
	uint32_t tmp_valsize = this->_val.dynamic_serialize_large(dynamic_data, tmpoff);
	tmpoff += tmp_valsize;
	return tmpoff;
}

template<class key_t, class val_t>
uint32_t LoadRequest<key_t, val_t>::serialize(char * const data, uint32_t max_size) {
	uint32_t my_size = this->size();
	INVARIANT(max_size >= my_size);
	char *begin = data;
	uint32_t tmp_ophdrsize = this->serialize_ophdr(begin, max_size);
	begin += tmp_ophdrsize;
	uint16_t bigendian_client_logical_idx = htons(this->_client_logical_idx);
	memcpy(begin, &bigendian_client_logical_idx, sizeof(uint16_t));
	begin += sizeof(uint16_t);
	uint32_t bigendian_fragseq = htonl(this->_fragseq);
	memcpy(begin, &bigendian_fragseq, sizeof(uint32_t));
	begin += sizeof(uint32_t);
	uint32_t tmp_valsize = this->_val.serialize_large(begin, max_size - uint32_t(begin - data));
	begin += tmp_valsize;
	//uint32_t tmp_shadowtypesize = serialize_packet_type(this->_type, begin, max_size - tmp_ophdrsize - tmp_valsize); // shadowtype
	//begin += tmp_shadowtypesize;
	return uint32_t(begin -data);// + tmp_shadowtypesize;
}

template<class key_t, class val_t>
void LoadRequest<key_t, val_t>::deserialize(const char * data, uint32_t recv_size) {
	uint32_t my_size = this->size();
	INVARIANT(my_size <= recv_size);
	const char *begin = data;
	uint32_t tmp_ophdrsize = this->deserialize_ophdr(begin, recv_size);
	begin += tmp_ophdrsize;
	memcpy(&this->_client_logical_idx, begin, sizeof(uint16_t));
	this->_client_logical_idx = ntohs(this->_client_logical_idx);
	begin += sizeof(uint16_t);
	memcpy(&this->_fragseq, begin, sizeof(uint32_t));
	this->_fragseq = ntohs(this->_fragseq);
	begin += sizeof(uint32_t);
	uint32_t tmp_valsize = this->_val.deserialize_large(begin, recv_size - uint32_t(begin - data));
	begin += tmp_valsize;
	//// deserialize shadowtype
}

// LoadAck

template<class key_t>
LoadAck<key_t>::LoadAck(method_t methodid, key_t key) 
	: WarmupRequest<key_t>(methodid, key) // LOADACK does NOT need spine/leafswitchidx for power-of-two-choices which is ONLY for GETREQ
{
	this->_type = static_cast<optype_t>(PacketType::LOADACK);
}

template<class key_t>
LoadAck<key_t>::LoadAck(method_t methodid, const char * data, uint32_t recv_size) {
	this->_methodid = methodid;
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::LOADACK);
}
 
// CachePopAck (only used by end-hosts)

template<class key_t>
CachePopAck<key_t>::CachePopAck(method_t methodid, key_t key) 
	: WarmupRequest<key_t>(methodid, key) // CACHE_POP_ACK does NOT need spine/leafswitchidx for power-of-two-choices which is ONLY for GETREQ
{
	this->_type = static_cast<optype_t>(PacketType::CACHE_POP_ACK);
}

template<class key_t>
CachePopAck<key_t>::CachePopAck(method_t methodid, const char * data, uint32_t recv_size) {
	this->_methodid = methodid;
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::CACHE_POP_ACK);
}

// CacheEvictLoadfreqInswitch

template<class key_t>
CacheEvictLoadfreqInswitch<key_t>::CacheEvictLoadfreqInswitch()
	: Packet<key_t>(), _evictidx(0)
{
	INVARIANT(_evictidx >= 0);
}

template<class key_t>
CacheEvictLoadfreqInswitch<key_t>::CacheEvictLoadfreqInswitch(method_t methodid, key_t key, uint16_t evictidx)
	: Packet<key_t>(methodid, PacketType::CACHE_EVICT_LOADFREQ_INSWITCH, key), _evictidx(evictidx)
{
	INVARIANT(Packet<key_t>::is_singleswitch(methodid));
	INVARIANT(evictidx >= 0);
}

template<class key_t>
CacheEvictLoadfreqInswitch<key_t>::CacheEvictLoadfreqInswitch(method_t methodid, switchidx_t switchidx, key_t key, uint16_t evictidx)
	: Packet<key_t>(methodid, PacketType::CACHE_EVICT_LOADFREQ_INSWITCH, switchidx, key), _evictidx(evictidx)
{
	INVARIANT(methodid == DISTFARREACH_ID || methodid == DISTNOCACHE_ID);
	INVARIANT(evictidx >= 0);
}

template<class key_t>
CacheEvictLoadfreqInswitch<key_t>::CacheEvictLoadfreqInswitch(method_t methodid, switchidx_t spineswitchidx, switchidx_t leafswitchidx, key_t key, uint16_t evictidx)
	: Packet<key_t>(methodid, PacketType::CACHE_EVICT_LOADFREQ_INSWITCH, spineswitchidx, leafswitchidx, key), _evictidx(evictidx)
{
	INVARIANT(methodid == DISTCACHE_ID);
	INVARIANT(evictidx >= 0);
}

template<class key_t>
uint32_t CacheEvictLoadfreqInswitch<key_t>::serialize(char * const data, uint32_t max_size) {
	uint32_t my_size = this->size();
	INVARIANT(max_size >= my_size);
	char *begin = data;
	uint32_t tmp_ophdrsize = this->serialize_ophdr(begin, max_size);
	begin += tmp_ophdrsize;
	uint32_t tmp_shadowtypesize = serialize_packet_type(this->_type, begin, max_size - uint32_t(begin - data)); // shadowtype
	begin += tmp_shadowtypesize;
	int tmp_inswitch_prev_bytes = Packet<key_t>::get_inswitch_prev_bytes(this->_methodid);
	memset(begin, 0, tmp_inswitch_prev_bytes); // the first bytes of inswitch_hdr
	begin += tmp_inswitch_prev_bytes;
	uint16_t bigendian_evictidx = htons(uint16_t(this->_evictidx));
	memcpy(begin, (void *)&bigendian_evictidx, sizeof(uint16_t)); // little-endian to big-endian
	begin += sizeof(uint16_t);
	return uint32_t(begin - data);
}

template<class key_t>
uint16_t CacheEvictLoadfreqInswitch<key_t>::evictidx() const {
	return _evictidx;
}

template<class key_t>
uint32_t CacheEvictLoadfreqInswitch<key_t>::size() { // unused
	return Packet<key_t>::get_ophdrsize(this->_methodid) + sizeof(optype_t) + Packet<key_t>::get_inswitch_prev_bytes(this->_methodid) + sizeof(uint16_t);
}

template<class key_t>
void CacheEvictLoadfreqInswitch<key_t>::deserialize(const char * data, uint32_t recv_size) {
	COUT_N_EXIT("Invalid invoke of serialize for CacheEvictLoadfreqInswitch");
}


// CacheEvictLoadfreqInswitchAck

template<class key_t>
CacheEvictLoadfreqInswitchAck<key_t>::CacheEvictLoadfreqInswitchAck(method_t methodid, const char * data, uint32_t recv_size)
{
	this->_methodid = methodid;
	this->deserialize(data, recv_size);
	INVARIANT(this->_type == optype_t(packet_type_t::CACHE_EVICT_LOADFREQ_INSWITCH_ACK));
	INVARIANT(_frequency >= 0);
}

template<class key_t>
void CacheEvictLoadfreqInswitchAck<key_t>::deserialize(const char * data, uint32_t recv_size) {
	uint32_t my_size = this->size();
	INVARIANT(recv_size >= my_size);
	const char *begin = data;
	uint32_t tmp_ophdrsize = this->deserialize_ophdr(begin, recv_size);
	begin += tmp_ophdrsize;
	memcpy(&this->_frequency, begin, sizeof(uint32_t));
	this->_frequency = ntohl(this->_frequency);
	begin += sizeof(uint32_t);
}

template<class key_t>
uint32_t CacheEvictLoadfreqInswitchAck<key_t>::frequency() const {
	return _frequency;
}

template<class key_t>
uint32_t CacheEvictLoadfreqInswitchAck<key_t>::size() { // unused
	return Packet<key_t>::get_ophdrsize(this->_methodid) + sizeof(uint32_t);
}

template<class key_t>
uint32_t CacheEvictLoadfreqInswitchAck<key_t>::serialize(char * const data, uint32_t max_size) {
	COUT_N_EXIT("Invalid invoke of deserialize for CacheEvictLoadfreqInswitchAck");
}

// CacheEvictLoaddataInswitch

template<class key_t>
CacheEvictLoaddataInswitch<key_t>::CacheEvictLoaddataInswitch(method_t methodid, key_t key, uint16_t evictidx)
	: CacheEvictLoadfreqInswitch<key_t>(methodid, key, evictidx)
{
	this->_type = optype_t(packet_type_t::CACHE_EVICT_LOADDATA_INSWITCH);
	INVARIANT(evictidx >= 0);
}

template<class key_t>
CacheEvictLoaddataInswitch<key_t>::CacheEvictLoaddataInswitch(method_t methodid, switchidx_t switchidx, key_t key, uint16_t evictidx)
	: CacheEvictLoadfreqInswitch<key_t>(methodid, switchidx, key, evictidx)
{
	this->_type = optype_t(packet_type_t::CACHE_EVICT_LOADDATA_INSWITCH);
	INVARIANT(evictidx >= 0);
}

template<class key_t>
CacheEvictLoaddataInswitch<key_t>::CacheEvictLoaddataInswitch(method_t methodid, switchidx_t spineswitchidx, switchidx_t leafswitchidx, key_t key, uint16_t evictidx)
	: CacheEvictLoadfreqInswitch<key_t>(methodid, spineswitchidx, leafswitchidx, key, evictidx)
{
	this->_type = optype_t(packet_type_t::CACHE_EVICT_LOADDATA_INSWITCH);
	INVARIANT(evictidx >= 0);
}

// CacheEvictLoaddataInswitchAck (value must <= 128B)

template<class key_t, class val_t>
CacheEvictLoaddataInswitchAck<key_t, val_t>::CacheEvictLoaddataInswitchAck(method_t methodid, const char * data, uint32_t recv_size) {
	this->_methodid = methodid;
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::CACHE_EVICT_LOADDATA_INSWITCH_ACK);
	INVARIANT(this->_val.val_length <= val_t::SWITCH_MAX_VALLEN);
	INVARIANT(this->_seq >= 0);
}

template<class key_t, class val_t>
val_t CacheEvictLoaddataInswitchAck<key_t, val_t>::val() const {
	return _val;
}

template<class key_t, class val_t>
uint32_t CacheEvictLoaddataInswitchAck<key_t, val_t>::seq() const {
	return _seq;
}

template<class key_t, class val_t>
bool CacheEvictLoaddataInswitchAck<key_t, val_t>::stat() const {
	return _stat;
}

template<class key_t, class val_t>
uint32_t CacheEvictLoaddataInswitchAck<key_t, val_t>::size() { // unused
	return Packet<key_t>::get_ophdrsize(this->_methodid) + sizeof(uint16_t) + val_t::SWITCH_MAX_VALLEN + sizeof(optype_t) + sizeof(uint32_t) + sizeof(bool) + sizeof(uint16_t) + Packet<key_t>::get_stat_padding_bytes(this->_methodid);
}

template<class key_t, class val_t>
void CacheEvictLoaddataInswitchAck<key_t, val_t>::deserialize(const char * data, uint32_t recv_size) {
	uint32_t my_size = this->size();
	// NOTE: loaded vallen may be smaller than 128B -> NOT check recv_size here
	//INVARIANT(my_size <= recv_size);
	const char *begin = data;
	uint32_t tmp_ophdrsize = this->deserialize_ophdr(begin, recv_size);
	begin += tmp_ophdrsize;
	uint32_t tmp_valsize = this->_val.deserialize(begin, recv_size - uint32_t(begin - data));
	begin += tmp_valsize;
	begin += sizeof(optype_t); // deserialize shadowtype
	memcpy((void *)&this->_seq, begin, sizeof(uint32_t));
	this->_seq = ntohl(this->_seq);
	begin += sizeof(uint32_t);
	memcpy((void *)&this->_stat, begin, sizeof(bool));
	begin += sizeof(bool); // stat_hdr.stat
	//begin += sizeof(uint16_t); // stat_hdr.nodeidx_foreval
	//begin += STAT_PADDING_BYTES; // stat_hdr.padding
}

template<class key_t, class val_t>
uint32_t CacheEvictLoaddataInswitchAck<key_t, val_t>::serialize(char * const data, uint32_t max_size) {
	COUT_N_EXIT("Invalid invoke of serialize for CacheEvictLoaddataInswitchAck");
}

// LoadsnapshotdataInswich

template<class key_t>
LoadsnapshotdataInswitch<key_t>::LoadsnapshotdataInswitch(method_t methodid, key_t key, uint16_t loadidx)
	: CacheEvictLoadfreqInswitch<key_t>(methodid, key, loadidx)
{
	this->_type = optype_t(packet_type_t::LOADSNAPSHOTDATA_INSWITCH);
	INVARIANT(loadidx >= 0);
}

template<class key_t>
LoadsnapshotdataInswitch<key_t>::LoadsnapshotdataInswitch(method_t methodid, switchidx_t switchidx, key_t key, uint16_t loadidx)
	: CacheEvictLoadfreqInswitch<key_t>(methodid, switchidx, key, loadidx)
{
	this->_type = optype_t(packet_type_t::LOADSNAPSHOTDATA_INSWITCH);
	INVARIANT(loadidx >= 0);
}

template<class key_t>
LoadsnapshotdataInswitch<key_t>::LoadsnapshotdataInswitch(method_t methodid, switchidx_t spineswitchidx, switchidx_t leafswitchidx, key_t key, uint16_t loadidx)
	: CacheEvictLoadfreqInswitch<key_t>(methodid, spineswitchidx, leafswitchidx, key, loadidx)
{
	this->_type = optype_t(packet_type_t::LOADSNAPSHOTDATA_INSWITCH);
	INVARIANT(loadidx >= 0);
}

template<class key_t>
uint16_t LoadsnapshotdataInswitch<key_t>::loadidx() const {
	return this->_evictidx;
}

// LoadsnapshotdataInswitchAck (value must <= 128B)

template<class key_t, class val_t>
LoadsnapshotdataInswitchAck<key_t, val_t>::LoadsnapshotdataInswitchAck(method_t methodid, const char * data, uint32_t recv_size) {
	this->_methodid = methodid;
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::LOADSNAPSHOTDATA_INSWITCH_ACK);
	INVARIANT(this->_val.val_length <= val_t::SWITCH_MAX_VALLEN);
	INVARIANT(this->_seq >= 0);
}

template<class key_t, class val_t>
uint32_t LoadsnapshotdataInswitchAck<key_t, val_t>::size() { // unused
	return Packet<key_t>::get_ophdrsize(this->_methodid) + sizeof(uint16_t) + val_t::SWITCH_MAX_VALLEN + sizeof(optype_t) + sizeof(uint32_t) + Packet<key_t>::get_inswitch_prev_bytes(this->_methodid) + sizeof(uint16_t) + sizeof(bool) + sizeof(uint16_t) + Packet<key_t>::get_stat_padding_bytes(this->_methodid);
}

template<class key_t, class val_t>
void LoadsnapshotdataInswitchAck<key_t, val_t>::deserialize(const char * data, uint32_t recv_size) {
	//uint32_t my_size = this->size();
	//INVARIANT(my_size == recv_size);
	const char *begin = data;
	uint32_t tmp_ophdrsize = this->deserialize_ophdr(begin, recv_size);
	begin += tmp_ophdrsize;
	uint32_t tmp_valsize = this->_val.deserialize(begin, recv_size - uint32_t(begin - data));
	begin += tmp_valsize;
	begin += sizeof(optype_t); // deserialize shadowtype
	memcpy((void *)&this->_seq, begin, sizeof(uint32_t));
	this->_seq = ntohl(this->_seq);
	begin += sizeof(uint32_t);
	begin += Packet<key_t>::get_inswitch_prev_bytes(this->_methodid); // the first bytes of inswitch_hdr
	memcpy((void *)&this->_idx, begin, sizeof(uint16_t));
	this->_idx = ntohs(this->_idx); // big-endian to little-endian
	begin += sizeof(uint16_t);
	memcpy((void *)&this->_stat, begin, sizeof(bool));
	begin += sizeof(bool); // stat_hdr.stat
	//begin += sizeof(uint16_t); // stat_hdr.nodeidx_foreval
	//begin += STAT_PADDING_BYTES; // stat_hdr.padding
}

// SetvalidInswitch

template<class key_t>
SetvalidInswitch<key_t>::SetvalidInswitch(method_t methodid, key_t key, uint16_t idx, uint8_t validvalue)
	: Packet<key_t>(methodid, PacketType::SETVALID_INSWITCH, key), _idx(idx), _validvalue(validvalue)
{
	INVARIANT(Packet<key_t>::is_singleswitch(methodid));
	INVARIANT(idx >= 0);
	INVARIANT(validvalue == 0 || validvalue == 1 || validvalue == 3);
}

template<class key_t>
SetvalidInswitch<key_t>::SetvalidInswitch(method_t methodid, switchidx_t switchidx, key_t key, uint16_t idx, uint8_t validvalue)
	: Packet<key_t>(methodid, PacketType::SETVALID_INSWITCH, switchidx, key), _idx(idx), _validvalue(validvalue)
{
	INVARIANT(methodid == DISTFARREACH_ID || methodid == DISTNOCACHE_ID);
	INVARIANT(idx >= 0);
	INVARIANT(validvalue == 0 || validvalue == 1 || validvalue == 3);
}

template<class key_t>
SetvalidInswitch<key_t>::SetvalidInswitch(method_t methodid, switchidx_t spineswitchidx, switchidx_t leafswitchidx, key_t key, uint16_t idx, uint8_t validvalue)
	: Packet<key_t>(methodid, PacketType::SETVALID_INSWITCH, spineswitchidx, leafswitchidx, key), _idx(idx), _validvalue(validvalue)
{
	INVARIANT(methodid == DISTCACHE_ID);
	INVARIANT(idx >= 0);
	INVARIANT(validvalue == 0 || validvalue == 1 || validvalue == 3);
}

template<class key_t>
uint32_t SetvalidInswitch<key_t>::serialize(char * const data, uint32_t max_size) {
	uint32_t my_size = this->size();
	INVARIANT(max_size >= my_size);
	char *begin = data;
	uint32_t tmp_ophdrsize = this->serialize_ophdr(begin, max_size);
	begin += tmp_ophdrsize;
	uint32_t tmp_shadowtypesize = serialize_packet_type(this->_type, begin, max_size - uint32_t(begin - data)); // shadowtype
	begin += tmp_shadowtypesize;
	int tmp_inswitch_prev_bytes = Packet<key_t>::get_inswitch_prev_bytes(this->_methodid);
	memset(begin, 0, tmp_inswitch_prev_bytes); // the first bytes of inswitch_hdr
	begin += tmp_inswitch_prev_bytes;
	uint16_t bigendian_idx = htons(uint16_t(this->_idx));
	memcpy(begin, (void *)&bigendian_idx, sizeof(uint16_t)); // little-endian to big-endian
	begin += sizeof(uint16_t);
	memcpy(begin, (void *)&_validvalue, sizeof(uint8_t));
	begin += sizeof(uint8_t);
	return uint32_t(begin - data);
}

template<class key_t>
uint16_t SetvalidInswitch<key_t>::idx() const {
	return _idx;
}

template<class key_t>
uint8_t SetvalidInswitch<key_t>::validvalue() const {
	return _validvalue;
}

template<class key_t>
uint32_t SetvalidInswitch<key_t>::size() { // unused
	return Packet<key_t>::get_ophdrsize(this->_methodid) + sizeof(optype_t) + Packet<key_t>::get_inswitch_prev_bytes(this->_methodid) + sizeof(uint16_t) + sizeof(uint8_t);
}

template<class key_t>
void SetvalidInswitch<key_t>::deserialize(const char * data, uint32_t recv_size) {
	COUT_N_EXIT("Invalid invoke of serialize for SetvalidInswitch");
}

// SetvalidInswitchAck (no value)

template<class key_t>
SetvalidInswitchAck<key_t>::SetvalidInswitchAck(method_t methodid, const char *data, uint32_t recv_size)
{
	this->_methodid = methodid;
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::SETVALID_INSWITCH_ACK);
}

template<class key_t>
uint32_t SetvalidInswitchAck<key_t>::serialize(char * const data, uint32_t max_size)
{
	COUT_N_EXIT("Invalid invoke of serialize for SetvalidInswitchAck");
}

// NetcacheGetRequestPop

template<class key_t>
NetcacheGetRequestPop<key_t>::NetcacheGetRequestPop(method_t methodid, key_t key)
	: GetRequest<key_t>(methodid, key) // NETCACHE_GETREQ_POP does NOT need spine/leafswitchidx for power-of-two-choices which is ONLY for GETREQ
{
	INVARIANT(methodid == NETCACHE_ID || methodid == DISTCACHE_ID);
	this->_type = optype_t(packet_type_t::NETCACHE_GETREQ_POP);
}

template<class key_t>
NetcacheGetRequestPop<key_t>::NetcacheGetRequestPop(method_t methodid, const char * data, uint32_t recv_size) {
	INVARIANT(methodid == NETCACHE_ID || methodid == DISTCACHE_ID);
	this->_methodid = methodid;
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == packet_type_t::NETCACHE_GETREQ_POP);
}

template<class key_t>
uint32_t NetcacheGetRequestPop<key_t>::size() {
	uint32_t size = Packet<key_t>::get_ophdrsize(this->_methodid) + Packet<key_t>::get_clone_bytes(this->_methodid);
	if (this->_methodid == DISTCACHE_ID) {
		size += (sizeof(optype_t) + sizeof(uint32_t) + sizeof(uint32_t));
	}
	return size;
}

template<class key_t>
uint32_t NetcacheGetRequestPop<key_t>::serialize(char * const data, uint32_t max_size) {
	COUT_N_EXIT("Invalid invoke of serialize for NetcacheGetRequestPop");
}

template<class key_t>
void NetcacheGetRequestPop<key_t>::deserialize(const char * data, uint32_t recv_size) {
	uint32_t my_size = this->size();
	INVARIANT(my_size <= recv_size);
	const char *begin = data;
	uint32_t tmp_ophdrsize = this->deserialize_ophdr(begin, recv_size);
	begin += tmp_ophdrsize;
	if (this->_methodid == DISTCACHE_ID) {
		begin += sizeof(optype_t); // shadowtype
		memcpy(&this->_spineload, begin, sizeof(uint32_t));
		this->_spineload = ntohl(this->_spineload);
		begin += sizeof(uint32_t);
		memcpy(&this->_leafload, begin, sizeof(uint32_t));
		this->_leafload = ntohl(this->_leafload);
		begin += sizeof(uint32_t);
	}
	begin += Packet<key_t>::get_clone_bytes(this->_methodid);
}

// NetcacheCachePop (only used in end-hosts)

template<class key_t>
NetcacheCachePop<key_t>::NetcacheCachePop()
	: WarmupRequest<key_t>(), _serveridx(0)
{
	this->_type = static_cast<optype_t>(PacketType::NETCACHE_CACHE_POP);
	INVARIANT(_serveridx >= 0);
}

template<class key_t>
NetcacheCachePop<key_t>::NetcacheCachePop(method_t methodid, key_t key, uint16_t serveridx)
	: WarmupRequest<key_t>(methodid, key), _serveridx(serveridx) // NETCACHE_CACHE_POP does NOT need spine/leafswitchidx for power-of-two-choices which is ONLY for GETREQ
{
	INVARIANT(methodid == NETCACHE_ID || methodid == DISTCACHE_ID);
	this->_type = static_cast<optype_t>(PacketType::NETCACHE_CACHE_POP);
	INVARIANT(serveridx >= 0);
}

template<class key_t>
NetcacheCachePop<key_t>::NetcacheCachePop(method_t methodid, const char * data, uint32_t recv_size) {
	INVARIANT(methodid == NETCACHE_ID || methodid == DISTCACHE_ID);
	this->_methodid = methodid;
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::NETCACHE_CACHE_POP);
	INVARIANT(this->_serveridx >= 0);
}

template<class key_t>
uint32_t NetcacheCachePop<key_t>::serialize(char * const data, uint32_t max_size) {
	uint32_t my_size = this->size();
	INVARIANT(max_size >= my_size);
	char *begin = data;
	uint32_t tmp_ophdrsize = this->serialize_ophdr(begin, max_size);
	begin += tmp_ophdrsize;
	uint16_t bigendian_serveridx = htons(uint16_t(this->_serveridx));
	memcpy(begin, (void *)&bigendian_serveridx, sizeof(uint16_t)); // little-endian to big-endian
	begin += sizeof(uint16_t);
	return uint32_t(begin - data);
}

template<class key_t>
uint16_t NetcacheCachePop<key_t>::serveridx() const {
	return _serveridx;
}

template<class key_t>
uint32_t NetcacheCachePop<key_t>::size() { // unused
	return Packet<key_t>::get_ophdrsize(this->_methodid) + sizeof(uint16_t);
}

template<class key_t>
void NetcacheCachePop<key_t>::deserialize(const char * data, uint32_t recv_size)
{
	uint32_t my_size = this->size();
	INVARIANT(my_size == recv_size);
	const char *begin = data;
	uint32_t tmp_ophdrsize = this->deserialize_ophdr(begin, recv_size);
	begin += tmp_ophdrsize;
	memcpy((void *)&this->_serveridx, begin, sizeof(uint16_t));
	this->_serveridx = ntohs(this->_serveridx); // Big-endian to little-endian
	begin += sizeof(uint16_t);
}

// NetcacheCachePopAck (value must <= 128B; only used in end-hosts)

template<class key_t, class val_t>
NetcacheCachePopAck<key_t, val_t>::NetcacheCachePopAck()
	: CachePop<key_t, val_t>()
{
}

template<class key_t, class val_t>
NetcacheCachePopAck<key_t, val_t>::NetcacheCachePopAck(method_t methodid, key_t key, val_t val, uint32_t seq, bool stat, uint16_t serveridx)
	: CachePop<key_t, val_t>(methodid, key, val, seq, stat, serveridx)
{
	INVARIANT(methodid == NETCACHE_ID || methodid == DISTCACHE_ID);
	this->_type = static_cast<optype_t>(PacketType::NETCACHE_CACHE_POP_ACK);
	INVARIANT(this->_val.val_length <= val_t::SWITCH_MAX_VALLEN);
	INVARIANT(seq >= 0);
	INVARIANT(serveridx >= 0);
}

template<class key_t, class val_t>
NetcacheCachePopAck<key_t, val_t>::NetcacheCachePopAck(method_t methodid, const char * data, uint32_t recv_size) {
	INVARIANT(methodid == NETCACHE_ID || methodid == DISTCACHE_ID);
	this->_methodid = methodid;
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::NETCACHE_CACHE_POP_ACK);
	INVARIANT(this->_val.val_length <= val_t::SWITCH_MAX_VALLEN);
	INVARIANT(this->_seq >= 0);
	INVARIANT(this->_serveridx >= 0);
}

// NetcacheCachePopFinish (only used in end-hosts)

template<class key_t>
NetcacheCachePopFinish<key_t>::NetcacheCachePopFinish(method_t methodid, key_t key, uint16_t serveridx)
	: NetcacheCachePop<key_t>(methodid, key, serveridx)
{
	INVARIANT(methodid == NETCACHE_ID);
	this->_type = static_cast<optype_t>(PacketType::NETCACHE_CACHE_POP_FINISH);
	INVARIANT(serveridx >= 0);
}

template<class key_t>
NetcacheCachePopFinish<key_t>::NetcacheCachePopFinish(method_t methodid, key_t key, uint16_t serveridx, uint16_t kvidx)
	: NetcacheCachePop<key_t>(methodid, key, serveridx), _kvidx(kvidx)
{
	INVARIANT(methodid == DISTCACHE_ID);
	this->_type = static_cast<optype_t>(PacketType::NETCACHE_CACHE_POP_FINISH);
	INVARIANT(serveridx >= 0);
}

template<class key_t>
NetcacheCachePopFinish<key_t>::NetcacheCachePopFinish(method_t methodid, const char * data, uint32_t recv_size) {
	INVARIANT(methodid == NETCACHE_ID || methodid == DISTCACHE_ID);
	this->_methodid = methodid;
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::NETCACHE_CACHE_POP_FINISH);
	INVARIANT(this->_serveridx >= 0);
}

template<class key_t>
uint32_t NetcacheCachePopFinish<key_t>::serialize(char * const data, uint32_t max_size) {
	uint32_t my_size = this->size();
	INVARIANT(max_size >= my_size);
	char *begin = data;
	uint32_t tmp_ophdrsize = this->serialize_ophdr(begin, max_size);
	begin += tmp_ophdrsize;
	uint16_t bigendian_serveridx = htons(uint16_t(this->_serveridx));
	memcpy(begin, (void *)&bigendian_serveridx, sizeof(uint16_t)); // little-endian to big-endian
	begin += sizeof(uint16_t);
	uint16_t bigendian_kvidx = htons(uint16_t(this->_kvidx));
	memcpy(begin, (void *)&bigendian_kvidx, sizeof(uint16_t)); // little-endian to big-endian
	begin += sizeof(uint16_t);
	return uint32_t(begin - data);
}

template<class key_t>
uint16_t NetcacheCachePopFinish<key_t>::kvidx() const {
	return _kvidx;
}

template<class key_t>
uint32_t NetcacheCachePopFinish<key_t>::size() { // unused
	return Packet<key_t>::get_ophdrsize(this->_methodid) + sizeof(uint16_t) + sizeof(uint16_t);
}

template<class key_t>
void NetcacheCachePopFinish<key_t>::deserialize(const char * data, uint32_t recv_size)
{
	uint32_t my_size = this->size();
	INVARIANT(my_size == recv_size);
	const char *begin = data;
	uint32_t tmp_ophdrsize = this->deserialize_ophdr(begin, recv_size);
	begin += tmp_ophdrsize;
	memcpy((void *)&this->_serveridx, begin, sizeof(uint16_t));
	this->_serveridx = ntohs(this->_serveridx); // Big-endian to little-endian
	begin += sizeof(uint16_t);
	memcpy((void *)&this->_kvidx, begin, sizeof(uint16_t));
	this->_kvidx = ntohs(this->_kvidx); // Big-endian to little-endian
	begin += sizeof(uint16_t);
}

// NetcacheCachePopFinishAck (only used in end-hosts)

template<class key_t>
NetcacheCachePopFinishAck<key_t>::NetcacheCachePopFinishAck(method_t methodid, key_t key, uint16_t serveridx)
	: NetcacheCachePop<key_t>(methodid, key, serveridx)
{
	INVARIANT(methodid == NETCACHE_ID || methodid == DISTCACHE_ID);
	this->_type = static_cast<optype_t>(PacketType::NETCACHE_CACHE_POP_FINISH_ACK);
	INVARIANT(serveridx >= 0);
}

template<class key_t>
NetcacheCachePopFinishAck<key_t>::NetcacheCachePopFinishAck(method_t methodid, const char * data, uint32_t recv_size) {
	INVARIANT(methodid == NETCACHE_ID || methodid == DISTCACHE_ID);
	this->_methodid = methodid;
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::NETCACHE_CACHE_POP_FINISH_ACK);
	INVARIANT(this->_serveridx >= 0);
}

// NetcacheWarmupRequestInswitchPop

template<class key_t>
NetcacheWarmupRequestInswitchPop<key_t>::NetcacheWarmupRequestInswitchPop(method_t methodid, key_t key)
	: CacheEvictLoadfreqInswitch<key_t>(methodid, key, 0, 0)
{
	INVARIANT(methodid == NETCACHE_ID || methodid == DISTCACHE_ID);
	this->_type = static_cast<optype_t>(PacketType::NETCACHE_WARMUPREQ_INSWITCH_POP);
}

template<class key_t>
NetcacheWarmupRequestInswitchPop<key_t>::NetcacheWarmupRequestInswitchPop(method_t methodid, const char * data, uint32_t recv_size) {
	INVARIANT(methodid == NETCACHE_ID || methodid == DISTCACHE_ID);
	this->_methodid = methodid;
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::NETCACHE_WARMUPREQ_INSWITCH_POP);
}

template<class key_t>
uint32_t NetcacheWarmupRequestInswitchPop<key_t>::serialize(char * const data, uint32_t max_size) {
	uint32_t my_size = this->size();
	INVARIANT(max_size >= my_size);
	char *begin = data;
	uint32_t tmp_ophdrsize = this->serialize_ophdr(begin, max_size);
	begin += tmp_ophdrsize;
	uint32_t tmp_shadowtypesize = serialize_packet_type(this->_type, begin, max_size - uint32_t(begin - data)); // shadowtype
	begin += tmp_shadowtypesize;
	int tmp_inswitch_prev_bytes = Packet<key_t>::get_inswitch_prev_bytes(this->_methodid);
	memset(begin, 0, tmp_inswitch_prev_bytes); // the first bytes of inswitch_hdr
	begin += tmp_inswitch_prev_bytes;
	memset(begin, 0, sizeof(uint16_t)); // inswitch_hdr.idx
	begin += sizeof(uint16_t);
	begin += Packet<key_t>::get_clone_bytes(this->_methodid); // clone_hdr
	return sizeof(begin - data);
}

template<class key_t>
uint32_t NetcacheWarmupRequestInswitchPop<key_t>::size() { // unused
	return Packet<key_t>::get_ophdrsize(this->_methodid) + sizeof(optype_t) + Packet<key_t>::get_inswitch_prev_bytes(this->_methodid) + sizeof(uint16_t) + Packet<key_t>::get_clone_bytes(this->_methodid);
}

template<class key_t>
void NetcacheWarmupRequestInswitchPop<key_t>::deserialize(const char * data, uint32_t recv_size) {
	uint32_t my_size = this->size();
	INVARIANT(recv_size >= my_size);
	const char *begin = data;
	uint32_t tmp_ophdrsize = this->deserialize_ophdr(begin, recv_size);
	begin += tmp_ophdrsize;
	uint32_t tmp_shadowtypesize = deserialize_packet_type(this->_type, begin, recv_size - uint32_t(begin - data)); // shadowtype
	begin += tmp_shadowtypesize;
	begin += Packet<key_t>::get_inswitch_prev_bytes(this->_methodid); // the first bytes of inswitch_hdr
	begin += sizeof(uint16_t); // inswitch_hdr.idx
	begin += Packet<key_t>::get_clone_bytes(this->_methodid); // clone_hdr
}

// NetcacheCacheEvict (only used in end-hosts)

template<class key_t>
NetcacheCacheEvict<key_t>::NetcacheCacheEvict(method_t methodid, key_t key, uint16_t serveridx)
	: NetcacheCachePop<key_t>(methodid, key, serveridx)
{
	this->_type = static_cast<optype_t>(PacketType::NETCACHE_CACHE_EVICT);
	INVARIANT(serveridx >= 0);
}

template<class key_t>
NetcacheCacheEvict<key_t>::NetcacheCacheEvict(method_t methodid, const char * data, uint32_t recv_size) {
	INVARIANT(methodid == NETCACHE_ID || methodid == DISTCACHE_ID);
	this->_methodid = methodid;
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::NETCACHE_CACHE_EVICT);
	INVARIANT(this->_serveridx >= 0);
}

// NetcacheCacheEvictAck (only used in end-hosts)

template<class key_t>
NetcacheCacheEvictAck<key_t>::NetcacheCacheEvictAck(method_t methodid, key_t key, uint16_t serveridx)
	: NetcacheCachePop<key_t>(methodid, key, serveridx)
{
	this->_type = static_cast<optype_t>(PacketType::NETCACHE_CACHE_EVICT_ACK);
	INVARIANT(serveridx >= 0);
}

template<class key_t>
NetcacheCacheEvictAck<key_t>::NetcacheCacheEvictAck(method_t methodid, const char * data, uint32_t recv_size) {
	INVARIANT(methodid == NETCACHE_ID || methodid == DISTCACHE_ID);
	this->_methodid = methodid;
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::NETCACHE_CACHE_EVICT_ACK);
	INVARIANT(this->_serveridx >= 0);
}

// NetcachePutRequestSeqCached

template<class key_t, class val_t>
NetcachePutRequestSeqCached<key_t, val_t>::NetcachePutRequestSeqCached(method_t methodid, const char * data, uint32_t recv_size) {
	INVARIANT(methodid == NETCACHE_ID || methodid == DISTCACHE_ID);
	this->_methodid = methodid;
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::NETCACHE_PUTREQ_SEQ_CACHED);
}

// NetcacheDelRequestSeqCached

template<class key_t>
NetcacheDelRequestSeqCached<key_t>::NetcacheDelRequestSeqCached(method_t methodid, const char * data, uint32_t recv_size) {
	INVARIANT(methodid == NETCACHE_ID || methodid == DISTCACHE_ID);
	this->_methodid = methodid;
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::NETCACHE_DELREQ_SEQ_CACHED);
}

// NetcacheValueupdate

template<class key_t, class val_t>
NetcacheValueupdate<key_t, val_t>::NetcacheValueupdate(method_t methodid, key_t key, val_t val, uint32_t seq, bool stat)
	: GetResponseLatestSeq<key_t, val_t>(methodid, key, val, seq, 0)
{
	INVARIANT(methodid == NETCACHE_ID);
	this->_stat = stat;
	this->_type = optype_t(packet_type_t::NETCACHE_VALUEUPDATE);
}

template<class key_t, class val_t>
NetcacheValueupdate<key_t, val_t>::NetcacheValueupdate(method_t methodid, switchidx_t spineswitchidx, switchidx_t leafswitchidx, key_t key, val_t val, uint32_t seq, bool stat)
	: GetResponseLatestSeq<key_t, val_t>(methodid, key, val, seq, 0)
{
	INVARIANT(methodid == DISTCACHE_ID);
	this->_stat = stat;
	this->_type = optype_t(packet_type_t::NETCACHE_VALUEUPDATE);
	this->_spineswitchidx = spineswitchidx;
	this->_leafswitchidx = leafswitchidx;
}

// NetcacheValueupdateAck

template<class key_t>
NetcacheValueupdateAck<key_t>::NetcacheValueupdateAck(method_t methodid, const char * data, uint32_t recv_size) {
	INVARIANT(methodid == NETCACHE_ID || methodid == DISTCACHE_ID);
	this->_methodid = methodid;
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::NETCACHE_VALUEUPDATE_ACK);
}

// WarmupAckServer 

// TODO: END HERE
template<class key_t>
WarmupAckServer<key_t>::WarmupAckServer(method_t methodid, key_t key) 
	: WarmupAck<key_t>(methodid, key)
{
	INVARIANT(!Packet<key_t>::is_singleswitch(methodid));
	this->_type = static_cast<optype_t>(PacketType::WARMUPACK_SERVER);
}

// LoadAckServer

template<class key_t>
LoadAckServer<key_t>::LoadAckServer(method_t methodid, key_t key) 
	: LoadAck<key_t>(methodid, key)
{
	INVARIANT(!Packet<key_t>::is_singleswitch(methodid));
	this->_type = static_cast<optype_t>(PacketType::LOADACK_SERVER);
}

// DistcacheCacheEvictVictim (only used by end-hosts)

template<class key_t>
DistcacheCacheEvictVictim<key_t>::DistcacheCacheEvictVictim()
	: Packet<key_t>(), _victimkey(key_t::min()), _victimidx(0)
{
}

template<class key_t>
DistcacheCacheEvictVictim<key_t>::DistcacheCacheEvictVictim(method_t methodid, key_t key, key_t victimkey, uint16_t victimidx)
	: Packet<key_t>(methodid, packet_type_t::DISTCACHE_CACHE_EVICT_VICTIM, 0, 0, key), _victimkey(victimkey), _victimidx(victimidx)
{
	INVARIANT(methodid == DISTCACHE_ID);
}

template<class key_t>
DistcacheCacheEvictVictim<key_t>::DistcacheCacheEvictVictim(method_t methodid, const char * data, uint32_t recv_size) {
	INVARIANT(methodid == DISTCACHE_ID);
	this->_methodid = methodid;
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == packet_type_t::DISTCACHE_CACHE_EVICT_VICTIM);
}

template<class key_t>
uint32_t DistcacheCacheEvictVictim<key_t>::size() {
	return Packet<key_t>::get_ophdrsize(this->_methodid) + sizeof(key_t) + sizeof(uint16_t);
}

template<class key_t>
key_t DistcacheCacheEvictVictim<key_t>::victimkey() const {
	return this->_victimkey;
}

template<class key_t>
uint16_t DistcacheCacheEvictVictim<key_t>::victimidx() const {
	return this->_victimidx;
}

template<class key_t>
uint32_t DistcacheCacheEvictVictim<key_t>::serialize(char * const data, uint32_t max_size) {
	uint32_t my_size = this->size();
	INVARIANT(max_size >= my_size);
	char *begin = data;
	uint32_t tmp_ophdrsize = this->serialize_ophdr(begin, max_size);
	begin += tmp_ophdrsize;
	uint32_t tmp_victimkeysize = this->_victimkey.serialize(begin, max_size - uint32_t(begin - data));
	begin += tmp_victimkeysize;
	memcpy(begin, (void *)&this->_victimidx, sizeof(uint16_t)); // only used by end-hosts
	begin += sizeof(uint16_t);
	return uint32_t(begin - data);
}

template<class key_t>
void DistcacheCacheEvictVictim<key_t>::deserialize(const char * data, uint32_t recv_size) {
	uint32_t my_size = this->size();
	INVARIANT(my_size <= recv_size);
	const char *begin = data;
	uint32_t tmp_ophdrsize = this->deserialize_ophdr(begin, recv_size);
	begin += tmp_ophdrsize;
	uint32_t tmp_victimkeysize = this->_victimkey.deserialize(begin, recv_size - uint32_t(begin - data));
	begin += tmp_victimkeysize;
	memcpy(&this->_victimidx, begin, sizeof(uint16_t)); // only used by end-hosts
	begin += sizeof(uint16_t);
}

// DistcacheCacheEvictVictimAck

template<class key_t>
DistcacheCacheEvictVictimAck<key_t>::DistcacheCacheEvictVictimAck(method_t methodid, key_t key) 
	: WarmupRequest<key_t>(methodid, key) // DISTCACHE_CACHE_EVICT_VICTIM_ACK does NOT need spine/leafswitchidx for power-of-two-choices which is ONLY for GETREQ
{
	INVARIANT(methodid == DISTCACHE_ID);
	this->_type = static_cast<optype_t>(PacketType::DISTCACHE_CACHE_EVICT_VICTIM_ACK);
}

template<class key_t>
DistcacheCacheEvictVictimAck<key_t>::DistcacheCacheEvictVictimAck(method_t methodid, const char * data, uint32_t recv_size) {
	INVARIANT(methodid == DISTCACHE_ID);
	this->_methodid = methodid;
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::DISTCACHE_CACHE_EVICT_VICTIM_ACK);
}

// DistcacheInvalidate

template<class key_t>
DistcacheInvalidate<key_t>::DistcacheInvalidate(method_t methodid, switchidx_t spineswitchidx, switchidx_t leafswitchidx, key_t key) 
	: WarmupRequest<key_t>(methodid, key) // DISTCACHE_INVALIDATE does NOT need spine/leafswitchidx for power-of-two-choices which is ONLY for GETREQ
{
	INVARIANT(methodid == DISTCACHE_ID);
	this->_type = static_cast<optype_t>(PacketType::DISTCACHE_INVALIDATE);
	this->_spineswitchidx = spineswitchidx;
	this->_leafswitchidx = leafswitchidx;
}

template<class key_t>
DistcacheInvalidate<key_t>::DistcacheInvalidate(method_t methodid, const char * data, uint32_t recv_size) {
	INVARIANT(methodid == DISTCACHE_ID);
	this->_methodid = methodid;
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::DISTCACHE_INVALIDATE);
}

// DistcacheInvalidateAck

template<class key_t>
DistcacheInvalidateAck<key_t>::DistcacheInvalidateAck(method_t methodid, key_t key) 
	: WarmupRequest<key_t>(methodid, key) // DISTCACHE_INVALIDATE_ACK does NOT need spine/leafswitchidx for power-of-two-choices which is ONLY for GETREQ
{
	INVARIANT(methodid == DISTCACHE_ID);
	this->_type = static_cast<optype_t>(PacketType::DISTCACHE_INVALIDATE_ACK);
}

template<class key_t>
DistcacheInvalidateAck<key_t>::DistcacheInvalidateAck(method_t methodid, const char * data, uint32_t recv_size) {
	INVARIANT(methodid == DISTCACHE_ID);
	this->_methodid = methodid;
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::DISTCACHE_INVALIDATE_ACK);
}

// DistcacheUpdateTrafficload

template<class key_t>
DistcacheUpdateTrafficload<key_t>::DistcacheUpdateTrafficload(method_t methodid, switchidx_t spineswitchidx, switchidx_t leafswitchidx, key_t key, uint32_t spineload, uint32_t leafload) 
	: GetRequest<key_t>(methodid, spineswitchidx, leafswitchidx, key)
{
	this->_type = static_cast<optype_t>(PacketType::DISTCACHE_UPDATE_TRAFFICLOAD);
	this->_spineload = spineload;
	this->_leafload = leafload;
}

// DistcacheSpineValueupdateInswitch (value must <= 128B)

template<class key_t, class val_t>
DistcacheSpineValueupdateInswitch<key_t, val_t>::DistcacheSpineValueupdateInswitch()
	: GetResponseLatestSeq<key_t, val_t>(), _kvidx(0)
{}

template<class key_t, class val_t>
DistcacheSpineValueupdateInswitch<key_t, val_t>::DistcacheSpineValueupdateInswitch(method_t methodid, switchidx_t spineswitchidx, switchidx_t leafswitchidx, key_t key, val_t val, uint32_t seq, bool stat, uint16_t kvidx)
	: GetResponseLatestSeq<key_t, val_t>(methodid, key, val, seq, 0), _kvidx(kvidx)
{
	INVARIANT(methodid == DISTCACHE_ID);
	this->_type = optype_t(packet_type_t::DISTCACHE_SPINE_VALUEUPDATE_INSWITCH);
	this->_spineswitchidx = spineswitchidx;
	this->_leafswitchidx = leafswitchidx;
	this->_stat = stat;
	INVARIANT(kvidx >= 0);
}

template<class key_t, class val_t>
uint32_t DistcacheSpineValueupdateInswitch<key_t, val_t>::serialize(char * const data, uint32_t max_size) {
	//uint32_t my_size = this->size();
	//INVARIANT(max_size >= my_size);
	char *begin = data;
	uint32_t tmp_ophdrsize = this->serialize_ophdr(begin, max_size);
	begin += tmp_ophdrsize;
	uint32_t tmp_valsize = this->_val.serialize(begin, max_size - uint32_t(begin - data));
	begin += tmp_valsize;
	uint32_t tmp_shadowtypesize = serialize_packet_type(this->_type, begin, max_size - uint32_t(begin - data)); // shadowtype
	begin += tmp_shadowtypesize;
	uint32_t bigendian_seq = htonl(this->_seq);
	memcpy(begin, (void *)&bigendian_seq, sizeof(uint32_t)); // little-endian to big-endian
	begin += sizeof(uint32_t);
	int tmp_inswitch_prev_bytes = Packet<key_t>::get_inswitch_prev_bytes(this->_methodid);
	memset(begin, 0, tmp_inswitch_prev_bytes); // the first bytes of inswitch_hdr
	begin += tmp_inswitch_prev_bytes;
	uint16_t bigendian_kvidx = htons(this->_kvidx);
	memcpy(begin, &bigendian_kvidx, sizeof(uint16_t));
	begin += sizeof(uint16_t);
	memcpy(begin, (void *)&this->_stat, sizeof(bool));
	begin += sizeof(bool);
	uint16_t bigendian_nodeidx_foreval = htons(this->_nodeidx_foreval);
	memcpy(begin, (void *)&bigendian_nodeidx_foreval, sizeof(uint16_t));
	begin += sizeof(uint16_t);
	begin += Packet<key_t>::get_stat_padding_bytes(this->_methodid);
	return uint32_t(begin - data);
}

template<class key_t, class val_t>
uint16_t DistcacheSpineValueupdateInswitch<key_t, val_t>::kvidx() const {
	return this->_kvidx;
}

template<class key_t, class val_t>
uint32_t DistcacheSpineValueupdateInswitch<key_t, val_t>::size() { // unused
	return Packet<key_t>::get_ophdrsize(this->_methodid) + sizeof(uint16_t) + val_t::SWITCH_MAX_VALLEN + sizeof(optype_t) + sizeof(uint32_t) + Packet<key_t>::get_inswitch_prev_bytes(this->_methodid) + sizeof(uint16_t) + sizeof(bool) + sizeof(uint16_t) + Packet<key_t>::get_stat_padding_bytes(this->_methodid);
}

// DistcacheLeafValueupdateInswitch (value must <= 128B)

template<class key_t, class val_t>
DistcacheLeafValueupdateInswitch<key_t, val_t>::DistcacheLeafValueupdateInswitch(method_t methodid, switchidx_t spineswitchidx, switchidx_t leafswitchidx, key_t key, val_t val, uint32_t seq, bool stat, uint16_t kvidx)
	: DistcacheSpineValueupdateInswitch<key_t, val_t>(methodid, spineswitchidx, leafswitchidx, key, val, seq, stat, kvidx)
{
	this->_type = optype_t(packet_type_t::DISTCACHE_LEAF_VALUEUPDATE_INSWITCH);
}

// DistcacheSpineValueupdateInswitchAck

/*template<class key_t>
DistcacheSpineValueupdateInswitchAck<key_t>::DistcacheSpineValueupdateInswitchAck(key_t key) 
	: WarmupRequest<key_t>(key) // DISTCACHE_SPINE_VALUEUPDATE_INSWITCH_ACK does NOT need spine/leafswitchidx for power-of-two-choices which is ONLY for GETREQ
{
	this->_type = static_cast<optype_t>(PacketType::DISTCACHE_SPINE_VALUEUPDATE_INSWITCH_ACK);
}*/

template<class key_t>
DistcacheSpineValueupdateInswitchAck<key_t>::DistcacheSpineValueupdateInswitchAck(method_t methodid, const char * data, uint32_t recv_size) {
	INVARIANT(methodid == DISTCACHE_ID);
	this->_methodid = methodid;
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::DISTCACHE_SPINE_VALUEUPDATE_INSWITCH_ACK);
}

// DistcacheLeafValueupdateInswitchAck

/*template<class key_t>
DistcacheLeafValueupdateInswitchAck<key_t>::DistcacheLeafValueupdateInswitchAck(key_t key) 
	: WarmupRequest<key_t>(key) // DISTCACHE_Leaf_VALUEUPDATE_INSWITCH_ACK does NOT need Leaf/leafswitchidx for power-of-two-choices which is ONLY for GETREQ
{
	this->_type = static_cast<optype_t>(PacketType::DISTCACHE_LEAF_VALUEUPDATE_INSWITCH_ACK);
}*/

template<class key_t>
DistcacheLeafValueupdateInswitchAck<key_t>::DistcacheLeafValueupdateInswitchAck(method_t methodid, const char * data, uint32_t recv_size) {
	INVARIANT(methodid == DISTCACHE_ID);
	this->_methodid = methodid;
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::DISTCACHE_LEAF_VALUEUPDATE_INSWITCH_ACK);
}

// DistcacheValueupdateInswitch (value must <= 128B)

template<class key_t, class val_t>
DistcacheValueupdateInswitch<key_t, val_t>::DistcacheValueupdateInswitch(method_t methodid, switchidx_t spineswitchidx, switchidx_t leafswitchidx, key_t key, val_t val, uint32_t seq, bool stat, uint16_t kvidx)
	: DistcacheSpineValueupdateInswitch<key_t, val_t>(methodid, spineswitchidx, leafswitchidx, key, val, seq, stat, kvidx)
{
	this->_type = optype_t(packet_type_t::DISTCACHE_VALUEUPDATE_INSWITCH);
}

// DistcacheValueupdateInswitchAck

template<class key_t>
DistcacheValueupdateInswitchAck<key_t>::DistcacheValueupdateInswitchAck(method_t methodid, const char * data, uint32_t recv_size) {
	INVARIANT(methodid == DISTCACHE_ID);
	this->_methodid = methodid;
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == packet_type_t::DISTCACHE_VALUEUPDATE_INSWITCH_ACK);
}

template<class key_t>
void DistcacheValueupdateInswitchAck<key_t>::deserialize(const char * data, uint32_t recv_size) {
	uint32_t my_size = this->size();
	INVARIANT(my_size <= recv_size);
	const char *begin = data;
	uint32_t tmp_ophdrsize = this->deserialize_ophdr(begin, recv_size);
	begin += tmp_ophdrsize;
	begin += sizeof(optype_t); // shadowtype
	// inswitch_hdr
	begin += Packet<key_t>::get_inswitch_prev_bytes(this->_methodid);
	begin += sizeof(uint16_t); // inswitch_hdr.idx
}

template<class key_t>
uint32_t DistcacheValueupdateInswitchAck<key_t>::size() {
	return Packet<key_t>::get_ophdrsize(this->_methodid) + Packet<key_t>::get_inswitch_prev_bytes(this->_methodid) + sizeof(uint16_t);
}

// PutRequestLargevalue (value must > 128B)

template<class key_t, class val_t>
PutRequestLargevalue<key_t, val_t>::PutRequestLargevalue()
	: Packet<key_t>(), _val(), _client_logical_idx(0), _fragseq(0)
{
}

template<class key_t, class val_t>
PutRequestLargevalue<key_t, val_t>::PutRequestLargevalue(method_t methodid, key_t key, val_t val, uint16_t client_logical_idx, uint32_t fragseq) 
	: Packet<key_t>(methodid, PacketType::PUTREQ_LARGEVALUE, 0, 0, key), _val(val), _client_logical_idx(client_logical_idx), _fragseq(fragseq)
{	
	INVARIANT(this->_val.val_length > val_t::SWITCH_MAX_VALLEN);
}

template<class key_t, class val_t>
PutRequestLargevalue<key_t, val_t>::PutRequestLargevalue(method_t methodid, const char * data, uint32_t recv_size) {
	this->_methodid = methodid;
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::PUTREQ_LARGEVALUE);
	INVARIANT(this->_val.val_length > val_t::SWITCH_MAX_VALLEN);
}

template<class key_t, class val_t>
val_t PutRequestLargevalue<key_t, val_t>::val() const {
	return _val;
}

template<class key_t, class val_t>
uint16_t PutRequestLargevalue<key_t, val_t>::client_logical_idx() const {
	return _client_logical_idx;
}

template<class key_t, class val_t>
uint32_t PutRequestLargevalue<key_t, val_t>::fragseq() const {
	return _fragseq;
}

template<class key_t, class val_t>
uint32_t PutRequestLargevalue<key_t, val_t>::size() { // not used
	return Packet<key_t>::get_ophdrsize(this->_methodid) + sizeof(uint16_t) + sizeof(uint32_t) + sizeof(uint32_t) + val_t::SWITCH_MAX_VALLEN;
}

template<class key_t, class val_t>
size_t PutRequestLargevalue<key_t, val_t>::get_frag_hdrsize(method_t methodid) {
	return Packet<key_t>::get_ophdrsize(methodid) + sizeof(uint16_t) + sizeof(uint32_t); // op_hdr + client_logical_idx + fragseq
}

template<class key_t, class val_t>
uint32_t PutRequestLargevalue<key_t, val_t>::dynamic_serialize(dynamic_array_t &dynamic_data) {
	int tmpoff = 0;
	uint32_t tmp_ophdrsize = this->dynamic_serialize_ophdr(dynamic_data);
	tmpoff += tmp_ophdrsize;
	uint16_t bigendian_client_logical_idx = htons(this->_client_logical_idx);
	dynamic_data.dynamic_memcpy(tmpoff, (char *)&bigendian_client_logical_idx, sizeof(uint16_t));
	tmpoff += sizeof(uint16_t);
	uint32_t bigendian_fragseq = htonl(this->_fragseq);
	dynamic_data.dynamic_memcpy(tmpoff, (char *)&bigendian_fragseq, sizeof(uint32_t));
	tmpoff += sizeof(uint32_t);
	uint32_t tmp_valsize = this->_val.dynamic_serialize_large(dynamic_data, tmpoff);
	tmpoff += tmp_valsize;
	return tmpoff;
}

template<class key_t, class val_t>
uint32_t PutRequestLargevalue<key_t, val_t>::serialize(char * const data, uint32_t max_size) {
	uint32_t my_size = this->size();
	INVARIANT(max_size >= my_size);
	char *begin = data;
	uint32_t tmp_ophdrsize = this->serialize_ophdr(begin, max_size);
	begin += tmp_ophdrsize;
	uint16_t bigendian_client_logical_idx = htons(this->_client_logical_idx);
	memcpy(begin, &bigendian_client_logical_idx, sizeof(uint16_t));
	begin += sizeof(uint16_t);
	uint32_t bigendian_fragseq = htonl(this->_fragseq);
	memcpy(begin, &bigendian_fragseq, sizeof(uint32_t));
	begin += sizeof(uint32_t);
	uint32_t tmp_valsize = this->_val.serialize_large(begin, max_size - uint32_t(begin - data));
	begin += tmp_valsize;
	return uint32_t(begin - data);
}

template<class key_t, class val_t>
void PutRequestLargevalue<key_t, val_t>::deserialize(const char * data, uint32_t recv_size) {
	uint32_t my_size = this->size();
	INVARIANT(my_size <= recv_size);
	const char *begin = data;
	uint32_t tmp_ophdrsize = this->deserialize_ophdr(begin, recv_size);
	begin += tmp_ophdrsize;
	memcpy(&this->_client_logical_idx, begin, sizeof(uint16_t));
	this->_client_logical_idx = ntohs(this->_client_logical_idx);
	begin += sizeof(uint16_t);
	memcpy(&this->_fragseq, begin, sizeof(uint32_t));
	this->_fragseq = ntohs(this->_fragseq);
	begin += sizeof(uint32_t);
	uint32_t tmp_valsize = this->_val.deserialize_large(begin, recv_size - uint32_t(begin - data));
	begin += tmp_valsize;
}

// PutRequestLargevalueSeq (value must > 128B)

template<class key_t, class val_t>
PutRequestLargevalueSeq<key_t, val_t>::PutRequestLargevalueSeq() 
	: PutRequestLargevalue<key_t, val_t>(), _seq(0)
{	
}

template<class key_t, class val_t>
PutRequestLargevalueSeq<key_t, val_t>::PutRequestLargevalueSeq(method_t methodid, key_t key, val_t val, uint32_t seq, uint16_t client_logical_idx, uint32_t fragseq) 
	: PutRequestLargevalue<key_t, val_t>(methodid, key, val, client_logical_idx, fragseq), _seq(seq)
{	
	this->_type = static_cast<optype_t>(packet_type_t::PUTREQ_LARGEVALUE_SEQ);
	INVARIANT(this->_val.val_length > val_t::SWITCH_MAX_VALLEN);
}

template<class key_t, class val_t>
PutRequestLargevalueSeq<key_t, val_t>::PutRequestLargevalueSeq(method_t methodid, const char * data, uint32_t recv_size) {
	this->_methodid = methodid;
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::PUTREQ_LARGEVALUE_SEQ);
	INVARIANT(this->_val.val_length > val_t::SWITCH_MAX_VALLEN);
}

template<class key_t, class val_t>
uint32_t PutRequestLargevalueSeq<key_t, val_t>::seq() const {
	return _seq;
}

template<class key_t, class val_t>
uint32_t PutRequestLargevalueSeq<key_t, val_t>::size() { // not used
	return Packet<key_t>::get_ophdrsize(this->_methodid) + sizeof(optype_t) + sizeof(uint32_t) + sizeof(uint16_t) + sizeof(uint32_t) + sizeof(uint32_t) + val_t::SWITCH_MAX_VALLEN;
}

template<class key_t, class val_t>
size_t PutRequestLargevalueSeq<key_t, val_t>::get_frag_hdrsize(method_t methodid) {
	return Packet<key_t>::get_ophdrsize(methodid) + sizeof(optype_t) + sizeof(uint32_t) + sizeof(uint16_t) + sizeof(uint32_t); // op_hdr + shadowtype_hdr + seq_hdr + client_logical_idx + fragseq
}

template<class key_t, class val_t>
uint32_t PutRequestLargevalueSeq<key_t, val_t>::serialize(char * const data, uint32_t max_size) {
	uint32_t my_size = this->size();
	INVARIANT(max_size >= my_size);
	char *begin = data;
	uint32_t tmp_ophdrsize = this->serialize_ophdr(begin, max_size);
	begin += tmp_ophdrsize;
	uint32_t tmp_shadowtypesize = serialize_packet_type(this->_type, begin, max_size - uint32_t(begin - data)); // shadowtype
	begin += tmp_shadowtypesize;
	uint32_t bigendian_seq = htonl(this->_seq);
	memcpy(begin, &bigendian_seq, sizeof(uint32_t));
	begin += sizeof(uint32_t);
	uint16_t bigendian_client_logical_idx = htons(this->_client_logical_idx);
	memcpy(begin, &bigendian_client_logical_idx, sizeof(uint16_t));
	begin += sizeof(uint16_t);
	uint32_t bigendian_fragseq = htonl(this->_fragseq);
	memcpy(begin, &bigendian_fragseq, sizeof(uint32_t));
	begin += sizeof(uint32_t);
	uint32_t tmp_valsize = this->_val.serialize_large(begin, max_size - uint32_t(begin - data));
	begin += tmp_valsize;
	return uint32_t(begin - data);
}

template<class key_t, class val_t>
void PutRequestLargevalueSeq<key_t, val_t>::deserialize(const char * data, uint32_t recv_size) {
	uint32_t my_size = this->size();
	INVARIANT(my_size <= recv_size);
	const char *begin = data;
	uint32_t tmp_ophdrsize = this->deserialize_ophdr(begin, recv_size);
	begin += tmp_ophdrsize;
	begin += sizeof(optype_t); // shadowtype
	memcpy(&this->_seq, begin, sizeof(uint32_t));
	this->_seq = ntohl(this->_seq);
	begin += sizeof(uint32_t);
	memcpy(&this->_client_logical_idx, begin, sizeof(uint16_t));
	this->_client_logical_idx = ntohs(this->_client_logical_idx);
	begin += sizeof(uint16_t);
	memcpy(&this->_fragseq, begin, sizeof(uint32_t));
	this->_fragseq = ntohs(this->_fragseq);
	begin += sizeof(uint32_t);
	uint32_t tmp_valsize = this->_val.deserialize_large(begin, recv_size - uint32_t(begin - data));
	begin += tmp_valsize;
}

// PutRequestLargevalueSeqCached (value must > 128B)

template<class key_t, class val_t>
PutRequestLargevalueSeqCached<key_t, val_t>::PutRequestLargevalueSeqCached(method_t methodid, key_t key, val_t val, uint32_t seq, uint16_t client_logical_idx, uint32_t fragseq) 
	: PutRequestLargevalueSeq<key_t, val_t>(methodid, key, val, seq, client_logical_idx, fragseq)
{	
	INVARIANT(methodid == NETCACHE_ID || methodid == DISTCACHE_ID);
	this->_type = static_cast<optype_t>(packet_type_t::PUTREQ_LARGEVALUE_SEQ_CACHED);
	INVARIANT(this->_val.val_length > val_t::SWITCH_MAX_VALLEN);
}

template<class key_t, class val_t>
PutRequestLargevalueSeqCached<key_t, val_t>::PutRequestLargevalueSeqCached(method_t methodid, const char * data, uint32_t recv_size) {
	INVARIANT(methodid == NETCACHE_ID || methodid == DISTCACHE_ID);
	this->_methodid = methodid;
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::PUTREQ_LARGEVALUE_SEQ_CACHED);
	INVARIANT(this->_val.val_length > val_t::SWITCH_MAX_VALLEN);
}

template<class key_t, class val_t>
size_t PutRequestLargevalueSeqCached<key_t, val_t>::get_frag_hdrsize(method_t methodid) {
	return Packet<key_t>::get_ophdrsize(methodid) + sizeof(optype_t) + sizeof(uint32_t) + sizeof(uint16_t) + sizeof(uint32_t); // op_hdr + shadowtype_hdr + seq_hdr + client_logical_idx + fragseq
}

// PutRequestLargevalueSeqCase3 (value must > 128B)

template<class key_t, class val_t>
PutRequestLargevalueSeqCase3<key_t, val_t>::PutRequestLargevalueSeqCase3(method_t methodid, key_t key, val_t val, uint32_t seq, uint16_t client_logical_idx, uint32_t fragseq) 
	: PutRequestLargevalueSeq<key_t, val_t>(methodid, key, val, seq, client_logical_idx, fragseq)
{	
	INVARIANT(methodid == FARREACH_ID || methodid == DISTFARREACH_ID);
	this->_type = static_cast<optype_t>(packet_type_t::PUTREQ_LARGEVALUE_SEQ_CASE3);
	INVARIANT(this->_val.val_length > val_t::SWITCH_MAX_VALLEN);
}

template<class key_t, class val_t>
PutRequestLargevalueSeqCase3<key_t, val_t>::PutRequestLargevalueSeqCase3(method_t methodid, const char * data, uint32_t recv_size) {
	INVARIANT(methodid == FARREACH_ID || methodid == DISTFARREACH_ID);
	this->_methodid = methodid;
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::PUTREQ_LARGEVALUE_SEQ_CASE3);
	INVARIANT(this->_val.val_length > val_t::SWITCH_MAX_VALLEN);
}

template<class key_t, class val_t>
size_t PutRequestLargevalueSeqCase3<key_t, val_t>::get_frag_hdrsize(method_t methodid) {
	return Packet<key_t>::get_ophdrsize(methodid) + sizeof(optype_t) + sizeof(uint32_t) + sizeof(uint16_t) + sizeof(uint32_t); // op_hdr + shadowtype_hdr + seq_hdr + client_logical_idx + fragseq
}

// GetResponseLargevalue (value must > 128B)

template<class key_t, class val_t>
GetResponseLargevalue<key_t, val_t>::GetResponseLargevalue()
	: Packet<key_t>(), _val(), _stat(false), _nodeidx_foreval(0), _spineload(0), _leafload(0)
{
}

template<class key_t, class val_t>
GetResponseLargevalue<key_t, val_t>::GetResponseLargevalue(method_t methodid, key_t key, val_t val, bool stat, uint16_t nodeidx_foreval) 
	: Packet<key_t>(methodid, PacketType::GETRES_LARGEVALUE, 0, 0, key), _val(val), _stat(stat), _nodeidx_foreval(nodeidx_foreval), _spineload(0), _leafload(0)
{	
	INVARIANT(this->_val.val_length > val_t::SWITCH_MAX_VALLEN);
}

template<class key_t, class val_t>
GetResponseLargevalue<key_t, val_t>::GetResponseLargevalue(method_t methodid, const char * data, uint32_t recv_size) {
	this->_methodid = methodid;
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::GETRES_LARGEVALUE);
	INVARIANT(this->_val.val_length > val_t::SWITCH_MAX_VALLEN);
}

template<class key_t, class val_t>
val_t GetResponseLargevalue<key_t, val_t>::val() const {
	return _val;
}

template<class key_t, class val_t>
bool GetResponseLargevalue<key_t, val_t>::stat() const {
	return _stat;
}

template<class key_t, class val_t>
uint16_t GetResponseLargevalue<key_t, val_t>::nodeidx_foreval() const {
	return _nodeidx_foreval;
}

template<class key_t, class val_t>
uint32_t GetResponseLargevalue<key_t, val_t>::spineload() const {
	return _spineload;
}

template<class key_t, class val_t>
uint32_t GetResponseLargevalue<key_t, val_t>::leafload() const {
	return _leafload;
}

template<class key_t, class val_t>
uint32_t GetResponseLargevalue<key_t, val_t>::size() { // unused
	return Packet<key_t>::get_ophdrsize(this->_methodid) + sizeof(uint32_t) + val_t::SWITCH_MAX_VALLEN + sizeof(bool) + sizeof(uint16_t) + Packet<key_t>::get_stat_padding_bytes(this->_methodid) + sizeof(uint32_t) + sizeof(uint32_t);
}

template<class key_t, class val_t>
size_t GetResponseLargevalue<key_t, val_t>::get_frag_hdrsize(method_t methodid) {
	return Packet<key_t>::get_ophdrsize(methodid); // op_hdr
}

template<class key_t, class val_t>
uint32_t GetResponseLargevalue<key_t, val_t>::dynamic_serialize(dynamic_array_t &dynamic_data) {
	int tmpoff = 0;
	uint32_t tmp_ophdrsize = this->dynamic_serialize_ophdr(dynamic_data);
	tmpoff += tmp_ophdrsize;
	uint32_t tmp_valsize = this->_val.dynamic_serialize_large(dynamic_data, tmpoff);
	tmpoff += tmp_valsize;
	dynamic_data.dynamic_memcpy(tmpoff, (char *)&this->_stat, sizeof(bool));
	tmpoff += sizeof(bool);
	uint16_t bigendian_nodeidx_foreval = htons(this->_nodeidx_foreval);
	dynamic_data.dynamic_memcpy(tmpoff, (char*)&bigendian_nodeidx_foreval, sizeof(uint16_t));
	tmpoff += sizeof(uint16_t);
	tmpoff += Packet<key_t>::get_stat_padding_bytes(this->_methodid);
	uint32_t bigendian_spineload = htonl(this->_spineload);
	dynamic_data.dynamic_memcpy(tmpoff, (char *)&bigendian_spineload, sizeof(uint32_t));
	tmpoff += sizeof(uint32_t);
	uint32_t bigendian_leafload = htonl(this->_leafload);
	dynamic_data.dynamic_memcpy(tmpoff, (char *)&bigendian_leafload, sizeof(uint32_t));
	tmpoff += sizeof(uint32_t);
	return tmpoff;
}

template<class key_t, class val_t>
uint32_t GetResponseLargevalue<key_t, val_t>::serialize(char * const data, uint32_t max_size) {
	uint32_t my_size = this->size();
	INVARIANT(max_size >= my_size);
	char *begin = data;
	uint32_t tmp_ophdrsize = this->serialize_ophdr(begin, max_size);
	begin += tmp_ophdrsize;
	uint32_t tmp_valsize = this->_val.serialize_large(begin, max_size - uint32_t(begin - data));
	begin += tmp_valsize;
	memcpy(begin, (void *)&this->_stat, sizeof(bool));
	begin += sizeof(bool);
	uint16_t bigendian_nodeidx_foreval = htons(this->_nodeidx_foreval);
	memcpy(begin, (void *)&bigendian_nodeidx_foreval, sizeof(uint16_t));
	begin += sizeof(uint16_t);
	begin += Packet<key_t>::get_stat_padding_bytes(this->_methodid);
	uint32_t bigendian_spineload = htonl(this->_spineload);
	memcpy(begin, (void *)&bigendian_spineload, sizeof(uint32_t));
	begin += sizeof(uint32_t);
	uint32_t bigendian_leafload = htonl(this->_leafload);
	memcpy(begin, (void *)&bigendian_leafload, sizeof(uint32_t));
	begin += sizeof(uint32_t);
	return uint32_t(begin - data);
}

template<class key_t, class val_t>
void GetResponseLargevalue<key_t, val_t>::deserialize(const char * data, uint32_t recv_size) {
	uint32_t my_size = this->size();
	INVARIANT(my_size <= recv_size);
	const char *begin = data;
	uint32_t tmp_ophdrsize = this->deserialize_ophdr(begin, recv_size);
	begin += tmp_ophdrsize;
	uint32_t tmp_valsize = this->_val.deserialize_large(begin, recv_size - uint32_t(begin - data));
	begin += tmp_valsize;
	memcpy((void *)&this->_stat, begin, sizeof(bool));
	begin += sizeof(bool);
	memcpy(&this->_nodeidx_foreval, begin, sizeof(uint16_t));
	this->_nodeidx_foreval = ntohs(this->_nodeidx_foreval);
	begin += sizeof(uint16_t);
	begin += Packet<key_t>::get_stat_padding_bytes(this->_methodid);
	memcpy(&this->_spineload, begin, sizeof(uint32_t));
	this->_spineload = ntohl(this->_spineload);
	begin += sizeof(uint32_t);
	memcpy(&this->_leafload, begin, sizeof(uint32_t));
	this->_leafload = ntohl(this->_leafload);
	begin += sizeof(uint32_t);
}

// GetResponseLargevalueServer (value must > 128B)

template<class key_t, class val_t>
GetResponseLargevalueServer<key_t, val_t>::GetResponseLargevalueServer(method_t methodid, key_t key, val_t val, bool stat, uint16_t nodeidx_foreval) 
	: GetResponseLargevalue<key_t, val_t>(methodid, key, val, stat, nodeidx_foreval)
{	
	INVARIANT(!Packet<key_t>::is_singleswitch(methodid));
	this->_type = static_cast<optype_t>(packet_type_t::GETRES_LARGEVALUE_SERVER);
}

template<class key_t, class val_t>
GetResponseLargevalueServer<key_t, val_t>::GetResponseLargevalueServer(method_t methodid, switchidx_t spineswitchidx, switchidx_t leafswitchidx, key_t key, val_t val, bool stat, uint16_t nodeidx_foreval, uint32_t spineload, uint32_t leafload) 
	: GetResponseLargevalue<key_t, val_t>(methodid, key, val, stat, nodeidx_foreval)
{	
	INVARIANT(methodid == DISTCACHE_ID);
	this->_type = static_cast<optype_t>(packet_type_t::GETRES_LARGEVALUE_SERVER);
	this->_spineswitchidx = spineswitchidx;
	this->_leafswitchidx = leafswitchidx;
	this->_spineload = spineload;
	this->_leafload = leafload;
}

template<class key_t, class val_t>
GetResponseLargevalueServer<key_t, val_t>::GetResponseLargevalueServer(method_t methodid, const char * data, uint32_t recv_size) {
	INVARIANT(!Packet<key_t>::is_singleswitch(methodid));
	this->_methodid = methodid;
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::GETRES_LARGEVALUE_SERVER);
	INVARIANT(this->_val.val_length > val_t::SWITCH_MAX_VALLEN);
}

template<class key_t, class val_t>
size_t GetResponseLargevalueServer<key_t, val_t>::get_frag_hdrsize(method_t methodid) {
	return Packet<key_t>::get_ophdrsize(methodid); // op_hdr
}

// For key being evicted

// GetRequestBeingevicted

template<class key_t>
GetRequestBeingevicted<key_t>::GetRequestBeingevicted()
	: GetRequest<key_t>()
{
}

template<class key_t>
GetRequestBeingevicted<key_t>::GetRequestBeingevicted(method_t methodid, const char *data, uint32_t recv_size)
{
	INVARIANT(methodid == FARREACH_ID || methodid == DISTFARREACH_ID);
	this->_methodid = methodid;
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::GETREQ_BEINGEVICTED);
}

template<class key_t>
uint32_t GetRequestBeingevicted<key_t>::serialize(char * const data, uint32_t max_size)
{
	COUT_N_EXIT("Invalid invoke of serialize for GetRequestBeingevicted");
}

// PutRequestSeqBeingevicted (value must <= 128B)

template<class key_t, class val_t>
PutRequestSeqBeingevicted<key_t, val_t>::PutRequestSeqBeingevicted(method_t methodid, const char * data, uint32_t recv_size)
{
	INVARIANT(methodid == FARREACH_ID || methodid == DISTFARREACH_ID);
	this->_methodid = methodid;
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::PUTREQ_SEQ_BEINGEVICTED);
	INVARIANT(this->_val.val_length <= val_t::SWITCH_MAX_VALLEN)
	INVARIANT(this->_seq >= 0);
}

template<class key_t, class val_t>
uint32_t PutRequestSeqBeingevicted<key_t, val_t>::serialize(char * const data, uint32_t max_size)
{
	COUT_N_EXIT("Invalid invoke of serialize for PutRequestSeqBeingevicted");
}

// PutRequestSeqCase3Beingevicted (value must <= 128B)

template<class key_t, class val_t>
PutRequestSeqCase3Beingevicted<key_t, val_t>::PutRequestSeqCase3Beingevicted(method_t methodid, const char * data, uint32_t recv_size)
{
	INVARIANT(methodid == FARREACH_ID || methodid == DISTFARREACH_ID);
	this->_methodid = methodid;
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::PUTREQ_SEQ_CASE3_BEINGEVICTED);
	INVARIANT(this->_val.val_length <= val_t::SWITCH_MAX_VALLEN)
	INVARIANT(this->_seq >= 0);
}

template<class key_t, class val_t>
uint32_t PutRequestSeqCase3Beingevicted<key_t, val_t>::serialize(char * const data, uint32_t max_size)
{
	COUT_N_EXIT("Invalid invoke of serialize for PutRequestSeqCase3Beingevicted");
}

// DelRequestSeqBeingevicted

template<class key_t>
DelRequestSeqBeingevicted<key_t>::DelRequestSeqBeingevicted(method_t methodid, const char * data, uint32_t recv_size)
{
	INVARIANT(methodid == FARREACH_ID || methodid == DISTFARREACH_ID);
	this->_methodid = methodid;
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::DELREQ_SEQ_BEINGEVICTED);
	INVARIANT(this->_seq >= 0);
}

template<class key_t>
uint32_t DelRequestSeqBeingevicted<key_t>::serialize(char * const data, uint32_t max_size)
{
	COUT_N_EXIT("Invalid invoke of serialize for DelRequestSeqBeingevicted");
}

// DelRequestSeqCase3Beingevicted

template<class key_t>
DelRequestSeqCase3Beingevicted<key_t>::DelRequestSeqCase3Beingevicted(method_t methodid, const char * data, uint32_t recv_size)
{
	INVARIANT(methodid == FARREACH_ID || methodid == DISTFARREACH_ID);
	this->_methodid = methodid;
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::DELREQ_SEQ_CASE3_BEINGEVICTED);
	INVARIANT(this->_seq >= 0);
}

template<class key_t>
uint32_t DelRequestSeqCase3Beingevicted<key_t>::serialize(char * const data, uint32_t max_size)
{
	COUT_N_EXIT("Invalid invoke of serialize for DelRequestSeqCase3Beingevicted");
}

// PutRequestLargevalueSeqBeingevicted (value must > 128B)

template<class key_t, class val_t>
PutRequestLargevalueSeqBeingevicted<key_t, val_t>::PutRequestLargevalueSeqBeingevicted(method_t methodid, key_t key, val_t val, uint32_t seq, uint16_t client_logical_idx, uint32_t fragseq) 
	: PutRequestLargevalueSeq<key_t, val_t>(methodid, key, val, seq, client_logical_idx, fragseq)
{	
	INVARIANT(methodid == FARREACH_ID || methodid == DISTFARREACH_ID);
	this->_type = static_cast<optype_t>(packet_type_t::PUTREQ_LARGEVALUE_SEQ_BEINGEVICTED);
	INVARIANT(this->_val.val_length > val_t::SWITCH_MAX_VALLEN);
}

template<class key_t, class val_t>
PutRequestLargevalueSeqBeingevicted<key_t, val_t>::PutRequestLargevalueSeqBeingevicted(method_t methodid, const char * data, uint32_t recv_size) {
	INVARIANT(methodid == FARREACH_ID || methodid == DISTFARREACH_ID);
	this->_methodid = methodid;
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::PUTREQ_LARGEVALUE_SEQ_BEINGEVICTED);
	INVARIANT(this->_val.val_length > val_t::SWITCH_MAX_VALLEN);
}

template<class key_t, class val_t>
size_t PutRequestLargevalueSeqBeingevicted<key_t, val_t>::get_frag_hdrsize(method_t methodid) {
	return Packet<key_t>::get_ophdrsize(methodid) + sizeof(optype_t) + sizeof(uint32_t) + sizeof(uint16_t) + sizeof(uint32_t); // op_hdr + shadowtype_hdr + seq_hdr + client_logical_idx + fragseq
}

// PutRequestLargevalueSeqCase3Beingevicted (value must > 128B)

template<class key_t, class val_t>
PutRequestLargevalueSeqCase3Beingevicted<key_t, val_t>::PutRequestLargevalueSeqCase3Beingevicted(method_t methodid, key_t key, val_t val, uint32_t seq, uint16_t client_logical_idx, uint32_t fragseq) 
	: PutRequestLargevalueSeq<key_t, val_t>(methodid, key, val, seq, client_logical_idx, fragseq)
{	
	INVARIANT(methodid == FARREACH_ID || methodid == DISTFARREACH_ID);
	this->_type = static_cast<optype_t>(packet_type_t::PUTREQ_LARGEVALUE_SEQ_CASE3_BEINGEVICTED);
	INVARIANT(this->_val.val_length > val_t::SWITCH_MAX_VALLEN);
}

template<class key_t, class val_t>
PutRequestLargevalueSeqCase3Beingevicted<key_t, val_t>::PutRequestLargevalueSeqCase3Beingevicted(method_t methodid, const char * data, uint32_t recv_size) {
	INVARIANT(methodid == FARREACH_ID || methodid == DISTFARREACH_ID);
	this->_methodid = methodid;
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::PUTREQ_LARGEVALUE_SEQ_CASE3_BEINGEVICTED);
	INVARIANT(this->_val.val_length > val_t::SWITCH_MAX_VALLEN);
}

template<class key_t, class val_t>
size_t PutRequestLargevalueSeqCase3Beingevicted<key_t, val_t>::get_frag_hdrsize(method_t methodid) {
	return Packet<key_t>::get_ophdrsize(methodid) + sizeof(optype_t) + sizeof(uint32_t) + sizeof(uint16_t) + sizeof(uint32_t); // op_hdr + shadowtype_hdr + seq_hdr + client_logical_idx + fragseq
}

// NetcacheCachePopAckNLatest (default value length must = 0B; only used in end-hosts)

template<class key_t, class val_t>
NetcacheCachePopAckNLatest<key_t, val_t>::NetcacheCachePopAckNLatest(method_t methodid, key_t key, uint32_t seq, bool stat, uint16_t serveridx)
	: NetcacheCachePopAck<key_t, val_t>(methodid, key, val_t(), seq, stat, serveridx)
{
	INVARIANT(methodid == NETCACHE_ID);
	this->_type = static_cast<optype_t>(PacketType::NETCACHE_CACHE_POP_ACK_NLATEST);
	INVARIANT(this->_val.val_length == 0);
	INVARIANT(seq >= 0);
	INVARIANT(serveridx >= 0);
}

template<class key_t, class val_t>
NetcacheCachePopAckNLatest<key_t, val_t>::NetcacheCachePopAckNLatest(method_t methodid, const char * data, uint32_t recv_size) {
	INVARIANT(methodid == NETCACHE_ID);
	this->_methodid = methodid;
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::NETCACHE_CACHE_POP_ACK_NLATEST);
	INVARIANT(this->_val.val_length == 0);
	INVARIANT(this->_seq >= 0);
	INVARIANT(this->_serveridx >= 0);
}

// NetcacheCachePopInswitchNLatest (default value length must = 0B)

template<class key_t, class val_t>
NetcacheCachePopInswitchNLatest<key_t, val_t>::NetcacheCachePopInswitchNLatest(method_t methodid, key_t key, uint32_t seq, uint16_t freeidx, bool stat)
	: CachePopInswitch<key_t, val_t>(methodid, key, val_t(), seq, freeidx, stat)
{
	INVARIANT(methodid == NETCACHE_ID);
	this->_type = static_cast<optype_t>(PacketType::NETCACHE_CACHE_POP_INSWITCH_NLATEST);
	INVARIANT(this->_val.val_length == 0);
	INVARIANT(seq >= 0);
	INVARIANT(freeidx >= 0);
}

// APIs
static uint32_t serialize_packet_type(optype_t type, char * data, uint32_t maxsize) {
	INVARIANT(maxsize >= sizeof(optype_t));
	uint16_t bigendian_type = htons(type);
	memcpy(data, (void *)&bigendian_type, sizeof(optype_t));
	return sizeof(optype_t);
}

static uint32_t dynamic_serialize_packet_type(optype_t type, dynamic_array_t &dynamic_data, int off) {
	uint16_t bigendian_type = htons(type);
	dynamic_data.dynamic_memcpy(off, (char *)&bigendian_type, sizeof(optype_t));
	return sizeof(optype_t);
}

static packet_type_t get_packet_type(const char * data, uint32_t recv_size) {
	INVARIANT(recv_size >= sizeof(optype_t));
	optype_t tmp;
	memcpy((void *)&tmp, data, sizeof(optype_t));
	tmp = ntohs(tmp);
	return static_cast<packet_type_t>(tmp);
}

static uint32_t deserialize_packet_type(optype_t &type, const char * data, uint32_t recvsize) {
	INVARIANT(recvsize >= sizeof(optype_t));
	memcpy(&type, data, sizeof(optype_t));
	type = ntohs(type);
	return sizeof(optype_t);
}

static uint32_t serialize_switchidx(switchidx_t switchidx, char * data, uint32_t maxsize) {
	INVARIANT(maxsize >= sizeof(switchidx_t));
	uint16_t bigendian_switchidx = htons(switchidx);
	memcpy(data, (void *)&bigendian_switchidx, sizeof(switchidx_t));
	return sizeof(switchidx_t);
}

static uint32_t dynamic_serialize_switchidx(switchidx_t switchidx, dynamic_array_t &dynamic_data, int off) {
	uint16_t bigendian_switchidx = htons(switchidx);
	dynamic_data.dynamic_memcpy(off, (char *)&bigendian_switchidx, sizeof(switchidx_t));
	return sizeof(switchidx_t);
}

static uint32_t deserialize_switchidx(switchidx_t &switchidx, const char * data, uint32_t recvsize) {
	INVARIANT(recvsize >= sizeof(switchidx_t));
	memcpy(&switchidx, data, sizeof(switchidx_t));
	switchidx = ntohs(switchidx);
	return sizeof(switchidx_t);
}







static netreach_key_t get_packet_key(method_t methodid, const char * data, uint32_t recvsize) {
	netreach_key_t tmpkey;
	int tmp_keyoff = Packet<key_t>::get_ophdrsize(methodid) - sizeof(netreach_key_t);
	const char *begin = data + tmp_keyoff;
	tmpkey.deserialize(begin, recvsize - tmp_keyoff);
	return tmpkey;
}

static bool is_same_optype(packet_type_t type1, packet_type_t type2) {
	if (type1 == packet_type_t::PUTREQ_LARGEVALUE_SEQ || type1 == packet_type_t::PUTREQ_LARGEVALUE_SEQ_CACHED || type1 == packet_type_t::PUTREQ_LARGEVALUE_SEQ_CASE3 || type1 == packet_type_t::PUTREQ_LARGEVALUE_SEQ_BEINGEVICTED || type1 == packet_type_t::PUTREQ_LARGEVALUE_SEQ_CASE3_BEINGEVICTED) {
		if (type2 == packet_type_t::PUTREQ_LARGEVALUE_SEQ || type2 == packet_type_t::PUTREQ_LARGEVALUE_SEQ_CACHED || type2 == packet_type_t::PUTREQ_LARGEVALUE_SEQ_CASE3 || type2 == packet_type_t::PUTREQ_LARGEVALUE_SEQ_BEINGEVICTED || type2 == packet_type_t::PUTREQ_LARGEVALUE_SEQ_CASE3_BEINGEVICTED) {
			return true;
		}
	}
	return type1 == type2;
}

// Util APIs for large value

static size_t get_frag_hdrsize(method_t methodid, packet_type_t type) {
	size_t frag_hdrsize = 0;
	if (type == packet_type_t::PUTREQ_LARGEVALUE) {
		frag_hdrsize = PutRequestLargevalue<netreach_key_t, val_t>::get_frag_hdrsize(methodid);
	}
	else if (type == packet_type_t::PUTREQ_LARGEVALUE_SEQ) {
		frag_hdrsize = PutRequestLargevalueSeq<netreach_key_t, val_t>::get_frag_hdrsize(methodid);
	}
	else if (type == packet_type_t::PUTREQ_LARGEVALUE_SEQ_CACHED) {
		frag_hdrsize = PutRequestLargevalueSeqCached<netreach_key_t, val_t>::get_frag_hdrsize(methodid);
	}
	else if (type == packet_type_t::PUTREQ_LARGEVALUE_SEQ_CASE3) {
		frag_hdrsize = PutRequestLargevalueSeqCase3<netreach_key_t, val_t>::get_frag_hdrsize(methodid);
	}
	else if (type == packet_type_t::PUTREQ_LARGEVALUE_SEQ_BEINGEVICTED) {
		frag_hdrsize = PutRequestLargevalueSeqBeingevicted<netreach_key_t, val_t>::get_frag_hdrsize(methodid);
	}
	else if (type == packet_type_t::PUTREQ_LARGEVALUE_SEQ_CASE3_BEINGEVICTED) {
		frag_hdrsize = PutRequestLargevalueSeqCase3Beingevicted<netreach_key_t, val_t>::get_frag_hdrsize(methodid);
	}
	else if (type == packet_type_t::GETRES_LARGEVALUE) {
		frag_hdrsize = GetResponseLargevalue<netreach_key_t, val_t>::get_frag_hdrsize(methodid);
	}
	else if (type == packet_type_t::LOADREQ) {
		frag_hdrsize = LoadRequest<netreach_key_t, val_t>::get_frag_hdrsize(methodid);
	}
	else {
		printf("[WARNING] get_frag_hdrsize: no matched optype: %x\n", type);
	}
	return frag_hdrsize;
}

// NOTE: frag_maxsize = frag_hdrsize of sent type + fragidx&fragnum + payload
// NOTE: frag_totalsize = frag_hdrsize of received type + fragidx&fragnum + payload = frag_hdrsize of sent type + extra packet headers added by switch + fragidx&fragnum + payload
static size_t get_frag_totalsize(method_t methodid, packet_type_t type, size_t frag_maxsize) {
	if (type == packet_type_t::PUTREQ_LARGEVALUE_SEQ || type == packet_type_t::PUTREQ_LARGEVALUE_SEQ_CACHED || type == packet_type_t::PUTREQ_LARGEVALUE_SEQ_CASE3 || type == packet_type_t::PUTREQ_LARGEVALUE_SEQ_BEINGEVICTED || type == packet_type_t::PUTREQ_LARGEVALUE_SEQ_CASE3_BEINGEVICTED) {
		int sent_frag_hdrsize = int(get_frag_hdrsize(methodid, packet_type_t::PUTREQ_LARGEVALUE));
		int received_frag_hdrsize = int(get_frag_hdrsize(methodid, packet_type_t::PUTREQ_LARGEVALUE_SEQ));
		int extra_frag_hdrsize = received_frag_hdrsize - sent_frag_hdrsize; // NOTE: may be negative

		int result = int(frag_maxsize) + extra_frag_hdrsize;
		INVARIANT(result >= 0);
		return size_t(result);
	}
	return frag_maxsize;
}

static uint16_t get_packet_clientlogicalidx(method_t methodid, const char * data, uint32_t recvsize) {
	packet_type_t tmp_optype = get_packet_type(data, recvsize);
	size_t tmp_ophdrsize = Packet<key_t>::get_ophdrsize(methodid);
	uint32_t prevbytes = 0;
	if (tmp_optype == packet_type_t::PUTREQ_LARGEVALUE) {
		prevbytes = tmp_ophdrsize; // op_hdr
	}
	else if (tmp_optype == packet_type_t::PUTREQ_LARGEVALUE_SEQ || tmp_optype == packet_type_t::PUTREQ_LARGEVALUE_SEQ_CACHED || tmp_optype == packet_type_t::PUTREQ_LARGEVALUE_SEQ_CASE3 || tmp_optype == packet_type_t::PUTREQ_LARGEVALUE_SEQ_BEINGEVICTED || tmp_optype == packet_type_t::PUTREQ_LARGEVALUE_SEQ_CASE3_BEINGEVICTED) {
		prevbytes = tmp_ophdrsize + sizeof(optype_t) + sizeof(uint32_t); // op_hdr + shadowtype + seq
	}
	else if (tmp_optype == packet_type_t::LOADREQ) {
		prevbytes = tmp_ophdrsize; // op_hdr
	}
	else {
		printf("[ERROR] invalid packet type for get_packet_clientlogicalidx: %x\n", optype_t(tmp_optype));
		exit(-1);
	}
	
	INVARIANT(recvsize >= prevbytes + sizeof(uint16_t));
	uint16_t tmp_clientlogicalidx = 0;
	memcpy(&tmp_clientlogicalidx, data + prevbytes, sizeof(uint16_t));
	tmp_clientlogicalidx = ntohs(tmp_clientlogicalidx);
	return tmp_clientlogicalidx;
}

static uint32_t get_packet_fragseq(method_t methodid, const char * data, uint32_t recvsize) {
	packet_type_t tmp_optype = get_packet_type(data, recvsize);
	size_t tmp_ophdrsize = Packet<key_t>::get_ophdrsize(methodid);
	uint32_t prevbytes = 0;
	if (tmp_optype == packet_type_t::PUTREQ_LARGEVALUE) {
		prevbytes = tmp_ophdrsize + sizeof(uint16_t); // op_hdr + client_logical_idx
	}
	else if (tmp_optype == packet_type_t::PUTREQ_LARGEVALUE_SEQ || tmp_optype == packet_type_t::PUTREQ_LARGEVALUE_SEQ_CACHED || tmp_optype == packet_type_t::PUTREQ_LARGEVALUE_SEQ_CASE3 || tmp_optype == packet_type_t::PUTREQ_LARGEVALUE_SEQ_BEINGEVICTED || tmp_optype == packet_type_t::PUTREQ_LARGEVALUE_SEQ_CASE3_BEINGEVICTED) {
		prevbytes = tmp_ophdrsize + sizeof(optype_t) + sizeof(uint32_t) + sizeof(uint16_t); // op_hdr + shadowtype + seq + client_logical_idx
	}
	else if (tmp_optype == packet_type_t::LOADREQ) {
		prevbytes = tmp_ophdrsize + sizeof(uint16_t); // op_hdr + client_logical_idx
	}
	else {
		printf("[ERROR] invalid packet type for get_packet_clientlogicalidx: %x\n", optype_t(tmp_optype));
		exit(-1);
	}
	
	INVARIANT(recvsize >= prevbytes + sizeof(uint32_t));
	uint32_t tmp_fragseq = 0;
	memcpy(&tmp_fragseq, data + prevbytes, sizeof(uint32_t));
	tmp_fragseq = ntohl(tmp_fragseq);
	return tmp_fragseq;
}

// whether the packet is large to be processed by udprecvlarge_ipfrag
static bool is_packet_with_largevalue(packet_type_t type) {
	for (uint32_t tmp_optype_for_udprecvlarge_ipfrag_idx = 0; tmp_optype_for_udprecvlarge_ipfrag_idx < optype_for_udprecvlarge_ipfrag_num; tmp_optype_for_udprecvlarge_ipfrag_idx++) {
		if (type == optype_for_udprecvlarge_ipfrag_list[tmp_optype_for_udprecvlarge_ipfrag_idx]) {
			return true;
		}
	}
	return false;
}

// whether the large packet is sent to server
static bool is_packet_with_clientlogicalidx(packet_type_t type) {
	for (uint32_t tmp_optype_with_clientlogicalidx_idx = 0; tmp_optype_with_clientlogicalidx_idx < optype_with_clientlogicalidx_num; tmp_optype_with_clientlogicalidx_idx++) {
		if (type == optype_with_clientlogicalidx_list[tmp_optype_with_clientlogicalidx_idx]) {
			return true;
		}
	}
	return false;
}

#endif
