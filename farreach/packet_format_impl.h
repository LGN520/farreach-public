#ifndef PACKET_FORMAT_IMPL_H
#define PACKET_FORMAT_IMPL_H

#include "packet_format.h"
#include <arpa/inet.h>

#include "key.h"
#include "val.h"

// Packet
template<class key_t>
Packet<key_t>::Packet() 
	: _type(static_cast<optype_t>(0)), _key(key_t::min())
{
}

template<class key_t>
Packet<key_t>::Packet(packet_type_t type, key_t key)
	: _type(static_cast<optype_t>(type)), _key(key)
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


// GetRequest

template<class key_t>
GetRequest<key_t>::GetRequest()
	: Packet<key_t>()
{
}

template<class key_t>
GetRequest<key_t>::GetRequest(key_t key)
	: Packet<key_t>(packet_type_t::GETREQ, key)
{
}

template<class key_t>
GetRequest<key_t>::GetRequest(const char * data, uint32_t recv_size) {
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == packet_type_t::GETREQ);
}

template<class key_t>
uint32_t GetRequest<key_t>::size() {
	//return sizeof(optype_t) + sizeof(key_t) + DEBUG_BYTES;
	return sizeof(optype_t) + sizeof(key_t);
}

template<class key_t>
uint32_t GetRequest<key_t>::serialize(char * const data, uint32_t max_size) {
	uint32_t my_size = this->size();
	INVARIANT(max_size >= my_size);
	char *begin = data;
	uint32_t tmp_typesize = serialize_packet_type(this->_type, begin, max_size);
	begin += tmp_typesize;
	uint32_t tmp_keysize = this->_key.serialize(begin, max_size - tmp_typesize);
	return tmp_typesize + tmp_keysize;
	//begin += tmp_keysize;
	//memset(begin, 0, DEBUG_BYTES);
	//return tmp_typesize + tmp_keysize + DEBUG_BYTES;
}

template<class key_t>
void GetRequest<key_t>::deserialize(const char * data, uint32_t recv_size) {
	uint32_t my_size = this->size();
	INVARIANT(my_size <= recv_size);
	const char *begin = data;
	uint32_t tmp_typesize = deserialize_packet_type(this->_type, begin, recv_size);
	begin += tmp_typesize;
	this->_key.deserialize(begin, recv_size - tmp_typesize);
}

// PutRequest (value must <= 128B)

template<class key_t, class val_t>
PutRequest<key_t, val_t>::PutRequest()
	: Packet<key_t>(), _val()
{
}

template<class key_t, class val_t>
PutRequest<key_t, val_t>::PutRequest(key_t key, val_t val) 
	: Packet<key_t>(PacketType::PUTREQ, key), _val(val)
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
	//return sizeof(optype_t) + sizeof(key_t) + sizeof(uint16_t) + val_t::MAX_VALLEN + sizeof(optype_t) + DEBUG_BYTES;
	return sizeof(optype_t) + sizeof(key_t) + sizeof(uint16_t) + val_t::MAX_VALLEN + sizeof(optype_t);
}

template<class key_t, class val_t>
uint32_t PutRequest<key_t, val_t>::serialize(char * const data, uint32_t max_size) {
	//uint32_t my_size = this->size();
	//INVARIANT(max_size >= my_size);
	char *begin = data;
	uint32_t tmp_typesize = serialize_packet_type(this->_type, begin, max_size);
	begin += tmp_typesize;
	uint32_t tmp_keysize = this->_key.serialize(begin, max_size - tmp_typesize);
	begin += tmp_keysize;
	uint32_t tmp_valsize = this->_val.serialize(begin, max_size - tmp_typesize - tmp_keysize);
	begin += tmp_valsize;
	uint32_t tmp_shadowtypesize = serialize_packet_type(this->_type, begin, max_size - tmp_typesize - tmp_keysize - tmp_valsize); // shadowtype
	return tmp_typesize + tmp_keysize + tmp_valsize + tmp_shadowtypesize;
	//begin += tmp_shadowtypesize;
	//memset(begin, 0, DEBUG_BYTES);
	//return tmp_typesize + tmp_keysize + tmp_valsize + tmp_shadowtypesize + DEBUG_BYTES;
}

template<class key_t, class val_t>
void PutRequest<key_t, val_t>::deserialize(const char * data, uint32_t recv_size) {
	//uint32_t my_size = this->size();
	//INVARIANT(my_size == recv_size);
	const char *begin = data;
	uint32_t tmp_typesize = deserialize_packet_type(this->_type, begin, recv_size);
	begin += tmp_typesize;
	uint32_t tmp_keysize = this->_key.deserialize(begin, recv_size - tmp_typesize);
	begin += tmp_keysize;
	uint32_t tmp_valsize = this->_val.deserialize(begin, recv_size-tmp_typesize-tmp_keysize);
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
	: Packet<key_t>(packet_type_t::DELREQ, key)
{
}

template<class key_t>
DelRequest<key_t>::DelRequest(const char * data, uint32_t recv_size) {
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == packet_type_t::DELREQ);
}

template<class key_t>
uint32_t DelRequest<key_t>::size() {
	//return sizeof(optype_t) + sizeof(key_t) + DEBUG_BYTES;
	return sizeof(optype_t) + sizeof(key_t);
}

template<class key_t>
uint32_t DelRequest<key_t>::serialize(char * const data, uint32_t max_size) {
	uint32_t my_size = this->size();
	INVARIANT(max_size >= my_size);
	char *begin = data;
	uint32_t tmp_typesize = serialize_packet_type(this->_type, begin, max_size);
	begin += tmp_typesize;
	uint32_t tmp_keysize = this->_key.serialize(begin, max_size - tmp_typesize);
	return sizeof(optype_t) + tmp_keysize;
	//begin += tmp_keysize;
	//memset(begin, 0, DEBUG_BYTES);
	//return tmp_typesize + tmp_keysize + DEBUG_BYTES;
}

template<class key_t>
void DelRequest<key_t>::deserialize(const char * data, uint32_t recv_size) {
	uint32_t my_size = this->size();
	INVARIANT(my_size == recv_size);
	const char *begin = data;
	uint32_t tmp_typesize = deserialize_packet_type(this->_type, begin, recv_size);
	begin += tmp_typesize;
	uint32_t tmp_keysize = this->_key.deserialize(begin, recv_size - tmp_typesize);
	UNUSED(tmp_keysize);
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
	: Packet<key_t>(packet_type_t::SCANREQ, key), _endkey(endkey)
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
	return sizeof(optype_t) + sizeof(key_t) + sizeof(key_t);// + sizeof(uint32_t);
}

template<class key_t>
uint32_t ScanRequest<key_t>::serialize(char * const data, uint32_t max_size) {
	uint32_t my_size = this->size();
	INVARIANT(max_size >= my_size);
	char *begin = data;
	uint32_t tmp_typesize = serialize_packet_type(this->_type, begin, max_size);
	begin += tmp_typesize;
	uint32_t tmp_keysize = this->_key.serialize(begin, max_size - tmp_typesize);
	begin += tmp_keysize;
	uint32_t tmp_endkeysize = this->_endkey.serialize(begin, max_size - tmp_typesize - tmp_keysize);
	//memcpy(begin, (void *)&this->_num, sizeof(uint32_t));
	return tmp_typesize + tmp_keysize + tmp_endkeysize; // + sizeof(uint32_t);
}

template<class key_t>
void ScanRequest<key_t>::deserialize(const char * data, uint32_t recv_size) {
	uint32_t my_size = this->size();
	INVARIANT(my_size == recv_size);
	const char *begin = data;
	uint32_t tmp_typesize = deserialize_packet_type(this->_type, begin, recv_size);
	begin += tmp_typesize;
	uint32_t tmp_keysize = this->_key.deserialize(begin, recv_size - tmp_typesize);
	begin += tmp_keysize;
	uint32_t tmp_endkeysize = this->_endkey.deserialize(begin, recv_size - tmp_typesize - tmp_keysize);
	UNUSED(tmp_endkeysize);
	//begin += tmp_endkeysize;
	//memcpy((void *)&this->_num, begin, sizeof(uint32_t));
}


// GetResponse (value must <= 128B)

template<class key_t, class val_t>
GetResponse<key_t, val_t>::GetResponse()
	: Packet<key_t>(), _val(), _stat(false), _nodeidx_foreval(0)
{
}

template<class key_t, class val_t>
GetResponse<key_t, val_t>::GetResponse(key_t key, val_t val, bool stat, uint16_t nodeidx_foreval) 
	: Packet<key_t>(PacketType::GETRES, key), _val(val), _stat(stat), _nodeidx_foreval(nodeidx_foreval)
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
uint32_t GetResponse<key_t, val_t>::size() { // unused
	//return sizeof(optype_t) + sizeof(key_t) + sizeof(uint16_t) + val_t::MAX_VALLEN + sizeof(optype_t) + sizeof(bool) + DEBUG_BYTES;
	return sizeof(optype_t) + sizeof(key_t) + sizeof(uint16_t) + val_t::MAX_VALLEN + sizeof(optype_t) + sizeof(bool) + sizeof(uint16_t) + STAT_PADDING_BYTES;
}

