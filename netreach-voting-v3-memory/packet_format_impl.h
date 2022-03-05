#include "packet_format.h"

// Packet
template<class key_t>
Packet<key_t>::Packet() 
	: _type(static_cast<uint8_t>(0)), _hashidx(0), _key(key_t::min())
{
}

template<class key_t>
Packet<key_t>::Packet(packet_type_t type, uint16_t hashidx, key_t key)
	: _type(static_cast<uint8_t>(type)), _hashidx(hashidx), _key(key)
{
}

template<class key_t>
packet_type_t Packet<key_t>::type() const {
	return _type;
}

template<class key_t>
uint16_t Packet<key_t>::hashidx() const {
	return _hashidx;
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
GetRequest<key_t>::GetRequest(uint16_t hashidx, key_t key)
	: Packet<key_t>(packet_type_t::GET_REQ, hashidx, key)
{
}

template<class key_t>
GetRequest<key_t>::GetRequest(const char * data, uint32_t recv_size) {
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == packet_type_t::GET_REQ);
}

template<class key_t>
uint32_t GetRequest<key_t>::size() {
	return sizeof(uint8_t) + sizeof(uint16_t) + sizeof(key_t);
}

template<class key_t>
uint32_t GetRequest<key_t>::serialize(char * const data, uint32_t max_size) {
	uint32_t my_size = this->size();
	INVARIANT(max_size >= my_size);
	char *begin = data;
	memcpy(begin, (void *)&this->_type, sizeof(uint8_t));
	begin += sizeof(uint8_t);
	uint16_t bigendian_hashidx = htons(this->_hashidx);
	memcpy(begin, (void *)&bigendian_hashidx, sizeof(uint16_t)); // Small-endian to big-endian
	begin += sizeof(uint16_t);
	memcpy(begin, (void *)&this->_key, sizeof(key_t));
	return my_size;
}

template<class key_t>
void GetRequest<key_t>::deserialize(const char * data, uint32_t recv_size) {
	uint32_t my_size = this->size();
	INVARIANT(my_size == recv_size);
	const char *begin = data;
	memcpy((void *)&this->_type, begin, sizeof(uint8_t));
	begin += sizeof(uint8_t);
	memcpy((void *)&this->_hashidx, begin, sizeof(uint16_t));
	this->_hashidx = ntohs(this->_hashidx); // Big-endian to small-endian
	begin += sizeof(uint16_t);
	memcpy((void *)&this->_key, begin, sizeof(key_t));
}

// PutRequest

template<class key_t, class val_t>
PutRequest<key_t, val_t>::PutRequest()
	: Packet<key_t>(), _val()
{
}

template<class key_t, class val_t>
PutRequest<key_t, val_t>::PutRequest(uint16_t hashidx, key_t key, val_t val) 
	: Packet<key_t>(PacketType::PUT_REQ, hashidx, key), _val(val)
{	
}

template<class key_t, class val_t>
PutRequest<key_t, val_t>::PutRequest(const char * data, uint32_t recv_size) {
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::PUT_REQ);
}

template<class key_t, class val_t>
val_t PutRequest<key_t, val_t>::val() const {
	return _val;
}

template<class key_t, class val_t>
uint32_t PutRequest<key_t, val_t>::size() {
	return sizeof(uint8_t) + sizeof(uint16_t) + sizeof(key_t) + sizeof(uint8_t) + val_t::max_bytesnum();
}

template<class key_t, class val_t>
uint32_t PutRequest<key_t, val_t>::serialize(char * const data, uint32_t max_size) {
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
	uint32_t tmpsize = this->_val.serialize(begin);
	return sizeof(uint8_t) + sizeof(uint16_t) + sizeof(key_t) + tmpsize;
}

template<class key_t, class val_t>
void PutRequest<key_t, val_t>::deserialize(const char * data, uint32_t recv_size) {
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
	uint32_t tmpsize = this->_val.deserialize(begin);
}

// DelRequest

template<class key_t>
DelRequest<key_t>::DelRequest()
	: Packet<key_t>()
{
}

template<class key_t>
DelRequest<key_t>::DelRequest(uint16_t hashidx, key_t key)
	: Packet<key_t>(packet_type_t::DEL_REQ, hashidx, key)
{
}

template<class key_t>
DelRequest<key_t>::DelRequest(const char * data, uint32_t recv_size) {
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == packet_type_t::DEL_REQ);
}

template<class key_t>
uint32_t DelRequest<key_t>::size() {
	return sizeof(uint8_t) + sizeof(uint16_t) + sizeof(key_t);
}

