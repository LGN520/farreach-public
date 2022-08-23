#ifndef PACKET_FORMAT_IMPL_H
#define PACKET_FORMAT_IMPL_H

#include "packet_format.h"
#include <arpa/inet.h>

// Packet
template<class key_t>
Packet<key_t>::Packet() 
	: _type(static_cast<optype_t>(0)), _spineswitchidx(0), _leafswitchidx(0), _key(key_t::min())
{
}

template<class key_t>
Packet<key_t>::Packet(packet_type_t type, switchidx_t spineswitchidx, switchidx_t leafswitchidx, key_t key)
	: _type(static_cast<optype_t>(type)), _spineswitchidx(spineswitchidx), _leafswitchidx(leafswitchidx), _key(key)
{
}

template<class key_t>
packet_type_t Packet<key_t>::type() const {
	return packet_type_t(_type);
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
key_t Packet<key_t>::key() const {
	return _key;
}

template<class key_t>
uint32_t Packet<key_t>::serialize_ophdr(char * const data, uint32_t max_size) {
	uint32_t ophdr_size = sizeof(optype_t) + sizeof(switchidx_t) + sizeof(switchidx_t) + sizeof(key_t);
	INVARIANT(max_size >= ophdr_size);

	char *begin = data;
	uint32_t tmp_typesize = serialize_packet_type(this->_type, begin, max_size);
	begin += tmp_typesize;
	uint32_t tmp_spineswitchidxsize = serialize_switchidx(this->_spineswitchidx, begin, max_size - tmp_typesize);
	begin += tmp_spineswitchidxsize;
	uint32_t tmp_leafswitchidxsize = serialize_switchidx(this->_leafswitchidx, begin, max_size - tmp_typesize - tmp_spineswitchidxsize);
	begin += tmp_leafswitchidxsize;
	uint32_t tmp_keysize = this->_key.serialize(begin, max_size - tmp_typesize - tmp_spineswitchidxsize - tmp_leafswitchidxsize);
	begin += tmp_keysize;
	return tmp_typesize + tmp_spineswitchidxsize + tmp_leafswitchidxsize + tmp_keysize;
}

template<class key_t>
uint32_t Packet<key_t>::dynamic_serialize_ophdr(dynamic_array_t &dynamic_data) {
	int tmpoff = 0;
	uint32_t tmp_typesize = dynamic_serialize_packet_type(this->_type, dynamic_data, tmpoff);
	tmpoff += tmp_typesize;
	uint32_t tmp_spineswitchidxsize = dynamic_serialize_switchidx(this->_spineswitchidx, dynamic_data, tmpoff);
	tmpoff += tmp_spineswitchidxsize;
	uint32_t tmp_leafswitchidxsize = dynamic_serialize_switchidx(this->_leafswitchidx, dynamic_data, tmpoff);
	tmpoff += tmp_leafswitchidxsize;
	uint32_t tmp_keysize = this->_key.dynamic_serialize(dynamic_data, tmpoff);
	tmpoff += tmp_keysize;
	return tmp_typesize + tmp_spineswitchidxsize + tmp_leafswitchidxsize + tmp_keysize;
}

template<class key_t>
uint32_t Packet<key_t>::deserialize_ophdr(const char * data, uint32_t recv_size) {
	uint32_t ophdr_size = sizeof(optype_t) + sizeof(switchidx_t) + sizeof(switchidx_t) + sizeof(key_t);
	INVARIANT(recv_size >= ophdr_size);

	const char *begin = data;
	uint32_t tmp_typesize = deserialize_packet_type(this->_type, begin, recv_size);
	begin += tmp_typesize;
	uint32_t tmp_spineswitchidxsize = deserialize_switchidx(this->_spineswitchidx, begin, recv_size - tmp_typesize);
	begin += tmp_spineswitchidxsize;
	uint32_t tmp_leafswitchidxsize = deserialize_switchidx(this->_leafswitchidx, begin, recv_size - tmp_typesize - tmp_spineswitchidxsize);
	begin += tmp_leafswitchidxsize;
	uint32_t tmp_keysize = this->_key.deserialize(begin, recv_size - tmp_typesize - tmp_spineswitchidxsize - tmp_leafswitchidxsize);
	begin += tmp_keysize;
	return tmp_typesize + tmp_spineswitchidxsize + tmp_leafswitchidxsize + tmp_keysize;
}

template<class key_t>
uint32_t Packet<key_t>::get_ophdrsize() {
	return sizeof(optype_t) + sizeof(switchidx_t) + sizeof(switchidx_t) + sizeof(key_t);
}


// GetRequest

template<class key_t>
GetRequest<key_t>::GetRequest()
	: Packet<key_t>(), _spineload(0), _leafload(0)
{
}

template<class key_t>
GetRequest<key_t>::GetRequest(switchidx_t spineswitchidx, switchidx_t leafswitchidx, key_t key)
	: Packet<key_t>(packet_type_t::GETREQ, spineswitchidx, leafswitchidx, key), _spineload(0), _leafload(0)
{
}

template<class key_t>
GetRequest<key_t>::GetRequest(const char * data, uint32_t recv_size) {
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
	return Packet<key_t>::get_ophdrsize() + sizeof(optype_t) + sizeof(uint32_t) + sizeof(uint32_t);
}

template<class key_t>
uint32_t GetRequest<key_t>::serialize(char * const data, uint32_t max_size) {
	uint32_t my_size = this->size();
	INVARIANT(max_size >= my_size);
	char *begin = data;
	uint32_t tmp_ophdrsize = this->serialize_ophdr(begin, max_size);
	begin += tmp_ophdrsize;
	uint32_t tmp_shadowtypesize = serialize_packet_type(this->_type, begin, max_size - tmp_ophdrsize); // shadowtype
	begin += tmp_shadowtypesize;
	uint32_t bigendian_spineload = htonl(this->_spineload);
	memcpy(begin, (void *)&bigendian_spineload, sizeof(uint32_t));
	begin += sizeof(uint32_t);
	uint32_t bigendian_leafload = htonl(this->_leafload);
	memcpy(begin, (void *)&bigendian_leafload, sizeof(uint32_t));
	begin += sizeof(uint32_t);
	return tmp_ophdrsize + tmp_shadowtypesize + sizeof(uint32_t) + sizeof(uint32_t);
}

template<class key_t>
void GetRequest<key_t>::deserialize(const char * data, uint32_t recv_size) {
	uint32_t my_size = this->size();
	INVARIANT(my_size <= recv_size);
	const char *begin = data;
	uint32_t tmp_ophdrsize = this->deserialize_ophdr(begin, recv_size);
	begin += tmp_ophdrsize;
	begin += sizeof(optype_t); // shadowtype
	memcpy(&this->_spineload, begin, sizeof(uint32_t));
	this->_spineload = ntohl(this->_spineload);
	begin += sizeof(uint32_t);
	memcpy(&this->_leafload, begin, sizeof(uint32_t));
	this->_leafload = ntohl(this->_leafload);
	begin += sizeof(uint32_t);
}

// PutRequest (value must <= 128B)

template<class key_t, class val_t>
PutRequest<key_t, val_t>::PutRequest()
	: Packet<key_t>(), _val()
{
}

template<class key_t, class val_t>
PutRequest<key_t, val_t>::PutRequest(key_t key, val_t val) 
	: Packet<key_t>(PacketType::PUTREQ, 0, 0, key), _val(val)
{	
	INVARIANT(this->_val.val_length <= val_t::SWITCH_MAX_VALLEN);
}

template<class key_t, class val_t>
PutRequest<key_t, val_t>::PutRequest(const char * data, uint32_t recv_size) {
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
	return Packet<key_t>::get_ophdrsize() + sizeof(uint16_t) + val_t::MAX_VALLEN + sizeof(optype_t);
}

template<class key_t, class val_t>
uint32_t PutRequest<key_t, val_t>::serialize(char * const data, uint32_t max_size) {
	//uint32_t my_size = this->size();
	//INVARIANT(max_size >= my_size);
	char *begin = data;
	uint32_t tmp_ophdrsize = this->serialize_ophdr(begin, max_size);
	begin += tmp_ophdrsize;
	uint32_t tmp_valsize = this->_val.serialize(begin, max_size - tmp_ophdrsize);
	begin += tmp_valsize;
	uint32_t tmp_shadowtypesize = serialize_packet_type(this->_type, begin, max_size - tmp_ophdrsize - tmp_valsize); // shadowtype
	return tmp_ophdrsize + tmp_valsize + tmp_shadowtypesize;
	//begin += tmp_shadowtypesize;
}

template<class key_t, class val_t>
void PutRequest<key_t, val_t>::deserialize(const char * data, uint32_t recv_size) {
	//uint32_t my_size = this->size();
	//INVARIANT(my_size == recv_size);
	const char *begin = data;
	uint32_t tmp_ophdrsize = this->deserialize_ophdr(begin, recv_size);
	begin += tmp_ophdrsize;
	uint32_t tmp_valsize = this->_val.deserialize(begin, recv_size-tmp_ophdrsize);
	UNUSED(tmp_valsize);
	// deserialize shadowtype
}

// DelRequest

template<class key_t>
DelRequest<key_t>::DelRequest()
	: Packet<key_t>()
{
}

template<class key_t>
DelRequest<key_t>::DelRequest(key_t key)
	: Packet<key_t>(packet_type_t::DELREQ, 0, 0, key)
{
}

template<class key_t>
DelRequest<key_t>::DelRequest(const char * data, uint32_t recv_size) {
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == packet_type_t::DELREQ);
}

template<class key_t>
uint32_t DelRequest<key_t>::size() {
	return Packet<key_t>::get_ophdrsize();
}