template<class key_t, class val_t>
uint32_t GetResponse<key_t, val_t>::serialize(char * const data, uint32_t max_size) {
	//uint32_t my_size = this->size();
	//INVARIANT(max_size >= my_size);
	char *begin = data;
	uint32_t tmp_typesize = serialize_packet_type(this->_type, begin, max_size);
	begin += tmp_typesize;
	uint32_t tmp_keysize = this->_key.serialize(begin, max_size - tmp_typesize);
	begin += tmp_keysize;
	uint32_t tmp_valsize = this->_val.serialize(begin, max_size-tmp_typesize-tmp_keysize);
	begin += tmp_valsize;
	uint32_t tmp_shadowtypesize = serialize_packet_type(this->_type, begin, max_size - tmp_typesize - tmp_keysize - tmp_valsize); // shadowtype
	begin += tmp_shadowtypesize;
	memcpy(begin, (void *)&this->_stat, sizeof(bool));
	begin += sizeof(bool);
	uint16_t bigendian_nodeidx_foreval = htons(this->_nodeidx_foreval);
	memcpy(begin, (void *)&bigendian_nodeidx_foreval, sizeof(uint16_t));
	begin += sizeof(uint16_t);
	begin += STAT_PADDING_BYTES;
	return tmp_typesize + tmp_keysize + tmp_valsize + tmp_shadowtypesize + sizeof(bool) + sizeof(uint16_t) + STAT_PADDING_BYTES;
	//memset(begin, 0, DEBUG_BYTES);
	//return tmp_typesize + tmp_keysize + tmp_valsize + tmp_shadowtypesize + sizeof(bool) + DEBUG_BYTES;
}

template<class key_t, class val_t>
void GetResponse<key_t, val_t>::deserialize(const char * data, uint32_t recv_size) {
	//uint32_t my_size = this->size();
	//INVARIANT(my_size == recv_size);
	const char *begin = data;
	uint32_t tmp_typesize = deserialize_packet_type(this->_type, begin, recv_size);
	begin += tmp_typesize;
	uint32_t tmp_keysize = this->_key.deserialize(begin, recv_size - tmp_typesize);
	begin += tmp_keysize;
	uint32_t tmp_valsize = this->_val.deserialize(begin, recv_size - tmp_typesize - tmp_keysize);
	begin += tmp_valsize;
	begin += sizeof(optype_t); // deserialize shadowtype
	memcpy((void *)&this->_stat, begin, sizeof(bool));
	begin += sizeof(bool);
	memcpy(&this->_nodeidx_foreval, begin, sizeof(uint16_t));
	this->_nodeidx_foreval = ntohs(this->_nodeidx_foreval);
	begin += sizeof(uint16_t);
	begin += STAT_PADDING_BYTES;
}

// PutResponse (value must be any size)

template<class key_t>
PutResponse<key_t>::PutResponse(key_t key, bool stat, uint16_t nodeidx_foreval) 
	: Packet<key_t>(PacketType::PUTRES, key), _stat(stat), _nodeidx_foreval(nodeidx_foreval)
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
	//return sizeof(optype_t) + sizeof(key_t) + sizeof(optype_t) + sizeof(bool) + DEBUG_BYTES;
	return sizeof(optype_t) + sizeof(key_t) + sizeof(optype_t) + sizeof(bool) + sizeof(uint16_t) + STAT_PADDING_BYTES;
}

template<class key_t>
uint32_t PutResponse<key_t>::serialize(char * const data, uint32_t max_size) {
	uint32_t my_size = this->size();
	INVARIANT(max_size >= my_size);
	char *begin = data;
	uint32_t tmp_typesize = serialize_packet_type(this->_type, begin, max_size);
	begin += tmp_typesize;
	uint32_t tmp_keysize = this->_key.serialize(begin, max_size - tmp_typesize);
	begin += tmp_keysize; 
	uint32_t tmp_shadowtypesize = serialize_packet_type(this->_type, begin, max_size - tmp_typesize - tmp_keysize); // shadowtype
	begin += tmp_shadowtypesize;
	memcpy(begin, (void *)&this->_stat, sizeof(bool));
	begin += sizeof(bool);
	uint16_t bigendian_nodeidx_foreval = htons(this->_nodeidx_foreval);
	memcpy(begin, (void *)&bigendian_nodeidx_foreval, sizeof(uint16_t));
	begin += sizeof(uint16_t);
	begin += STAT_PADDING_BYTES;
	return tmp_typesize + tmp_keysize + tmp_shadowtypesize + sizeof(bool) + sizeof(uint16_t) + STAT_PADDING_BYTES;
	//memset(begin, 0, DEBUG_BYTES);
	//return tmp_typesize + tmp_keysize + tmp_shadowtypesize + sizeof(bool) + DEBUG_BYTES;
}

template<class key_t>
void PutResponse<key_t>::deserialize(const char * data, uint32_t recv_size) {
	uint32_t my_size = this->size();
	INVARIANT(my_size <= recv_size);
	const char *begin = data;
	uint32_t tmp_typesize = deserialize_packet_type(this->_type, begin, recv_size);
	begin += tmp_typesize;
	uint32_t tmp_keysize = this->_key.deserialize(begin, recv_size - tmp_typesize);
	begin += tmp_keysize;
	begin += sizeof(optype_t); // deserialize shadowtype
	memcpy((void *)&this->_stat, begin, sizeof(bool));
	begin += sizeof(bool);
	memcpy(&this->_nodeidx_foreval, begin, sizeof(uint16_t));
	this->_nodeidx_foreval = ntohs(this->_nodeidx_foreval);
	begin += sizeof(uint16_t);
	begin += STAT_PADDING_BYTES;
}

// DelResponse

template<class key_t>
DelResponse<key_t>::DelResponse(key_t key, bool stat, uint16_t nodeidx_foreval) 
	: Packet<key_t>(PacketType::DELRES, key), _stat(stat), _nodeidx_foreval(nodeidx_foreval)
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
	//return sizeof(optype_t) + sizeof(key_t) + sizeof(optype_t) + sizeof(bool) + DEBUG_BYTES;
	return sizeof(optype_t) + sizeof(key_t) + sizeof(optype_t) + sizeof(bool) + sizeof(uint16_t) + STAT_PADDING_BYTES;
}

template<class key_t>
uint32_t DelResponse<key_t>::serialize(char * const data, uint32_t max_size) {
	uint32_t my_size = this->size();
	INVARIANT(max_size >= my_size);
	char *begin = data;
	uint32_t tmp_typesize = serialize_packet_type(this->_type, begin, max_size);
	begin += tmp_typesize;
	uint32_t tmp_keysize = this->_key.serialize(begin, max_size - tmp_typesize);
	begin += tmp_keysize;
	uint32_t tmp_shadowtypesize = serialize_packet_type(this->_type, begin, max_size - tmp_typesize - tmp_keysize); // shadowtype
	begin += tmp_shadowtypesize;
	memcpy(begin, (void *)&this->_stat, sizeof(bool));
	begin += sizeof(bool);
	uint16_t bigendian_nodeidx_foreval = htons(this->_nodeidx_foreval);
	memcpy(begin, (void *)&bigendian_nodeidx_foreval, sizeof(uint16_t));
	begin += sizeof(uint16_t);
	begin += STAT_PADDING_BYTES;
	return tmp_typesize + tmp_keysize + tmp_shadowtypesize + sizeof(bool) + sizeof(uint16_t) + STAT_PADDING_BYTES;
	//memset(begin, 0, DEBUG_BYTES);
	//return tmp_typesize + tmp_keysize + tmp_shadowtypesize + sizeof(bool) + DEBUG_BYTES;
}

template<class key_t>
void DelResponse<key_t>::deserialize(const char * data, uint32_t recv_size) {
	uint32_t my_size = this->size();
	INVARIANT(my_size <= recv_size);
	const char *begin = data;
	uint32_t tmp_typesize = deserialize_packet_type(this->_type, begin, recv_size);
	begin += tmp_typesize;
	uint32_t tmp_keysize = this->_key.deserialize(begin, recv_size - tmp_typesize);
	begin += tmp_keysize;
	begin += sizeof(optype_t); // deserialize shadowtype
	memcpy((void *)&this->_stat, begin, sizeof(bool));
	begin += sizeof(bool);
	memcpy(&this->_nodeidx_foreval, begin, sizeof(uint16_t));
	this->_nodeidx_foreval = ntohs(this->_nodeidx_foreval);
	begin += sizeof(uint16_t);
	begin += STAT_PADDING_BYTES;
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
ScanResponseSplit<key_t, val_t>::ScanResponseSplit(key_t key, key_t endkey, uint16_t cur_scanidx, uint16_t max_scannum, uint16_t nodeidx_foreval, int snapshotid, int32_t pairnum, std::vector<std::pair<key_t, snapshot_record_t>> pairs) 
	: ScanRequestSplit<key_t>(key, endkey, cur_scanidx, max_scannum), _nodeidx_foreval(nodeidx_foreval), _snapshotid(snapshotid), _pairnum(pairnum)
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
	//return sizeof(optype_t) + sizeof(key_t) + sizeof(key_t) + sizeof(uint32_t) + sizeof(uint16_t) + sizeof(uint16_t) + sizeof(int32_t);
	return sizeof(optype_t) + sizeof(key_t) + sizeof(key_t) + SPLIT_PREV_BYTES + sizeof(uint16_t) + sizeof(uint16_t) + sizeof(uint16_t) + sizeof(int) + sizeof(int32_t);
}

