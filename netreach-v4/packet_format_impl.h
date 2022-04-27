#include "packet_format.h"

// Packet
template<class key_t>
Packet<key_t>::Packet() 
	: _type(static_cast<int8_t>(0)), _key(key_t::min())
{
}

template<class key_t>
Packet<key_t>::Packet(packet_type_t type, key_t key)
	: _type(static_cast<int8_t>(type)), _key(key)
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
	return sizeof(int8_t) + sizeof(key_t);
}

template<class key_t>
uint32_t GetRequest<key_t>::serialize(char * const data, uint32_t max_size) {
	uint32_t my_size = this->size();
	INVARIANT(max_size >= my_size);
	char *begin = data;
	memcpy(begin, (void *)&this->_type, sizeof(int8_t));
	begin += sizeof(int8_t);
	this->_key.serialize(begin, max_size - sizeof(int8_t));
	return my_size;
}

template<class key_t>
void GetRequest<key_t>::deserialize(const char * data, uint32_t recv_size) {
	uint32_t my_size = this->size();
	INVARIANT(my_size == recv_size);
	const char *begin = data;
	memcpy((void *)&this->_type, begin, sizeof(int8_t));
	begin += sizeof(int8_t);
	this->_key.deserialize(begin, recv_size - sizeof(int8_t));
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
	return sizeof(int8_t) + sizeof(key_t) + sizeof(uint32_t) + val_t::max_bytesnum();
}

template<class key_t, class val_t>
uint32_t PutRequest<key_t, val_t>::serialize(char * const data, uint32_t max_size) {
	//uint32_t my_size = this->size();
	//INVARIANT(max_size >= my_size);
	char *begin = data;
	memcpy(begin, (void *)&this->_type, sizeof(int8_t));
	begin += sizeof(int8_t);
	uint32_t tmp_keysize = this->_key.serialize(begin, max_size - sizeof(int8_t));
	begin += tmp_keysize;
	uint32_t tmp_valsize = this->_val.serialize(begin, max_size-sizeof(int8_t)-tmp_keysize);
	return sizeof(int8_t) + tmp_keysize + tmp_valsize;
}

template<class key_t, class val_t>
void PutRequest<key_t, val_t>::deserialize(const char * data, uint32_t recv_size) {
	//uint32_t my_size = this->size();
	//INVARIANT(my_size == recv_size);
	const char *begin = data;
	memcpy((void *)&this->_type, begin, sizeof(int8_t));
	begin += sizeof(int8_t);
	uint32_t tmp_keysize = this->_key.deserialize(begin, recv_size - sizeof(int8_t));
	begin += tmp_keysize;
	uint32_t tmp_valsize = this->_val.deserialize(begin, recv_size-sizeof(int8_t)-tmp_keysize);
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
	return sizeof(int8_t) + sizeof(key_t);
}

template<class key_t>
uint32_t DelRequest<key_t>::serialize(char * const data, uint32_t max_size) {
	uint32_t my_size = this->size();
	INVARIANT(max_size >= my_size);
	char *begin = data;
	memcpy(begin, (void *)&this->_type, sizeof(int8_t));
	begin += sizeof(int8_t);
	uint32_t tmp_keysize = this->_key.serialize(begin, max_size - sizeof(int8_t));
	return sizeof(int8_t) + tmp_keysize;
}

template<class key_t>
void DelRequest<key_t>::deserialize(const char * data, uint32_t recv_size) {
	uint32_t my_size = this->size();
	INVARIANT(my_size == recv_size);
	const char *begin = data;
	memcpy((void *)&this->_type, begin, sizeof(int8_t));
	begin += sizeof(int8_t);
	uint32_t tmp_keysize = this->_key.deserialize(begin, recv_size - sizeof(int8_t));
}

// ScanRequest
template<class key_t>
ScanRequest<key_t>::ScanRequest(uint16_t hashidx, key_t key, key_t endkey, uint32_t num)
	: Packet<key_t>(packet_type_t::SCAN_REQ, hashidx, key), _endkey(endkey), _num(num)
{
}

template<class key_t>
ScanRequest<key_t>::ScanRequest(const char * data, uint32_t recv_size) {
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == packet_type_t::SCAN_REQ);
}

template<class key_t>
key_t ScanRequest<key_t>::endkey() const {
	return this->_endkey;
}

template<class key_t>
uint32_t ScanRequest<key_t>::num() const {
	return this->_num;
}

template<class key_t>
uint32_t ScanRequest<key_t>::size() {
	return sizeof(uint8_t) + sizeof(uint16_t) + sizeof(key_t) + sizeof(key_t) + sizeof(uint32_t);
}

template<class key_t>
uint32_t ScanRequest<key_t>::serialize(char * const data, uint32_t max_size) {
	uint32_t my_size = this->size();
	INVARIANT(max_size >= my_size);
	char *begin = data;
	memcpy(begin, (void *)&this->_type, sizeof(uint8_t));
	begin += sizeof(uint8_t);
	uint16_t bigendian_hashidx = htons(this->_hashidx);
	memcpy(begin, (void *)&bigendian_hashidx, sizeof(uint16_t)); // Small-endian to big-endian
	begin += sizeof(uint16_t);
	memcpy(begin, (void *)&this->_key, sizeof(key_t));
	begin += sizeof(key_t);
	memcpy(begin, (void *)&this->_endkey, sizeof(key_t));
	begin += sizeof(key_t);
	memcpy(begin, (void *)&this->_num, sizeof(uint32_t));
	return my_size;
}

template<class key_t>
void ScanRequest<key_t>::deserialize(const char * data, uint32_t recv_size) {
	uint32_t my_size = this->size();
	INVARIANT(my_size == recv_size);
	const char *begin = data;
	memcpy((void *)&this->_type, begin, sizeof(uint8_t));
	begin += sizeof(uint8_t);
	memcpy((void *)&this->_hashidx, begin, sizeof(uint16_t));
	this->_hashidx = ntohs(this->_hashidx); // Big-endian to small-endian
	begin += sizeof(uint16_t);
	memcpy((void *)&this->_key, begin, sizeof(key_t));
	begin += sizeof(key_t);
	memcpy((void *)&this->_endkey, begin, sizeof(key_t));
	begin += sizeof(key_t);
	memcpy((void *)&this->_num, begin, sizeof(uint32_t));
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
	return sizeof(int8_t) + sizeof(key_t) + sizeof(int32_t) + val_t::MAX_VALLEN + sizeof(bool);
}

template<class key_t, class val_t>
uint32_t GetResponse<key_t, val_t>::serialize(char * const data, uint32_t max_size) {
	//uint32_t my_size = this->size();
	//INVARIANT(max_size >= my_size);
	char *begin = data;
	memcpy(begin, (void *)&this->_type, sizeof(int8_t));
	begin += sizeof(int8_t);
	uint32_t tmp_keysize = this->_key.serialize(begin, max_size - sizeof(int8_t));
	begin += tmp_keysize;
	uint32_t tmp_valsize = this->_val.serialize(begin, max_size-sizeof(int8_t)-tmp_keysize);
	begin += tmp_valsize;
	memcpy(begin, (void *)&this->_stat, sizeof(bool));
	return sizeof(int8_t) + tmp_keysize + tmp_valsize + sizeof(bool);
}

template<class key_t, class val_t>
void GetResponse<key_t, val_t>::deserialize(const char * data, uint32_t recv_size) {
	//uint32_t my_size = this->size();
	//INVARIANT(my_size == recv_size);
	const char *begin = data;
	memcpy((void *)&this->_type, begin, sizeof(int8_t));
	begin += sizeof(int8_t);
	uint32_t tmp_keysize = this->_key.deserialize(begin, recv_size - sizeof(int8_t));
	begin += tmp_keysize;
	uint32_t tmp_valsize = this->_val.deserialize(begin, recv_size - sizeof(int8_t) - tmp_keysize);
	begin += tmp_valsize;
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
	return sizeof(int8_t) + sizeof(key_t) + sizeof(bool);
}