template<class key_t>
uint32_t DelRequest<key_t>::serialize(char * const data, uint32_t max_size) {
	uint32_t my_size = this->size();
	INVARIANT(max_size >= my_size);
	char *begin = data;
	memcpy(begin, (void *)&this->_type, sizeof(uint8_t));
	begin += sizeof(uint8_t);
	uint16_t bigendian_hashidx = htons(this->_hashidx);
	memcpy(begin, (void *)&bigendian_hashidx, sizeof(uint16_t)); // Small-endian to big-endian
	begin += sizeof(uint16_t);
	memcpy(begin, (void *)&this->_key, sizeof(key_t));
	return my_size;
}

template<class key_t>
void DelRequest<key_t>::deserialize(const char * data, uint32_t recv_size) {
	uint32_t my_size = this->size();
	INVARIANT(my_size == recv_size);
	const char *begin = data;
	memcpy((void *)&this->_type, begin, sizeof(uint8_t));
	begin += sizeof(uint8_t);
	memcpy((void *)&this->_hashidx, begin, sizeof(uint16_t));
	this->_hashidx = ntohs(this->_hashidx); // Big-endian to small-endian
	begin += sizeof(uint16_t);
	memcpy((void *)&this->_key, begin, sizeof(key_t));
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


// GetResponse

template<class key_t, class val_t>
GetResponse<key_t, val_t>::GetResponse()
	: Packet<key_t>(), _val()
{
}

template<class key_t, class val_t>
GetResponse<key_t, val_t>::GetResponse(uint16_t hashidx, key_t key, val_t val) 
	: Packet<key_t>(PacketType::GET_RES, hashidx, key), _val(val)
{	
}

template<class key_t, class val_t>
GetResponse<key_t, val_t>::GetResponse(const char * data, uint32_t recv_size) {
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::GET_RES);
}

template<class key_t, class val_t>
val_t GetResponse<key_t, val_t>::val() const {
	return _val;
}

template<class key_t, class val_t>
uint32_t GetResponse<key_t, val_t>::size() {
	return sizeof(uint8_t) + sizeof(uint16_t) + sizeof(key_t) + sizeof(uint8_t) + val_t::max_bytesnum();
}

template<class key_t, class val_t>
uint32_t GetResponse<key_t, val_t>::serialize(char * const data, uint32_t max_size) {
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
	uint32_t tmpsize = this->_val.serialize(begin);
	return sizeof(uint8_t) + sizeof(uint16_t) + sizeof(key_t) + tmpsize;
}

template<class key_t, class val_t>
void GetResponse<key_t, val_t>::deserialize(const char * data, uint32_t recv_size) {
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
	uint32_t tmpsize = this->_val.deserialize(begin);
}

// PutResponse

template<class key_t>
PutResponse<key_t>::PutResponse(uint16_t hashidx, key_t key, bool stat) 
	: Packet<key_t>(PacketType::PUT_RES, hashidx, key), _stat(stat)
{	
}

template<class key_t>
PutResponse<key_t>::PutResponse(const char * data, uint32_t recv_size) {
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::PUT_RES);
}

template<class key_t>
bool PutResponse<key_t>::stat() const {
	return _stat;
}

template<class key_t>
uint32_t PutResponse<key_t>::size() {
	return sizeof(uint8_t) + sizeof(uint16_t) + sizeof(key_t) + sizeof(bool);
}

template<class key_t>
uint32_t PutResponse<key_t>::serialize(char * const data, uint32_t max_size) {
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
	memcpy(begin, (void *)&this->_stat, sizeof(bool));
	return my_size;
}

template<class key_t>
void PutResponse<key_t>::deserialize(const char * data, uint32_t recv_size) {
	uint32_t my_size = this->size();
	INVARIANT(my_size >= recv_size);
	const char *begin = data;
	memcpy((void *)&this->_type, begin, sizeof(uint8_t));
	begin += sizeof(uint8_t);
	memcpy((void *)&this->_hashidx, begin, sizeof(uint16_t));
	this->_hashidx = ntohs(this->_hashidx); // Big-endian to small-endian
	begin += sizeof(uint16_t);
	memcpy((void *)&this->_key, begin, sizeof(key_t));
	begin += sizeof(key_t);
	memcpy((void *)&this->_stat, begin, sizeof(bool));
}

// DelResponse

template<class key_t>
DelResponse<key_t>::DelResponse(uint16_t hashidx, key_t key, bool stat) 
	: Packet<key_t>(PacketType::DEL_RES, hashidx, key), _stat(stat)
{	
}

template<class key_t>
DelResponse<key_t>::DelResponse(const char * data, uint32_t recv_size) {
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::DEL_RES);
}

template<class key_t>
bool DelResponse<key_t>::stat() const {
	return _stat;
}

template<class key_t>
uint32_t DelResponse<key_t>::size() {
	return sizeof(uint8_t) + sizeof(uint16_t) + sizeof(key_t) + sizeof(bool);
}