template<class key_t>
uint32_t DelRequest<key_t>::serialize(char * const data, uint32_t max_size) {
	uint32_t my_size = this->size();
	INVARIANT(max_size >= my_size);
	char *begin = data;
	uint32_t tmp_ophdrsize = this->serialize_ophdr(begin, max_size);
	begin += tmp_ophdrsize;
	return tmp_ophdrsize;
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
ScanRequest<key_t>::ScanRequest(key_t key, key_t endkey)
	: Packet<key_t>(packet_type_t::SCANREQ, 0, 0, key), _endkey(endkey)
{
}

template<class key_t>
ScanRequest<key_t>::ScanRequest(const char * data, uint32_t recv_size) {
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
	return Packet<key_t>::get_ophdrsize() + sizeof(key_t);// + sizeof(uint32_t);
}

template<class key_t>
uint32_t ScanRequest<key_t>::serialize(char * const data, uint32_t max_size) {
	uint32_t my_size = this->size();
	INVARIANT(max_size >= my_size);
	char *begin = data;
	uint32_t tmp_ophdrsize = this->serialize_ophdr(begin, max_size);
	begin += tmp_ophdrsize;
	uint32_t tmp_endkeysize = this->_endkey.serialize(begin, max_size - tmp_ophdrsize);
	//memcpy(begin, (void *)&this->_num, sizeof(uint32_t));
	return tmp_ophdrsize + tmp_endkeysize; // + sizeof(uint32_t);
}

template<class key_t>
void ScanRequest<key_t>::deserialize(const char * data, uint32_t recv_size) {
	uint32_t my_size = this->size();
	INVARIANT(my_size == recv_size);
	const char *begin = data;
	uint32_t tmp_ophdrsize = this->deserialize_ophdr(begin, recv_size);
	begin += tmp_ophdrsize;
	uint32_t tmp_endkeysize = this->_endkey.deserialize(begin, recv_size - tmp_ophdrsize);
	UNUSED(tmp_endkeysize);
	//begin += tmp_endkeysize;
	//memcpy((void *)&this->_num, begin, sizeof(uint32_t));
}


// GetResponse (value must <= 128B)

template<class key_t, class val_t>
GetResponse<key_t, val_t>::GetResponse()
	: Packet<key_t>(), _val(), _stat(false), _nodeidx_foreval(0), _spineload(0), _leafload(0)
{
}

template<class key_t, class val_t>
GetResponse<key_t, val_t>::GetResponse(key_t key, val_t val, bool stat, uint16_t nodeidx_foreval) 
	: Packet<key_t>(PacketType::GETRES, 0, 0, key), _val(val), _stat(stat), _nodeidx_foreval(nodeidx_foreval), _spineload(0), _leafload(0)
{	
	INVARIANT(this->_val.val_length <= val_t::SWITCH_MAX_VALLEN);
}

template<class key_t, class val_t>
GetResponse<key_t, val_t>::GetResponse(const char * data, uint32_t recv_size) {
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
	return Packet<key_t>::get_ophdrsize() + sizeof(uint16_t) + val_t::MAX_VALLEN + sizeof(optype_t) + sizeof(bool) + sizeof(uint16_t) + STAT_PADDING_BYTES + sizeof(uint32_t) + sizeof(uint32_t);
}

template<class key_t, class val_t>
uint32_t GetResponse<key_t, val_t>::serialize(char * const data, uint32_t max_size) {
	//uint32_t my_size = this->size();
	//INVARIANT(max_size >= my_size);
	char *begin = data;
	uint32_t tmp_ophdrsize = this->serialize_ophdr(begin, max_size);
	begin += tmp_ophdrsize;
	uint32_t tmp_valsize = this->_val.serialize(begin, max_size-tmp_ophdrsize);
	begin += tmp_valsize;
	uint32_t tmp_shadowtypesize = serialize_packet_type(this->_type, begin, max_size - tmp_ophdrsize - tmp_valsize); // shadowtype
	begin += tmp_shadowtypesize;
	memcpy(begin, (void *)&this->_stat, sizeof(bool));
	begin += sizeof(bool);
	uint16_t bigendian_nodeidx_foreval = htons(this->_nodeidx_foreval);
	memcpy(begin, (void *)&bigendian_nodeidx_foreval, sizeof(uint16_t));
	begin += sizeof(uint16_t);
	begin += STAT_PADDING_BYTES;
	uint32_t bigendian_spineload = htonl(this->_spineload);
	memcpy(begin, (void *)&bigendian_spineload, sizeof(uint32_t));
	begin += sizeof(uint32_t);
	uint32_t bigendian_leafload = htonl(this->_leafload);
	memcpy(begin, (void *)&bigendian_leafload, sizeof(uint32_t));
	begin += sizeof(uint32_t);
	return tmp_ophdrsize + tmp_valsize + tmp_shadowtypesize + sizeof(bool) + sizeof(uint16_t) + STAT_PADDING_BYTES + sizeof(uint32_t) + sizeof(uint32_t);
}

template<class key_t, class val_t>
void GetResponse<key_t, val_t>::deserialize(const char * data, uint32_t recv_size) {
	//uint32_t my_size = this->size();
	//INVARIANT(my_size == recv_size);
	const char *begin = data;
	uint32_t tmp_ophdrsize = this->deserialize_ophdr(begin, recv_size);
	begin += tmp_ophdrsize;
	uint32_t tmp_valsize = this->_val.deserialize(begin, recv_size - tmp_ophdrsize);
	begin += tmp_valsize;
	begin += sizeof(optype_t); // deserialize shadowtype
	memcpy((void *)&this->_stat, begin, sizeof(bool));
	begin += sizeof(bool);
	memcpy(&this->_nodeidx_foreval, begin, sizeof(uint16_t));
	this->_nodeidx_foreval = ntohs(this->_nodeidx_foreval);
	begin += sizeof(uint16_t);
	begin += STAT_PADDING_BYTES;
	memcpy(&this->_spineload, begin, sizeof(uint32_t));
	this->_spineload = ntohl(this->_spineload);
	begin += sizeof(uint32_t);
	memcpy(&this->_leafload, begin, sizeof(uint32_t));
	this->_leafload = ntohl(this->_leafload);
	begin += sizeof(uint32_t);
}

// GetResponseServer (value must <= 128B)

template<class key_t, class val_t>
GetResponseServer<key_t, val_t>::GetResponseServer(switchidx_t spineswitchidx, switchidx_t leafswitchidx, key_t key, val_t val, bool stat, uint16_t nodeidx_foreval, uint32_t spineload, uint32_t leafload) 
	: GetResponse<key_t, val_t>(key, val, stat, nodeidx_foreval)
{	
	this->_type = optype_t(packet_type_t::GETRES_SERVER);
	this->_spineswitchidx = spineswitchidx;
	this->_leafswitchidx = leafswitchidx;
	this->_spineload = spineload;
	this->_leafload = leafload;
}

// PutResponse (value must be any size)

template<class key_t>
PutResponse<key_t>::PutResponse(key_t key, bool stat, uint16_t nodeidx_foreval) 
	: Packet<key_t>(PacketType::PUTRES, 0, 0, key), _stat(stat), _nodeidx_foreval(nodeidx_foreval)
{	
}

template<class key_t>
PutResponse<key_t>::PutResponse(const char * data, uint32_t recv_size) {
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
	return Packet<key_t>::get_ophdrsize() + sizeof(optype_t) + sizeof(bool) + sizeof(uint16_t) + STAT_PADDING_BYTES;
}

template<class key_t>
uint32_t PutResponse<key_t>::serialize(char * const data, uint32_t max_size) {
	uint32_t my_size = this->size();
	INVARIANT(max_size >= my_size);
	char *begin = data;
	uint32_t tmp_ophdrsize = this->serialize_ophdr(begin, max_size);
	begin += tmp_ophdrsize;
	uint32_t tmp_shadowtypesize = serialize_packet_type(this->_type, begin, max_size - tmp_ophdrsize); // shadowtype
	begin += tmp_shadowtypesize;
	memcpy(begin, (void *)&this->_stat, sizeof(bool));
	begin += sizeof(bool);
	uint16_t bigendian_nodeidx_foreval = htons(this->_nodeidx_foreval);
	memcpy(begin, (void *)&bigendian_nodeidx_foreval, sizeof(uint16_t));
	begin += sizeof(uint16_t);
	begin += STAT_PADDING_BYTES;
	return tmp_ophdrsize + tmp_shadowtypesize + sizeof(bool) + sizeof(uint16_t) + STAT_PADDING_BYTES;
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
	begin += STAT_PADDING_BYTES;
}

// PutResponseServer

template<class key_t>
PutResponseServer<key_t>::PutResponseServer(key_t key, bool stat, uint16_t nodeidx_foreval) 
	: PutResponse<key_t>(key, stat, nodeidx_foreval)
{	
	this->_type = optype_t(packet_type_t::PUTRES_SERVER);
}

// DelResponse

template<class key_t>
DelResponse<key_t>::DelResponse(key_t key, bool stat, uint16_t nodeidx_foreval) 
	: Packet<key_t>(PacketType::DELRES, 0, 0, key), _stat(stat), _nodeidx_foreval(nodeidx_foreval)
{	
}

template<class key_t>
DelResponse<key_t>::DelResponse(const char * data, uint32_t recv_size) {
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
	return Packet<key_t>::get_ophdrsize() + sizeof(optype_t) + sizeof(bool) + sizeof(uint16_t) + STAT_PADDING_BYTES;
}

template<class key_t>
uint32_t DelResponse<key_t>::serialize(char * const data, uint32_t max_size) {
	uint32_t my_size = this->size();
	INVARIANT(max_size >= my_size);
	char *begin = data;
	uint32_t tmp_ophdrsize = this->serialize_ophdr(begin, max_size);
	begin += tmp_ophdrsize;
	uint32_t tmp_shadowtypesize = serialize_packet_type(this->_type, begin, max_size - tmp_ophdrsize); // shadowtype
	begin += tmp_shadowtypesize;
	memcpy(begin, (void *)&this->_stat, sizeof(bool));
	begin += sizeof(bool);
	uint16_t bigendian_nodeidx_foreval = htons(this->_nodeidx_foreval);
	memcpy(begin, (void *)&bigendian_nodeidx_foreval, sizeof(uint16_t));
	begin += sizeof(uint16_t);
	begin += STAT_PADDING_BYTES;
	return tmp_ophdrsize + tmp_shadowtypesize + sizeof(bool) + sizeof(uint16_t) + STAT_PADDING_BYTES;
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
	begin += STAT_PADDING_BYTES;
}

// DelResponseServer

template<class key_t>
DelResponseServer<key_t>::DelResponseServer(key_t key, bool stat, uint16_t nodeidx_foreval) 
	: DelResponse<key_t>(key, stat, nodeidx_foreval)
{	
	this->_type = optype_t(packet_type_t::DELRES_SERVER);
}

// ScanResponseSplit

/*template<class key_t, class val_t>
ScanResponseSplit<key_t, val_t>::ScanResponseSplit(key_t key, key_t endkey, uint32_t num, uint16_t cur_scanidx, uint16_t max_scannum, int32_t parinum, std::vector<std::pair<key_t, val_t>> pairs) 
	: ScanRequestSplit<key_t>(PacketType::SCANRES_SPLIT, key, endkey, num, cur_scanidx, max_scannum), _pairnum(pairnum)
{	
	INVARIANT(pairs.size() == num);
	this->_pairs.assign(pairs.begin(), pairs.end());
}*/
template<class key_t, class val_t>
ScanResponseSplit<key_t, val_t>::ScanResponseSplit(key_t key, key_t endkey, uint16_t cur_scanidx, uint16_t max_scannum, uint16_t cur_scanswitchidx, uint16_t max_scanswitchnum, uint16_t nodeidx_foreval, int snapshotid, int32_t pairnum, std::vector<std::pair<key_t, snapshot_record_t>> pairs) 
	: ScanRequestSplit<key_t>(key, endkey, cur_scanidx, max_scannum, cur_scanswitchidx, max_scanswitchnum), _nodeidx_foreval(nodeidx_foreval), _snapshotid(snapshotid), _pairnum(pairnum)
{	
	this->_type = static_cast<optype_t>(PacketType::SCANRES_SPLIT);
	INVARIANT(snapshotid >= 0);
	INVARIANT(pairnum == int32_t(pairs.size()));
	this->_pairs.assign(pairs.begin(), pairs.end());
}

template<class key_t, class val_t>
ScanResponseSplit<key_t, val_t>::ScanResponseSplit(const char * data, uint32_t recv_size) {
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
	return Packet<key_t>::get_ophdrsize() + sizeof(key_t) + SPLIT_PREV_BYTES + sizeof(uint16_t) + sizeof(uint16_t) + sizeof(uint16_t) + sizeof(uint16_t) + sizeof(uint16_t) + sizeof(int) + sizeof(int32_t); // ophdr + scanhdr.endkey + splithdr (isclone + globalserveridx + cur_scanidx, max_scannum, cur_scanswitchidx, max_scanswitchnum) + nodeidx_foreval + snapshotid + pairnum
}

template<class key_t, class val_t>
uint32_t ScanResponseSplit<key_t, val_t>::serialize(char * const data, uint32_t max_size) {
	uint32_t my_size = this->size();
	INVARIANT(max_size >= my_size);
	char *begin = data;
	uint32_t tmp_ophdrsize = this->serialize_ophdr(begin, max_size);
	begin += tmp_ophdrsize;
	uint32_t tmp_endkeysize = this->_endkey.serialize(begin, max_size - tmp_ophdrsize);
	begin += tmp_endkeysize;
	//memcpy(begin, (void *)&this->_num, sizeof(uint32_t));
	//begin += sizeof(uint32_t);
	memset(begin, 0, SPLIT_PREV_BYTES);
	begin += SPLIT_PREV_BYTES;
	uint16_t bigendian_cur_scanidx = htons(uint16_t(this->_cur_scanidx));
	memcpy(begin, (void *)&bigendian_cur_scanidx, sizeof(uint16_t));
	begin += sizeof(uint16_t);
	uint16_t bigendian_max_scannum = htons(uint16_t(this->_max_scannum));
	memcpy(begin, (void *)&bigendian_max_scannum, sizeof(uint16_t));
	begin += sizeof(uint16_t);
	uint16_t bigendian_cur_scanswitchidx = htons(uint16_t(this->_cur_scanswitchidx));
	memcpy(begin, (void *)&bigendian_cur_scanswitchidx, sizeof(uint16_t));
	begin += sizeof(uint16_t);
	uint16_t bigendian_max_scanswitchnum = htons(uint16_t(this->_max_scanswitchnum));
	memcpy(begin, (void *)&bigendian_max_scanswitchnum, sizeof(uint16_t));
	begin += sizeof(uint16_t);
	uint16_t bigendian_nodeidx_foreval = htons(this->_nodeidx_foreval);
	memcpy(begin, (void *)&bigendian_nodeidx_foreval, sizeof(uint16_t));
	begin += sizeof(uint16_t);
	memcpy(begin, (void *)&this->_snapshotid, sizeof(int)); // directly use little-endian
	begin += sizeof(int);

	uint32_t bigendian_pairnum = htonl(uint32_t(this->_pairnum));
	memcpy(begin, (void *)&bigendian_pairnum, sizeof(int32_t));
	begin += sizeof(int32_t);
	uint32_t totalsize = tmp_ophdrsize + tmp_endkeysize + SPLIT_PREV_BYTES + sizeof(uint16_t) + sizeof(uint16_t) + sizeof(uint16_t) + sizeof(int) + sizeof(int32_t);
	for (uint32_t pair_i = 0; pair_i < this->_pairs.size(); pair_i++) {
		uint32_t tmp_pair_keysize = this->_pairs[pair_i].first.serialize(begin, max_size - totalsize);
		begin += tmp_pair_keysize;
		totalsize += tmp_pair_keysize;
		uint32_t tmp_pair_valsize = this->_pairs[pair_i].second.val.serialize(begin, max_size - totalsize);
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
	dynamic_data.dynamic_memset(tmpoff, 0, SPLIT_PREV_BYTES);
	tmpoff += SPLIT_PREV_BYTES;
	uint16_t bigendian_cur_scanidx = htons(uint16_t(this->_cur_scanidx));
	dynamic_data.dynamic_memcpy(tmpoff, (char *)&bigendian_cur_scanidx, sizeof(uint16_t));
	tmpoff += sizeof(uint16_t);
	uint16_t bigendian_max_scannum = htons(uint16_t(this->_max_scannum));
	dynamic_data.dynamic_memcpy(tmpoff, (char *)&bigendian_max_scannum, sizeof(uint16_t));
	tmpoff += sizeof(uint16_t);
	uint16_t bigendian_cur_scanswitchidx = htons(uint16_t(this->_cur_scanswitchidx));
	dynamic_data.dynamic_memcpy(tmpoff, (char *)&bigendian_cur_scanswitchidx, sizeof(uint16_t));
	tmpoff += sizeof(uint16_t);
	uint16_t bigendian_max_scanswitchnum = htons(uint16_t(this->_max_scanswitchnum));
	dynamic_data.dynamic_memcpy(tmpoff, (char *)&bigendian_max_scanswitchnum, sizeof(uint16_t));
	tmpoff += sizeof(uint16_t);
	uint16_t bigendian_nodeidx_foreval = htons(this->_nodeidx_foreval);
	dynamic_data.dynamic_memcpy(tmpoff, (char *)&bigendian_nodeidx_foreval, sizeof(uint16_t));
	tmpoff += sizeof(uint16_t);
	dynamic_data.dynamic_memcpy(tmpoff, (char *)&this->_snapshotid, sizeof(int)); // directly use little-endian
	tmpoff += sizeof(int);

	uint32_t bigendian_pairnum = htonl(uint32_t(this->_pairnum));
	dynamic_data.dynamic_memcpy(tmpoff, (char *)&bigendian_pairnum, sizeof(int32_t));
	tmpoff += sizeof(int32_t);
	uint32_t totalsize = tmp_ophdrsize + tmp_endkeysize + SPLIT_PREV_BYTES + sizeof(uint16_t) + sizeof(uint16_t) + sizeof(uint16_t) + sizeof(uint16_t) + sizeof(uint16_t) + sizeof(int) + sizeof(int32_t);
	for (uint32_t pair_i = 0; pair_i < this->_pairs.size(); pair_i++) {
		uint32_t tmp_pair_keysize = this->_pairs[pair_i].first.dynamic_serialize(dynamic_data, tmpoff);
		tmpoff += tmp_pair_keysize;
		totalsize += tmp_pair_keysize;
		uint32_t tmp_pair_valsize = this->_pairs[pair_i].second.val.dynamic_serialize(dynamic_data, tmpoff);
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
	uint32_t tmp_endkeysize = this->_endkey.deserialize(begin, recv_size - tmp_ophdrsize);
	begin += tmp_endkeysize;
	//memcpy((void *)&this->_num, begin, sizeof(uint32_t));
	//begin += sizeof(uint32_t);
	begin += SPLIT_PREV_BYTES;
	memcpy((void *)&this->_cur_scanidx, begin, sizeof(uint16_t));
	this->_cur_scanidx = ntohs(this->_cur_scanidx);
	begin += sizeof(uint16_t);
	memcpy((void *)&this->_max_scannum, begin, sizeof(uint16_t));
	this->_max_scannum = ntohs(this->_max_scannum);
	begin += sizeof(uint16_t);
	memcpy((void *)&this->_cur_scanswitchidx, begin, sizeof(uint16_t));
	this->_cur_scanswitchidx = ntohs(this->_cur_scanswitchidx);
	begin += sizeof(uint16_t);
	memcpy((void *)&this->_max_scanswitchnum, begin, sizeof(uint16_t));
	this->_max_scanswitchnum = ntohs(this->_max_scanswitchnum);
	begin += sizeof(uint16_t);
	memcpy(&this->_nodeidx_foreval, begin, sizeof(uint16_t));
	this->_nodeidx_foreval = ntohs(this->_nodeidx_foreval);
	begin += sizeof(uint16_t);
	memcpy(&this->_snapshotid, begin, sizeof(int));
	begin += sizeof(int);

	memcpy((void *)&this->_pairnum, begin, sizeof(int32_t));
	this->_pairnum = int32_t(ntohl(uint32_t(this->_pairnum)));
	begin += sizeof(int32_t);
	uint32_t totalsize = my_size;
	this->_pairs.resize(this->_pairnum); // change size to this->_pairnum (not just reserve)
	for (int32_t pair_i = 0; pair_i < this->_pairnum; pair_i++) {
		uint32_t tmp_pair_keysize = this->_pairs[pair_i].first.deserialize(begin, recv_size - totalsize);
		begin += tmp_pair_keysize;
		totalsize += tmp_pair_keysize;
		uint32_t tmp_pair_valsize = this->_pairs[pair_i].second.val.deserialize(begin, recv_size - totalsize);
		begin += tmp_pair_valsize;
		totalsize += tmp_pair_valsize;
	}
}

template<class key_t, class val_t>
size_t ScanResponseSplit<key_t, val_t>::get_frag_hdrsize() {
	//return sizeof(optype_t) + sizeof(key_t) + sizeof(key_t) + SPLIT_PREV_BYTES + sizeof(uint16_t) + sizeof(uint16_t) + sizeof(uint16_t); // op_hdr + scan_hdr + split_hdr + nodeidx_foreval (used to identify fragments from different servers)
	// NOTE: we only need nodeidx_foreval for entire split packet instead of each fragment; so we can place it in fragment body isntead of fragment header in udpsendlarge_ipfrag (see socket_helper.c)
	return Packet<key_t>::get_ophdrsize() + sizeof(key_t) + SPLIT_PREV_BYTES + sizeof(uint16_t) + sizeof(uint16_t) + sizeof(uint16_t) + sizeof(uint16_t); // op_hdr + scan_hdr + split_hdr (isclone + globalserveridx + cur_scanidx + max_scannum + cur_scanswitchidx + max_scanswitchnum)
}

template<class key_t, class val_t>
size_t ScanResponseSplit<key_t, val_t>::get_srcnum_off() {
	return Packet<key_t>::get_ophdrsize() + sizeof(key_t) + SPLIT_PREV_BYTES + sizeof(uint16_t); // offset of split_hdr.max_scannum
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
size_t ScanResponseSplit<key_t, val_t>::get_srcid_off() {
	return Packet<key_t>::get_ophdrsize() + sizeof(key_t) + SPLIT_PREV_BYTES; // offset of split_hdr.cur_scanidx
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
size_t ScanResponseSplit<key_t, val_t>::get_srcswitchnum_off() {
	return Packet<key_t>::get_ophdrsize() + sizeof(key_t) + SPLIT_PREV_BYTES + sizeof(uint16_t) + sizeof(uint16_t) + sizeof(uint16_t); // offset of split_hdr.max_scanswitchnum
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
size_t ScanResponseSplit<key_t, val_t>::get_srcswitchid_off() {
	return Packet<key_t>::get_ophdrsize() + sizeof(key_t) + SPLIT_PREV_BYTES + sizeof(uint16_t) + sizeof(uint16_t); // offset of split_hdr.cur_scanswitchidx
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
ScanResponseSplitServer<key_t, val_t>::ScanResponseSplitServer(key_t key, key_t endkey, uint16_t cur_scanidx, uint16_t max_scannum, uint16_t cur_scanswitchidx, uint16_t max_scanswitchnum, uint16_t nodeidx_foreval, int snapshotid, int32_t pairnum, std::vector<std::pair<key_t, snapshot_record_t>> pairs) 
	: ScanResponseSplit<key_t, val_t>(key, endkey, cur_scanidx, max_scannum, cur_scanswitchidx, max_scanswitchnum, nodeidx_foreval, snapshotid, pairnum, pairs)
{	
	this->_type = static_cast<optype_t>(PacketType::SCANRES_SPLIT_SERVER);
}

// GetRequestPOP

template<class key_t>
GetRequestPOP<key_t>::GetRequestPOP(const char *data, uint32_t recv_size)
{
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
GetRequestNLatest<key_t>::GetRequestNLatest(const char *data, uint32_t recv_size)
{
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
GetResponseLatestSeq<key_t, val_t>::GetResponseLatestSeq(key_t key, val_t val, uint32_t seq, uint16_t nodeidx_foreval)
	: PutRequestSeq<key_t, val_t>(key, val, seq), _stat(true), _nodeidx_foreval(nodeidx_foreval)
{
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
	uint32_t tmp_valsize = this->_val.serialize(begin, max_size-tmp_ophdrsize);
	begin += tmp_valsize;
	uint32_t tmp_shadowtypesize = serialize_packet_type(this->_type, begin, max_size - tmp_ophdrsize - tmp_valsize); // shadowtype
	begin += tmp_shadowtypesize;
	uint32_t bigendian_seq = htonl(this->_seq);
	memcpy(begin, (void *)&bigendian_seq, sizeof(uint32_t)); // little-endian to big-endian
	begin += sizeof(uint32_t);
	memcpy(begin, (void *)&this->_stat, sizeof(bool));
	begin += sizeof(bool);
	uint16_t bigendian_nodeidx_foreval = htons(this->_nodeidx_foreval);
	memcpy(begin, (void *)&bigendian_nodeidx_foreval, sizeof(uint16_t));
	begin += sizeof(uint16_t);
	begin += STAT_PADDING_BYTES;
	return tmp_ophdrsize + tmp_valsize + tmp_shadowtypesize + sizeof(uint32_t) + sizeof(bool) + sizeof(uint16_t) + STAT_PADDING_BYTES;
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
	return Packet<key_t>::get_ophdrsize() + sizeof(uint16_t) + val_t::MAX_VALLEN + sizeof(optype_t) + sizeof(uint32_t) + sizeof(bool) + sizeof(uint16_t) + STAT_PADDING_BYTES;
}

template<class key_t, class val_t>
void GetResponseLatestSeq<key_t, val_t>::deserialize(const char * data, uint32_t recv_size)
{
	COUT_N_EXIT("Invalid invoke of deserialize for GetResponseLatestSeq");
}

// GetResponseLatestSeqServer (value must <= 128B)

template<class key_t, class val_t>
GetResponseLatestSeqServer<key_t, val_t>::GetResponseLatestSeqServer(switchidx_t switchidx, key_t key, val_t val, uint32_t seq, uint16_t nodeidx_foreval)
	: GetResponseLatestSeq<key_t, val_t>(key, val, seq, nodeidx_foreval)
{
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
GetResponseLatestSeqInswitchCase1<key_t, val_t>::GetResponseLatestSeqInswitchCase1(key_t key, val_t val, uint32_t seq, uint16_t idx, bool stat) 
	: GetResponseLatestSeq<key_t, val_t>(key, val, seq, 0), _idx(idx)
{
	this->_stat = stat;
	this->_type = static_cast<optype_t>(PacketType::GETRES_LATEST_SEQ_INSWITCH_CASE1);
	INVARIANT(this->_val.val_length <= val_t::SWITCH_MAX_VALLEN);
	INVARIANT(seq >= 0);
	INVARIANT(idx >= 0);
}

template<class key_t, class val_t>
GetResponseLatestSeqInswitchCase1<key_t, val_t>::GetResponseLatestSeqInswitchCase1(const char * data, uint32_t recv_size) {
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
	return Packet<key_t>::get_ophdrsize() + sizeof(uint16_t) + val_t::MAX_VALLEN + sizeof(optype_t) + sizeof(uint32_t) + INSWITCH_PREV_BYTES + sizeof(uint16_t) + sizeof(bool) + sizeof(uint16_t) + STAT_PADDING_BYTES + CLONE_BYTES;
}

template<class key_t, class val_t>
uint32_t GetResponseLatestSeqInswitchCase1<key_t, val_t>::serialize(char * const data, uint32_t max_size) {
	//uint32_t my_size = this->size();
	//INVARIANT(max_size >= my_size);
	char *begin = data;
	uint32_t tmp_ophdrsize = this->serialize_ophdr(begin, max_size);
	begin += tmp_ophdrsize;
	uint32_t tmp_valsize = this->_val.serialize(begin, max_size-tmp_ophdrsize);
	begin += tmp_valsize;
	uint32_t tmp_shadowtypesize = serialize_packet_type(this->_type, begin, max_size - tmp_ophdrsize - tmp_valsize); // shadowtype
	begin += tmp_shadowtypesize;
	uint32_t bigendian_seq = htonl(this->_seq);
	memcpy(begin, (void *)&bigendian_seq, sizeof(uint32_t));
	begin += sizeof(uint32_t);
	memset(begin, 0, INSWITCH_PREV_BYTES); // the first bytes of inswitch_hdr
	begin += INSWITCH_PREV_BYTES;
	uint16_t bigendian_idx = htons(this->_idx);
	memcpy(begin, (void *)&bigendian_idx, sizeof(uint16_t)); // little-endian to big-endian
	begin += sizeof(uint16_t);
	memcpy(begin, (void *)&this->_stat, sizeof(bool));
	begin += sizeof(bool); // stat_hdr.stat
	memset(begin, 0, sizeof(uint16_t)); // stat_hdr.nodeidx_foreval
	begin += sizeof(uint16_t);
	begin += STAT_PADDING_BYTES;
	memset(begin, 0, CLONE_BYTES); // clone_hdr
	return tmp_ophdrsize + tmp_valsize + tmp_shadowtypesize + sizeof(uint32_t) + INSWITCH_PREV_BYTES + sizeof(uint16_t) + sizeof(bool) + sizeof(uint16_t) + STAT_PADDING_BYTES + CLONE_BYTES;
	//begin += CLONE_BYTES;
}

template<class key_t, class val_t>
void GetResponseLatestSeqInswitchCase1<key_t, val_t>::deserialize(const char * data, uint32_t recv_size) {
	//uint32_t my_size = this->size();
	//INVARIANT(my_size == recv_size);
	const char *begin = data;
	uint32_t tmp_ophdrsize = this->deserialize_ophdr(begin, recv_size);
	begin += tmp_ophdrsize;
	uint32_t tmp_valsize = this->_val.deserialize(begin, recv_size - tmp_ophdrsize);
	begin += tmp_valsize;
	begin += sizeof(optype_t); // deserialize shadowtype
	memcpy((void *)&this->_seq, begin, sizeof(uint32_t));
	this->_seq = ntohl(this->_seq);
	begin += sizeof(uint32_t);
	begin += INSWITCH_PREV_BYTES; // the first bytes of inswitch_hdr
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
GetResponseDeletedSeq<key_t, val_t>::GetResponseDeletedSeq(key_t key, val_t val, uint32_t seq, uint16_t nodeidx_foreval)
	: GetResponseLatestSeq<key_t, val_t>(key, val, seq, nodeidx_foreval)
{
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
GetResponseDeletedSeqServer<key_t, val_t>::GetResponseDeletedSeqServer(switchidx_t switchidx, key_t key, val_t val, uint32_t seq, uint16_t nodeidx_foreval)
	: GetResponseDeletedSeq<key_t, val_t>(key, val, seq, nodeidx_foreval)
{
	this->_type = static_cast<optype_t>(PacketType::GETRES_DELETED_SEQ_SERVER);
	this->_leafswitchidx = switchidx;
}

// GetResponseDeletedSeqInswitchCase1 (value must <= 128B)

template<class key_t, class val_t>
GetResponseDeletedSeqInswitchCase1<key_t, val_t>::GetResponseDeletedSeqInswitchCase1(key_t key, val_t val, uint32_t seq, uint16_t idx, bool stat) 
	: GetResponseLatestSeqInswitchCase1<key_t, val_t>(key, val, seq, idx, stat)
{
	this->_type = static_cast<optype_t>(PacketType::GETRES_DELETED_SEQ_INSWITCH_CASE1);
	INVARIANT(this->_val.val_length <= val_t::SWITCH_MAX_VALLEN);
	INVARIANT(seq >= 0);
	INVARIANT(idx >= 0);
}

template<class key_t, class val_t>
GetResponseDeletedSeqInswitchCase1<key_t, val_t>::GetResponseDeletedSeqInswitchCase1(const char * data, uint32_t recv_size) {
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
PutRequestSeq<key_t, val_t>::PutRequestSeq(key_t key, val_t val, uint32_t seq)
	: Packet<key_t>(packet_type_t::PUTREQ_SEQ, 0, 0, key), _val(val), _seq(seq)
{
}

template<class key_t, class val_t>
PutRequestSeq<key_t, val_t>::PutRequestSeq(const char * data, uint32_t recv_size)
{
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
	return Packet<key_t>::get_ophdrsize() + sizeof(uint16_t) + val_t::MAX_VALLEN + sizeof(optype_t) + sizeof(uint32_t);
}

template<class key_t, class val_t>
void PutRequestSeq<key_t, val_t>::deserialize(const char * data, uint32_t recv_size)
{
	//uint32_t my_size = this->size();
	//INVARIANT(my_size == recv_size);
	const char *begin = data;
	uint32_t tmp_ophdrsize = this->deserialize_ophdr(begin, recv_size);
	begin += tmp_ophdrsize;
	uint32_t tmp_valsize = this->_val.deserialize(begin, recv_size - tmp_ophdrsize);
	begin += tmp_valsize;
	begin += sizeof(optype_t); // deserialize shadowtype
	memcpy((void *)&this->_seq, begin, sizeof(uint32_t));
	this->_seq = ntohl(this->_seq); // Big-endian to little-endian
}

template<class key_t, class val_t>
uint32_t PutRequestSeq<key_t, val_t>::serialize(char * const data, uint32_t max_size)
{
	COUT_N_EXIT("Invalid invoke of serialize for PutRequestSeq");
}

// PutRequestPopSeq (value must <= 128B)

template<class key_t, class val_t>
PutRequestPopSeq<key_t, val_t>::PutRequestPopSeq(const char * data, uint32_t recv_size)
{
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
PutRequestSeqInswitchCase1<key_t, val_t>::PutRequestSeqInswitchCase1(key_t key, val_t val, uint32_t seq, uint16_t idx, bool stat) 
	: GetResponseLatestSeqInswitchCase1<key_t, val_t>(key, val, seq, idx, stat)
{
	this->_type = static_cast<optype_t>(PacketType::PUTREQ_SEQ_INSWITCH_CASE1);
	INVARIANT(this->_val.val_length <= val_t::SWITCH_MAX_VALLEN);
	INVARIANT(seq >= 0);
	INVARIANT(idx >= 0);
}

template<class key_t, class val_t>
PutRequestSeqInswitchCase1<key_t, val_t>::PutRequestSeqInswitchCase1(const char * data, uint32_t recv_size) {
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::PUTREQ_SEQ_INSWITCH_CASE1);
	INVARIANT(this->_val.val_length <= val_t::SWITCH_MAX_VALLEN);
	INVARIANT(this->_seq >= 0);
	INVARIANT(this->_idx >= 0);
}

// PutRequestSeqCase3 (value must <= 128B)

template<class key_t, class val_t>
PutRequestSeqCase3<key_t, val_t>::PutRequestSeqCase3(const char * data, uint32_t recv_size)
{
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
PutRequestPopSeqCase3<key_t, val_t>::PutRequestPopSeqCase3(const char * data, uint32_t recv_size)
{
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
DelRequestSeq<key_t>::DelRequestSeq(const char * data, uint32_t recv_size) {
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
	return Packet<key_t>::get_ophdrsize() + sizeof(optype_t) + sizeof(uint32_t);
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
}

// DelRequestSeqInswitchCase1 (value must <= 128B)

template<class key_t, class val_t>
DelRequestSeqInswitchCase1<key_t, val_t>::DelRequestSeqInswitchCase1(key_t key, val_t val, uint32_t seq, uint16_t idx, bool stat) 
	: GetResponseLatestSeqInswitchCase1<key_t, val_t>(key, val, seq, idx, stat)
{
	this->_type = static_cast<optype_t>(PacketType::DELREQ_SEQ_INSWITCH_CASE1);
	INVARIANT(this->_val.val_length <= val_t::SWITCH_MAX_VALLEN);
	INVARIANT(seq >= 0);
	INVARIANT(idx >= 0);
}

template<class key_t, class val_t>
DelRequestSeqInswitchCase1<key_t, val_t>::DelRequestSeqInswitchCase1(const char * data, uint32_t recv_size) {
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::DELREQ_SEQ_INSWITCH_CASE1);
	INVARIANT(this->_val.val_length <= val_t::SWITCH_MAX_VALLEN);
	INVARIANT(this->_seq >= 0);
	INVARIANT(this->_idx >= 0);
}

// DelRequestSeqCase3

template<class key_t>
DelRequestSeqCase3<key_t>::DelRequestSeqCase3(const char * data, uint32_t recv_size)
{
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
ScanRequestSplit<key_t>::ScanRequestSplit(key_t key, key_t endkey, uint16_t cur_scanidx, uint16_t max_scannum, uint16_t cur_scanswitchidx, uint16_t max_scanswitchnum)
	: ScanRequest<key_t>(key, endkey), _cur_scanidx(cur_scanidx), _max_scannum(max_scannum), _cur_scanswitchidx(cur_scanswitchidx), _max_scanswitchnum(max_scanswitchnum)
{
	this->_type = static_cast<optype_t>(packet_type_t::SCANREQ_SPLIT);
}

template<class key_t>
ScanRequestSplit<key_t>::ScanRequestSplit(const char * data, uint32_t recv_size) {
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
	return Packet<key_t>::get_ophdrsize() + sizeof(key_t) + SPLIT_PREV_BYTES + sizeof(uint16_t) + sizeof(uint16_t) + sizeof(uint16_t) + sizeof(uint16_t);// + sizeof(uint32_t);
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
	uint32_t tmp_endkeysize = this->_endkey.deserialize(begin, recv_size - tmp_ophdrsize);
	begin += tmp_endkeysize;
	//memcpy((void *)&this->_num, begin, sizeof(uint32_t));
	//begin += sizeof(uint32_t);
	begin += SPLIT_PREV_BYTES;
	memcpy((void *)&this->_cur_scanidx, begin, sizeof(uint16_t));
	this->_cur_scanidx = ntohs(this->_cur_scanidx);
	begin += sizeof(uint16_t);
	memcpy((void *)&this->_max_scannum, begin, sizeof(uint16_t));
	this->_max_scannum = ntohs(this->_max_scannum);
	begin += sizeof(uint16_t);
	memcpy((void *)&this->_cur_scanswitchidx, begin, sizeof(uint16_t));
	this->_cur_scanswitchidx = ntohs(this->_cur_scanswitchidx);
	begin += sizeof(uint16_t);
	memcpy((void *)&this->_max_scanswitchnum, begin, sizeof(uint16_t));
	this->_max_scanswitchnum = ntohs(this->_max_scanswitchnum);
	begin += sizeof(uint16_t);
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
CachePop<key_t, val_t>::CachePop(key_t key, val_t val, uint32_t seq, bool stat, uint16_t serveridx)
	: PutRequestSeq<key_t, val_t>(key, val, seq), _stat(stat), _serveridx(serveridx)
{
	this->_type = static_cast<optype_t>(PacketType::CACHE_POP);
	INVARIANT(this->_val.val_length <= val_t::SWITCH_MAX_VALLEN);
	INVARIANT(seq >= 0);
	INVARIANT(serveridx >= 0);
}

template<class key_t, class val_t>
CachePop<key_t, val_t>::CachePop(const char * data, uint32_t recv_size) {
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
	uint32_t tmp_valsize = this->_val.serialize(begin, max_size-tmp_ophdrsize);
	begin += tmp_valsize;
	uint32_t bigendian_seq = htonl(this->_seq);
	memcpy(begin, (void *)&bigendian_seq, sizeof(uint32_t)); // little-endian to big-endian
	begin += sizeof(uint32_t);
	memcpy(begin, (void *)&this->_stat, sizeof(bool));
	begin += sizeof(bool);
	uint16_t bigendian_serveridx = htons(uint16_t(this->_serveridx));
	memcpy(begin, (void *)&bigendian_serveridx, sizeof(uint16_t)); // little-endian to big-endian
	return tmp_ophdrsize + tmp_valsize + sizeof(uint32_t) + sizeof(bool) + sizeof(uint16_t);
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
	return Packet<key_t>::get_ophdrsize() + sizeof(uint16_t) + val_t::MAX_VALLEN + sizeof(uint32_t) + sizeof(bool) + sizeof(uint16_t);
}

template<class key_t, class val_t>
void CachePop<key_t, val_t>::deserialize(const char * data, uint32_t recv_size)
{
	//uint32_t my_size = this->size();
	//INVARIANT(my_size == recv_size);
	const char *begin = data;
	uint32_t tmp_ophdrsize = this->deserialize_ophdr(begin, recv_size);
	begin += tmp_ophdrsize;
	uint32_t tmp_valsize = this->_val.deserialize(begin, recv_size - tmp_ophdrsize);
	begin += tmp_valsize;
	memcpy((void *)&this->_seq, begin, sizeof(uint32_t));
	this->_seq = ntohl(this->_seq); // Big-endian to little-endian
	begin += sizeof(uint32_t);
	memcpy((void *)&this->_stat, begin, sizeof(bool));
	begin += sizeof(bool);
	memcpy((void *)&this->_serveridx, begin, sizeof(uint16_t));
	this->_serveridx = ntohs(this->_serveridx); // Big-endian to little-endian
}

// CachePopInswitch (value must <= 128B)

template<class key_t, class val_t>
CachePopInswitch<key_t, val_t>::CachePopInswitch(switchidx_t spineswitchidx, switchidx_t leafswitchidx, key_t key, val_t val, uint32_t seq, uint16_t freeidx, bool stat)
	: PutRequestSeq<key_t, val_t>(key, val, seq), _freeidx(freeidx), _stat(stat)
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
	uint32_t tmp_valsize = this->_val.serialize(begin, max_size-tmp_ophdrsize);
	begin += tmp_valsize;
	uint32_t tmp_shadowtypesize = serialize_packet_type(this->_type, begin, max_size - tmp_ophdrsize - tmp_valsize); // shadowtype
	begin += tmp_shadowtypesize;
	uint32_t bigendian_seq = htonl(this->_seq);
	memcpy(begin, (void *)&bigendian_seq, sizeof(uint32_t)); // little-endian to big-endian
	begin += sizeof(uint32_t);
	memset(begin, 0, INSWITCH_PREV_BYTES); // the first bytes of inswitch_hdr
	begin += INSWITCH_PREV_BYTES;
	uint16_t bigendian_freeidx = htons(uint16_t(this->_freeidx));
	memcpy(begin, (void *)&bigendian_freeidx, sizeof(uint16_t)); // little-endian to big-endian
	begin += sizeof(uint16_t);
	memcpy(begin, (void *)&this->_stat, sizeof(bool));
	begin += sizeof(bool); // stat_hdr.stat
	memset(begin, 0, sizeof(uint16_t)); // stat_hdr.nodeidx_foreval
	begin += sizeof(uint16_t);
	begin += STAT_PADDING_BYTES;
	return tmp_ophdrsize + tmp_valsize + tmp_shadowtypesize + sizeof(uint32_t) + INSWITCH_PREV_BYTES + sizeof(uint16_t) + sizeof(bool) + sizeof(uint16_t) + STAT_PADDING_BYTES;
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
	return Packet<key_t>::get_ophdrsize() + sizeof(uint16_t) + val_t::MAX_VALLEN + sizeof(optype_t) + sizeof(uint32_t) + INSWITCH_PREV_BYTES + sizeof(uint16_t) + sizeof(bool) + sizeof(uint16_t) + STAT_PADDING_BYTES;
}

template<class key_t, class val_t>
void CachePopInswitch<key_t, val_t>::deserialize(const char * data, uint32_t recv_size)
{
	COUT_N_EXIT("Invalid invoke of deserialize for CachePopInswitch");
}

// CachePopInswitchAck (no value)

template<class key_t>
CachePopInswitchAck<key_t>::CachePopInswitchAck(const char *data, uint32_t recv_size)
{
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
CacheEvict<key_t, val_t>::CacheEvict(key_t key, val_t val, uint32_t seq, bool stat, uint16_t serveridx) 
	: GetResponseLatestSeq<key_t, val_t>(key, val, seq, 0), _serveridx(serveridx)
{
	this->_stat = stat;
	this->_type = static_cast<optype_t>(PacketType::CACHE_EVICT);
	INVARIANT(this->_val.val_length <= val_t::SWITCH_MAX_VALLEN);
	INVARIANT(seq >= 0);
	INVARIANT(serveridx >= 0);
}

template<class key_t, class val_t>
CacheEvict<key_t, val_t>::CacheEvict(const char * data, uint32_t recv_size) {
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
	return Packet<key_t>::get_ophdrsize() + sizeof(uint16_t) + val_t::MAX_VALLEN + sizeof(uint32_t) + sizeof(bool) + sizeof(uint16_t);
}

template<class key_t, class val_t>
uint32_t CacheEvict<key_t, val_t>::serialize(char * const data, uint32_t max_size) {
	//uint32_t my_size = this->size();
	//INVARIANT(max_size >= my_size);
	char *begin = data;
	uint32_t tmp_ophdrsize = this->serialize_ophdr(begin, max_size);
	begin += tmp_ophdrsize;
	uint32_t tmp_valsize = this->_val.serialize(begin, max_size-tmp_ophdrsize);
	begin += tmp_valsize;
	uint32_t bigendian_seq = htonl(this->_seq);
	memcpy(begin, (void *)&bigendian_seq, sizeof(uint32_t));
	begin += sizeof(uint32_t);
	memcpy(begin, (void *)&this->_stat, sizeof(bool));
	begin += sizeof(bool);
	uint16_t bigendian_serveridx = htons(uint16_t(this->_serveridx));
	memcpy(begin, (void *)&bigendian_serveridx, sizeof(uint16_t));
	return tmp_ophdrsize + tmp_valsize + sizeof(uint32_t) + sizeof(bool) + sizeof(uint16_t);
}

template<class key_t, class val_t>
void CacheEvict<key_t, val_t>::deserialize(const char * data, uint32_t recv_size) {
	//uint32_t my_size = this->size();
	//INVARIANT(my_size == recv_size);
	const char *begin = data;
	uint32_t tmp_ophdrsize = this->deserialize_ophdr(begin, recv_size);
	begin += tmp_ophdrsize;
	uint32_t tmp_valsize = this->_val.deserialize(begin, recv_size - tmp_ophdrsize);
	begin += tmp_valsize;
	memcpy((void *)&this->_seq, begin, sizeof(uint32_t));
	this->_seq = ntohl(this->_seq);
	begin += sizeof(uint32_t);
	memcpy((void *)&this->_stat, begin, sizeof(bool));
	begin += sizeof(bool);
	memcpy((void *)&this->_serveridx, begin, sizeof(uint16_t));
	this->_serveridx = ntohs(this->_serveridx);
}

// CacheEvictAck (only used in end-hosts)

template<class key_t>
CacheEvictAck<key_t>::CacheEvictAck(key_t key) 
	: WarmupRequest<key_t>(key) // CACHE_EVICT_ACK does NOT need spine/leafswitchidx for power-of-two-choices which is ONLY for GETREQ
{
	this->_type = static_cast<optype_t>(PacketType::CACHE_EVICT_ACK);
}

template<class key_t>
CacheEvictAck<key_t>::CacheEvictAck(const char * data, uint32_t recv_size) {
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::CACHE_EVICT_ACK);
}

// CacheEvictCase2 (value must <= 128B; only used in end-hosts)

template<class key_t, class val_t>
CacheEvictCase2<key_t, val_t>::CacheEvictCase2(key_t key, val_t val, uint32_t seq, bool stat, uint16_t serveridx) 
	: CacheEvict<key_t, val_t>(key, val, seq, stat, serveridx)
{
	this->_type = static_cast<optype_t>(PacketType::CACHE_EVICT_CASE2);
	INVARIANT(this->_val.val_length <= val_t::SWITCH_MAX_VALLEN);
	INVARIANT(seq >= 0);
	INVARIANT(serveridx >= 0);
}

template<class key_t, class val_t>
CacheEvictCase2<key_t, val_t>::CacheEvictCase2(const char * data, uint32_t recv_size) {
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
WarmupRequest<key_t>::WarmupRequest(key_t key)
	: Packet<key_t>(packet_type_t::WARMUPREQ, 0, 0, key) // WARMUPREQ does NOT need spine/leafswitchidx for power-of-two-choices which is ONLY for GETREQ
{
}

template<class key_t>
WarmupRequest<key_t>::WarmupRequest(const char * data, uint32_t recv_size) {
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::WARMUPREQ);
}

template<class key_t>
uint32_t WarmupRequest<key_t>::size() {
	return Packet<key_t>::get_ophdrsize();
}

template<class key_t>
uint32_t WarmupRequest<key_t>::serialize(char * const data, uint32_t max_size) {
	uint32_t my_size = this->size();
	INVARIANT(max_size >= my_size);
	char *begin = data;
	uint32_t tmp_ophdrsize = this->serialize_ophdr(begin, max_size);
	begin += tmp_ophdrsize;
	return tmp_ophdrsize;
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
WarmupAck<key_t>::WarmupAck(key_t key) 
	: WarmupRequest<key_t>(key) // WARMUPACK does NOT need spine/leafswitchidx for power-of-two-choices which is ONLY for GETREQ
{
	this->_type = static_cast<optype_t>(PacketType::WARMUPACK);
}

template<class key_t>
WarmupAck<key_t>::WarmupAck(const char * data, uint32_t recv_size) {
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::WARMUPACK);
}

// LoadRequest

template<class key_t, class val_t>
LoadRequest<key_t, val_t>::LoadRequest(key_t key, val_t val) 
	: PutRequest<key_t, val_t>(key, val)
{
	this->_type = static_cast<optype_t>(PacketType::LOADREQ);
}

template<class key_t, class val_t>
LoadRequest<key_t, val_t>::LoadRequest(const char * data, uint32_t recv_size) {
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::LOADREQ);
}

// LoadAck

template<class key_t>
LoadAck<key_t>::LoadAck(key_t key) 
	: WarmupRequest<key_t>(key) // LOADACK does NOT need spine/leafswitchidx for power-of-two-choices which is ONLY for GETREQ
{
	this->_type = static_cast<optype_t>(PacketType::LOADACK);
}

template<class key_t>
LoadAck<key_t>::LoadAck(const char * data, uint32_t recv_size) {
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::LOADACK);
}

// CachePopAck (only used by end-hosts)

template<class key_t>
CachePopAck<key_t>::CachePopAck(key_t key) 
	: WarmupRequest<key_t>(key) // CACHE_POP_ACK does NOT need spine/leafswitchidx for power-of-two-choices which is ONLY for GETREQ
{
	this->_type = static_cast<optype_t>(PacketType::CACHE_POP_ACK);
}

template<class key_t>
CachePopAck<key_t>::CachePopAck(const char * data, uint32_t recv_size) {
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
CacheEvictLoadfreqInswitch<key_t>::CacheEvictLoadfreqInswitch(switchidx_t spineswitchidx, switchidx_t leafswitchidx, key_t key, uint16_t evictidx)
	: Packet<key_t>(PacketType::CACHE_EVICT_LOADFREQ_INSWITCH, spineswitchidx, leafswitchidx, key), _evictidx(evictidx)
{
	INVARIANT(evictidx >= 0);
}

template<class key_t>
uint32_t CacheEvictLoadfreqInswitch<key_t>::serialize(char * const data, uint32_t max_size) {
	uint32_t my_size = this->size();
	INVARIANT(max_size >= my_size);
	char *begin = data;
	uint32_t tmp_ophdrsize = this->serialize_ophdr(begin, max_size);
	begin += tmp_ophdrsize;
	uint32_t tmp_shadowtypesize = serialize_packet_type(this->_type, begin, max_size - tmp_ophdrsize); // shadowtype
	begin += tmp_shadowtypesize;
	memset(begin, 0, INSWITCH_PREV_BYTES); // the first bytes of inswitch_hdr
	begin += INSWITCH_PREV_BYTES;
	uint16_t bigendian_evictidx = htons(uint16_t(this->_evictidx));
	memcpy(begin, (void *)&bigendian_evictidx, sizeof(uint16_t)); // little-endian to big-endian
	return tmp_ophdrsize + tmp_shadowtypesize + INSWITCH_PREV_BYTES + sizeof(uint16_t);
}

template<class key_t>
uint16_t CacheEvictLoadfreqInswitch<key_t>::evictidx() const {
	return _evictidx;
}

template<class key_t>
uint32_t CacheEvictLoadfreqInswitch<key_t>::size() { // unused
	return Packet<key_t>::get_ophdrsize() + sizeof(optype_t) + INSWITCH_PREV_BYTES + sizeof(uint16_t);
}

template<class key_t>
void CacheEvictLoadfreqInswitch<key_t>::deserialize(const char * data, uint32_t recv_size) {
	COUT_N_EXIT("Invalid invoke of serialize for CacheEvictLoadfreqInswitch");
}


// CacheEvictLoadfreqInswitchAck

template<class key_t>
CacheEvictLoadfreqInswitchAck<key_t>::CacheEvictLoadfreqInswitchAck(const char * data, uint32_t recv_size)
{
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
}

template<class key_t>
uint32_t CacheEvictLoadfreqInswitchAck<key_t>::frequency() const {
	return _frequency;
}

template<class key_t>
uint32_t CacheEvictLoadfreqInswitchAck<key_t>::size() { // unused
	return Packet<key_t>::get_ophdrsize() + sizeof(uint32_t);
}

template<class key_t>
uint32_t CacheEvictLoadfreqInswitchAck<key_t>::serialize(char * const data, uint32_t max_size) {
	COUT_N_EXIT("Invalid invoke of deserialize for CacheEvictLoadfreqInswitchAck");
}

// CacheEvictLoaddataInswitch

template<class key_t>
CacheEvictLoaddataInswitch<key_t>::CacheEvictLoaddataInswitch(switchidx_t spineswitchidx, switchidx_t leafswitchidx, key_t key, uint16_t evictidx)
	: CacheEvictLoadfreqInswitch<key_t>(spineswitchidx, leafswitchidx, key, evictidx)
{
	this->_type = optype_t(packet_type_t::CACHE_EVICT_LOADDATA_INSWITCH);
	INVARIANT(evictidx >= 0);
}

// CacheEvictLoaddataInswitchAck (value must <= 128B)

template<class key_t, class val_t>
CacheEvictLoaddataInswitchAck<key_t, val_t>::CacheEvictLoaddataInswitchAck(const char * data, uint32_t recv_size) {
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
	return Packet<key_t>::get_ophdrsize() + sizeof(uint16_t) + val_t::MAX_VALLEN + sizeof(optype_t) + sizeof(uint32_t) + sizeof(bool) + sizeof(uint16_t) + STAT_PADDING_BYTES;
}

template<class key_t, class val_t>
void CacheEvictLoaddataInswitchAck<key_t, val_t>::deserialize(const char * data, uint32_t recv_size) {
	uint32_t my_size = this->size();
	INVARIANT(my_size <= recv_size);
	const char *begin = data;
	uint32_t tmp_ophdrsize = this->deserialize_ophdr(begin, recv_size);
	begin += tmp_ophdrsize;
	uint32_t tmp_valsize = this->_val.deserialize(begin, recv_size - tmp_ophdrsize);
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
LoadsnapshotdataInswitch<key_t>::LoadsnapshotdataInswitch(switchidx_t spineswitchidx, switchidx_t leafswitchidx, key_t key, uint16_t loadidx)
	: CacheEvictLoadfreqInswitch<key_t>(spineswitchidx, leafswitchidx, key, loadidx)
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
LoadsnapshotdataInswitchAck<key_t, val_t>::LoadsnapshotdataInswitchAck(const char * data, uint32_t recv_size) {
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::LOADSNAPSHOTDATA_INSWITCH_ACK);
	INVARIANT(this->_val.val_length <= val_t::SWITCH_MAX_VALLEN);
	INVARIANT(this->_seq >= 0);
}

template<class key_t, class val_t>
uint32_t LoadsnapshotdataInswitchAck<key_t, val_t>::size() { // unused
	return Packet<key_t>::get_ophdrsize() + sizeof(uint16_t) + val_t::MAX_VALLEN + sizeof(optype_t) + sizeof(uint32_t) + INSWITCH_PREV_BYTES + sizeof(uint16_t) + sizeof(bool) + sizeof(uint16_t) + STAT_PADDING_BYTES;
}

template<class key_t, class val_t>
void LoadsnapshotdataInswitchAck<key_t, val_t>::deserialize(const char * data, uint32_t recv_size) {
	//uint32_t my_size = this->size();
	//INVARIANT(my_size == recv_size);
	const char *begin = data;
	uint32_t tmp_ophdrsize = this->deserialize_ophdr(begin, recv_size);
	begin += tmp_ophdrsize;
	uint32_t tmp_valsize = this->_val.deserialize(begin, recv_size - tmp_ophdrsize);
	begin += tmp_valsize;
	begin += sizeof(optype_t); // deserialize shadowtype
	memcpy((void *)&this->_seq, begin, sizeof(uint32_t));
	this->_seq = ntohl(this->_seq);
	begin += sizeof(uint32_t);
	begin += INSWITCH_PREV_BYTES; // the first bytes of inswitch_hdr
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
SetvalidInswitch<key_t>::SetvalidInswitch(switchidx_t spineswitchidx, switchidx_t leafswitchidx, key_t key, uint16_t idx, uint8_t validvalue)
	: Packet<key_t>(PacketType::SETVALID_INSWITCH, spineswitchidx, leafswitchidx, key), _idx(idx), _validvalue(validvalue)
{
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
	uint32_t tmp_shadowtypesize = serialize_packet_type(this->_type, begin, max_size - tmp_ophdrsize); // shadowtype
	begin += tmp_shadowtypesize;
	memset(begin, 0, INSWITCH_PREV_BYTES); // the first bytes of inswitch_hdr
	begin += INSWITCH_PREV_BYTES;
	uint16_t bigendian_idx = htons(uint16_t(this->_idx));
	memcpy(begin, (void *)&bigendian_idx, sizeof(uint16_t)); // little-endian to big-endian
	begin += sizeof(uint16_t);
	memcpy(begin, (void *)&_validvalue, sizeof(uint8_t));
	return tmp_ophdrsize + tmp_shadowtypesize + INSWITCH_PREV_BYTES + sizeof(uint16_t) + sizeof(uint8_t);
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
	return Packet<key_t>::get_ophdrsize() + sizeof(optype_t) + INSWITCH_PREV_BYTES + sizeof(uint16_t) + sizeof(uint8_t);
}

template<class key_t>
void SetvalidInswitch<key_t>::deserialize(const char * data, uint32_t recv_size) {
	COUT_N_EXIT("Invalid invoke of serialize for SetvalidInswitch");
}

// SetvalidInswitchAck (no value)

template<class key_t>
SetvalidInswitchAck<key_t>::SetvalidInswitchAck(const char *data, uint32_t recv_size)
{
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
NetcacheGetRequestPop<key_t>::NetcacheGetRequestPop(key_t key)
	: GetRequest<key_t>(0, 0, key) // NETCACHE_GETREQ_POP does NOT need spine/leafswitchidx for power-of-two-choices which is ONLY for GETREQ
{
	this->_type = optype_t(packet_type_t::NETCACHE_GETREQ_POP);
}

template<class key_t>
NetcacheGetRequestPop<key_t>::NetcacheGetRequestPop(const char * data, uint32_t recv_size) {
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == packet_type_t::NETCACHE_GETREQ_POP);
}

template<class key_t>
uint32_t NetcacheGetRequestPop<key_t>::size() {
	return Packet<key_t>::get_ophdrsize() + sizeof(optype_t) + sizeof(uint32_t) + sizeof(uint32_t) + CLONE_BYTES;
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
	begin += sizeof(optype_t); // shadowtype
	memcpy(&this->_spineload, begin, sizeof(uint32_t));
	this->_spineload = ntohl(this->_spineload);
	begin += sizeof(uint32_t);
	memcpy(&this->_leafload, begin, sizeof(uint32_t));
	this->_leafload = ntohl(this->_leafload);
	begin += sizeof(uint32_t);
	begin += CLONE_BYTES;
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
NetcacheCachePop<key_t>::NetcacheCachePop(key_t key, uint16_t serveridx)
	: WarmupRequest<key_t>(key), _serveridx(serveridx) // NETCACHE_CACHE_POP does NOT need spine/leafswitchidx for power-of-two-choices which is ONLY for GETREQ
{
	this->_type = static_cast<optype_t>(PacketType::NETCACHE_CACHE_POP);
	INVARIANT(serveridx >= 0);
}

template<class key_t>
NetcacheCachePop<key_t>::NetcacheCachePop(const char * data, uint32_t recv_size) {
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
	return tmp_ophdrsize + sizeof(uint16_t);
}

template<class key_t>
uint16_t NetcacheCachePop<key_t>::serveridx() const {
	return _serveridx;
}

template<class key_t>
uint32_t NetcacheCachePop<key_t>::size() { // unused
	return Packet<key_t>::get_ophdrsize() + sizeof(uint16_t);
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
}

// NetcacheCachePopAck (value must <= 128B; only used in end-hosts)

template<class key_t, class val_t>
NetcacheCachePopAck<key_t, val_t>::NetcacheCachePopAck(key_t key, val_t val, uint32_t seq, bool stat, uint16_t serveridx)
	: CachePop<key_t, val_t>(key, val, seq, stat, serveridx)
{
	this->_type = static_cast<optype_t>(PacketType::NETCACHE_CACHE_POP_ACK);
	INVARIANT(this->_val.val_length <= val_t::SWITCH_MAX_VALLEN);
	INVARIANT(seq >= 0);
	INVARIANT(serveridx >= 0);
}

template<class key_t, class val_t>
NetcacheCachePopAck<key_t, val_t>::NetcacheCachePopAck(const char * data, uint32_t recv_size) {
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::NETCACHE_CACHE_POP_ACK);
	INVARIANT(this->_val.val_length <= val_t::SWITCH_MAX_VALLEN)
	INVARIANT(this->_seq >= 0);
	INVARIANT(this->_serveridx >= 0);
}

// NetcacheCachePopFinish (only used in end-hosts)

template<class key_t>
NetcacheCachePopFinish<key_t>::NetcacheCachePopFinish(key_t key, uint16_t serveridx, uint16_t kvidx)
	: NetcacheCachePop<key_t>(key, serveridx), _kvidx(kvidx)
{
	this->_type = static_cast<optype_t>(PacketType::NETCACHE_CACHE_POP_FINISH);
	INVARIANT(serveridx >= 0);
}

template<class key_t>
NetcacheCachePopFinish<key_t>::NetcacheCachePopFinish(const char * data, uint32_t recv_size) {
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
	return tmp_ophdrsize + sizeof(uint16_t) + sizeof(uint16_t);
}

template<class key_t>
uint16_t NetcacheCachePopFinish<key_t>::kvidx() const {
	return _kvidx;
}

template<class key_t>
uint32_t NetcacheCachePopFinish<key_t>::size() { // unused
	return Packet<key_t>::get_ophdrsize() + sizeof(uint16_t) + sizeof(uint16_t);
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
NetcacheCachePopFinishAck<key_t>::NetcacheCachePopFinishAck(key_t key, uint16_t serveridx)
	: NetcacheCachePop<key_t>(key, serveridx)
{
	this->_type = static_cast<optype_t>(PacketType::NETCACHE_CACHE_POP_FINISH_ACK);
	INVARIANT(serveridx >= 0);
}

template<class key_t>
NetcacheCachePopFinishAck<key_t>::NetcacheCachePopFinishAck(const char * data, uint32_t recv_size) {
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::NETCACHE_CACHE_POP_FINISH_ACK);
	INVARIANT(this->_serveridx >= 0);
}

// NetcacheWarmupRequestInswitchPop

template<class key_t>
NetcacheWarmupRequestInswitchPop<key_t>::NetcacheWarmupRequestInswitchPop(key_t key)
	: CacheEvictLoadfreqInswitch<key_t>(key, 0, 0)
{
	this->_type = static_cast<optype_t>(PacketType::NETCACHE_WARMUPREQ_INSWITCH_POP);
}

template<class key_t>
NetcacheWarmupRequestInswitchPop<key_t>::NetcacheWarmupRequestInswitchPop(const char * data, uint32_t recv_size) {
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
	uint32_t tmp_shadowtypesize = serialize_packet_type(this->_type, begin, max_size - tmp_ophdrsize); // shadowtype
	begin += tmp_shadowtypesize;
	memset(begin, 0, INSWITCH_PREV_BYTES); // the first bytes of inswitch_hdr
	begin += INSWITCH_PREV_BYTES;
	memset(begin, 0, sizeof(uint16_t)); // inswitch_hdr.idx
	begin += sizeof(uint16_t);
	begin += CLONE_BYTES; // clone_hdr
	return tmp_ophdrsize + tmp_shadowtypesize + INSWITCH_PREV_BYTES + sizeof(uint16_t);
}

template<class key_t>
uint32_t NetcacheWarmupRequestInswitchPop<key_t>::size() { // unused
	return Packet<key_t>::get_ophdrsize() + sizeof(optype_t) + INSWITCH_PREV_BYTES + sizeof(uint16_t) + CLONE_BYTES;
}

template<class key_t>
void NetcacheWarmupRequestInswitchPop<key_t>::deserialize(const char * data, uint32_t recv_size) {
	uint32_t my_size = this->size();
	INVARIANT(recv_size >= my_size);
	const char *begin = data;
	uint32_t tmp_ophdrsize = this->deserialize_ophdr(begin, recv_size);
	begin += tmp_ophdrsize;
	uint32_t tmp_shadowtypesize = deserialize_packet_type(this->_type, begin, recv_size - tmp_ophdrsize); // shadowtype
	begin += tmp_shadowtypesize;
	begin += INSWITCH_PREV_BYTES; // the first bytes of inswitch_hdr
	begin += sizeof(uint16_t); // inswitch_hdr.idx
	begin += CLONE_BYTES; // clone_hdr
}

// NetcacheCacheEvict (only used in end-hosts)

template<class key_t>
NetcacheCacheEvict<key_t>::NetcacheCacheEvict(key_t key, uint16_t serveridx)
	: NetcacheCachePop<key_t>(key, serveridx)
{
	this->_type = static_cast<optype_t>(PacketType::NETCACHE_CACHE_EVICT);
	INVARIANT(serveridx >= 0);
}

template<class key_t>
NetcacheCacheEvict<key_t>::NetcacheCacheEvict(const char * data, uint32_t recv_size) {
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::NETCACHE_CACHE_EVICT);
	INVARIANT(this->_serveridx >= 0);
}

// NetcacheCacheEvictAck (only used in end-hosts)

template<class key_t>
NetcacheCacheEvictAck<key_t>::NetcacheCacheEvictAck(key_t key, uint16_t serveridx)
	: NetcacheCachePop<key_t>(key, serveridx)
{
	this->_type = static_cast<optype_t>(PacketType::NETCACHE_CACHE_EVICT_ACK);
	INVARIANT(serveridx >= 0);
}

template<class key_t>
NetcacheCacheEvictAck<key_t>::NetcacheCacheEvictAck(const char * data, uint32_t recv_size) {
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::NETCACHE_CACHE_EVICT_ACK);
	INVARIANT(this->_serveridx >= 0);
}

// NetcachePutRequestSeqCached

template<class key_t, class val_t>
NetcachePutRequestSeqCached<key_t, val_t>::NetcachePutRequestSeqCached(const char * data, uint32_t recv_size) {
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::NETCACHE_PUTREQ_SEQ_CACHED);
}

// NetcacheDelRequestSeqCached

template<class key_t>
NetcacheDelRequestSeqCached<key_t>::NetcacheDelRequestSeqCached(const char * data, uint32_t recv_size) {
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::NETCACHE_DELREQ_SEQ_CACHED);
}

// NetcacheValueupdate

template<class key_t, class val_t>
NetcacheValueupdate<key_t, val_t>::NetcacheValueupdate(switchidx_t spineswitchidx, switchidx_t leafswitchidx, key_t key, val_t val, uint32_t seq, bool stat)
	: GetResponseLatestSeq<key_t, val_t>(key, val, seq, 0)
{
	this->_stat = stat;
	this->_type = optype_t(packet_type_t::NETCACHE_VALUEUPDATE);
	this->_spineswitchidx = spineswitchidx;
	this->_leafswitchidx = leafswitchidx;
}

// NetcacheValueupdateAck

template<class key_t>
NetcacheValueupdateAck<key_t>::NetcacheValueupdateAck(const char * data, uint32_t recv_size) {
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::NETCACHE_VALUEUPDATE_ACK);
}

// WarmupAckServer 

template<class key_t>
WarmupAckServer<key_t>::WarmupAckServer(key_t key) 
	: WarmupAck<key_t>(key)
{
	this->_type = static_cast<optype_t>(PacketType::WARMUPACK_SERVER);
}

// LoadAckServer

template<class key_t>
LoadAckServer<key_t>::LoadAckServer(key_t key) 
	: LoadAck<key_t>(key)
{
	this->_type = static_cast<optype_t>(PacketType::LOADACK_SERVER);
}

// DistcacheCacheEvictVictim (only used by end-hosts)

template<class key_t>
DistcacheCacheEvictVictim<key_t>::DistcacheCacheEvictVictim()
	: Packet<key_t>(), _victimkey(key_t::min()), _victimidx(0)
{
}

template<class key_t>
DistcacheCacheEvictVictim<key_t>::DistcacheCacheEvictVictim(key_t key, key_t victimkey, uint16_t victimidx)
	: Packet<key_t>(packet_type_t::DISTCACHE_CACHE_EVICT_VICTIM, 0, 0, key), _victimkey(victimkey), _victimidx(victimidx)
{
}

template<class key_t>
DistcacheCacheEvictVictim<key_t>::DistcacheCacheEvictVictim(const char * data, uint32_t recv_size) {
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == packet_type_t::DISTCACHE_CACHE_EVICT_VICTIM);
}

template<class key_t>
uint32_t DistcacheCacheEvictVictim<key_t>::size() {
	return Packet<key_t>::get_ophdrsize() + sizeof(key_t) + sizeof(uint16_t);
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
	uint32_t tmp_victimkeysize = this->_victimkey.serialize(begin, max_size - tmp_ophdrsize);
	begin += tmp_victimkeysize;
	memcpy(begin, (void *)&this->_victimidx, sizeof(uint16_t)); // only used by end-hosts
	begin += sizeof(uint16_t);
	return tmp_ophdrsize + tmp_victimkeysize + sizeof(uint16_t);
}

template<class key_t>
void DistcacheCacheEvictVictim<key_t>::deserialize(const char * data, uint32_t recv_size) {
	uint32_t my_size = this->size();
	INVARIANT(my_size <= recv_size);
	const char *begin = data;
	uint32_t tmp_ophdrsize = this->deserialize_ophdr(begin, recv_size);
	begin += tmp_ophdrsize;
	uint32_t tmp_victimkeysize = this->_victimkey.deserialize(begin, recv_size - tmp_ophdrsize);
	begin += tmp_victimkeysize;
	memcpy(&this->_victimidx, begin, sizeof(uint16_t)); // only used by end-hosts
	begin += sizeof(uint16_t);
}

// DistcacheCacheEvictVictimAck

template<class key_t>
DistcacheCacheEvictVictimAck<key_t>::DistcacheCacheEvictVictimAck(key_t key) 
	: WarmupRequest<key_t>(key) // DISTCACHE_CACHE_EVICT_VICTIM_ACK does NOT need spine/leafswitchidx for power-of-two-choices which is ONLY for GETREQ
{
	this->_type = static_cast<optype_t>(PacketType::DISTCACHE_CACHE_EVICT_VICTIM_ACK);
}

template<class key_t>
DistcacheCacheEvictVictimAck<key_t>::DistcacheCacheEvictVictimAck(const char * data, uint32_t recv_size) {
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::DISTCACHE_CACHE_EVICT_VICTIM_ACK);
}

// DistcacheInvalidate

template<class key_t>
DistcacheInvalidate<key_t>::DistcacheInvalidate(switchidx_t spineswitchidx, switchidx_t leafswitchidx, key_t key) 
	: WarmupRequest<key_t>(key) // DISTCACHE_INVALIDATE does NOT need spine/leafswitchidx for power-of-two-choices which is ONLY for GETREQ
{
	this->_type = static_cast<optype_t>(PacketType::DISTCACHE_INVALIDATE);
	this->_spineswitchidx = spineswitchidx;
	this->_leafswitchidx = leafswitchidx;
}

template<class key_t>
DistcacheInvalidate<key_t>::DistcacheInvalidate(const char * data, uint32_t recv_size) {
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::DISTCACHE_INVALIDATE);
}

// DistcacheInvalidateAck

template<class key_t>
DistcacheInvalidateAck<key_t>::DistcacheInvalidateAck(key_t key) 
	: WarmupRequest<key_t>(key) // DISTCACHE_INVALIDATE_ACK does NOT need spine/leafswitchidx for power-of-two-choices which is ONLY for GETREQ
{
	this->_type = static_cast<optype_t>(PacketType::DISTCACHE_INVALIDATE_ACK);
}

template<class key_t>
DistcacheInvalidateAck<key_t>::DistcacheInvalidateAck(const char * data, uint32_t recv_size) {
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::DISTCACHE_INVALIDATE_ACK);
}

// DistcacheUpdateTrafficload

template<class key_t>
DistcacheUpdateTrafficload<key_t>::DistcacheUpdateTrafficload(switchidx_t spineswitchidx, switchidx_t leafswitchidx, key_t key, uint32_t spineload, uint32_t leafload) 
	: GetRequest<key_t>(spineswitchidx, leafswitchidx, key)
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
DistcacheSpineValueupdateInswitch<key_t, val_t>::DistcacheSpineValueupdateInswitch(switchidx_t spineswitchidx, switchidx_t leafswitchidx, key_t key, val_t val, uint32_t seq, bool stat, uint16_t kvidx)
	: GetResponseLatestSeq<key_t, val_t>(key, val, seq, 0), _kvidx(kvidx)
{
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
	uint32_t tmp_valsize = this->_val.serialize(begin, max_size-tmp_ophdrsize);
	begin += tmp_valsize;
	uint32_t tmp_shadowtypesize = serialize_packet_type(this->_type, begin, max_size - tmp_ophdrsize - tmp_valsize); // shadowtype
	begin += tmp_shadowtypesize;
	uint32_t bigendian_seq = htonl(this->_seq);
	memcpy(begin, (void *)&bigendian_seq, sizeof(uint32_t)); // little-endian to big-endian
	begin += sizeof(uint32_t);
	memset(begin, 0, INSWITCH_PREV_BYTES); // the first bytes of inswitch_hdr
	begin += INSWITCH_PREV_BYTES;
	uint16_t bigendian_kvidx = htons(this->_kvidx);
	memcpy(begin, &bigendian_kvidx, sizeof(uint16_t));
	begin += sizeof(uint16_t);
	memcpy(begin, (void *)&this->_stat, sizeof(bool));
	begin += sizeof(bool);
	uint16_t bigendian_nodeidx_foreval = htons(this->_nodeidx_foreval);
	memcpy(begin, (void *)&bigendian_nodeidx_foreval, sizeof(uint16_t));
	begin += sizeof(uint16_t);
	begin += STAT_PADDING_BYTES;
	return tmp_ophdrsize + tmp_valsize + tmp_shadowtypesize + sizeof(uint32_t) + INSWITCH_PREV_BYTES + sizeof(uint16_t) + sizeof(bool) + sizeof(uint16_t) + STAT_PADDING_BYTES;
}

template<class key_t, class val_t>
uint16_t DistcacheSpineValueupdateInswitch<key_t, val_t>::kvidx() const {
	return this->_kvidx;
}

template<class key_t, class val_t>
uint32_t DistcacheSpineValueupdateInswitch<key_t, val_t>::size() { // unused
	return Packet<key_t>::get_ophdrsize() + sizeof(uint16_t) + val_t::MAX_VALLEN + sizeof(optype_t) + sizeof(uint32_t) + INSWITCH_PREV_BYTES + sizeof(uint16_t) + sizeof(bool) + sizeof(uint16_t) + STAT_PADDING_BYTES;
}

// DistcacheLeafValueupdateInswitch (value must <= 128B)

template<class key_t, class val_t>
DistcacheLeafValueupdateInswitch<key_t, val_t>::DistcacheLeafValueupdateInswitch(switchidx_t spineswitchidx, switchidx_t leafswitchidx, key_t key, val_t val, uint32_t seq, bool stat, uint16_t kvidx)
	: DistcacheSpineValueupdateInswitch<key_t, val_t>(spineswitchidx, leafswitchidx, key, val, seq, stat, kvidx)
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
DistcacheSpineValueupdateInswitchAck<key_t>::DistcacheSpineValueupdateInswitchAck(const char * data, uint32_t recv_size) {
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
DistcacheLeafValueupdateInswitchAck<key_t>::DistcacheLeafValueupdateInswitchAck(const char * data, uint32_t recv_size) {
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::DISTCACHE_LEAF_VALUEUPDATE_INSWITCH_ACK);
}

// DistcacheValueupdateInswitch (value must <= 128B)

template<class key_t, class val_t>
DistcacheValueupdateInswitch<key_t, val_t>::DistcacheValueupdateInswitch(switchidx_t spineswitchidx, switchidx_t leafswitchidx, key_t key, val_t val, uint32_t seq, bool stat, uint16_t kvidx)
	: DistcacheSpineValueupdateInswitch<key_t, val_t>(spineswitchidx, leafswitchidx, key, val, seq, stat, kvidx)
{
	this->_type = optype_t(packet_type_t::DISTCACHE_VALUEUPDATE_INSWITCH);
}

// DistcacheValueupdateInswitchAck

template<class key_t>
DistcacheValueupdateInswitchAck<key_t>::DistcacheValueupdateInswitchAck(const char * data, uint32_t recv_size) {
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
	begin += INSWITCH_PREV_BYTES;
	begin += sizeof(uint16_t); // inswitch_hdr.idx
}

template<class key_t>
uint32_t DistcacheValueupdateInswitchAck<key_t>::size() {
	return Packet<key_t>::get_ophdrsize() + INSWITCH_PREV_BYTES + sizeof(uint16_t);
}

// PutRequestLargevalue (value must > 128B)

template<class key_t, class val_t>
PutRequestLargevalue<key_t, val_t>::PutRequestLargevalue()
	: Packet<key_t>(), _val()
{
}

template<class key_t, class val_t>
PutRequestLargevalue<key_t, val_t>::PutRequestLargevalue(key_t key, val_t val) 
	: Packet<key_t>(PacketType::PUTREQ_LARGEVALUE, 0, 0, key), _val(val)
{	
	INVARIANT(this->_val.val_length > val_t::SWITCH_MAX_VALLEN);
}

template<class key_t, class val_t>
PutRequestLargevalue<key_t, val_t>::PutRequestLargevalue(const char * data, uint32_t recv_size) {
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::PUTREQ_LARGEVALUE);
	INVARIANT(this->_val.val_length > val_t::SWITCH_MAX_VALLEN);
}

template<class key_t, class val_t>
val_t PutRequestLargevalue<key_t, val_t>::val() const {
	return _val;
}

template<class key_t, class val_t>
uint32_t PutRequestLargevalue<key_t, val_t>::size() { // not used
	return sizeof(optype_t) + sizeof(switchidx_t) + sizeof(key_t) + sizeof(uint16_t) + val_t::SWITCH_MAX_VALLEN;
}

template<class key_t, class val_t>
uint32_t PutRequestLargevalue<key_t, val_t>::serialize(char * const data, uint32_t max_size) {
	uint32_t my_size = this->size();
	INVARIANT(max_size >= my_size);
	char *begin = data;
	uint32_t tmp_ophdrsize = this->serialize_ophdr(begin, max_size);
	begin += tmp_ophdrsize;
	uint32_t tmp_valsize = this->_val.serialize(begin, max_size - tmp_ophdrsize);
	begin += tmp_valsize;
	return tmp_ophdrsize + tmp_valsize;
}

template<class key_t, class val_t>
void PutRequestLargevalue<key_t, val_t>::deserialize(const char * data, uint32_t recv_size) {
	uint32_t my_size = this->size();
	INVARIANT(my_size <= recv_size);
	const char *begin = data;
	uint32_t tmp_ophdrsize = this->deserialize_ophdr(begin, recv_size);
	begin += tmp_ophdrsize;
	uint32_t tmp_valsize = this->_val.deserialize(begin, recv_size-tmp_ophdrsize);
	UNUSED(tmp_valsize);
	begin += tmp_valsize;
}

// PutRequestLargevalueSeq (value must > 128B)

template<class key_t, class val_t>
PutRequestLargevalueSeq<key_t, val_t>::PutRequestLargevalueSeq(key_t key, val_t val, uint32_t seq) 
	: PutRequesetLargevalue<key_t, val_t>(key, val), _seq(seq)
{	
	this->_type = static_cast<optype_t>(packet_type_t::PUTREQ_LARGEVALUE_SEQ);
	INVARIANT(this->_val.val_length > val_t::SWITCH_MAX_VALLEN);
}

template<class key_t, class val_t>
PutRequestLargevalueSeq<key_t, val_t>::PutRequestLargevalueSeq(const char * data, uint32_t recv_size) {
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
	return sizeof(optype_t) + sizeof(switchidx_t) + sizeof(key_t) + sizeof(optype_t) + sizeof(uint32_t) + sizeof(uint16_t) + val_t::SWITCH_MAX_VALLEN;
}

template<class key_t, class val_t>
uint32_t PutRequestLargevalueSeq<key_t, val_t>::serialize(char * const data, uint32_t max_size) {
	uint32_t my_size = this->size();
	INVARIANT(max_size >= my_size);
	char *begin = data;
	uint32_t tmp_ophdrsize = this->serialize_ophdr(begin, max_size);
	begin += tmp_ophdrsize;
	uint32_t tmp_shadowtypesize = serialize_packet_type(this->_type, begin, max_size - tmp_ophdrsize); // shadowtype
	begin += tmp_shadowtypesize;
	uint32_t bigendian_seq = htonl(this->_seq);
	memcpy(begin, &bigendian_seq, sizeof(uint32_t));
	begin += sizeof(uint32_t);
	uint32_t tmp_valsize = this->_val.serialize(begin, max_size - tmp_ophdrsize - tmp_shadowtypesize - sizeof(uint32_t));
	begin += tmp_valsize;
	return tmp_ophdrsize + tmp_shadowtypesize + sizeof(uint32_t) + tmp_valsize;
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
	uint32_t tmp_valsize = this->_val.deserialize(begin, recv_size-tmp_ophdrsize);
	UNUSED(tmp_valsize);
	begin += tmp_valsize;
}

// GetResponseLargevalue (value must > 128B)

template<class key_t, class val_t>
GetResponseLargevalue<key_t, val_t>::GetResponseLargevalue()
	: Packet<key_t>(), _val(), _stat(false), _nodeidx_foreval(0)
{
}

template<class key_t, class val_t>
GetResponseLargevalue<key_t, val_t>::GetResponseLargevalue(key_t key, val_t val, bool stat, uint16_t nodeidx_foreval) 
	: Packet<key_t>(PacketType::GETRES_LARGEVALUE, 0, 0, key), _val(val), _stat(stat), _nodeidx_foreval(nodeidx_foreval)
{	
	INVARIANT(this->_val.val_length > val_t::SWITCH_MAX_VALLEN);
}

template<class key_t, class val_t>
GetResponseLargevalue<key_t, val_t>::GetResponseLargevalue(const char * data, uint32_t recv_size) {
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::GETRES_LARGEVALUE);
	INVARIANT(this->_val.val_length > val_t::SWITCH_MAX_VALLEN);
}

template<class key_t, class val_t>
uint32_t GetResponseLargevalue<key_t, val_t>::size() { // unused
	return sizeof(optype_t) + sizeof(switchidx_t) + sizeof(key_t) + sizeof(uint16_t) + val_t::SWITCH_MAX_VALLEN + sizeof(bool) + sizeof(uint16_t) + STAT_PADDING_BYTES;
}

template<class key_t, class val_t>
uint32_t GetResponseLargevalue<key_t, val_t>::serialize(char * const data, uint32_t max_size) {
	uint32_t my_size = this->size();
	INVARIANT(max_size >= my_size);
	char *begin = data;
	uint32_t tmp_ophdrsize = this->serialize_ophdr(begin, max_size);
	begin += tmp_ophdrsize;
	uint32_t tmp_valsize = this->_val.serialize(begin, max_size-tmp_ophdrsize);
	begin += tmp_valsize;
	memcpy(begin, (void *)&this->_stat, sizeof(bool));
	begin += sizeof(bool);
	uint16_t bigendian_nodeidx_foreval = htons(this->_nodeidx_foreval);
	memcpy(begin, (void *)&bigendian_nodeidx_foreval, sizeof(uint16_t));
	begin += sizeof(uint16_t);
	begin += STAT_PADDING_BYTES;
	return tmp_ophdrsize + tmp_valsize + sizeof(bool) + sizeof(uint16_t) + STAT_PADDING_BYTES;
}

template<class key_t, class val_t>
void GetResponseLargevalue<key_t, val_t>::deserialize(const char * data, uint32_t recv_size) {
	uint32_t my_size = this->size();
	INVARIANT(my_size <= recv_size);
	const char *begin = data;
	uint32_t tmp_ophdrsize = this->deserialize_ophdr(begin, recv_size);
	begin += tmp_ophdrsize;
	uint32_t tmp_valsize = this->_val.deserialize(begin, recv_size - tmp_ophdrsize);
	begin += tmp_valsize;
	memcpy((void *)&this->_stat, begin, sizeof(bool));
	begin += sizeof(bool);
	memcpy(&this->_nodeidx_foreval, begin, sizeof(uint16_t));
	this->_nodeidx_foreval = ntohs(this->_nodeidx_foreval);
	begin += sizeof(uint16_t);
	begin += STAT_PADDING_BYTES;
}

// GetResponseLargevalueServer (value must > 128B)

template<class key_t, class val_t>
GetResponseLargevalueServer<key_t, val_t>::GetResponseLargevalueServer(key_t key, val_t val, bool stat, uint16_t nodeidx_foreval) 
	: GetResponseLargevalue<key_t, val_t>(key, val, stat, nodeidx_foreval)
{	
	this->_type = static_cast<optype_t>(packet_type_t::GETRES_LARGEVALUE_SERVER)
}

template<class key_t, class val_t>
GetResponseLargevalueServer<key_t, val_t>::GetResponseLargevalueServer(const char * data, uint32_t recv_size) {
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::GETRES_LARGEVALUE_SERVER);
	INVARIANT(this->_val.val_length > val_t::SWITCH_MAX_VALLEN);
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

#endif