template<class key_t>
uint32_t PutResponse<key_t>::serialize(char * const data, uint32_t max_size) {
	uint32_t my_size = this->size();
	INVARIANT(max_size >= my_size);
	char *begin = data;
	memcpy(begin, (void *)&this->_type, sizeof(int8_t));
	begin += sizeof(int8_t);
	uint32_t tmp_keysize = this->_key.serialize(begin, max_size - sizeof(int8_t));
	begin += tmp_keysize; 
	memcpy(begin, (void *)&this->_stat, sizeof(bool));
	return my_size;
}

template<class key_t>
void PutResponse<key_t>::deserialize(const char * data, uint32_t recv_size) {
	uint32_t my_size = this->size();
	INVARIANT(my_size >= recv_size);
	const char *begin = data;
	memcpy((void *)&this->_type, begin, sizeof(int8_t));
	begin += sizeof(int8_t);
	uint32_t tmp_keysize = this->_key.deserialize(begin, recv_size - sizeof(int8_t));
	begin += tmp_keysize;
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
	return sizeof(int8_t) + sizeof(key_t) + sizeof(bool);
}

template<class key_t>
uint32_t DelResponse<key_t>::serialize(char * const data, uint32_t max_size) {
	uint32_t my_size = this->size();
	INVARIANT(max_size >= my_size);
	char *begin = data;
	memcpy(begin, (void *)&this->_type, sizeof(int8_t));
	begin += sizeof(int8_t);
	uint32_t tmp_keysize = this->_key.serialize(begin, max_size - sizeof(int8_t));
	begin += tmp_keysize;
	memcpy(begin, (void *)&this->_stat, sizeof(bool));
	return sizeof(int8_t) + tmp_keysize + sizeof(bool);
}

template<class key_t>
void DelResponse<key_t>::deserialize(const char * data, uint32_t recv_size) {
	uint32_t my_size = this->size();
	INVARIANT(my_size == recv_size);
	const char *begin = data;
	memcpy((void *)&this->_type, begin, sizeof(int8_t));
	begin += sizeof(int8_t);
	uint32_t tmp_keysize = this->_key.deserialize(begin, recv_size - sizeof(int8_t));
	begin += tmp_keysize;
	memcpy((void *)&this->_stat, begin, sizeof(bool));
}

// ScanResponse

template<class key_t, class val_t>
ScanResponse<key_t, val_t>::ScanResponse(uint16_t hashidx, key_t key, key_t endkey, uint32_t num, std::vector<std::pair<key_t, val_t>> pairs) 
	: Packet<key_t>(PacketType::SCAN_RES, hashidx, key), _endkey(endkey), _num(num)
{	
	INVARIANT(pairs.size() == num);
	this->_pairs.assign(pairs.begin(), pairs.end());
}

template<class key_t, class val_t>
ScanResponse<key_t, val_t>::ScanResponse(const char * data, uint32_t recv_size) {
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::SCAN_RES);
}

template<class key_t, class val_t>
key_t ScanResponse<key_t, val_t>::endkey() const {
	return this->_endkey;
}

template<class key_t, class val_t>
uint32_t ScanResponse<key_t, val_t>::num() const {
	return this->_num;
}

template<class key_t, class val_t>
std::vector<std::pair<key_t, val_t>> ScanResponse<key_t, val_t>::pairs() const {
	return this->_pairs;
}

template<class key_t, class val_t>
uint32_t ScanResponse<key_t, val_t>::size() {
	// NOTE: invoke size() after getting correct num
	// NOTE: PUTREQ and GETRES needs alignment since they should be processed in tofino
	// However, SCANRES does't need alignment
	//return sizeof(uint8_t) + sizeof(uint8_t) + sizeof(key_t) + sizeof(uint32_t) + 
	//	this->_num*(sizeof(key_t) + sizeof(uint8_t) + val_t::max_bytesnum();
	return sizeof(uint8_t) + sizeof(uint16_t) + sizeof(key_t) + sizeof(key_t) + sizeof(uint32_t);
}

template<class key_t, class val_t>
uint32_t ScanResponse<key_t, val_t>::serialize(char * const data, uint32_t max_size) {
	uint32_t my_size = this->size();
	INVARIANT(max_size >= my_size);
	char *begin = data;
	memcpy(begin, (void *)&this->_type, sizeof(uint8_t));
	begin += sizeof(uint8_t);
	uint16_t bigendian_hashidx = htons(this->_hashidx);
	memcpy(begin, (void *)&bigendian_hashidx, sizeof(uint16_t)); // Small-endian to big-endian
	begin += sizeof(uint16_t);
	memcpy(begin, (void *)&this->_key, sizeof(key_t));
	begin += sizeof(key_t);
	memcpy(begin, (void *)&this->_endkey, sizeof(key_t));
	begin += sizeof(key_t);
	memcpy(begin, (void *)&this->_num, sizeof(val_t));
	begin += sizeof(uint32_t);
	for (uint32_t pair_i = 0; pair_i < this->_pairs.size(); pair_i++) {
		memcpy(begin, (void *)&this->_pairs[pair_i].first, sizeof(key_t));
		begin += sizeof(key_t);
		uint32_t tmp = this->_pairs[pair_i].second.serialize(begin);
		if (pair_i != this->_pairs.size() - 1) {
			begin += tmp;
		}

		my_size = my_size + sizeof(key_t) + tmp;
		INVARIANT(max_size >= my_size);
	}
	return my_size;
}

