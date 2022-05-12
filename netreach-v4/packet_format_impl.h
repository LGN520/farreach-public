#ifndef PACKET_FORMAT_IMPL_H
#define PACKET_FORMAT_IMPL_H

#include "packet_format.h"
#include <arpa/inet.h>

// Packet
template<class key_t>
Packet<key_t>::Packet() 
	: _type(static_cast<uint8_t>(0)), _key(key_t::min())
{
}

template<class key_t>
Packet<key_t>::Packet(packet_type_t type, key_t key)
	: _type(static_cast<uint8_t>(type)), _key(key)
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
	return sizeof(uint8_t) + sizeof(key_t);
}

template<class key_t>
uint32_t GetRequest<key_t>::serialize(char * const data, uint32_t max_size) {
	uint32_t my_size = this->size();
	INVARIANT(max_size >= my_size);
	char *begin = data;
	memcpy(begin, (void *)&this->_type, sizeof(uint8_t));
	begin += sizeof(uint8_t);
	uint32_t tmp_keysize = this->_key.serialize(begin, max_size - sizeof(uint8_t));
	begin += tmp_keysize;
	memset(begin, 0, sizeof(DEBUG_BYTES));
	return my_size + DEBUG_BYTES;
}

template<class key_t>
void GetRequest<key_t>::deserialize(const char * data, uint32_t recv_size) {
	uint32_t my_size = this->size();
	INVARIANT(my_size <= recv_size);
	const char *begin = data;
	memcpy((void *)&this->_type, begin, sizeof(uint8_t));
	begin += sizeof(uint8_t);
	this->_key.deserialize(begin, recv_size - sizeof(uint8_t));
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
	return sizeof(uint8_t) + sizeof(key_t) + sizeof(uint32_t) + val_t::MAX_VALLEN;
}

template<class key_t, class val_t>
uint32_t PutRequest<key_t, val_t>::serialize(char * const data, uint32_t max_size) {
	//uint32_t my_size = this->size();
	//INVARIANT(max_size >= my_size);
	char *begin = data;
	memcpy(begin, (void *)&this->_type, sizeof(uint8_t));
	begin += sizeof(uint8_t);
	uint32_t tmp_keysize = this->_key.serialize(begin, max_size - sizeof(uint8_t));
	begin += tmp_keysize;
	uint32_t tmp_valsize = this->_val.serialize(begin, max_size-sizeof(uint8_t)-tmp_keysize);
	begin += tmp_valsize;
	memcpy(begin, (void *)&this->_type, sizeof(uint8_t)); // shadowtype
	begin += sizeof(uint8_t);
	memset(begin, 0, sizeof(DEBUG_BYTES));
	return sizeof(uint8_t) + tmp_keysize + tmp_valsize + sizeof(uint8_t) + DEBUG_BYTES;
}

template<class key_t, class val_t>
void PutRequest<key_t, val_t>::deserialize(const char * data, uint32_t recv_size) {
	//uint32_t my_size = this->size();
	//INVARIANT(my_size == recv_size);
	const char *begin = data;
	memcpy((void *)&this->_type, begin, sizeof(uint8_t));
	begin += sizeof(uint8_t);
	uint32_t tmp_keysize = this->_key.deserialize(begin, recv_size - sizeof(uint8_t));
	begin += tmp_keysize;
	uint32_t tmp_valsize = this->_val.deserialize(begin, recv_size-sizeof(uint8_t)-tmp_keysize);
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
	return sizeof(uint8_t) + sizeof(key_t);
}

template<class key_t>
uint32_t DelRequest<key_t>::serialize(char * const data, uint32_t max_size) {
	uint32_t my_size = this->size();
	INVARIANT(max_size >= my_size);
	char *begin = data;
	memcpy(begin, (void *)&this->_type, sizeof(uint8_t));
	begin += sizeof(uint8_t);
	uint32_t tmp_keysize = this->_key.serialize(begin, max_size - sizeof(uint8_t));
	begin += tmp_keysize;
	memset(begin, 0, sizeof(DEBUG_BYTES));
	return sizeof(uint8_t) + tmp_keysize + DEBUG_BYTES;
}

template<class key_t>
void DelRequest<key_t>::deserialize(const char * data, uint32_t recv_size) {
	uint32_t my_size = this->size();
	INVARIANT(my_size == recv_size);
	const char *begin = data;
	memcpy((void *)&this->_type, begin, sizeof(uint8_t));
	begin += sizeof(uint8_t);
	uint32_t tmp_keysize = this->_key.deserialize(begin, recv_size - sizeof(uint8_t));
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
	return sizeof(uint8_t) + sizeof(key_t) + sizeof(key_t);// + sizeof(uint32_t);
}