template<class key_t, class val_t>
uint32_t ScanResponseSplit<key_t, val_t>::serialize(char * const data, uint32_t max_size) {
	uint32_t my_size = this->size();
	INVARIANT(max_size >= my_size);
	char *begin = data;
	uint32_t tmp_typesize = serialize_packet_type(this->_type, begin, max_size);
	begin += tmp_typesize;
	uint32_t tmp_keysize = this->_key.serialize(begin, max_size - tmp_typesize);
	begin += tmp_keysize;
	uint32_t tmp_endkeysize = this->_endkey.serialize(begin, max_size - tmp_typesize - tmp_keysize);
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
	uint16_t bigendian_nodeidx_foreval = htons(this->_nodeidx_foreval);
	memcpy(begin, (void *)&bigendian_nodeidx_foreval, sizeof(uint16_t));
	begin += sizeof(uint16_t);
	memcpy(begin, (void *)&this->_snapshotid, sizeof(int)); // directly use little-endian
	begin += sizeof(int);

	uint32_t bigendian_pairnum = htonl(uint32_t(this->_pairnum));
	memcpy(begin, (void *)&bigendian_pairnum, sizeof(int32_t));
	begin += sizeof(int32_t);
	uint32_t totalsize = tmp_typesize + tmp_keysize + tmp_endkeysize + SPLIT_PREV_BYTES + sizeof(uint16_t) + sizeof(uint16_t) + sizeof(uint16_t) + sizeof(int) + sizeof(int32_t);
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
	uint32_t tmp_typesize = dynamic_serialize_packet_type(this->_type, dynamic_data);
	tmpoff += tmp_typesize;
	uint32_t tmp_keysize = this->_key.dynamic_serialize(dynamic_data, tmpoff);
	tmpoff += tmp_keysize;
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
	uint16_t bigendian_nodeidx_foreval = htons(this->_nodeidx_foreval);
	dynamic_data.dynamic_memcpy(tmpoff, (char *)&bigendian_nodeidx_foreval, sizeof(uint16_t));
	tmpoff += sizeof(uint16_t);
	dynamic_data.dynamic_memcpy(tmpoff, (char *)&this->_snapshotid, sizeof(int)); // directly use little-endian
	tmpoff += sizeof(int);

	uint32_t bigendian_pairnum = htonl(uint32_t(this->_pairnum));
	dynamic_data.dynamic_memcpy(tmpoff, (char *)&bigendian_pairnum, sizeof(int32_t));
	tmpoff += sizeof(int32_t);
	uint32_t totalsize = tmp_typesize + tmp_keysize + tmp_endkeysize + SPLIT_PREV_BYTES + sizeof(uint16_t) + sizeof(uint16_t) + sizeof(uint16_t) + sizeof(int) + sizeof(int32_t);
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
	uint32_t tmp_typesize = deserialize_packet_type(this->_type, begin, recv_size);
	begin += tmp_typesize;
	uint32_t tmp_keysize = this->_key.deserialize(begin, recv_size - tmp_typesize);
	begin += tmp_keysize;
	uint32_t tmp_endkeysize = this->_endkey.deserialize(begin, recv_size - tmp_typesize - tmp_keysize);
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
		uint32_t tmp_pair_valsize = this->_pairs[pair_i].second.val.deserialize_large(begin, recv_size - totalsize);
		begin += tmp_pair_valsize;
		totalsize += tmp_pair_valsize;
	}
}

template<class key_t, class val_t>
size_t ScanResponseSplit<key_t, val_t>::get_frag_hdrsize() {
	//return sizeof(optype_t) + sizeof(key_t) + sizeof(key_t) + SPLIT_PREV_BYTES + sizeof(uint16_t) + sizeof(uint16_t) + sizeof(uint16_t); // op_hdr + scan_hdr + split_hdr + nodeidx_foreval (used to identify fragments from different servers)
	// NOTE: we only need nodeidx_foreval for entire split packet instead of each fragment; so we can place it in fragment body isntead of fragment header in udpsendlarge_ipfrag (see socket_helper.c)
	return sizeof(optype_t) + sizeof(key_t) + sizeof(key_t) + SPLIT_PREV_BYTES + sizeof(uint16_t) + sizeof(uint16_t); // op_hdr + scan_hdr + split_hdr
}

template<class key_t, class val_t>
size_t ScanResponseSplit<key_t, val_t>::get_srcnum_off() {
	return sizeof(optype_t) + sizeof(key_t) + sizeof(key_t) + SPLIT_PREV_BYTES + sizeof(uint16_t); // offset of split_hdr.max_scannum
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
	return sizeof(optype_t) + sizeof(key_t) + sizeof(key_t) + SPLIT_PREV_BYTES; // offset of split_hdr.cur_scanidx
}

template<class key_t, class val_t>
size_t ScanResponseSplit<key_t, val_t>::get_srcid_len() {
	return sizeof(uint16_t);
}

template<class key_t, class val_t>
bool ScanResponseSplit<key_t, val_t>::get_srcid_conversion() {
	return true;
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
	uint32_t tmp_typesize = serialize_packet_type(this->_type, begin, max_size);
	begin += tmp_typesize;
	uint32_t tmp_keysize = this->_key.serialize(begin, max_size - tmp_typesize);
	begin += tmp_keysize;
	uint32_t tmp_valsize = this->_val.serialize(begin, max_size-tmp_typesize-tmp_keysize);
	begin += tmp_valsize;
	uint32_t tmp_shadowtypesize = serialize_packet_type(this->_type, begin, max_size - tmp_typesize - tmp_keysize - tmp_valsize); // shadowtype
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
	return tmp_typesize + tmp_keysize + tmp_valsize + tmp_shadowtypesize + sizeof(uint32_t) + sizeof(bool) + sizeof(uint16_t) + STAT_PADDING_BYTES;
	//memset(begin, 0, DEBUG_BYTES);
	//return tmp_typesize + tmp_keysize + tmp_valsize + tmp_shadowtypesize + sizeof(uint32_t) + DEBUG_BYTES;
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
	//return sizeof(optype_t) + sizeof(key_t) + sizeof(uint16_t) + val_t::MAX_VALLEN + sizeof(optype_t) + sizeof(uint32_t) + sizeof(bool) + sizeof(uint16_t) + DEBUG_BYTES;
	return sizeof(optype_t) + sizeof(key_t) + sizeof(uint16_t) + val_t::MAX_VALLEN + sizeof(optype_t) + sizeof(uint32_t) + sizeof(bool) + sizeof(uint16_t) + STAT_PADDING_BYTES;
}

template<class key_t, class val_t>
void GetResponseLatestSeq<key_t, val_t>::deserialize(const char * data, uint32_t recv_size)
{
	COUT_N_EXIT("Invalid invoke of deserialize for GetResponseLatestSeq");
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
	//return sizeof(optype_t) + sizeof(key_t) + sizeof(uint16_t) + val_t::MAX_VALLEN + sizeof(optype_t) + sizeof(uint32_t) + sizeof(uint16_t) + sizeof(bool) + DEBUG_BYTES;
	return sizeof(optype_t) + sizeof(key_t) + sizeof(uint16_t) + val_t::MAX_VALLEN + sizeof(optype_t) + sizeof(uint32_t) + INSWITCH_PREV_BYTES + sizeof(uint16_t) + sizeof(bool) + sizeof(uint16_t) + STAT_PADDING_BYTES + CLONE_BYTES;
}

template<class key_t, class val_t>
uint32_t GetResponseLatestSeqInswitchCase1<key_t, val_t>::serialize(char * const data, uint32_t max_size) {
	//uint32_t my_size = this->size();
	//INVARIANT(max_size >= my_size);
	char *begin = data;
	uint32_t tmp_typesize = serialize_packet_type(this->_type, begin, max_size);
	begin += tmp_typesize;
	uint32_t tmp_keysize = this->_key.serialize(begin, max_size - tmp_typesize);
	begin += tmp_keysize;
	uint32_t tmp_valsize = this->_val.serialize(begin, max_size-tmp_typesize-tmp_keysize);
	begin += tmp_valsize;
	uint32_t tmp_shadowtypesize = serialize_packet_type(this->_type, begin, max_size - tmp_typesize - tmp_keysize - tmp_valsize); // shadowtype
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
	return tmp_typesize + tmp_keysize + tmp_valsize + tmp_shadowtypesize + sizeof(uint32_t) + INSWITCH_PREV_BYTES + sizeof(uint16_t) + sizeof(bool) + sizeof(uint16_t) + STAT_PADDING_BYTES + CLONE_BYTES;
	//begin += CLONE_BYTES;
	//memset(begin, 0, DEBUG_BYTES);
	//return tmp_typesize + tmp_keysize + tmp_valsize + tmp_shadowtypesize + sizeof(uint32_t) + INSWITCH_PREV_BYTES + sizeof(uint16_t) + sizeof(bool) + CLONE_BYTES + DEBUG_BYTES;
}