template<class key_t, class val_t>
void ScanResponse<key_t, val_t>::deserialize(const char * data, uint32_t recv_size) {
	uint32_t my_size = this->size();
	INVARIANT(recv_size >= my_size);
	const char *begin = data;
	memcpy((void *)&this->_type, begin, sizeof(uint8_t));
	begin += sizeof(uint8_t);
	memcpy((void *)&this->_hashidx, begin, sizeof(uint16_t));
	this->_hashidx = ntohs(this->_hashidx); // Big-endian to small-endian
	begin += sizeof(uint16_t);
	memcpy((void *)&this->_key, begin, sizeof(key_t));
	begin += sizeof(key_t);
	memcpy((void *)&this->_endkey, begin, sizeof(key_t));
	begin += sizeof(key_t);
	memcpy((void *)&this->_num, begin, sizeof(uint32_t));
	begin += sizeof(uint32_t);

	key_t tmp_key;
	val_t tmp_val;
	this->_pairs.reserve(this->_num);
	for (uint32_t pair_i = 0; pair_i < this->_num; pair_i++) {
		memcpy((void *)&tmp_key, begin, sizeof(key_t));
		begin += sizeof(key_t);
		uint32_t tmp = tmp_val.deserialize(begin);
		if (pair_i != uint32_t(this->_num - 1)) {
			begin += tmp;
		}
		this->_pairs.push_back(std::pair<key_t, val_t>(tmp_key, tmp_val));

		my_size = my_size + sizeof(key_t) + tmp;
		INVARIANT(recv_size >= my_size);
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
GetResponseLatestSeq<key_t, val_t>::GetResponseLatestSeq(key_t key, val_t val, int32_t seq)
	: Packet<key_t>::Packet(key), _val(val), _seq(seq)
{
	this->_type = static_cast<int8_t>(PacketType::GETRES_LATEST_SEQ);
	INVARIANT(this->_val.val_length <= val_t::SWITCH_MAX_VALLEN);
	INVARIANT(seq >= 0);
}

template<class key_t, class val_t>
uint32_t GetResponseLatestSeq<key_t, val_t>::serialize(char * const data, uint32_t max_size) {
	//uint32_t my_size = this->size();
	//INVARIANT(max_size >= my_size);
	char *begin = data;
	memcpy(begin, (void *)&this->_type, sizeof(int8_t));
	begin += sizeof(int8_t);
	uint32_t tmp_keysize = this->_key.serialize(begin, max_size - sizeof(int8_t));
	begin += tmp_keysize;
	uint32_t tmp_valsize = this->_val.serialize(begin, max_size-sizeof(int8_t)-tmp_keysize);
	begin += tmp_valsize;
	uint32_t bigendian_seq = htonl(uint32_t(this->_seq));
	memcpy(begin, (void *)&bigendian_seq, sizeof(uint32_t)); // little-endian to big-endian
	return sizeof(int8_t) + tmp_keysize + tmp_valsize + sizeof(uint32_t);
}

template<class key_t, class val_t>
int32_t GetResponseLatestSeq<key_t, val_t>::val() {
	return this->_val;
}

template<class key_t, class val_t>
int32_t GetResponseLatestSeq<key_t, val_t>::seq() {
	return this->_seq;
}

template<class key_t, class val_t>
uint32_t GetResponseLatestSeq<key_t, val_t>::size() { // unused
	return sizeof(int8_t) + sizeof(key_t) + sizeof(int32_t) + val_t::MAX_VALLEN + sizeof(int32_t);
}

template<class key_t, class val_t>
void GetResponseLatestSeq<key_t, val_t>::deserialize(const char * data, uint32_t recv_size)
{
	COUT_N_EXIT("Invalid invoke of deserialize for GetResponseLatestSeq");
}

// GetResponseLatestSeqInswitchCase1 (value must <= 128B; only used by end-hosts)

template<class key_t, class val_t>
GetResponseLatestSeqInswitchCase1<key_t, val_t>::GetResponseLatestSeqInswitchCase1(key_t key, val_t val, int32_t seq, int16_t idx, bool stat) 
	: GetResponseLatestSeq<key_t, val_t>::GetResponse(key, val, seq), _idx(idx), _stat(stat)
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
int16_t GetResponseLatestSeqInswitchCase1<key_t, val_t>::idx() const {
	return _idx;
}

template<class key_t, class val_t>
bool GetResponseLatestSeqInswitchCase1<key_t, val_t>::stat() const {
	return _stat;
}

template<class key_t, class val_t>
uint32_t GetResponseLatestSeqInswitchCase1<key_t, val_t>::size() { // unused
	return sizeof(int8_t) + sizeof(key_t) + sizeof(int32_t) + val_t::MAX_VALLEN + sizeof(int32_t) + sizeof(int16_t) + sizeof(bool);
}

template<class key_t, class val_t>
uint32_t GetResponseLatestSeqInswitchCase1<key_t, val_t>::serialize(char * const data, uint32_t max_size) {
	//uint32_t my_size = this->size();
	//INVARIANT(max_size >= my_size);
	char *begin = data;
	memcpy(begin, (void *)&this->_type, sizeof(int8_t));
	begin += sizeof(int8_t);
	uint32_t tmp_keysize = this->_key.serialize(begin, max_size - sizeof(int8_t));
	begin += tmp_keysize;
	uint32_t tmp_valsize = this->_val.serialize(begin, max_size-sizeof(int8_t)-tmp_keysize);
	begin += tmp_valsize;
	uint32_t bigendian_seq = htonl(uint32_t(this->_seq));
	memcpy(begin, (void *)&bigendian_seq, sizeof(uint32_t));
	begin += sizeof(uint32_t);
	memset(begin, 0, INSWITCH_PREV_BYTES); // the first bytes of inswitch_hdr
	begin += INSWITCH_PREV_BYTES;
	uint16_t bigendian_idx = htons(uint16_t(this->_idx));
	memcpy(begin, (void *)&bigendian_idx, sizeof(uint16_t)); // little-endian to big-endian
	begin += sizeof(uint16_t);
	memcpy(begin, (void *)&this->_stat, sizeof(bool));
	begin += sizeof(bool);
	return sizeof(int8_t) + tmp_keysize + tmp_valsize + sizeof(int32_t) + sizeof(int16_t) + sizeof(bool);
}

template<class key_t, class val_t>
void GetResponseLatestSeqInswitchCase1<key_t, val_t>::deserialize(const char * data, uint32_t recv_size) {
	//uint32_t my_size = this->size();
	//INVARIANT(my_size == recv_size);
	const char *begin = data;
	memcpy((void *)&this->_type, begin, sizeof(int8_t));
	begin += sizeof(int8_t);
	uint32_t tmp_keysize = this->_key.deserialize(begin, recv_size - sizeof(int8_t));
	begin += tmp_keysize;
	uint32_t tmp_valsize = this->_val.deserialize(begin, recv_size - sizeof(int8_t) - tmp_keysize);
	begin += tmp_valsize;
	memcpy((void *)&this->_seq, begin, sizeof(int32_t));
	this->_seq = int32_t(ntohl(uint32_t(this->seq)));
	begin += sizeof(int32_t);
	begin += INSWITCH_PREV_BYTES; // the first bytes of inswitch_hdr
	memcpy((void *)&this->_idx, begin, sizeof(int16_t));
	this->_idx = int16_t(ntohs(uint16_t(this->_idx))); // big-endian to little-endian
	begin += sizeof(int16_t);
	memcpy((void *)&this->_stat, begin, sizeof(bool));
	begin += sizeof(bool);
}

// GetResponseDeletedSeq (value must = 0B)

template<class key_t, class val_t>
GetResponseDeletedSeq<key_t, val_t>::GetResponseDeletedSeq(key_t key, val_t val, int32_t seq)
	: GetResponseLatestSeq<key_t, val_t>::GetResponseLatestSeq(key, val, seq)
{
	this->_type = static_cast<int8_t>(PacketType::GETRES_DELETED_SEQ);
	INVARIANT(this->_val.val_length == 0);
	INVARIANT(seq >= 0);
}

template<class key_t, class val_t>
void GetResponseDeletedSeq<key_t, val_t>::deserialize(const char * data, uint32_t recv_size)
{
	COUT_N_EXIT("Invalid invoke of deserialize for GetResponseDeletedSeq");
}

// GetResponseDeletedSeqInswitchCase1 (value must <= 128B; only used by end-hosts)

template<class key_t, class val_t>
GetResponseDeletedSeqInswitchCase1<key_t, val_t>::GetResponseDeletedSeqInswitchCase1(key_t key, val_t val, int32_t seq, int16_t idx, bool stat) 
	: GetResponseLatestSeqInswitchCase1<key_t, val_t>::GetResponseLatestSeqInswitchCase1(key, val, seq, idx, stat)
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
PutRequestSeq<key_t, val_t>::PutRequestSeq(const char * data, uint32_t recv_size)
{
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::PUTREQ_SEQ);
	INVARIANT(this->_val.val_length <= val_t::SWITCH_MAX_VALLEN)
	INVARIANT(seq >= 0);
}

template<class key_t, class val_t>
void PutRequestSeq<key_t, val_t>::deserialize(const char * data, uint32_t recv_size)
{
	//uint32_t my_size = this->size();
	//INVARIANT(my_size == recv_size);
	const char *begin = data;
	memcpy((void *)&this->_type, begin, sizeof(int8_t));
	begin += sizeof(int8_t);
	uint32_t tmp_keysize = this->_key.deserialize(begin, recv_size - sizeof(int8_t));
	begin += tmp_keysize;
	uint32_t tmp_valsize = this->_val.deserialize(begin, recv_size - sizeof(int8_t) - tmp_keysize);
	begin += tmp_valsize;
	memcpy((void *)&this->_seq, begin, sizeof(int32_t));
	this->_seq = int32_t(ntohl(uint32_t(this->seq))); // Big-endian to little-endian
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
	INVARIANT(seq >= 0);
}

template<class key_t, class val_t>
uint32_t PutRequestPopSeq<key_t, val_t>::serialize(char * const data, uint32_t max_size)
{
	COUT_N_EXIT("Invalid invoke of serialize for PutRequestPopSeq");
}

// PutRequestSeqInswitchCase1 (value must <= 128B; only used by end-hosts)

template<class key_t, class val_t>
PutRequestSeqInswitchCase1<key_t, val_t>::PutRequestSeqInswitchCase1(key_t key, val_t val, int32_t seq, int16_t idx, bool stat) 
	: GetResponseLatestSeqInswitchCase1<key_t, val_t>::GetResponseLatestSeqInswitchCase1(key, val, seq, idx, stat)
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
	INVARIANT(seq >= 0);
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
	INVARIANT(seq >= 0);
}