template<class key_t>
uint32_t ScanRequest<key_t>::serialize(char * const data, uint32_t max_size) {
	uint32_t my_size = this->size();
	INVARIANT(max_size >= my_size);
	char *begin = data;
	memcpy(begin, (void *)&this->_type, sizeof(uint8_t));
	begin += sizeof(uint8_t);
	uint32_t tmp_keysize = this->_key.serialize(begin, max_size - sizeof(uint8_t));
	begin += tmp_keysize;
	uint32_t tmp_endkeysize = this->_endkey.serialize(begin, max_size - sizeof(uint8_t) - tmp_keysize);
	//memcpy(begin, (void *)&this->_num, sizeof(uint32_t));
	return sizeof(uint8_t) + tmp_keysize + tmp_endkeysize; // + sizeof(uint32_t);
}

template<class key_t>
void ScanRequest<key_t>::deserialize(const char * data, uint32_t recv_size) {
	uint32_t my_size = this->size();
	INVARIANT(my_size == recv_size);
	const char *begin = data;
	memcpy((void *)&this->_type, begin, sizeof(uint8_t));
	begin += sizeof(uint8_t);
	uint32_t tmp_keysize = this->_key.deserialize(begin, recv_size - sizeof(uint8_t));
	begin += tmp_keysize;
	uint32_t tmp_endkeysize = this->_endkey.deserialize(begin, recv_size - sizeof(uint8_t) - tmp_keysize);
	UNUSED(tmp_endkeysize);
	//begin += tmp_endkeysize;
	//memcpy((void *)&this->_num, begin, sizeof(uint32_t));
}


// GetResponse (value can be any size)

template<class key_t, class val_t>
GetResponse<key_t, val_t>::GetResponse()
	: Packet<key_t>(), _val(), _stat(false)
{
}

template<class key_t, class val_t>
GetResponse<key_t, val_t>::GetResponse(key_t key, val_t val, bool stat) 
	: Packet<key_t>(PacketType::GETRES, key), _val(val), _stat(stat)
{	
}

template<class key_t, class val_t>
GetResponse<key_t, val_t>::GetResponse(const char * data, uint32_t recv_size) {
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::GETRES);
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
uint32_t GetResponse<key_t, val_t>::size() { // unused
	return sizeof(uint8_t) + sizeof(key_t) + sizeof(uint32_t) + val_t::MAX_VALLEN + sizeof(bool);
}

template<class key_t, class val_t>
uint32_t GetResponse<key_t, val_t>::serialize(char * const data, uint32_t max_size) {
	//uint32_t my_size = this->size();
	//INVARIANT(max_size >= my_size);
	char *begin = data;
	memcpy(begin, (void *)&this->_type, sizeof(uint8_t));
	begin += sizeof(uint8_t);
	uint32_t tmp_keysize = this->_key.serialize(begin, max_size - sizeof(uint8_t));
	begin += tmp_keysize;
	uint32_t tmp_valsize = this->_val.serialize(begin, max_size-sizeof(uint8_t)-tmp_keysize);
	begin += tmp_valsize;
	memcpy(begin, (void *)&this->_type, sizeof(uint8_t)); // shadowtype
	begin += sizeof(uint8_t);
	memcpy(begin, (void *)&this->_stat, sizeof(bool));
	begin += sizeof(bool);
	memset(begin, 0, sizeof(DEBUG_BYTES));
	return sizeof(uint8_t) + tmp_keysize + tmp_valsize + sizeof(uint8_t) + sizeof(bool) + DEBUG_BYTES;
}

template<class key_t, class val_t>
void GetResponse<key_t, val_t>::deserialize(const char * data, uint32_t recv_size) {
	//uint32_t my_size = this->size();
	//INVARIANT(my_size == recv_size);
	const char *begin = data;
	memcpy((void *)&this->_type, begin, sizeof(uint8_t));
	begin += sizeof(uint8_t);
	uint32_t tmp_keysize = this->_key.deserialize(begin, recv_size - sizeof(uint8_t));
	begin += tmp_keysize;
	uint32_t tmp_valsize = this->_val.deserialize(begin, recv_size - sizeof(uint8_t) - tmp_keysize);
	begin += tmp_valsize;
	begin += sizeof(uint8_t); // deserialize shadowtype
	memcpy((void *)&this->_stat, begin, sizeof(bool));
}

// PutResponse (value must be any size)

template<class key_t>
PutResponse<key_t>::PutResponse(key_t key, bool stat) 
	: Packet<key_t>(PacketType::PUTRES, key), _stat(stat)
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
uint32_t PutResponse<key_t>::size() {
	return sizeof(uint8_t) + sizeof(key_t) + sizeof(bool);
}