template<class key_t, class val_t>
void GetResponseLatestSeqInswitchCase1<key_t, val_t>::deserialize(const char * data, uint32_t recv_size) {
	//uint32_t my_size = this->size();
	//INVARIANT(my_size == recv_size);
	const char *begin = data;
	uint32_t tmp_typesize = deserialize_packet_type(this->_type, begin, recv_size);
	begin += tmp_typesize;
	uint32_t tmp_keysize = this->_key.deserialize(begin, recv_size - tmp_typesize);
	begin += tmp_keysize;
	uint32_t tmp_valsize = this->_val.deserialize(begin, recv_size - tmp_typesize - tmp_keysize);
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
	: Packet<key_t>(packet_type_t::PUTREQ_SEQ, key), _val(val), _seq(seq)
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
	return sizeof(optype_t) + sizeof(key_t) + sizeof(uint16_t) + val_t::MAX_VALLEN + sizeof(optype_t) + sizeof(uint32_t);
}

template<class key_t, class val_t>
void PutRequestSeq<key_t, val_t>::deserialize(const char * data, uint32_t recv_size)
{
	//uint32_t my_size = this->size();
	//INVARIANT(my_size == recv_size);
	const char *begin = data;
	uint32_t tmp_typesize = deserialize_packet_type(this->_type, begin, recv_size);
	begin += tmp_typesize;
	uint32_t tmp_keysize = this->_key.deserialize(begin, recv_size - tmp_typesize);
	begin += tmp_keysize;
	uint32_t tmp_valsize = this->_val.deserialize(begin, recv_size - tmp_typesize - tmp_keysize);
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
	return sizeof(optype_t) + sizeof(key_t) + sizeof(optype_t) + sizeof(uint32_t);
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
	uint32_t tmp_typesize = deserialize_packet_type(this->_type, begin, recv_size);
	begin += tmp_typesize;
	uint32_t tmp_keysize = this->_key.deserialize(begin, recv_size - tmp_typesize);
	begin += tmp_keysize;
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
	: ScanRequest<key_t>(), _cur_scanidx(0), _max_scannum(0)
{
}

template<class key_t>
ScanRequestSplit<key_t>::ScanRequestSplit(key_t key, key_t endkey, uint16_t cur_scanidx, uint16_t max_scannum)
	: ScanRequest<key_t>(key, endkey), _cur_scanidx(cur_scanidx), _max_scannum(max_scannum)
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
uint32_t ScanRequestSplit<key_t>::size() {
	return sizeof(optype_t) + sizeof(key_t) + sizeof(key_t) + SPLIT_PREV_BYTES + sizeof(uint16_t) + sizeof(uint16_t);// + sizeof(uint32_t);
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
	uint32_t tmp_typesize = deserialize_packet_type(this->_type, begin, recv_size);
	begin += tmp_typesize;
	uint32_t tmp_keysize = this->_key.deserialize(begin, recv_size - tmp_typesize);
	begin += tmp_keysize;
	uint32_t tmp_endkeysize = this->_endkey.deserialize(begin, recv_size - tmp_typesize - tmp_keysize);
	begin += tmp_endkeysize;
	//memcpy((void *)&this->_num, begin, sizeof(uint32_t));
	//begin += sizeof(uint32_t);
	begin += SPLIT_PREV_BYTES;
	memcpy((void *)&this->_cur_scanidx, begin, sizeof(uint16_t));
	this->_cur_scanidx = ntohs(this->_cur_scanidx);
	begin += sizeof(uint16_t);
	memcpy((void *)&this->_max_scannum, begin, sizeof(uint16_t));
	this->_max_scannum = ntohs(this->_max_scannum);
	//begin += sizeof(uint16_t);
}

// CachePop (value must <= 128B; only used in end-hosts)

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
	uint32_t tmp_typesize = serialize_packet_type(this->_type, begin, max_size);
	begin += tmp_typesize;
	uint32_t tmp_keysize = this->_key.serialize(begin, max_size - tmp_typesize);
	begin += tmp_keysize;
	uint32_t tmp_valsize = this->_val.serialize(begin, max_size-tmp_typesize-tmp_keysize);
	begin += tmp_valsize;
	uint32_t bigendian_seq = htonl(this->_seq);
	memcpy(begin, (void *)&bigendian_seq, sizeof(uint32_t)); // little-endian to big-endian
	begin += sizeof(uint32_t);
	memcpy(begin, (void *)&this->_stat, sizeof(bool));
	begin += sizeof(bool);
	uint16_t bigendian_serveridx = htons(uint16_t(this->_serveridx));
	memcpy(begin, (void *)&bigendian_serveridx, sizeof(uint16_t)); // little-endian to big-endian
	return tmp_typesize + tmp_keysize + tmp_valsize + sizeof(uint32_t) + sizeof(bool) + sizeof(uint16_t);
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
	return sizeof(optype_t) + sizeof(key_t) + sizeof(uint16_t) + val_t::MAX_VALLEN + sizeof(uint32_t) + sizeof(bool) + sizeof(uint16_t);
}

template<class key_t, class val_t>
void CachePop<key_t, val_t>::deserialize(const char * data, uint32_t recv_size)
{
	//uint32_t my_size = this->size();
	//INVARIANT(my_size == recv_size);
	const char *begin = data;
	uint32_t tmp_typesize = deserialize_packet_type(this->_type, begin, recv_size);
	begin += tmp_typesize;
	uint32_t tmp_keysize = this->_key.deserialize(begin, recv_size - tmp_typesize);
	begin += tmp_keysize;
	uint32_t tmp_valsize = this->_val.deserialize(begin, recv_size - tmp_typesize - tmp_keysize);
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
CachePopInswitch<key_t, val_t>::CachePopInswitch(key_t key, val_t val, uint32_t seq, uint16_t freeidx, bool stat)
	: PutRequestSeq<key_t, val_t>(key, val, seq), _freeidx(freeidx), _stat(stat)
{
	this->_type = static_cast<optype_t>(PacketType::CACHE_POP_INSWITCH);
	INVARIANT(this->_val.val_length <= val_t::SWITCH_MAX_VALLEN);
	INVARIANT(seq >= 0);
	INVARIANT(freeidx >= 0);
}

template<class key_t, class val_t>
uint32_t CachePopInswitch<key_t, val_t>::serialize(char * const data, uint32_t max_size) {
	//uint32_t my_size = this->size();
	//INVARIANT(max_size >= my_size);
	char *begin = data;
	uint32_t tmp_typesize = serialize_packet_type(this->_type, begin, max_size);
	begin += tmp_typesize;
	uint32_t tmp_keysize = this->_key.serialize(begin, max_size - tmp_typesize);
	begin += tmp_keysize;
	uint32_t tmp_valsize = this->_val.serialize(begin, max_size-tmp_typesize-tmp_keysize);
	begin += tmp_valsize;
	uint32_t tmp_shadowtypesize = serialize_packet_type(this->_type, begin, max_size - tmp_typesize - tmp_keysize - tmp_valsize); // shadowtype
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
	return tmp_typesize + tmp_keysize + tmp_valsize + tmp_shadowtypesize + sizeof(uint32_t) + INSWITCH_PREV_BYTES + sizeof(uint16_t) + sizeof(bool) + sizeof(uint16_t) + STAT_PADDING_BYTES;
	//memset(begin, 0, DEBUG_BYTES);
	//return tmp_typesize + tmp_keysize + tmp_valsize + tmp_shadowtypesize + sizeof(uint32_t) + INSWITCH_PREV_BYTES + sizeof(uint16_t) + DEBUG_BYTES;
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
	return sizeof(optype_t) + sizeof(key_t) + sizeof(uint16_t) + val_t::MAX_VALLEN + sizeof(optype_t) + sizeof(uint32_t) + INSWITCH_PREV_BYTES + sizeof(uint16_t) + sizeof(bool) + sizeof(uint16_t) + STAT_PADDING_BYTES;
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
	return sizeof(optype_t) + sizeof(key_t) + sizeof(uint16_t) + val_t::MAX_VALLEN + sizeof(uint32_t) + sizeof(bool) + sizeof(uint16_t);
}

template<class key_t, class val_t>
uint32_t CacheEvict<key_t, val_t>::serialize(char * const data, uint32_t max_size) {
	//uint32_t my_size = this->size();
	//INVARIANT(max_size >= my_size);
	char *begin = data;
	uint32_t tmp_typesize = serialize_packet_type(this->_type, begin, max_size);
	begin += tmp_typesize;
	uint32_t tmp_keysize = this->_key.serialize(begin, max_size - tmp_typesize);
	begin += tmp_keysize;
	uint32_t tmp_valsize = this->_val.serialize(begin, max_size-tmp_typesize-tmp_keysize);
	begin += tmp_valsize;
	uint32_t bigendian_seq = htonl(this->_seq);
	memcpy(begin, (void *)&bigendian_seq, sizeof(uint32_t));
	begin += sizeof(uint32_t);
	memcpy(begin, (void *)&this->_stat, sizeof(bool));
	begin += sizeof(bool);
	uint16_t bigendian_serveridx = htons(uint16_t(this->_serveridx));
	memcpy(begin, (void *)&bigendian_serveridx, sizeof(uint16_t));
	return tmp_typesize + tmp_keysize + tmp_valsize + sizeof(uint32_t) + sizeof(bool) + sizeof(uint16_t);
}

template<class key_t, class val_t>
void CacheEvict<key_t, val_t>::deserialize(const char * data, uint32_t recv_size) {
	//uint32_t my_size = this->size();
	//INVARIANT(my_size == recv_size);
	const char *begin = data;
	uint32_t tmp_typesize = deserialize_packet_type(this->_type, begin, recv_size);
	begin += tmp_typesize;
	uint32_t tmp_keysize = this->_key.deserialize(begin, recv_size - tmp_typesize);
	begin += tmp_keysize;
	uint32_t tmp_valsize = this->_val.deserialize(begin, recv_size - tmp_typesize - tmp_keysize);
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
	: GetRequest<key_t>(key)
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
WarmupRequest<key_t>::WarmupRequest(key_t key) 
	: GetRequest<key_t>(key)
{
	this->_type = static_cast<optype_t>(PacketType::WARMUPREQ);
}

template<class key_t>
WarmupRequest<key_t>::WarmupRequest(const char * data, uint32_t recv_size) {
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::WARMUPREQ);
}

// WarmupAck

template<class key_t>
WarmupAck<key_t>::WarmupAck(key_t key) 
	: GetRequest<key_t>(key)
{
	this->_type = static_cast<optype_t>(PacketType::WARMUPACK);
}

template<class key_t>
WarmupAck<key_t>::WarmupAck(const char * data, uint32_t recv_size) {
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::WARMUPACK);
}