template<class key_t, class val_t>
uint32_t PutRequestPopSeqCase3<key_t, val_t>::serialize(char * const data, uint32_t max_size)
{
	COUT_N_EXIT("Invalid invoke of serialize for PutRequestPopSeqCase3");
}

// DelRequestSeq

template<class key_t>
DelRequestSeq<key_t>::DelRequestSeq(const char * data, uint32_t recv_size) {
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == packet_type_t::DELREQ_SEQ);
}

template<class key_t>
uint32_t DelRequestSeq<key_t>::size() {
	return sizeof(int8_t) + sizeof(key_t) + sizeof(int32_t);
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
	memcpy((void *)&this->_type, begin, sizeof(int8_t));
	begin += sizeof(int8_t);
	uint32_t tmp_keysize = this->_key.deserialize(begin, recv_size - sizeof(int8_t));
	begin += tmp_keysize;
	memcpy((void *)&this->_seq, begin, sizeof(int32_t));
	this->_seq = int32_t(ntohl(uint32_t(this->_seq)));
}

// DelRequestSeqInswitchCase1 (value must <= 128B; only used by end-hosts)

template<class key_t, class val_t>
DelRequestSeqInswitchCase1<key_t, val_t>::DelRequestSeqInswitchCase1(key_t key, val_t val, int32_t seq, int16_t idx, bool stat) 
	: GetResponseLatestSeqInswitchCase1<key_t, val_t>::GetResponseLatestSeqInswitchCase1(key, val, seq, idx, stat)
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

// CachePop (value must <= 128B; only used by end-hosts)

template<class key_t, class val_t>
CachePop<key_t, val_t>::CachePop(key_t key, val_t val, int32_t seq, int16_t serveridx)
	: GetResponseLatestSeq<key_t, val_t>::GetResponseLatestSeq(key, val, seq), _serveridx(serveridx)
{
	this->_type = static_cast<int8_t>(PacketType::CACHE_POP);
	INVARIANT(this->_val.val_length <= val_t::SWITCH_MAX_VALLEN);
	INVARIANT(seq >= 0);
	INVARIANT(serveridx >= 0);
}

template<class key_t, class val_t>
CachePop<key_t, val_t>::CachePop(const char * data, uint32_t recv_size) {
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::CACHE_POP);
	INVARIANT(this->_val.val_length <= val_t::SWITCH_MAX_VALLEN)
	INVARIANT(seq >= 0);
	INVARIANT(serveridx >= 0);
}

template<class key_t, class val_t>
uint32_t CachePop<key_t, val_t>::serialize(char * const data, uint32_t max_size) {
	//uint32_t my_size = this->size();
	//INVARIANT(max_size >= my_size);
	char *begin = data;
	memcpy(begin, (void *)&this->_type, sizeof(int8_t));
	begin += sizeof(int8_t);
	uint32_t tmp_keysize = this->_key.serialize(begin, max_size - sizeof(int8_t));
	begin += tmp_keysize;
	uint32_t tmp_valsize = this->_val.serialize(begin, max_size-sizeof(int8_t)-tmp_keysize);
	begin += tmp_valsize;
	uint32_t bigendian_seq = htonl(uint32_t(this->_seq));
	memcpy(begin, (void *)&bigendian_seq, sizeof(uint32_t)); // little-endian to big-endian
	begin += sizeof(uint32_t);
	uint16_t bigendian_serveridx = htons(uint16_t(this->_serveridx));
	memcpy(begin, (void *)&bigendian_serveridx, sizeof(uint16_t)); // little-endian to big-endian
	return sizeof(int8_t) + tmp_keysize + tmp_valsize + sizeof(bool) + sizeof(uint32_t) + sizeof(uint16_t);
}

template<class key_t, class val_t>
uint16_t CachePop<key_t, val_t>::serveridx() const {
	return _serveridx;
}

template<class key_t, class val_t>
uint32_t CachePop<key_t, val_t>::size() { // unused
	return sizeof(int8_t) + sizeof(key_t) + sizeof(int32_t) + val_t::MAX_VALLEN + sizeof(int32_t) + sizeof(int16_t);
}

template<class key_t, class val_t>
void CachePop<key_t, val_t>::deserialize(const char * data, uint32_t recv_size)
{
	//uint32_t my_size = this->size();
	//INVARIANT(my_size == recv_size);
	const char *begin = data;
	memcpy((void *)&this->_type, begin, sizeof(int8_t));
	begin += sizeof(int8_t);
	uint32_t tmp_keysize = this->_key.deserialize(begin, recv_size - sizeof(int8_t));
	begin += tmp_keysize;
	uint32_t tmp_valsize = this->_val.deserialize(begin, recv_size - sizeof(int8_t) - tmp_keysize);
	begin += tmp_valsize;
	memcpy((void *)&this->_seq, begin, sizeof(int32_t));
	this->_seq = int32_t(ntohl(uint32_t(this->seq))); // Big-endian to little-endian
	begin += sizeof(int32_t);
	mempcy((void *)&this->_serveridx, begin, sizeof(int16_t));
	this->_serveridx = int16_t(ntohs(uint16_t(this->_serveridx))); // Big-endian to little-endian
}

// CachePopInSwitch (valud must <= 128B)

template<class key_t, class val_t>
CachePopInSwitch<key_t, val_t>::CachePopInSwitch(key_t key, val_t val, int32_t seq, int16_t freeidx)
	: GetResponseLatestSeq<key_t, val_t>::GetResponseLatestSeq(key, val, seq), _freeidx(freeidx)
{
	this->_type = static_cast<uint8_t>(PacketType::CACHE_POP_INSWITCH);
	INVARIANT(this->_val.val_length <= val_t::SWITCH_MAX_VALLEN);
	INVARIANT(seq >= 0);
	INVARIANT(freeidx >= 0);
}

template<class key_t, class val_t>
uint32_t CachePopInSwitch<key_t, val_t>::serialize(char * const data, uint32_t max_size) {
	//uint32_t my_size = this->size();
	//INVARIANT(max_size >= my_size);
	char *begin = data;
	memcpy(begin, (void *)&this->_type, sizeof(int8_t));
	begin += sizeof(int8_t);
	uint32_t tmp_keysize = this->_key.serialize(begin, max_size - sizeof(int8_t));
	begin += tmp_keysize;
	uint32_t tmp_valsize = this->_val.serialize(begin, max_size-sizeof(int8_t)-tmp_keysize);
	begin += tmp_valsize;
	uint32_t bigendian_seq = htonl(uint32_t(this->_seq));
	memcpy(begin, (void *)&bigendian_seq, sizeof(uint32_t)); // little-endian to big-endian
	begin += sizeof(uint32_t);
	memset(begin, 0, INSWITCH_PREV_BYTES); // the first bytes of inswitch_hdr
	begin += INSWITCH_PREV_BYTES;
	uint16_t bigendian_freeidx = htons(uint16_t(this->_freeidx));
	memcpy(begin, (void *)&bigendian_freeidx, sizeof(uint16_t)); // little-endian to big-endian
	return sizeof(int8_t) + tmp_keysize + tmp_valsize + sizeof(bool) + sizeof(uint32_t) + 5 + sizeof(uint16_t);
}