template<class key_t>
uint32_t PutResponse<key_t>::serialize(char * const data, uint32_t max_size) {
	uint32_t my_size = this->size();
	INVARIANT(max_size >= my_size);
	char *begin = data;
	memcpy(begin, (void *)&this->_type, sizeof(uint8_t));
	begin += sizeof(uint8_t);
	uint32_t tmp_keysize = this->_key.serialize(begin, max_size - sizeof(uint8_t));
	begin += tmp_keysize; 
	memcpy(begin, (void *)&this->_type, sizeof(uint8_t)); // shadowtype
	begin += sizeof(uint8_t);
	memcpy(begin, (void *)&this->_stat, sizeof(bool));
	begin += sizeof(bool);
	memset(begin, 0, sizeof(DEBUG_BYTES));
	return sizeof(uint8_t) + tmp_keysize + sizeof(uint8_t) + sizeof(bool) + DEBUG_BYTES;
}

template<class key_t>
void PutResponse<key_t>::deserialize(const char * data, uint32_t recv_size) {
	uint32_t my_size = this->size();
	INVARIANT(my_size <= recv_size);
	const char *begin = data;
	memcpy((void *)&this->_type, begin, sizeof(uint8_t));
	begin += sizeof(uint8_t);
	uint32_t tmp_keysize = this->_key.deserialize(begin, recv_size - sizeof(uint8_t));
	begin += tmp_keysize;
	begin += sizeof(uint8_t); // deserialize shadowtype
	memcpy((void *)&this->_stat, begin, sizeof(bool));
}

// DelResponse

template<class key_t>
DelResponse<key_t>::DelResponse(key_t key, bool stat) 
	: Packet<key_t>(PacketType::DELRES, key), _stat(stat)
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
uint32_t DelResponse<key_t>::size() {
	return sizeof(uint8_t) + sizeof(key_t) + sizeof(bool);
}

template<class key_t>
uint32_t DelResponse<key_t>::serialize(char * const data, uint32_t max_size) {
	uint32_t my_size = this->size();
	INVARIANT(max_size >= my_size);
	char *begin = data;
	memcpy(begin, (void *)&this->_type, sizeof(uint8_t));
	begin += sizeof(uint8_t);
	uint32_t tmp_keysize = this->_key.serialize(begin, max_size - sizeof(uint8_t));
	begin += tmp_keysize;
	memcpy(begin, (void *)&this->_type, sizeof(uint8_t)); // shadowtype
	begin += sizeof(uint8_t);
	memcpy(begin, (void *)&this->_stat, sizeof(bool));
	begin += sizeof(bool);
	memset(begin, 0, sizeof(DEBUG_BYTES));
	return sizeof(uint8_t) + tmp_keysize + sizeof(bool) + DEBUG_BYTES;
}

template<class key_t>
void DelResponse<key_t>::deserialize(const char * data, uint32_t recv_size) {
	uint32_t my_size = this->size();
	INVARIANT(my_size == recv_size);
	const char *begin = data;
	memcpy((void *)&this->_type, begin, sizeof(uint8_t));
	begin += sizeof(uint8_t);
	uint32_t tmp_keysize = this->_key.deserialize(begin, recv_size - sizeof(uint8_t));
	begin += tmp_keysize;
	begin += sizeof(uint8_t); // deserialize shadowtype
	memcpy((void *)&this->_stat, begin, sizeof(bool));
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
ScanResponseSplit<key_t, val_t>::ScanResponseSplit(key_t key, key_t endkey, uint16_t cur_scanidx, uint16_t max_scannum, int32_t pairnum, std::vector<std::pair<key_t, val_t>> pairs) 
	: ScanRequestSplit<key_t>(key, endkey, cur_scanidx, max_scannum), _pairnum(pairnum)
{	
	this->_type = static_cast<uint8_t>(PacketType::SCANRES_SPLIT);
	INVARIANT(pairnum == int32_t(pairs.size()));
	this->_pairs.assign(pairs.begin(), pairs.end());
}

template<class key_t, class val_t>
ScanResponseSplit<key_t, val_t>::ScanResponseSplit(const char * data, uint32_t recv_size) {
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::SCANRES_SPLIT);
}

template<class key_t, class val_t>
int32_t ScanResponseSplit<key_t, val_t>::pairnum() const {
	return this->_pairnum;
}

template<class key_t, class val_t>
std::vector<std::pair<key_t, val_t>> ScanResponseSplit<key_t, val_t>::pairs() const {
	return this->_pairs;
}

template<class key_t, class val_t>
uint32_t ScanResponseSplit<key_t, val_t>::size() {
	//return sizeof(uint8_t) + sizeof(key_t) + sizeof(key_t) + sizeof(uint32_t) + sizeof(uint16_t) + sizeof(uint16_t) + sizeof(int32_t);
	return sizeof(uint8_t) + sizeof(key_t) + sizeof(key_t) + sizeof(uint16_t) + sizeof(uint16_t) + sizeof(int32_t);
}