// LoadRequest (value can be any size)

template<class key_t, class val_t>
LoadRequest<key_t, val_t>::LoadRequest(key_t key, val_t val, uint16_t client_logical_idx, uint32_t fragseq) 
	: Packet<key_t>(PacketType::LOADREQ, key), _val(val), _client_logical_idx(client_logical_idx), _fragseq(fragseq)
{
	// NOTE: val can be any size for LOADREQ
}

template<class key_t, class val_t>
LoadRequest<key_t, val_t>::LoadRequest(const char * data, uint32_t recv_size) {
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
	return sizeof(optype_t) + sizeof(key_t) + sizeof(uint16_t) + sizeof(uint32_t) + sizeof(uint32_t) + val_t::SWITCH_MAX_VALLEN;
}

template<class key_t, class val_t>
size_t LoadRequest<key_t, val_t>::get_frag_hdrsize() {
	return sizeof(optype_t) + sizeof(key_t) + sizeof(uint16_t) + sizeof(uint32_t); // op_hdr + client_logical_idx + fragseq
}

template<class key_t, class val_t>
uint32_t LoadRequest<key_t, val_t>::dynamic_serialize(dynamic_array_t &dynamic_data) {
	int tmpoff = 0;
	uint32_t tmp_typesize = dynamic_serialize_packet_type(this->_type, dynamic_data);
	tmpoff += tmp_typesize;
	uint32_t tmp_keysize = this->_key.dynamic_serialize(dynamic_data, tmpoff);
	tmpoff += tmp_keysize;
	uint16_t bigendian_client_logical_idx = htons(this->_client_logical_idx);
	dynamic_data.dynamic_memcpy(tmpoff, (char *)&bigendian_client_logical_idx, sizeof(uint16_t));
	tmpoff += sizeof(uint16_t);
	uint32_t bigendian_fragseq = htonl(this->_fragseq);
	dynamic_data.dynamic_memcpy(tmpoff, (char *)&bigendian_fragseq, sizeof(uint32_t));
	tmpoff += sizeof(uint32_t);
	uint32_t tmp_valsize = this->_val.dynamic_serialize_large(dynamic_data, tmpoff);
	tmpoff += tmp_valsize;
	return tmp_typesize + tmp_keysize + sizeof(uint16_t) + sizeof(uint32_t) + tmp_valsize;
}

template<class key_t, class val_t>
uint32_t LoadRequest<key_t, val_t>::serialize(char * const data, uint32_t max_size) {
	uint32_t my_size = this->size();
	INVARIANT(max_size >= my_size);
	char *begin = data;
	uint32_t tmp_typesize = serialize_packet_type(this->_type, begin, max_size);
	begin += tmp_typesize;
	uint32_t tmp_keysize = this->_key.serialize(begin, max_size - tmp_typesize);
	begin += tmp_keysize;
	uint16_t bigendian_client_logical_idx = htons(this->_client_logical_idx);
	memcpy(begin, &bigendian_client_logical_idx, sizeof(uint16_t));
	begin += sizeof(uint16_t);
	uint32_t bigendian_fragseq = htonl(this->_fragseq);
	memcpy(begin, &bigendian_fragseq, sizeof(uint32_t));
	begin += sizeof(uint32_t);
	uint32_t tmp_valsize = this->_val.serialize_large(begin, max_size - tmp_typesize - tmp_keysize - sizeof(uint16_t) - sizeof(uint32_t));
	begin += tmp_valsize;
	//uint32_t tmp_shadowtypesize = serialize_packet_type(this->_type, begin, max_size - tmp_ophdrsize - tmp_valsize); // shadowtype
	//begin += tmp_shadowtypesize;
	return tmp_typesize + tmp_keysize + sizeof(uint16_t) + sizeof(uint32_t) + tmp_valsize;// + tmp_shadowtypesize;
}

template<class key_t, class val_t>
void LoadRequest<key_t, val_t>::deserialize(const char * data, uint32_t recv_size) {
	uint32_t my_size = this->size();
	INVARIANT(my_size <= recv_size);
	const char *begin = data;
	uint32_t tmp_typesize = deserialize_packet_type(this->_type, begin, recv_size);
	begin += tmp_typesize;
	uint32_t tmp_keysize = this->_key.deserialize(begin, recv_size - tmp_typesize);
	begin += tmp_keysize;
	memcpy(&this->_client_logical_idx, begin, sizeof(uint16_t));
	this->_client_logical_idx = ntohs(this->_client_logical_idx);
	begin += sizeof(uint16_t);
	memcpy(&this->_fragseq, begin, sizeof(uint32_t));
	this->_fragseq = ntohs(this->_fragseq);
	begin += sizeof(uint32_t);
	uint32_t tmp_valsize = this->_val.deserialize_large(begin, recv_size - tmp_typesize - tmp_keysize - sizeof(uint16_t) - sizeof(uint32_t));
	UNUSED(tmp_valsize);
	begin += tmp_valsize;
	//// deserialize shadowtype
}

// LoadAck

template<class key_t>
LoadAck<key_t>::LoadAck(key_t key) 
	: GetRequest<key_t>(key)
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
	: GetRequest<key_t>(key)
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
CacheEvictLoadfreqInswitch<key_t>::CacheEvictLoadfreqInswitch(key_t key, uint16_t evictidx)
	: Packet<key_t>(PacketType::CACHE_EVICT_LOADFREQ_INSWITCH, key), _evictidx(evictidx)
{
	INVARIANT(evictidx >= 0);
}

template<class key_t>
uint32_t CacheEvictLoadfreqInswitch<key_t>::serialize(char * const data, uint32_t max_size) {
	uint32_t my_size = this->size();
	INVARIANT(max_size >= my_size);
	char *begin = data;
	uint32_t tmp_typesize = serialize_packet_type(this->_type, begin, max_size);
	begin += tmp_typesize;
	uint32_t tmp_keysize = this->_key.serialize(begin, max_size - tmp_typesize);
	begin += tmp_keysize;
	uint32_t tmp_shadowtypesize = serialize_packet_type(this->_type, begin, max_size - tmp_typesize - tmp_keysize); // shadowtype
	begin += tmp_shadowtypesize;
	memset(begin, 0, INSWITCH_PREV_BYTES); // the first bytes of inswitch_hdr
	begin += INSWITCH_PREV_BYTES;
	uint16_t bigendian_evictidx = htons(uint16_t(this->_evictidx));
	memcpy(begin, (void *)&bigendian_evictidx, sizeof(uint16_t)); // little-endian to big-endian
	return tmp_typesize + tmp_keysize + tmp_shadowtypesize + INSWITCH_PREV_BYTES + sizeof(uint16_t);
}

template<class key_t>
uint16_t CacheEvictLoadfreqInswitch<key_t>::evictidx() const {
	return _evictidx;
}