template<class key_t, class val_t>
uint16_t CachePopInSwitch<key_t, val_t>::freeidx() const {
	return _freeidx;
}

template<class key_t, class val_t>
uint32_t CachePopInSwitch<key_t, val_t>::size() { // unused
	return sizeof(int8_t) + sizeof(key_t) + sizeof(int32_t) + val_t::MAX_VALLEN + sizeof(int32_t) + 5 + sizeof(int16_t);
}

template<class key_t, class val_t>
void CachePopInSwitch<key_t, val_t>::deserialize(const char * data, uint32_t recv_size)
{
	COUT_N_EXIT("Invalid invoke of deserialize for CachePopInSwitch");
}

// CachePopInSwitchAck (no value)

template<class key_t>
CachePopInSwitchAck<key_t>::CachePopInSwitchAck(const char *data, uint32_t recv_size)
{
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::CACHE_POP_INSWITCH_ACK);
}

template<class key_t>
uint32_t CachePopInSwitchAck<key_t>::serialize(char * const data, uint32_t max_size)
{
	COUT_N_EXIT("Invalid invoke of serialize for CachePopInSwitchAck");
}

// CacheEvict (value must <= 128B; only used by end-hosts)

template<class key_t, class val_t>
CacheEvict<key_t, val_t>::CacheEvict(key_t key, val_t val, int32_t seq, bool stat, int16_t serveridx) 
	: GetResponse<key_t, val_t>::GetResponseLatestSeq(key, val, seq), _stat(stat), _serveridx(serveridx)
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
int16_t CacheEvict<key_t, val_t>::serveridx() const {
	return _serveridx;
}

template<class key_t, class val_t>
uint32_t CacheEvict<key_t, val_t>::size() { // unused
	return sizeof(int8_t) + sizeof(key_t) + sizeof(int32_t) + val_t::MAX_VALLEN + sizeof(int32_t) + sizeof(bool) + sizeof(int16_t);
}

template<class key_t, class val_t>
uint32_t CacheEvict<key_t, val_t>::serialize(char * const data, uint32_t max_size) {
	//uint32_t my_size = this->size();
	//INVARIANT(max_size >= my_size);
	char *begin = data;
	memcpy(begin, (void *)&this->_type, sizeof(int8_t));
	begin += sizeof(int8_t);
	uint32_t tmp_keysize = this->_key.serialize(begin, max_size - sizeof(int8_t));
	begin += tmp_keysize;
	uint32_t tmp_valsize = this->_val.serialize(begin, max_size-sizeof(int8_t)-tmp_keysize);
	begin += tmp_valsize;
	uint32_t bigendian_seq = htonl(uint32_t(this->_seq));
	memcpy(begin, (void *)&bigendian_seq, sizeof(uint32_t));
	begin += sizeof(uint32_t);
	memcpy(begin, (void *)&this->_stat, sizeof(bool));
	begin += sizeof(bool);
	uint16_t bigendian_serveridx = htons(uint16_t(this->_serveridx));
	memcpy(begin, (void *)&bigendian_serveridx, sizeof(uint16_t));
	return sizeof(int8_t) + tmp_keysize + tmp_valsize + sizeof(int32_t) + sizeof(bool) + sizeof(int16_t);
}

template<class key_t, class val_t>
void CacheEvict<key_t, val_t>::deserialize(const char * data, uint32_t recv_size) {
	//uint32_t my_size = this->size();
	//INVARIANT(my_size == recv_size);
	const char *begin = data;
	memcpy((void *)&this->_type, begin, sizeof(int8_t));
	begin += sizeof(int8_t);
	uint32_t tmp_keysize = this->_key.deserialize(begin, recv_size - sizeof(int8_t));
	begin += tmp_keysize;
	uint32_t tmp_valsize = this->_val.deserialize(begin, recv_size - sizeof(int8_t) - tmp_keysize);
	begin += tmp_valsize;
	memcpy((void *)&this->_seq, begin, sizeof(int32_t));
	this->_seq = int32_t(ntohl(uint32_t(this->seq)));
	begin += sizeof(int32_t);
	memcpy((void *)&this->_stat, begin, sizeof(bool));
	begin += sizeof(bool);
	memcpy((void *)&this->_serveridx, begin, sizeof(int16_t));
	this->_serveridx = int16_t(ntohs(uint16_t(this->serveridx)));
}

// CacheEvictAck

template<class key_t>
CacheEvictAck<key_t>::CacheEvictAck(key_t key) 
	: GetRequest<key_t>::GetRequest(key)
{
	this->_type = static_cast<uint8_t>(PacketType::CACHE_EVICT_ACK);
}

template<class key_t>
CacheEvict<key_t>::CacheEvict(const char * data, uint32_t recv_size) {
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::CACHE_EVICT_ACK);
}











// GetResponsePOP (value must <= 128B)

template<class key_t, class val_t>
GetResponsePOP<key_t, val_t>::GetResponsePOP(uint16_t hashidx, key_t key, val_t val, int32_t seq)
	: GetResponse<key_t, val_t>::GetResponse(hashidx, key, val), _seq(seq)
{
	this->_type = static_cast<uint8_t>(PacketType::GET_RES_POP);
	INVARIANT(this->_val.val_length <= val_t::SWITCH_MAX_VALLEN);
}

template<class key_t, class val_t>
uint32_t GetResponsePOP<key_t, val_t>::serialize(char * const data, uint32_t max_size) {
	//uint32_t my_size = this->size();
	//INVARIANT(max_size >= my_size);
	char *begin = data;
	memcpy(begin, (void *)&this->_type, sizeof(uint8_t));
	begin += sizeof(uint8_t);
	uint16_t bigendian_hashidx = htons(this->_hashidx);
	memcpy(begin, (void *)&bigendian_hashidx, sizeof(uint16_t)); // Small-endian to big-endian
	begin += sizeof(uint16_t);
	memcpy(begin, (void *)&this->_key, sizeof(key_t));
	begin += sizeof(key_t);
	uint32_t tmpsize = this->_val.serialize(begin, max_size-sizeof(uint8_t)-sizeof(uint16_t)-sizeof(key_t));
	begin += tmpsize;
	uint32_t bigendian_seq = htonl(uint32_t(this->_seq));
	memcpy(begin, (void *)&bigendian_seq, sizeof(uint32_t)); // little-endian to big-endian
	return sizeof(uint8_t) + sizeof(uint16_t) + sizeof(key_t) + tmpsize + sizeof(uint32_t);
}

template<class key_t, class val_t>
int32_t GetResponsePOP<key_t, val_t>::seq() {
	return this->_seq;
}

template<class key_t, class val_t>
uint32_t GetResponsePOP<key_t, val_t>::size() { // unused
	return sizeof(uint8_t) + sizeof(uint16_t) + sizeof(key_t) + sizeof(uint32_t) + val_t::MAX_VALLEN + sizeof(int32_t);
}

template<class key_t, class val_t>
void GetResponsePOP<key_t, val_t>::deserialize(const char * data, uint32_t recv_size)
{
	COUT_N_EXIT("Invalid invoke of deserialize for GetResponsePOP");
}

// GetResponseNPOP (value must = 0B)

template<class key_t, class val_t>
GetResponseNPOP<key_t, val_t>::GetResponseNPOP(uint16_t hashidx, key_t key, val_t val)
	: GetResponse<key_t, val_t>::GetResponse(hashidx, key, val)
{
	this->_type = static_cast<uint8_t>(PacketType::GET_RES_NPOP);
	INVARIANT(this->_val.val_length == 0);
}

template<class key_t, class val_t>
void GetResponseNPOP<key_t, val_t>::deserialize(const char * data, uint32_t recv_size)
{
	COUT_N_EXIT("Invalid invoke of deserialize for GetResponseNPOP");
}