template<class key_t>
uint32_t DelResponse<key_t>::serialize(char * const data, uint32_t max_size) {
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
	memcpy(begin, (void *)&this->_stat, sizeof(bool));
	return my_size;
}

template<class key_t>
void DelResponse<key_t>::deserialize(const char * data, uint32_t recv_size) {
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
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::GET_REQ_POP);
}

template<class key_t>
uint32_t GetRequestPOP<key_t>::serialize(char * const data, uint32_t max_size)
{
	COUT_N_EXIT("Invalid invoke of serialize for GetRequestPOP");
}

// GetResponsePOP

template<class key_t, class val_t>
GetResponsePOP<key_t, val_t>::GetResponsePOP(uint16_t hashidx, key_t key, val_t val)
	: GetResponse<key_t, val_t>::GetResponse(hashidx, key, val)
{
	this->_type = static_cast<uint8_t>(PacketType::GET_RES_POP);
}

template<class key_t, class val_t>
void GetResponsePOP<key_t, val_t>::deserialize(const char * data, uint32_t recv_size)
{
	COUT_N_EXIT("Invalid invoke of deserialize for GetResponsePOP");
}

// GetResponseNPOP

template<class key_t, class val_t>
GetResponseNPOP<key_t, val_t>::GetResponseNPOP(uint16_t hashidx, key_t key, val_t val)
	: GetResponse<key_t, val_t>::GetResponse(hashidx, key, val)
{
	this->_type = static_cast<uint8_t>(PacketType::GET_RES_NPOP);
}

template<class key_t, class val_t>
void GetResponseNPOP<key_t, val_t>::deserialize(const char * data, uint32_t recv_size)
{
	COUT_N_EXIT("Invalid invoke of deserialize for GetResponseNPOP");
}

// GetResponsePOPEvict

template<class key_t, class val_t>
GetResponsePOPEvict<key_t, val_t>::GetResponsePOPEvict(const char * data, uint32_t recv_size) {
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::GET_RES_POP_EVICT);
}

template<class key_t, class val_t>
uint32_t GetResponsePOPEvict<key_t, val_t>::serialize(char * const data, uint32_t max_size) {
	COUT_N_EXIT("Invalid invoke of serialize for GetResponsePOPEvict");
}

// PutRequestPOPEvict

template<class key_t, class val_t>
PutRequestPOPEvict<key_t, val_t>::PutRequestPOPEvict(const char * data, uint32_t recv_size) {
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::PUT_REQ_POP_EVICT);
}

template<class key_t, class val_t>
uint32_t PutRequestPOPEvict<key_t, val_t>::serialize(char * const data, uint32_t max_size) {
	COUT_N_EXIT("Invalid invoke of serialize for PutRequestPOPEvict");
}

// PutRequestCase1

template<class key_t, class val_t>
PutRequestCase1<key_t, val_t>::PutRequestCase1(const char * data, uint32_t recv_size) {
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::PUT_REQ_CASE1);
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

// GetResponsePOPEvictCase2

template<class key_t, class val_t>
GetResponsePOPEvictCase2<key_t, val_t>::GetResponsePOPEvictCase2(const char * data, uint32_t recv_size) {
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::GET_RES_POP_EVICT_CASE2);
}

template<class key_t, class val_t>
uint32_t GetResponsePOPEvictCase2<key_t, val_t>::serialize(char * const data, uint32_t max_size) {
	COUT_N_EXIT("Invalid invoke of serialize for GetResponsePOPEvictCase2");
}

// PutRequestPOPEvictCase2

template<class key_t, class val_t>
PutRequestPOPEvictCase2<key_t, val_t>::PutRequestPOPEvictCase2(const char * data, uint32_t recv_size) {
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::PUT_REQ_POP_EVICT_CASE2);
}

template<class key_t, class val_t>
uint32_t PutRequestPOPEvictCase2<key_t, val_t>::serialize(char * const data, uint32_t max_size) {
	COUT_N_EXIT("Invalid invoke of serialize for PutRequestPOPEvictCase2");
}

// PutRequestCase3

template<class key_t, class val_t>
PutRequestCase3<key_t, val_t>::PutRequestCase3(const char * data, uint32_t recv_size) {
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::PUT_REQ_CASE3);
}

template<class key_t, class val_t>
uint32_t PutRequestCase3<key_t, val_t>::serialize(char * const data, uint32_t max_size) {
	COUT_N_EXIT("Invalid invoke of serialize for PutRequestCase3");
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

// APIs
packet_type_t get_packet_type(const char * data, uint32_t recv_size) {
	INVARIANT(recv_size >= sizeof(uint8_t));
	uint8_t tmp;
	memcpy((void *)&tmp, data, sizeof(uint8_t));
	return static_cast<packet_type_t>(tmp);
}