template<class key_t>
uint32_t CacheEvictLoadfreqInswitch<key_t>::size() { // unused
	return sizeof(optype_t) + sizeof(key_t) + sizeof(optype_t) + INSWITCH_PREV_BYTES + sizeof(uint16_t);
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
	uint32_t tmp_typesize = deserialize_packet_type(this->_type, begin, recv_size);
	begin += tmp_typesize;
	uint32_t tmp_keysize = this->_key.deserialize(begin, recv_size - tmp_typesize);
	begin += tmp_keysize;
	memcpy(&this->_frequency, begin, sizeof(uint32_t));
	this->_frequency = ntohl(this->_frequency);
}

template<class key_t>
uint32_t CacheEvictLoadfreqInswitchAck<key_t>::frequency() const {
	return _frequency;
}

template<class key_t>
uint32_t CacheEvictLoadfreqInswitchAck<key_t>::size() { // unused
	return sizeof(optype_t) + sizeof(key_t) + sizeof(uint32_t);
}

template<class key_t>
uint32_t CacheEvictLoadfreqInswitchAck<key_t>::serialize(char * const data, uint32_t max_size) {
	COUT_N_EXIT("Invalid invoke of deserialize for CacheEvictLoadfreqInswitchAck");
}

// CacheEvictLoaddataInswitch

template<class key_t>
CacheEvictLoaddataInswitch<key_t>::CacheEvictLoaddataInswitch(key_t key, uint16_t evictidx)
	: CacheEvictLoadfreqInswitch<key_t>(key, evictidx)
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
	return sizeof(optype_t) + sizeof(key_t) + sizeof(uint16_t) + val_t::MAX_VALLEN + sizeof(optype_t) + sizeof(uint32_t) + sizeof(bool) + sizeof(uint16_t) + STAT_PADDING_BYTES;
}

template<class key_t, class val_t>
void CacheEvictLoaddataInswitchAck<key_t, val_t>::deserialize(const char * data, uint32_t recv_size) {
	uint32_t my_size = this->size();
	INVARIANT(my_size <= recv_size);
	const char *begin = data;
	uint32_t tmp_typesize = deserialize_packet_type(this->_type, begin, recv_size);
	begin += tmp_typesize;
	uint32_t tmp_keysize = this->_key.deserialize(begin, recv_size - tmp_typesize);
	begin += tmp_keysize;
	uint32_t tmp_valsize = this->_val.deserialize(begin, recv_size - tmp_typesize - tmp_keysize);
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
LoadsnapshotdataInswitch<key_t>::LoadsnapshotdataInswitch(key_t key, uint16_t loadidx)
	: CacheEvictLoadfreqInswitch<key_t>(key, loadidx)
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
	return sizeof(optype_t) + sizeof(key_t) + sizeof(uint16_t) + val_t::MAX_VALLEN + sizeof(optype_t) + sizeof(uint32_t) + INSWITCH_PREV_BYTES + sizeof(uint16_t) + sizeof(bool) + sizeof(uint16_t) + STAT_PADDING_BYTES;
}

template<class key_t, class val_t>
void LoadsnapshotdataInswitchAck<key_t, val_t>::deserialize(const char * data, uint32_t recv_size) {
	//uint32_t my_size = this->size();
	//INVARIANT(my_size == recv_size);
	const char *begin = data;
	uint32_t tmp_typesize = deserialize_packet_type(this->_type, begin, recv_size);
	begin += tmp_typesize;
	uint32_t tmp_keysize = this->_key.deserialize(begin, recv_size - tmp_typesize);
	begin += tmp_keysize;
	uint32_t tmp_valsize = this->_val.deserialize(begin, recv_size - tmp_typesize - tmp_keysize);
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
SetvalidInswitch<key_t>::SetvalidInswitch(key_t key, uint16_t idx, uint8_t validvalue)
	: Packet<key_t>(PacketType::SETVALID_INSWITCH, key), _idx(idx), _validvalue(validvalue)
{
	INVARIANT(idx >= 0);
	INVARIANT(validvalue == 0 || validvalue == 1 || validvalue == 3);
}

template<class key_t>
uint32_t SetvalidInswitch<key_t>::serialize(char * const data, uint32_t max_size) {
	uint32_t my_size = this->size();
	INVARIANT(max_size >= my_size);
	char *begin = data;
	uint32_t tmp_typesize = serialize_packet_type(this->_type, begin, max_size);
	begin += tmp_typesize;
	uint32_t tmp_keysize = this->_key.serialize(begin, max_size - tmp_typesize);
	begin += tmp_keysize;
	uint32_t tmp_shadowtypesize = serialize_packet_type(this->_type, begin, max_size - tmp_typesize - tmp_keysize); // shadowtype
	begin += tmp_shadowtypesize;
	memset(begin, 0, INSWITCH_PREV_BYTES); // the first bytes of inswitch_hdr
	begin += INSWITCH_PREV_BYTES;
	uint16_t bigendian_idx = htons(uint16_t(this->_idx));
	memcpy(begin, (void *)&bigendian_idx, sizeof(uint16_t)); // little-endian to big-endian
	begin += sizeof(uint16_t);
	memcpy(begin, (void *)&_validvalue, sizeof(uint8_t));
	return tmp_typesize + tmp_keysize + tmp_shadowtypesize + INSWITCH_PREV_BYTES + sizeof(uint16_t) + sizeof(uint8_t);
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
	return sizeof(optype_t) + sizeof(key_t) + sizeof(optype_t) + INSWITCH_PREV_BYTES + sizeof(uint16_t) + sizeof(uint8_t);
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

// PutRequestLargevalue (value must > 128B)

template<class key_t, class val_t>
PutRequestLargevalue<key_t, val_t>::PutRequestLargevalue()
	: Packet<key_t>(), _val(), _client_logical_idx(0), _fragseq(0)
{
}

template<class key_t, class val_t>
PutRequestLargevalue<key_t, val_t>::PutRequestLargevalue(key_t key, val_t val, uint16_t client_logical_idx, uint32_t fragseq) 
	: Packet<key_t>(PacketType::PUTREQ_LARGEVALUE, key), _val(val), _client_logical_idx(client_logical_idx), _fragseq(fragseq)
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
uint16_t PutRequestLargevalue<key_t, val_t>::client_logical_idx() const {
	return _client_logical_idx;
}

template<class key_t, class val_t>
uint32_t PutRequestLargevalue<key_t, val_t>::fragseq() const {
	return _fragseq;
}

template<class key_t, class val_t>
uint32_t PutRequestLargevalue<key_t, val_t>::size() { // not used
	return sizeof(optype_t) + sizeof(key_t) + sizeof(uint16_t) + sizeof(uint32_t) + sizeof(uint32_t) + val_t::SWITCH_MAX_VALLEN;
}

template<class key_t, class val_t>
size_t PutRequestLargevalue<key_t, val_t>::get_frag_hdrsize() {
	return sizeof(optype_t) + sizeof(key_t) + sizeof(uint16_t) + sizeof(uint32_t); // op_hdr + client_logical_idx + fragseq
}

template<class key_t, class val_t>
uint32_t PutRequestLargevalue<key_t, val_t>::dynamic_serialize(dynamic_array_t &dynamic_data) {
	int tmpoff = 0;
	uint32_t tmp_typesize = dynamic_serialize_packet_type(this->_type, dynamic_data);
	tmpoff += tmp_typesize;
	uint32_t tmp_keysize = this->_key.dynamic_serialize(dynamic_data, tmpoff);
	tmpoff += tmp_keysize;
	uint16_t bigendian_client_logical_idx = htons(this->_client_logical_idx);
	dynamic_data.dynamic_memcpy(tmpoff, (char *)&bigendian_client_logical_idx, sizeof(uint16_t));
	tmpoff += sizeof(uint16_t);
	uint32_t bigendian_fragseq = htonl(this->_fragseq);
	dynamic_data.dynamic_memcpy(tmpoff, (char *)&bigendian_fragseq, sizeof(uint32_t));
	tmpoff += sizeof(uint32_t);
	uint32_t tmp_valsize = this->_val.dynamic_serialize_large(dynamic_data, tmpoff);
	tmpoff += tmp_valsize;
	return tmp_typesize + tmp_keysize + sizeof(uint16_t) + sizeof(uint32_t) + tmp_valsize;
}

template<class key_t, class val_t>
uint32_t PutRequestLargevalue<key_t, val_t>::serialize(char * const data, uint32_t max_size) {
	uint32_t my_size = this->size();
	INVARIANT(max_size >= my_size);
	char *begin = data;
	uint32_t tmp_typesize = serialize_packet_type(this->_type, begin, max_size);
	begin += tmp_typesize;
	uint32_t tmp_keysize = this->_key.serialize(begin, max_size - tmp_typesize);
	begin += tmp_keysize;
	uint16_t bigendian_client_logical_idx = htons(this->_client_logical_idx);
	memcpy(begin, &bigendian_client_logical_idx, sizeof(uint16_t));
	begin += sizeof(uint16_t);
	uint32_t bigendian_fragseq = htonl(this->_fragseq);
	memcpy(begin, &bigendian_fragseq, sizeof(uint32_t));
	begin += sizeof(uint32_t);
	uint32_t tmp_valsize = this->_val.serialize_large(begin, max_size - tmp_typesize - tmp_keysize - sizeof(uint16_t) - sizeof(uint32_t));
	begin += tmp_valsize;
	return tmp_typesize + tmp_keysize + sizeof(uint16_t) + sizeof(uint32_t) + tmp_valsize;
}

template<class key_t, class val_t>
void PutRequestLargevalue<key_t, val_t>::deserialize(const char * data, uint32_t recv_size) {
	uint32_t my_size = this->size();
	INVARIANT(my_size <= recv_size);
	const char *begin = data;
	uint32_t tmp_typesize = deserialize_packet_type(this->_type, begin, recv_size);
	begin += tmp_typesize;
	uint32_t tmp_keysize = this->_key.deserialize(begin, recv_size - tmp_typesize);
	begin += tmp_keysize;
	memcpy(&this->_client_logical_idx, begin, sizeof(uint16_t));
	this->_client_logical_idx = ntohs(this->_client_logical_idx);
	begin += sizeof(uint16_t);
	memcpy(&this->_fragseq, begin, sizeof(uint32_t));
	this->_fragseq = ntohs(this->_fragseq);
	begin += sizeof(uint32_t);
	uint32_t tmp_valsize = this->_val.deserialize_large(begin, recv_size - tmp_typesize - tmp_keysize - sizeof(uint16_t) - sizeof(uint32_t));
	UNUSED(tmp_valsize);
	begin += tmp_valsize;
}

// PutRequestLargevalueSeq (value must > 128B)

template<class key_t, class val_t>
PutRequestLargevalueSeq<key_t, val_t>::PutRequestLargevalueSeq(key_t key, val_t val, uint32_t seq, uint16_t client_logical_idx, uint32_t fragseq) 
	: PutRequestLargevalue<key_t, val_t>(key, val, client_logical_idx, fragseq), _seq(seq)
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
	return sizeof(optype_t) + sizeof(key_t) + sizeof(optype_t) + sizeof(uint32_t) + sizeof(uint16_t) + sizeof(uint32_t) + sizeof(uint32_t) + val_t::SWITCH_MAX_VALLEN;
}

template<class key_t, class val_t>
size_t PutRequestLargevalueSeq<key_t, val_t>::get_frag_hdrsize() {
	return sizeof(optype_t) + sizeof(key_t) + sizeof(optype_t) + sizeof(uint32_t) + sizeof(uint16_t) + sizeof(uint32_t); // op_hdr + shadowtype_hdr + seq_hdr + client_logical_idx + fragseq
}

template<class key_t, class val_t>
uint32_t PutRequestLargevalueSeq<key_t, val_t>::serialize(char * const data, uint32_t max_size) {
	uint32_t my_size = this->size();
	INVARIANT(max_size >= my_size);
	char *begin = data;
	uint32_t tmp_typesize = serialize_packet_type(this->_type, begin, max_size);
	begin += tmp_typesize;
	uint32_t tmp_keysize = this->_key.serialize(begin, max_size - tmp_typesize);
	begin += tmp_keysize;
	uint32_t tmp_shadowtypesize = serialize_packet_type(this->_type, begin, max_size - tmp_typesize - tmp_keysize); // shadowtype
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
	uint32_t tmp_valsize = this->_val.serialize_large(begin, max_size - tmp_typesize - tmp_keysize - tmp_shadowtypesize - sizeof(uint32_t) - sizeof(uint16_t) - sizeof(uint32_t));
	begin += tmp_valsize;
	return tmp_typesize + tmp_keysize + tmp_shadowtypesize + sizeof(uint32_t) + sizeof(uint16_t) + sizeof(uint32_t) + tmp_valsize;
}

template<class key_t, class val_t>
void PutRequestLargevalueSeq<key_t, val_t>::deserialize(const char * data, uint32_t recv_size) {
	uint32_t my_size = this->size();
	INVARIANT(my_size <= recv_size);
	const char *begin = data;
	uint32_t tmp_typesize = deserialize_packet_type(this->_type, begin, recv_size);
	begin += tmp_typesize;
	uint32_t tmp_keysize = this->_key.deserialize(begin, recv_size - tmp_typesize);
	begin += tmp_keysize;
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
	uint32_t tmp_valsize = this->_val.deserialize_large(begin, recv_size - tmp_typesize - tmp_keysize - sizeof(optype_t) - sizeof(uint32_t) - sizeof(uint16_t) - sizeof(uint32_t));
	UNUSED(tmp_valsize);
	begin += tmp_valsize;
}

// PutRequestLargevalueSeqCached (value must > 128B)

template<class key_t, class val_t>
PutRequestLargevalueSeqCached<key_t, val_t>::PutRequestLargevalueSeqCached(key_t key, val_t val, uint32_t seq, uint16_t client_logical_idx, uint32_t fragseq) 
	: PutRequestLargevalueSeq<key_t, val_t>(key, val, seq, client_logical_idx, fragseq)
{	
	this->_type = static_cast<optype_t>(packet_type_t::PUTREQ_LARGEVALUE_SEQ_CACHED);
	INVARIANT(this->_val.val_length > val_t::SWITCH_MAX_VALLEN);
}

template<class key_t, class val_t>
PutRequestLargevalueSeqCached<key_t, val_t>::PutRequestLargevalueSeqCached(const char * data, uint32_t recv_size) {
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::PUTREQ_LARGEVALUE_SEQ_CACHED);
	INVARIANT(this->_val.val_length > val_t::SWITCH_MAX_VALLEN);
}