// GetResponsePOPLarge (value must > 128B)

template<class key_t, class val_t>
GetResponsePOPLarge<key_t, val_t>::GetResponsePOPLarge(uint16_t hashidx, key_t key, val_t val)
	: GetResponse<key_t, val_t>::GetResponse(hashidx, key, val)
{
	this->_type = static_cast<uint8_t>(PacketType::GET_RES_POP_LARGE);
	INVARIANT(this->_val.val_length > val_t::SWITCH_MAX_VALLEN);
}

template<class key_t, class val_t>
void GetResponsePOPLarge<key_t, val_t>::deserialize(const char * data, uint32_t recv_size)
{
	COUT_N_EXIT("Invalid invoke of deserialize for GetResponsePOPLarge");
}

// GetResponsePOPEvict (value must <= 128B)

template<class key_t, class val_t>
GetResponsePOPEvict<key_t, val_t>::GetResponsePOPEvict(const char * data, uint32_t recv_size) {
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::GET_RES_POP_EVICT);
	INVARIANT(this->_val.val_length <= val_t::SWITCH_MAX_VALLEN);
}

template<class key_t, class val_t>
uint32_t GetResponsePOPEvict<key_t, val_t>::serialize(char * const data, uint32_t max_size) {
	COUT_N_EXIT("Invalid invoke of serialize for GetResponsePOPEvict");
}

// PutRequestSeq (value must <= 128B)

template<class key_t, class val_t>
PutRequestSeq<key_t, val_t>::PutRequestSeq(const char * data, uint32_t recv_size) {
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::PUT_REQ_SEQ);
	INVARIANT(this->_val.val_length <= val_t::SWITCH_MAX_VALLEN);
}

template<class key_t, class val_t>
uint32_t PutRequestSeq<key_t, val_t>::serialize(char * const data, uint32_t max_size) {
	COUT_N_EXIT("Invalid invoke of serialize for PutRequestSeq");
}

template<class key_t, class val_t>
int32_t PutRequestSeq<key_t, val_t>::seq() const {
	return this->_seq;
}

template<class key_t, class val_t>
uint32_t PutRequestSeq<key_t, val_t>::size() {
	return sizeof(uint8_t) + sizeof(uint16_t) + sizeof(key_t) + sizeof(uint32_t) + val_t::max_bytesnum() + sizeof(int32_t);
}

template<class key_t, class val_t>
void PutRequestSeq<key_t, val_t>::deserialize(const char * data, uint32_t recv_size) {
	//uint32_t my_size = this->size();
	//INVARIANT(my_size == recv_size);
	const char *begin = data;
	memcpy((void *)&this->_type, begin, sizeof(uint8_t));
	begin += sizeof(uint8_t);
	memcpy((void *)&this->_hashidx, begin, sizeof(uint16_t));
	this->_hashidx = ntohs(this->_hashidx); // Big-endian to small-endian
	begin += sizeof(uint16_t);
	memcpy((void *)&this->_key, begin, sizeof(key_t));
	begin += sizeof(key_t);
	uint32_t tmpsize = this->_val.deserialize(begin, recv_size-sizeof(uint8_t)-sizeof(uint16_t)-sizeof(key_t));
	begin += tmpsize;
	memcpy((void *)&this->_seq, begin, sizeof(int32_t));
	this->_seq = int32_t(ntohl(uint32_t(this->_seq))); // Big-endian -> little-endian
}

// PutRequestPOPEvict (value must <= 128B)

template<class key_t, class val_t>
PutRequestPOPEvict<key_t, val_t>::PutRequestPOPEvict(const char * data, uint32_t recv_size) {
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::PUT_REQ_POP_EVICT);
	INVARIANT(this->_val.val_length <= val_t::SWITCH_MAX_VALLEN);
}

template<class key_t, class val_t>
uint32_t PutRequestPOPEvict<key_t, val_t>::serialize(char * const data, uint32_t max_size) {
	COUT_N_EXIT("Invalid invoke of serialize for PutRequestPOPEvict");
}

// PutRequestLarge (value must > 128B)

template<class key_t, class val_t>
PutRequestLarge<key_t, val_t>::PutRequestLarge(uint16_t hashidx, key_t key, val_t val)
	: PutRequest<key_t, val_t>::PutRequest(hashidx, key, val)
{
	this->_type = static_cast<uint8_t>(PacketType::PUT_REQ_LARGE);
	INVARIANT(this->_val.val_length > val_t::SWITCH_MAX_VALUE);
}

template<class key_t, class val_t>
PutRequestLarge<key_t, val_t>::PutRequestLarge(const char * data, uint32_t recv_size) {
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::PUT_REQ_LARGE);
	INVARIANT(this->_val.val_length > val_t::SWITCH_MAX_VALLEN);
}

// PutRequestLargeSeq (value must > 128B)

template<class key_t, class val_t>
PutRequestLargeSeq<key_t, val_t>::PutRequestLargeSeq(const char * data, uint32_t recv_size) {
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::PUT_REQ_LARGE_SEQ);
	INVARIANT(this->_val.val_length > val_t::SWITCH_MAX_VALLEN);
}

template<class key_t, class val_t>
void PutRequestLargeSeq<key_t, val_t>::deserialize(const char * data, uint32_t recv_size) {
	//uint32_t my_size = this->size();
	//INVARIANT(my_size == recv_size);
	const char *begin = data;
	memcpy((void *)&this->_type, begin, sizeof(uint8_t));
	begin += sizeof(uint8_t);
	memcpy((void *)&this->_hashidx, begin, sizeof(uint16_t));
	this->_hashidx = ntohs(this->_hashidx); // Big-endian to small-endian
	begin += sizeof(uint16_t);
	memcpy((void *)&this->_key, begin, sizeof(key_t));
	begin += sizeof(key_t);
	uint32_t tmpsize_vallen = this->_val.deserialize_vallen(begin, recv_size-sizeof(uint8_t)-sizeof(uint16_t)-sizeof(key_t));
	begin += tmpsize_vallen;
	memcpy((void *)&this->_seq, begin, sizeof(int32_t));
	this->_seq = int32_t(ntohl(uint32_t(this->_seq))); // Big-endian -> little-endian
	begin += sizeof(int32_t);
	uint32_t tmpsize_val = this->_val.deserialize_val(begin, recv_size-sizeof(uint8_t)-sizeof(uint16_t)-sizeof(key_t)-tmpsize_vallen-sizeof(int32_t));
}

template<class key_t, class val_t>
uint32_t PutRequestLargeSeq<key_t, val_t>::serialize(char * const data, uint32_t max_size) {
	COUT_N_EXIT("Invalid invoke of serialize for PutRequestLargeSeq");
}

// PutRequestLargeEvict (value must <= 128B)

template<class key_t, class val_t>
PutRequestLargeEvict<key_t, val_t>::PutRequestLargeEvict(const char * data, uint32_t recv_size) {
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::PUT_REQ_LARGE_EVICT);
	INVARIANT(this->_val.val_length <= val_t::SWITCH_MAX_VALLEN);
}

template<class key_t, class val_t>
uint32_t PutRequestLargeEvict<key_t, val_t>::serialize(char * const data, uint32_t max_size) {
	COUT_N_EXIT("Invalid invoke of serialize for PutRequestLargeEvict");
}

// DelRequestSeq

template<class key_t, class val_t>
DelRequestSeq<key_t, val_t>::DelRequestSeq(const char * data, uint32_t recv_size) {
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::DEL_REQ_SEQ);
}

template<class key_t, class val_t>
uint32_t DelRequestSeq<key_t, val_t>::serialize(char * const data, uint32_t max_size) {
	COUT_N_EXIT("Invalid invoke of serialize for DelRequestSeq");
}

template<class key_t, class val_t>
int32_t DelRequestSeq<key_t, val_t>::seq() const {
	return this->_seq;
}