template<class key_t, class val_t>
uint32_t ScanResponseSplit<key_t, val_t>::serialize(char * const data, uint32_t max_size) {
	uint32_t my_size = this->size();
	INVARIANT(max_size >= my_size);
	char *begin = data;
	memcpy(begin, (void *)&this->_type, sizeof(uint8_t));
	begin += sizeof(uint8_t);
	uint32_t tmp_keysize = this->_key.serialize(begin, max_size - sizeof(uint8_t));
	begin += tmp_keysize;
	uint32_t tmp_endkeysize = this->_endkey.serialize(begin, max_size - sizeof(uint8_t) - tmp_keysize);
	begin += tmp_endkeysize;
	//memcpy(begin, (void *)&this->_num, sizeof(uint32_t));
	//begin += sizeof(uint32_t);
	uint16_t bigendian_cur_scanidx = htons(uint16_t(this->_cur_scanidx));
	memcpy(begin, (void *)&bigendian_cur_scanidx, sizeof(uint16_t));
	begin += sizeof(uint16_t);
	uint16_t bigendian_max_scannum = htons(uint16_t(this->_max_scannum));
	memcpy(begin, (void *)&bigendian_max_scannum, sizeof(uint16_t));
	begin += sizeof(uint16_t);

	uint32_t bigendian_pairnum = htonl(uint32_t(this->_pairnum));
	memcpy(begin, (void *)&bigendian_pairnum, sizeof(int32_t));
	begin += sizeof(int32_t);
	uint32_t totalsize = my_size;
	for (uint32_t pair_i = 0; pair_i < this->_pairs.size(); pair_i++) {
		uint32_t tmp_pair_keysize = this->_pairs[pair_i].first.serialize(begin, max_size - totalsize);
		begin += tmp_pair_keysize;
		totalsize += tmp_pair_keysize;
		uint32_t tmp_pair_valsize = this->_pairs[pair_i].second.serialize(begin, max_size - totalsize);
		begin += tmp_pair_valsize;
		totalsize += tmp_pair_valsize;
	}
	return totalsize;
}