template<class key_t, class val_t>
size_t PutRequestLargevalueSeqCached<key_t, val_t>::get_frag_hdrsize() {
	return sizeof(optype_t) + sizeof(key_t) + sizeof(optype_t) + sizeof(uint32_t) + sizeof(uint16_t) + sizeof(uint32_t); // op_hdr + shadowtype_hdr + seq_hdr + client_logical_idx + fragseq
}

// PutRequestLargevalueSeqCase3 (value must > 128B)

template<class key_t, class val_t>
PutRequestLargevalueSeqCase3<key_t, val_t>::PutRequestLargevalueSeqCase3(key_t key, val_t val, uint32_t seq, uint16_t client_logical_idx, uint32_t fragseq) 
	: PutRequestLargevalueSeq<key_t, val_t>(key, val, seq, client_logical_idx, fragseq)
{	
	this->_type = static_cast<optype_t>(packet_type_t::PUTREQ_LARGEVALUE_SEQ_CASE3);
	INVARIANT(this->_val.val_length > val_t::SWITCH_MAX_VALLEN);
}

template<class key_t, class val_t>
PutRequestLargevalueSeqCase3<key_t, val_t>::PutRequestLargevalueSeqCase3(const char * data, uint32_t recv_size) {
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::PUTREQ_LARGEVALUE_SEQ_CASE3);
	INVARIANT(this->_val.val_length > val_t::SWITCH_MAX_VALLEN);
}

template<class key_t, class val_t>
size_t PutRequestLargevalueSeqCase3<key_t, val_t>::get_frag_hdrsize() {
	return sizeof(optype_t) + sizeof(key_t) + sizeof(optype_t) + sizeof(uint32_t) + sizeof(uint16_t) + sizeof(uint32_t); // op_hdr + shadowtype_hdr + seq_hdr + client_logical_idx + fragseq
}

// GetResponseLargevalue (value must > 128B)

template<class key_t, class val_t>
GetResponseLargevalue<key_t, val_t>::GetResponseLargevalue()
	: Packet<key_t>(), _val(), _stat(false), _nodeidx_foreval(0)
{
}

template<class key_t, class val_t>
GetResponseLargevalue<key_t, val_t>::GetResponseLargevalue(key_t key, val_t val, bool stat, uint16_t nodeidx_foreval) 
	: Packet<key_t>(PacketType::GETRES_LARGEVALUE, key), _val(val), _stat(stat), _nodeidx_foreval(nodeidx_foreval)
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
uint32_t GetResponseLargevalue<key_t, val_t>::size() { // unused
	return sizeof(optype_t) + sizeof(key_t) + sizeof(uint32_t) + val_t::SWITCH_MAX_VALLEN + sizeof(bool) + sizeof(uint16_t) + STAT_PADDING_BYTES;
}

template<class key_t, class val_t>
size_t GetResponseLargevalue<key_t, val_t>::get_frag_hdrsize() {
	return sizeof(optype_t) + sizeof(key_t); // op_hdr
}