template<class key_t, class val_t>
uint32_t DelRequestSeq<key_t, val_t>::size() {
	return sizeof(uint8_t) + sizeof(uint16_t) + sizeof(key_t) + sizeof(int32_t);
}

template<class key_t, class val_t>
void DelRequestSeq<key_t, val_t>::deserialize(const char * data, uint32_t recv_size) {
	uint32_t my_size = this->size();
	INVARIANT(my_size <= recv_size);
	const char *begin = data;
	memcpy((void *)&this->_type, begin, sizeof(uint8_t));
	begin += sizeof(uint8_t);
	memcpy((void *)&this->_hashidx, begin, sizeof(uint16_t));
	this->_hashidx = ntohs(this->_hashidx); // Big-endian to small-endian
	begin += sizeof(uint16_t);
	memcpy((void *)&this->_key, begin, sizeof(key_t));
	begin += sizeof(key_t);
	memcpy((void *)&this->_seq, begin, sizeof(int32_t));
	this->_seq = int32_t(ntohl(uint32_t(this->_seq))); // Big-endian -> little-endian
}


// PutRequestCase1 (value must <= 128B)

template<class key_t, class val_t>
PutRequestCase1<key_t, val_t>::PutRequestCase1(const char * data, uint32_t recv_size) {
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::PUT_REQ_CASE1);
	INVARIANT(this->_val.val_length <= val_t::SWITCH_MAX_VALLEN);
}

template<class key_t, class val_t>
uint32_t PutRequestCase1<key_t, val_t>::serialize(char * const data, uint32_t max_size) {
	COUT_N_EXIT("Invalid invoke of serialize for PutRequestCase1");
}

// DelRequestCase1

template<class key_t, class val_t>
DelRequestCase1<key_t, val_t>::DelRequestCase1(const char * data, uint32_t recv_size) {
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::DEL_REQ_CASE1);
}

template<class key_t, class val_t>
uint32_t DelRequestCase1<key_t, val_t>::serialize(char * const data, uint32_t max_size) {
	COUT_N_EXIT("Invalid invoke of serialize for DelRequestCase1");
}

// GetResponsePOPEvictCase2 (value must <= 128B)

template<class key_t, class val_t>
GetResponsePOPEvictCase2<key_t, val_t>::GetResponsePOPEvictCase2(const char * data, uint32_t recv_size) {
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::GET_RES_POP_EVICT_CASE2);
	INVARIANT(this->_val.val_length <= val_t::SWITCH_MAX_VALLEN);
}

template<class key_t, class val_t>
uint32_t GetResponsePOPEvictCase2<key_t, val_t>::serialize(char * const data, uint32_t max_size) {
	COUT_N_EXIT("Invalid invoke of serialize for GetResponsePOPEvictCase2");
}

// PutRequestPOPEvictCase2 (value must <= 128B)

template<class key_t, class val_t>
PutRequestPOPEvictCase2<key_t, val_t>::PutRequestPOPEvictCase2(const char * data, uint32_t recv_size) {
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::PUT_REQ_POP_EVICT_CASE2);
	INVARIANT(this->_val.val_length <= val_t::SWITCH_MAX_VALLEN);
}

template<class key_t, class val_t>
uint32_t PutRequestPOPEvictCase2<key_t, val_t>::serialize(char * const data, uint32_t max_size) {
	COUT_N_EXIT("Invalid invoke of serialize for PutRequestPOPEvictCase2");
}

template<class key_t, class val_t>
bool PutRequestPOPEvictCase2<key_t, val_t>::valid() const {
	return this->_valid;
}

template<class key_t, class val_t>
uint32_t PutRequestPOPEvictCase2<key_t, val_t>::size() {
	return sizeof(uint8_t) + sizeof(uint16_t) + sizeof(key_t) + sizeof(uint32_t) + val_t::max_bytesnum() + sizeof(int32_t) + sizeof(int8_t);
}

template<class key_t, class val_t>
void PutRequestPOPEvictCase2<key_t, val_t>::deserialize(const char * data, uint32_t recv_size) {
	//uint32_t my_size = this->size();
	//INVARIANT(my_size == recv_size);
	const char *begin = data;
	memcpy((void *)&this->_type, begin, sizeof(uint8_t));
	begin += sizeof(uint8_t);
	memcpy((void *)&this->_hashidx, begin, sizeof(uint16_t));
	this->_hashidx = ntohs(this->_hashidx); // Big-endian to small-endian
	begin += sizeof(uint16_t);
	memcpy((void *)&this->_key, begin, sizeof(key_t));
	begin += sizeof(key_t);
	uint32_t tmpsize = this->_val.deserialize(begin, recv_size-sizeof(uint8_t)-sizeof(uint16_t)-sizeof(key_t));
	begin += tmpsize;
	memcpy((void *)&this->_seq, begin, sizeof(int32_t));
	this->_seq = int32_t(ntohl(uint32_t(this->_seq))); // Big-endian -> little-endian
	begin += sizeof(int32_t);
	int8_t otherhdr;
	memcpy((void *)&otherhdr, begin, sizeof(int8_t));
	this->_valid = ((otherhdr & VALID_MASK) == 1);
}

// PutRequestLargeEvictCase2 (value must <= 128B)

template<class key_t, class val_t>
PutRequestLargeEvictCase2<key_t, val_t>::PutRequestLargeEvictCase2(const char * data, uint32_t recv_size) {
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::PUT_REQ_LARGE_EVICT_CASE2);
	INVARIANT(this->_val.val_length <= val_t::SWITCH_MAX_VALLEN);
}

template<class key_t, class val_t>
uint32_t PutRequestLargeEvictCase2<key_t, val_t>::serialize(char * const data, uint32_t max_size) {
	COUT_N_EXIT("Invalid invoke of serialize for PutRequestLargeEvictCase2");
}

// PutRequestCase3 (value must <= 128B)

template<class key_t, class val_t>
PutRequestCase3<key_t, val_t>::PutRequestCase3(const char * data, uint32_t recv_size) {
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::PUT_REQ_CASE3);
	INVARIANT(this->_val.val_length <= val_t::SWITCH_MAX_VALLEN);
}

template<class key_t, class val_t>
uint32_t PutRequestCase3<key_t, val_t>::serialize(char * const data, uint32_t max_size) {
	COUT_N_EXIT("Invalid invoke of serialize for PutRequestCase3");
}

// PutRequestLargeCase3 (value must > 128B)

template<class key_t, class val_t>
PutRequestLargeCase3<key_t, val_t>::PutRequestLargeCase3(const char * data, uint32_t recv_size) {
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::PUT_REQ_LARGE_CASE3);
	INVARIANT(this->_val.val_length > val_t::SWITCH_MAX_VALLEN);
}

template<class key_t, class val_t>
uint32_t PutRequestLargeCase3<key_t, val_t>::serialize(char * const data, uint32_t max_size) {
	COUT_N_EXIT("Invalid invoke of serialize for PutRequestLargeCase3");
}

// DelRequestCase3

template<class key_t>
DelRequestCase3<key_t>::DelRequestCase3(const char * data, uint32_t recv_size) {
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::DEL_REQ_CASE3);
}

template<class key_t>
uint32_t DelRequestCase3<key_t>::serialize(char * const data, uint32_t max_size) {
	COUT_N_EXIT("Invalid invoke of serialize for DelRequestCase3");
}

// PutResponseCase3

template<class key_t>
PutResponseCase3<key_t>::PutResponseCase3(uint16_t hashidx, key_t key, int16_t serveridx, bool stat)
	: PutResponse<key_t>::PutResponse(hashidx, key, stat), _serveridx(serveridx)
{
	this->_type = static_cast<uint8_t>(PacketType::PUT_RES_CASE3);
}

template<class key_t>
int16_t PutResponseCase3<key_t>::serveridx() const {
	return _serveridx;
}