template<class key_t, class val_t>
void ScanResponseSplit<key_t, val_t>::deserialize(const char * data, uint32_t recv_size) {
	uint32_t my_size = this->size();
	INVARIANT(recv_size >= my_size);
	const char *begin = data;
	memcpy((void *)&this->_type, begin, sizeof(uint8_t));
	begin += sizeof(uint8_t);
	uint32_t tmp_keysize = this->_key.deserialize(begin, recv_size - sizeof(uint8_t));
	begin += tmp_keysize;
	uint32_t tmp_endkeysize = this->_endkey.deserialize(begin, recv_size - sizeof(uint8_t) - tmp_keysize);
	begin += tmp_endkeysize;
	//memcpy((void *)&this->_num, begin, sizeof(uint32_t));
	//begin += sizeof(uint32_t);
	memcpy((void *)&this->_cur_scanidx, begin, sizeof(uint16_t));
	this->_cur_scanidx = ntohs(this->_cur_scanidx);
	begin += sizeof(uint16_t);
	memcpy((void *)&this->_max_scannum, begin, sizeof(uint16_t));
	this->_max_scannum = ntohs(this->_max_scannum);
	begin += sizeof(uint16_t);

	memcpy((void *)&this->_pairnum, begin, sizeof(int32_t));
	this->_pairnum = int32_t(ntohl(uint32_t(this->_pairnum)));
	begin += sizeof(int32_t);
	uint32_t totalsize = my_size;
	this->_pairs.resize(this->_pairnum); // change size to this->_pairnum (not just reserve)
	for (int32_t pair_i = 0; pair_i < this->_pairnum; pair_i++) {
		uint32_t tmp_pair_keysize = this->_pairs[pair_i].first.deserialize(begin, recv_size - totalsize);
		begin += tmp_pair_keysize;
		totalsize += tmp_pair_keysize;
		uint32_t tmp_pair_valsize = this->_pairs[pair_i].second.deserialize(begin, recv_size - totalsize);
		begin += tmp_pair_valsize;
		totalsize += tmp_pair_valsize;
	}
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
	: Packet<key_t>(), _val(), _seq(0)
{}

template<class key_t, class val_t>
GetResponseLatestSeq<key_t, val_t>::GetResponseLatestSeq(key_t key, val_t val, uint32_t seq)
	: Packet<key_t>(packet_type_t::GETRES_LATEST_SEQ, key), _val(val), _seq(seq)
{
	INVARIANT(this->_val.val_length <= val_t::SWITCH_MAX_VALLEN);
	INVARIANT(seq >= 0);
}

template<class key_t, class val_t>
uint32_t GetResponseLatestSeq<key_t, val_t>::serialize(char * const data, uint32_t max_size) {
	//uint32_t my_size = this->size();
	//INVARIANT(max_size >= my_size);
	char *begin = data;
	memcpy(begin, (void *)&this->_type, sizeof(uint8_t));
	begin += sizeof(uint8_t);
	uint32_t tmp_keysize = this->_key.serialize(begin, max_size - sizeof(uint8_t));
	begin += tmp_keysize;
	uint32_t tmp_valsize = this->_val.serialize(begin, max_size-sizeof(uint8_t)-tmp_keysize);
	begin += tmp_valsize;
	memcpy(begin, (void *)&this->_type, sizeof(uint8_t)); // shadowtype
	begin += sizeof(uint8_t);
	uint32_t bigendian_seq = htonl(this->_seq);
	memcpy(begin, (void *)&bigendian_seq, sizeof(uint32_t)); // little-endian to big-endian
	begin += sizeof(uint32_t);
	memset(begin, 0, sizeof(DEBUG_BYTES));
	return sizeof(uint8_t) + tmp_keysize + tmp_valsize + sizeof(uint32_t) + DEBUG_BYTES;
}

template<class key_t, class val_t>
val_t GetResponseLatestSeq<key_t, val_t>::val() const {
	return this->_val;
}

template<class key_t, class val_t>
uint32_t GetResponseLatestSeq<key_t, val_t>::seq() const {
	return this->_seq;
}

template<class key_t, class val_t>
uint32_t GetResponseLatestSeq<key_t, val_t>::size() { // unused
	return sizeof(uint8_t) + sizeof(key_t) + sizeof(uint32_t) + val_t::MAX_VALLEN + sizeof(uint32_t);
}

template<class key_t, class val_t>
void GetResponseLatestSeq<key_t, val_t>::deserialize(const char * data, uint32_t recv_size)
{
	COUT_N_EXIT("Invalid invoke of deserialize for GetResponseLatestSeq");
}

// GetResponseLatestSeqInswitchCase1 (value must <= 128B)

template<class key_t, class val_t>
GetResponseLatestSeqInswitchCase1<key_t, val_t>::GetResponseLatestSeqInswitchCase1()
	: GetResponseLatestSeq<key_t, val_t>(), _idx(0), _stat(false)
{
}

template<class key_t, class val_t>
GetResponseLatestSeqInswitchCase1<key_t, val_t>::GetResponseLatestSeqInswitchCase1(key_t key, val_t val, uint32_t seq, uint16_t idx, bool stat) 
	: GetResponseLatestSeq<key_t, val_t>(key, val, seq), _idx(idx), _stat(stat)
{
	this->_type = static_cast<uint8_t>(PacketType::GETRES_LATEST_SEQ_INSWITCH_CASE1);
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
	return sizeof(uint8_t) + sizeof(key_t) + sizeof(uint32_t) + val_t::MAX_VALLEN + sizeof(uint32_t) + sizeof(uint16_t) + sizeof(bool);
}

template<class key_t, class val_t>
uint32_t GetResponseLatestSeqInswitchCase1<key_t, val_t>::serialize(char * const data, uint32_t max_size) {
	//uint32_t my_size = this->size();
	//INVARIANT(max_size >= my_size);
	char *begin = data;
	memcpy(begin, (void *)&this->_type, sizeof(uint8_t));
	begin += sizeof(uint8_t);
	uint32_t tmp_keysize = this->_key.serialize(begin, max_size - sizeof(uint8_t));
	begin += tmp_keysize;
	uint32_t tmp_valsize = this->_val.serialize(begin, max_size-sizeof(uint8_t)-tmp_keysize);
	begin += tmp_valsize;
	memcpy(begin, (void *)&this->_type, sizeof(uint8_t)); // shadowtype
	begin += sizeof(uint8_t);
	uint32_t bigendian_seq = htonl(this->_seq);
	memcpy(begin, (void *)&bigendian_seq, sizeof(uint32_t));
	begin += sizeof(uint32_t);
	memset(begin, 0, INSWITCH_PREV_BYTES); // the first bytes of inswitch_hdr
	begin += INSWITCH_PREV_BYTES;
	uint16_t bigendian_idx = htons(this->_idx);
	memcpy(begin, (void *)&bigendian_idx, sizeof(uint16_t)); // little-endian to big-endian
	begin += sizeof(uint16_t);
	memcpy(begin, (void *)&this->_stat, sizeof(bool));
	begin += sizeof(bool);
	memset(begin, 0, sizeof(DEBUG_BYTES));
	return sizeof(uint8_t) + tmp_keysize + tmp_valsize + sizeof(uint32_t) + sizeof(uint16_t) + sizeof(bool) + DEBUG_BYTES;
}

template<class key_t, class val_t>
void GetResponseLatestSeqInswitchCase1<key_t, val_t>::deserialize(const char * data, uint32_t recv_size) {
	//uint32_t my_size = this->size();
	//INVARIANT(my_size == recv_size);
	const char *begin = data;
	memcpy((void *)&this->_type, begin, sizeof(uint8_t));
	begin += sizeof(uint8_t);
	uint32_t tmp_keysize = this->_key.deserialize(begin, recv_size - sizeof(uint8_t));
	begin += tmp_keysize;
	uint32_t tmp_valsize = this->_val.deserialize(begin, recv_size - sizeof(uint8_t) - tmp_keysize);
	begin += tmp_valsize;
	begin += sizeof(uint8_t); // deserialize shadowtype
	memcpy((void *)&this->_seq, begin, sizeof(uint32_t));
	this->_seq = ntohl(this->_seq);
	begin += sizeof(uint32_t);
	begin += INSWITCH_PREV_BYTES; // the first bytes of inswitch_hdr
	memcpy((void *)&this->_idx, begin, sizeof(uint16_t));
	this->_idx = ntohs(this->_idx); // big-endian to little-endian
	begin += sizeof(uint16_t);
	memcpy((void *)&this->_stat, begin, sizeof(bool));
	begin += sizeof(bool);
}

// GetResponseDeletedSeq (value must = 0B)

template<class key_t, class val_t>
GetResponseDeletedSeq<key_t, val_t>::GetResponseDeletedSeq(key_t key, val_t val, uint32_t seq)
	: GetResponseLatestSeq<key_t, val_t>(key, val, seq)
{
	this->_type = static_cast<uint8_t>(PacketType::GETRES_DELETED_SEQ);
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
	this->_type = static_cast<uint8_t>(PacketType::GETRES_DELETED_SEQ_INSWITCH_CASE1);
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
	: GetResponseLatestSeq<key_t, val_t>()
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
void PutRequestSeq<key_t, val_t>::deserialize(const char * data, uint32_t recv_size)
{
	//uint32_t my_size = this->size();
	//INVARIANT(my_size == recv_size);
	const char *begin = data;
	memcpy((void *)&this->_type, begin, sizeof(uint8_t));
	begin += sizeof(uint8_t);
	uint32_t tmp_keysize = this->_key.deserialize(begin, recv_size - sizeof(uint8_t));
	begin += tmp_keysize;
	uint32_t tmp_valsize = this->_val.deserialize(begin, recv_size - sizeof(uint8_t) - tmp_keysize);
	begin += tmp_valsize;
	begin += sizeof(uint8_t); // deserialize shadowtype
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
	this->_type = static_cast<uint8_t>(PacketType::PUTREQ_SEQ_INSWITCH_CASE1);
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
	return sizeof(uint8_t) + sizeof(key_t) + sizeof(uint32_t);
}

template<class key_t>
uint32_t DelRequestSeq<key_t>::serialize(char * const data, uint32_t max_size) {
	COUT_N_EXIT("Invalid invoke of serialize for DelRequestSeq");
}

template<class key_t>
void DelRequestSeq<key_t>::deserialize(const char * data, uint32_t recv_size) {
	uint32_t my_size = this->size();
	INVARIANT(my_size == recv_size);
	const char *begin = data;
	memcpy((void *)&this->_type, begin, sizeof(uint8_t));
	begin += sizeof(uint8_t);
	uint32_t tmp_keysize = this->_key.deserialize(begin, recv_size - sizeof(uint8_t));
	begin += tmp_keysize;
	begin += sizeof(uint8_t); // deserialize shadowtype
	memcpy((void *)&this->_seq, begin, sizeof(uint32_t));
	this->_seq = ntohl(this->_seq);
}

// DelRequestSeqInswitchCase1 (value must <= 128B)

template<class key_t, class val_t>
DelRequestSeqInswitchCase1<key_t, val_t>::DelRequestSeqInswitchCase1(key_t key, val_t val, uint32_t seq, uint16_t idx, bool stat) 
	: GetResponseLatestSeqInswitchCase1<key_t, val_t>(key, val, seq, idx, stat)
{
	this->_type = static_cast<uint8_t>(PacketType::DELREQ_SEQ_INSWITCH_CASE1);
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
	this->_type = static_cast<uint8_t>(packet_type_t::SCANREQ_SPLIT);
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
	return sizeof(uint8_t) + sizeof(key_t) + sizeof(key_t) + sizeof(uint16_t) + sizeof(uint16_t);// + sizeof(uint32_t);
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
	memcpy((void *)&this->_type, begin, sizeof(uint8_t));
	begin += sizeof(uint8_t);
	uint32_t tmp_keysize = this->_key.deserialize(begin, recv_size - sizeof(uint8_t));
	begin += tmp_keysize;
	uint32_t tmp_endkeysize = this->_endkey.deserialize(begin, recv_size - sizeof(uint8_t) - tmp_keysize);
	begin += tmp_endkeysize;
	//memcpy((void *)&this->_num, begin, sizeof(uint32_t));
	//begin += sizeof(uint32_t);
	memcpy((void *)&this->_cur_scanidx, begin, sizeof(uint16_t));
	this->_cur_scanidx = ntohs(this->_cur_scanidx);
	begin += sizeof(uint16_t);
	memcpy((void *)&this->_max_scannum, begin, sizeof(uint16_t));
	this->_max_scannum = ntohs(this->_max_scannum);
	//begin += sizeof(uint16_t);
}

// CachePop (value must <= 128B; only used in end-hosts)

template<class key_t, class val_t>
CachePop<key_t, val_t>::CachePop(key_t key, val_t val, uint32_t seq, uint16_t serveridx)
	: GetResponseLatestSeq<key_t, val_t>(key, val, seq), _serveridx(serveridx)
{
	this->_type = static_cast<uint8_t>(PacketType::CACHE_POP);
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
	memcpy(begin, (void *)&this->_type, sizeof(uint8_t));
	begin += sizeof(uint8_t);
	uint32_t tmp_keysize = this->_key.serialize(begin, max_size - sizeof(uint8_t));
	begin += tmp_keysize;
	uint32_t tmp_valsize = this->_val.serialize(begin, max_size-sizeof(uint8_t)-tmp_keysize);
	begin += tmp_valsize;
	uint32_t bigendian_seq = htonl(this->_seq);
	memcpy(begin, (void *)&bigendian_seq, sizeof(uint32_t)); // little-endian to big-endian
	begin += sizeof(uint32_t);
	uint16_t bigendian_serveridx = htons(uint16_t(this->_serveridx));
	memcpy(begin, (void *)&bigendian_serveridx, sizeof(uint16_t)); // little-endian to big-endian
	return sizeof(uint8_t) + tmp_keysize + tmp_valsize + sizeof(bool) + sizeof(uint32_t) + sizeof(uint16_t);
}

template<class key_t, class val_t>
uint16_t CachePop<key_t, val_t>::serveridx() const {
	return _serveridx;
}

template<class key_t, class val_t>
uint32_t CachePop<key_t, val_t>::size() { // unused
	return sizeof(uint8_t) + sizeof(key_t) + sizeof(uint32_t) + val_t::MAX_VALLEN + sizeof(uint32_t) + sizeof(uint16_t);
}

template<class key_t, class val_t>
void CachePop<key_t, val_t>::deserialize(const char * data, uint32_t recv_size)
{
	//uint32_t my_size = this->size();
	//INVARIANT(my_size == recv_size);
	const char *begin = data;
	memcpy((void *)&this->_type, begin, sizeof(uint8_t));
	begin += sizeof(uint8_t);
	uint32_t tmp_keysize = this->_key.deserialize(begin, recv_size - sizeof(uint8_t));
	begin += tmp_keysize;
	uint32_t tmp_valsize = this->_val.deserialize(begin, recv_size - sizeof(uint8_t) - tmp_keysize);
	begin += tmp_valsize;
	memcpy((void *)&this->_seq, begin, sizeof(uint32_t));
	this->_seq = ntohl(this->_seq); // Big-endian to little-endian
	begin += sizeof(uint32_t);
	memcpy((void *)&this->_serveridx, begin, sizeof(uint16_t));
	this->_serveridx = ntohs(this->_serveridx); // Big-endian to little-endian
}

// CachePopInswitch (valud must <= 128B)

template<class key_t, class val_t>
CachePopInswitch<key_t, val_t>::CachePopInswitch(key_t key, val_t val, uint32_t seq, uint16_t freeidx)
	: GetResponseLatestSeq<key_t, val_t>(key, val, seq), _freeidx(freeidx)
{
	this->_type = static_cast<uint8_t>(PacketType::CACHE_POP_INSWITCH);
	INVARIANT(this->_val.val_length <= val_t::SWITCH_MAX_VALLEN);
	INVARIANT(seq >= 0);
	INVARIANT(freeidx >= 0);
}

template<class key_t, class val_t>
uint32_t CachePopInswitch<key_t, val_t>::serialize(char * const data, uint32_t max_size) {
	//uint32_t my_size = this->size();
	//INVARIANT(max_size >= my_size);
	char *begin = data;
	memcpy(begin, (void *)&this->_type, sizeof(uint8_t));
	begin += sizeof(uint8_t);
	uint32_t tmp_keysize = this->_key.serialize(begin, max_size - sizeof(uint8_t));
	begin += tmp_keysize;
	uint32_t tmp_valsize = this->_val.serialize(begin, max_size-sizeof(uint8_t)-tmp_keysize);
	begin += tmp_valsize;
	memcpy(begin, (void *)&this->_type, sizeof(uint8_t)); // shadowtype
	begin += sizeof(uint8_t);
	uint32_t bigendian_seq = htonl(this->_seq);
	memcpy(begin, (void *)&bigendian_seq, sizeof(uint32_t)); // little-endian to big-endian
	begin += sizeof(uint32_t);
	memset(begin, 0, INSWITCH_PREV_BYTES); // the first bytes of inswitch_hdr
	begin += INSWITCH_PREV_BYTES;
	uint16_t bigendian_freeidx = htons(uint16_t(this->_freeidx));
	memcpy(begin, (void *)&bigendian_freeidx, sizeof(uint16_t)); // little-endian to big-endian
	begin += sizeof(uint16_t);
	memset(begin, 0, sizeof(DEBUG_BYTES));
	return sizeof(uint8_t) + tmp_keysize + tmp_valsize + sizeof(bool) + sizeof(uint32_t) + INSWITCH_PREV_BYTES + sizeof(uint16_t) + DEBUG_BYTES;
}

template<class key_t, class val_t>
uint16_t CachePopInswitch<key_t, val_t>::freeidx() const {
	return _freeidx;
}

template<class key_t, class val_t>
uint32_t CachePopInswitch<key_t, val_t>::size() { // unused
	return sizeof(uint8_t) + sizeof(key_t) + sizeof(uint32_t) + val_t::MAX_VALLEN + sizeof(uint32_t) + INSWITCH_PREV_BYTES + sizeof(uint16_t);
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
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::CACHE_POP_INSWITCH_ACK);
}

template<class key_t>
uint32_t CachePopInswitchAck<key_t>::serialize(char * const data, uint32_t max_size)
{
	COUT_N_EXIT("Invalid invoke of serialize for CachePopInswitchAck");
}

// CacheEvict (value must <= 128B; only used in end-hosts)

template<class key_t, class val_t>
CacheEvict<key_t, val_t>::CacheEvict() 
	: GetResponseLatestSeq<key_t, val_t>(), _stat(false), _serveridx(0)
{
}

template<class key_t, class val_t>
CacheEvict<key_t, val_t>::CacheEvict(key_t key, val_t val, uint32_t seq, bool stat, uint16_t serveridx) 
	: GetResponseLatestSeq<key_t, val_t>(key, val, seq), _stat(stat), _serveridx(serveridx)
{
	this->_type = static_cast<uint8_t>(PacketType::CACHE_EVICT);
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
bool CacheEvict<key_t, val_t>::stat() const {
	return _stat;
}

template<class key_t, class val_t>
uint16_t CacheEvict<key_t, val_t>::serveridx() const {
	return _serveridx;
}

template<class key_t, class val_t>
uint32_t CacheEvict<key_t, val_t>::size() { // unused
	return sizeof(uint8_t) + sizeof(key_t) + sizeof(uint32_t) + val_t::MAX_VALLEN + sizeof(uint32_t) + sizeof(bool) + sizeof(uint16_t);
}

template<class key_t, class val_t>
uint32_t CacheEvict<key_t, val_t>::serialize(char * const data, uint32_t max_size) {
	//uint32_t my_size = this->size();
	//INVARIANT(max_size >= my_size);
	char *begin = data;
	memcpy(begin, (void *)&this->_type, sizeof(uint8_t));
	begin += sizeof(uint8_t);
	uint32_t tmp_keysize = this->_key.serialize(begin, max_size - sizeof(uint8_t));
	begin += tmp_keysize;
	uint32_t tmp_valsize = this->_val.serialize(begin, max_size-sizeof(uint8_t)-tmp_keysize);
	begin += tmp_valsize;
	uint32_t bigendian_seq = htonl(this->_seq);
	memcpy(begin, (void *)&bigendian_seq, sizeof(uint32_t));
	begin += sizeof(uint32_t);
	memcpy(begin, (void *)&this->_stat, sizeof(bool));
	begin += sizeof(bool);
	uint16_t bigendian_serveridx = htons(uint16_t(this->_serveridx));
	memcpy(begin, (void *)&bigendian_serveridx, sizeof(uint16_t));
	return sizeof(uint8_t) + tmp_keysize + tmp_valsize + sizeof(uint32_t) + sizeof(bool) + sizeof(uint16_t);
}

template<class key_t, class val_t>
void CacheEvict<key_t, val_t>::deserialize(const char * data, uint32_t recv_size) {
	//uint32_t my_size = this->size();
	//INVARIANT(my_size == recv_size);
	const char *begin = data;
	memcpy((void *)&this->_type, begin, sizeof(uint8_t));
	begin += sizeof(uint8_t);
	uint32_t tmp_keysize = this->_key.deserialize(begin, recv_size - sizeof(uint8_t));
	begin += tmp_keysize;
	uint32_t tmp_valsize = this->_val.deserialize(begin, recv_size - sizeof(uint8_t) - tmp_keysize);
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
	this->_type = static_cast<uint8_t>(PacketType::CACHE_EVICT_ACK);
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
	this->_type = static_cast<uint8_t>(PacketType::CACHE_EVICT_CASE2);
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

// APIs
packet_type_t get_packet_type(const char * data, uint32_t recv_size) {
	INVARIANT(recv_size >= sizeof(uint8_t));
	uint8_t tmp;
	memcpy((void *)&tmp, data, sizeof(uint8_t));
	return static_cast<packet_type_t>(tmp);
}

#endif