template<class key_t, class val_t>
uint32_t GetResponseLargevalue<key_t, val_t>::dynamic_serialize(dynamic_array_t &dynamic_data) {
	int tmpoff = 0;
	uint32_t tmp_typesize = dynamic_serialize_packet_type(this->_type, dynamic_data);
	tmpoff += tmp_typesize;
	uint32_t tmp_keysize = this->_key.dynamic_serialize(dynamic_data, tmpoff);
	tmpoff += tmp_keysize;
	uint32_t tmp_valsize = this->_val.dynamic_serialize_large(dynamic_data, tmpoff);
	tmpoff += tmp_valsize;
	dynamic_data.dynamic_memcpy(tmpoff, (char *)&this->_stat, sizeof(bool));
	tmpoff += sizeof(bool);
	uint16_t bigendian_nodeidx_foreval = htons(this->_nodeidx_foreval);
	dynamic_data.dynamic_memcpy(tmpoff, (char*)&bigendian_nodeidx_foreval, sizeof(uint16_t));
	tmpoff += sizeof(uint16_t);
	tmpoff += STAT_PADDING_BYTES;
	return tmp_typesize + tmp_keysize + tmp_valsize + sizeof(bool) + sizeof(uint16_t) + STAT_PADDING_BYTES;
}

template<class key_t, class val_t>
uint32_t GetResponseLargevalue<key_t, val_t>::serialize(char * const data, uint32_t max_size) {
	uint32_t my_size = this->size();
	INVARIANT(max_size >= my_size);
	char *begin = data;
	uint32_t tmp_typesize = serialize_packet_type(this->_type, begin, max_size);
	begin += tmp_typesize;
	uint32_t tmp_keysize = this->_key.serialize(begin, max_size - tmp_typesize);
	begin += tmp_keysize;
	uint32_t tmp_valsize = this->_val.serialize_large(begin, max_size - tmp_typesize - tmp_keysize);
	begin += tmp_valsize;
	memcpy(begin, (void *)&this->_stat, sizeof(bool));
	begin += sizeof(bool);
	uint16_t bigendian_nodeidx_foreval = htons(this->_nodeidx_foreval);
	memcpy(begin, (void *)&bigendian_nodeidx_foreval, sizeof(uint16_t));
	begin += sizeof(uint16_t);
	begin += STAT_PADDING_BYTES;
	return tmp_typesize + tmp_keysize + tmp_valsize + sizeof(bool) + sizeof(uint16_t) + STAT_PADDING_BYTES;
}

template<class key_t, class val_t>
void GetResponseLargevalue<key_t, val_t>::deserialize(const char * data, uint32_t recv_size) {
	uint32_t my_size = this->size();
	INVARIANT(my_size <= recv_size);
	const char *begin = data;
	uint32_t tmp_typesize = deserialize_packet_type(this->_type, begin, recv_size);
	begin += tmp_typesize;
	uint32_t tmp_keysize = this->_key.deserialize(begin, recv_size - tmp_typesize);
	begin += tmp_keysize;
	uint32_t tmp_valsize = this->_val.deserialize_large(begin, recv_size - tmp_typesize - tmp_keysize);
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
	this->_type = static_cast<optype_t>(packet_type_t::GETRES_LARGEVALUE_SERVER);
}

template<class key_t, class val_t>
GetResponseLargevalueServer<key_t, val_t>::GetResponseLargevalueServer(const char * data, uint32_t recv_size) {
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::GETRES_LARGEVALUE_SERVER);
	INVARIANT(this->_val.val_length > val_t::SWITCH_MAX_VALLEN);
}

template<class key_t, class val_t>
size_t GetResponseLargevalueServer<key_t, val_t>::get_frag_hdrsize() {
	return sizeof(optype_t) + sizeof(key_t); // op_hdr
}

// APIs
static uint32_t serialize_packet_type(optype_t type, char * data, uint32_t maxsize) {
	INVARIANT(maxsize >= sizeof(optype_t));
	uint16_t bigendian_type = htons(type);
	memcpy(data, (void *)&bigendian_type, sizeof(optype_t));
	return sizeof(optype_t);
}

static uint32_t dynamic_serialize_packet_type(optype_t type, dynamic_array_t &dynamic_data) {
	uint16_t bigendian_type = htons(type);
	dynamic_data.dynamic_memcpy(0, (char *)&bigendian_type, sizeof(optype_t));
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







static netreach_key_t get_packet_key(const char * data, uint32_t recvsize) {
	netreach_key_t tmpkey;
	const char *begin = data + sizeof(optype_t);
	tmpkey.deserialize(begin, recvsize - sizeof(optype_t));
	return tmpkey;
}

// Util APIs for large value

static size_t get_frag_hdrsize(packet_type_t type) {
	size_t frag_hdrsize = 0;
	if (type == packet_type_t::PUTREQ_LARGEVALUE) {
		frag_hdrsize = PutRequestLargevalue<netreach_key_t, val_t>::get_frag_hdrsize();
	}
	else if (type == packet_type_t::PUTREQ_LARGEVALUE_SEQ) {
		frag_hdrsize = PutRequestLargevalueSeq<netreach_key_t, val_t>::get_frag_hdrsize();
	}
	else if (type == packet_type_t::PUTREQ_LARGEVALUE_SEQ_CACHED) {
		frag_hdrsize = PutRequestLargevalueSeqCached<netreach_key_t, val_t>::get_frag_hdrsize();
	}
	else if (type == packet_type_t::PUTREQ_LARGEVALUE_SEQ_CASE3) {
		frag_hdrsize = PutRequestLargevalueSeqCase3<netreach_key_t, val_t>::get_frag_hdrsize();
	}
	else if (type == packet_type_t::GETRES_LARGEVALUE) {
		frag_hdrsize = GetResponseLargevalue<netreach_key_t, val_t>::get_frag_hdrsize();
	}
	else if (type == packet_type_t::LOADREQ) {
		frag_hdrsize = LoadRequest<netreach_key_t, val_t>::get_frag_hdrsize();
	}
	else {
		printf("[WARNING] get_frag_hdrsize: no matched optype: %x\n", type);
	}
	return frag_hdrsize;
}

static uint16_t get_packet_clientlogicalidx(const char * data, uint32_t recvsize) {
	packet_type_t tmp_optype = get_packet_type(data, recvsize);
	uint32_t prevbytes = 0;
	if (tmp_optype == packet_type_t::PUTREQ_LARGEVALUE) {
		prevbytes = sizeof(optype_t) + sizeof(netreach_key_t); // op_hdr
	}
	else if (tmp_optype == packet_type_t::PUTREQ_LARGEVALUE_SEQ || tmp_optype == packet_type_t::PUTREQ_LARGEVALUE_SEQ_CACHED || tmp_optype == packet_type_t::PUTREQ_LARGEVALUE_SEQ_CASE3) {
		prevbytes = sizeof(optype_t) + sizeof(netreach_key_t) + sizeof(optype_t) + sizeof(uint32_t); // op_hdr + shadowtype + seq
	}
	else if (tmp_optype == packet_type_t::LOADREQ) {
		prevbytes = sizeof(optype_t) + sizeof(netreach_key_t); // op_hdr
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

static uint32_t get_packet_fragseq(const char * data, uint32_t recvsize) {
	packet_type_t tmp_optype = get_packet_type(data, recvsize);
	uint32_t prevbytes = 0;
	if (tmp_optype == packet_type_t::PUTREQ_LARGEVALUE) {
		prevbytes = sizeof(optype_t) + sizeof(netreach_key_t) + sizeof(uint16_t); // op_hdr + client_logical_idx
	}
	else if (tmp_optype == packet_type_t::PUTREQ_LARGEVALUE_SEQ || tmp_optype == packet_type_t::PUTREQ_LARGEVALUE_SEQ_CACHED || tmp_optype == packet_type_t::PUTREQ_LARGEVALUE_SEQ_CASE3) {
		prevbytes = sizeof(optype_t) + sizeof(netreach_key_t) + sizeof(optype_t) + sizeof(uint32_t) + sizeof(uint16_t); // op_hdr + shadowtype + seq + client_logical_idx
	}
	else if (tmp_optype == packet_type_t::LOADREQ) {
		prevbytes = sizeof(optype_t) + sizeof(netreach_key_t) + sizeof(uint16_t); // op_hdr + client_logical_idx
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
bool is_packet_with_largevalue(packet_type_t type) {
	for (uint32_t tmp_optype_for_udprecvlarge_ipfrag_idx = 0; tmp_optype_for_udprecvlarge_ipfrag_idx < optype_for_udprecvlarge_ipfrag_num; tmp_optype_for_udprecvlarge_ipfrag_idx++) {
		if (type == optype_for_udprecvlarge_ipfrag_list[tmp_optype_for_udprecvlarge_ipfrag_idx]) {
			return true;
		}
	}
	return false;
}

// whether the large packet is sent to server
bool is_packet_with_clientlogicalidx(packet_type_t type) {
	for (uint32_t tmp_optype_with_clientlogicalidx_idx = 0; tmp_optype_with_clientlogicalidx_idx < optype_with_clientlogicalidx_num; tmp_optype_with_clientlogicalidx_idx++) {
		if (type == optype_with_clientlogicalidx_list[tmp_optype_with_clientlogicalidx_idx]) {
			return true;
		}
	}
	return false;
}

#endif