template<class key_t>
uint32_t PutResponseCase3<key_t>::size() {
	return sizeof(uint8_t) + sizeof(uint16_t) + sizeof(key_t) + sizeof(int16_t) + sizeof(bool);
}

template<class key_t>
uint32_t PutResponseCase3<key_t>::serialize(char * const data, uint32_t max_size) {
	uint32_t my_size = this->size();
	INVARIANT(max_size >= my_size);
	char *begin = data;
	memcpy(begin, (void *)&this->_type, sizeof(uint8_t));
	begin += sizeof(uint8_t);
	uint16_t bigendian_hashidx = htons(this->_hashidx);
	memcpy(begin, (void *)&bigendian_hashidx, sizeof(uint16_t)); // Small-endian to big-endian
	begin += sizeof(uint16_t);
	memcpy(begin, (void *)&this->_key, sizeof(key_t));
	begin += sizeof(key_t);
	int16_t bigendian_serveridx = int16_t(htons(uint16_t(this->_serveridx))); // Small-endian to big-endian
	memcpy(begin, (void *)&bigendian_serveridx, sizeof(int16_t));
	begin += sizeof(int16_t);
	memcpy(begin, (void *)&this->_stat, sizeof(bool));
	return my_size;
}

template<class key_t>
void PutResponseCase3<key_t>::deserialize(const char * data, uint32_t recv_size)
{
	COUT_N_EXIT("Invalid invoke of deserialize for PutResponseCase3");
}

// DelResponseCase3

template<class key_t>
DelResponseCase3<key_t>::DelResponseCase3(uint16_t hashidx, key_t key, int16_t serveridx, bool stat)
	: DelResponse<key_t>::DelResponse(hashidx, key, stat), _serveridx(serveridx)
{
	this->_type = static_cast<uint8_t>(PacketType::DEL_RES_CASE3);
}

template<class key_t>
int16_t DelResponseCase3<key_t>::serveridx() const {
	return _serveridx;
}

template<class key_t>
uint32_t DelResponseCase3<key_t>::size() {
	return sizeof(uint8_t) + sizeof(uint16_t) + sizeof(key_t) + sizeof(int16_t) + sizeof(bool);
}

template<class key_t>
uint32_t DelResponseCase3<key_t>::serialize(char * const data, uint32_t max_size) {
	uint32_t my_size = this->size();
	INVARIANT(max_size >= my_size);
	char *begin = data;
	memcpy(begin, (void *)&this->_type, sizeof(uint8_t));
	begin += sizeof(uint8_t);
	uint16_t bigendian_hashidx = htons(this->_hashidx);
	memcpy(begin, (void *)&bigendian_hashidx, sizeof(uint16_t)); // Small-endian to big-endian
	begin += sizeof(uint16_t);
	memcpy(begin, (void *)&this->_key, sizeof(key_t));
	begin += sizeof(key_t);
	int16_t bigendian_serveridx = int16_t(htons(uint16_t(this->_serveridx))); // Small-endian to big-endian
	memcpy(begin, (void *)&bigendian_serveridx, sizeof(int16_t));
	begin += sizeof(int16_t);
	memcpy(begin, (void *)&this->_stat, sizeof(bool));
	return my_size;
}

template<class key_t>
void DelResponseCase3<key_t>::deserialize(const char * data, uint32_t recv_size)
{
	COUT_N_EXIT("Invalid invoke of deserialize for DelResponseCase3");
}

// GetResponsePOPEvictSwitch (value must <= 128B)

template<class key_t, class val_t>
GetResponsePOPEvictSwitch<key_t, val_t>::GetResponsePOPEvictSwitch(const char * data, uint32_t recv_size) {
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::GET_RES_POP_EVICT_SWITCH);
	INVARIANT(this->_val.val_length <= val_t::SWITCH_MAX_VALLEN);
}

template<class key_t, class val_t>
uint32_t GetResponsePOPEvictSwitch<key_t, val_t>::serialize(char * const data, uint32_t max_size) {
	COUT_N_EXIT("Invalid invoke of serialize for GetResponsePOPEvictSwitch");
}

// GetResponsePOPEvictCase2Switch (value must <= 128B)

template<class key_t, class val_t>
GetResponsePOPEvictCase2Switch<key_t, val_t>::GetResponsePOPEvictCase2Switch(const char * data, uint32_t recv_size) {
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::GET_RES_POP_EVICT_CASE2_SWITCH);
	INVARIANT(this->_val.val_length <= val_t::SWITCH_MAX_VALLEN);
}

template<class key_t, class val_t>
uint32_t GetResponsePOPEvictCase2Switch<key_t, val_t>::serialize(char * const data, uint32_t max_size) {
	COUT_N_EXIT("Invalid invoke of serialize for GetResponsePOPEvictCase2Switch");
}

// PutRequestPOPEvictSwitch (value must <= 128B)

template<class key_t, class val_t>
PutRequestPOPEvictSwitch<key_t, val_t>::PutRequestPOPEvictSwitch(const char * data, uint32_t recv_size) {
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::GET_RES_POP_EVICT_SWITCH);
	INVARIANT(this->_val.val_length <= val_t::SWITCH_MAX_VALLEN);
}

template<class key_t, class val_t>
uint32_t PutRequestPOPEvictSwitch<key_t, val_t>::serialize(char * const data, uint32_t max_size) {
	COUT_N_EXIT("Invalid invoke of serialize for PutRequestPOPEvictSwitch");
}

// PutRequestPOPEvictCase2Switch (value must <= 128B)

template<class key_t, class val_t>
PutRequestPOPEvictCase2Switch<key_t, val_t>::PutRequestPOPEvictCase2Switch(const char * data, uint32_t recv_size) {
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::GET_RES_POP_EVICT_CASE2_SWITCH);
	INVARIANT(this->_val.val_length <= val_t::SWITCH_MAX_VALLEN);
}

template<class key_t, class val_t>
uint32_t PutRequestPOPEvictCase2Switch<key_t, val_t>::serialize(char * const data, uint32_t max_size) {
	COUT_N_EXIT("Invalid invoke of serialize for PutRequestPOPEvictCase2Switch");
}

// PutRequestLargeEvictSwitch (value must <= 128B)

template<class key_t, class val_t>
PutRequestLargeEvictSwitch<key_t, val_t>::PutRequestLargeEvictSwitch(const char * data, uint32_t recv_size) {
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::GET_RES_Large_EVICT_SWITCH);
	INVARIANT(this->_val.val_length <= val_t::SWITCH_MAX_VALLEN);
}

template<class key_t, class val_t>
uint32_t PutRequestLargeEvictSwitch<key_t, val_t>::serialize(char * const data, uint32_t max_size) {
	COUT_N_EXIT("Invalid invoke of serialize for PutRequestLargeEvictSwitch");
}

// PutRequestLargeEvictCase2Switch (value must <= 128B)

template<class key_t, class val_t>
PutRequestLargeEvictCase2Switch<key_t, val_t>::PutRequestLargeEvictCase2Switch(const char * data, uint32_t recv_size) {
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::GET_RES_Large_EVICT_CASE2_SWITCH);
	INVARIANT(this->_val.val_length <= val_t::SWITCH_MAX_VALLEN);
}

template<class key_t, class val_t>
uint32_t PutRequestLargeEvictCase2Switch<key_t, val_t>::serialize(char * const data, uint32_t max_size) {
	COUT_N_EXIT("Invalid invoke of serialize for PutRequestLargeEvictCase2Switch");
}

// APIs
packet_type_t get_packet_type(const char * data, uint32_t recv_size) {
	INVARIANT(recv_size >= sizeof(uint8_t));
	uint8_t tmp;
	memcpy((void *)&tmp, data, sizeof(uint8_t));
	return static_cast<packet_type_t>(tmp);
}
