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
	: Packet<key_t>(), _val(), _seq(0), _is_assigned(0)
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
uint32_t PutRequest<key_t, val_t>::seq() const {
	return _seq;
}

template<class key_t, class val_t>
uint8_t PutRequest<key_t, val_t>::is_assigned() const {
	return _is_assigned;
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
	begin += tmpsize;
	memcpy(begin, (void *)&this->_seq, sizeof(uint32_t));
	begin += sizeof(uint32_t);
	memcpy(begin, (void *)&this->_is_assigned, sizeof(uint8_t));
	return sizeof(uint8_t) + sizeof(uint16_t) + sizeof(key_t) + tmpsize + sizeof(uint32_t) + sizeof(uint8_t);
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
	begin += sizeof(uint8_t);
	memcpy((void *)&this->_key, begin, sizeof(key_t));
	begin += sizeof(key_t);
	uint32_t tmpsize = this->_val.deserialize(begin);
	begin += tmpsize;
	memcpy((void *)&this->_seq, begin, sizeof(uint32_t));
	begin += sizeof(uint32_t);
	memcpy((void *)&this->_is_assigned, begin, sizeof(uint8_t));
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
	: Packet<key_t>(), _val(), _seq(0), _is_assigned(0)
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
	begin += tmpsize;
	memcpy(begin, (void *)&this->_seq, sizeof(uint32_t));
	begin += sizeof(uint32_t);
	memcpy(begin, (void *)&this->_is_assigned, sizeof(uint8_t));
	return sizeof(uint8_t) + sizeof(uint16_t) + sizeof(key_t) + tmpsize + sizeof(uint32_t) + sizeof(uint8_t);
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
	begin += tmpsize;
	memcpy((void *)&this->_seq, begin, sizeof(uint32_t));
	begin += sizeof(uint32_t);
	memcpy((void *)&this->_is_assigned, begin, sizeof(uint8_t));
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

// GetRequestS

template<class key_t>
GetRequestS<key_t>::GetRequestS(uint8_t thread_id, key_t key)
	: GetRequest<key_t>::GetRequest(thread_id, key)
{
	this->_type = static_cast<uint8_t>(PacketType::GET_REQ_S);
}

template<class key_t>
GetRequestS<key_t>::GetRequestS(const char *data, uint32_t recv_size)
{
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::GET_REQ_S);
}

template<class key_t>
uint32_t GetRequestS<key_t>::serialize(char * const data, uint32_t max_size)
{
	COUT_N_EXIT("Invalid invoke of serialize for GetReqestS");
}

// DelRequestS

template<class key_t>
DelRequestS<key_t>::DelRequestS(uint8_t thread_id, key_t key) 
	: DelRequest<key_t>::DelRequest(thread_id, key)
{	
	this->_type = static_cast<uint8_t>(PacketType::DEL_REQ_S);
}

template<class key_t>
DelRequestS<key_t>::DelRequestS(const char * data, uint32_t recv_size) {
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::DEL_REQ_S);
}

template<class key_t>
uint32_t DelRequestS<key_t>::serialize(char * const data, uint32_t max_size) {
	COUT_N_EXIT("Invalid invoke of serialize for DelReqestS");
}

// GetResponseS

template<class key_t, class val_t>
GetResponseS<key_t, val_t>::GetResponseS(uint8_t thread_id, key_t key, val_t val)
	: GetResponse<key_t, val_t>(thread_id, key, val)
{
	this->_type = static_cast<uint8_t>(PacketType::GET_RES_S);
}

template<class key_t, class val_t>
GetResponseS<key_t, val_t>::GetResponseS(const char *data, uint32_t max_size) {
	COUT_N_EXIT("Invalid invoke of GetResponseS(char * const data, uint32_t max_size)");
}

template<class key_t, class val_t>
void GetResponseS<key_t, val_t>::deserialize(const char *data, uint32_t recv_size) {
	COUT_N_EXIT("Invalid invoke of deserialize of GetResponseS");
}

// GetResponseNS

template<class key_t, class val_t>
GetResponseNS<key_t, val_t>::GetResponseNS(uint8_t thread_id, key_t key, val_t val)
	: GetResponse<key_t, val_t>(thread_id, key, val)
{
	this->_type = static_cast<uint8_t>(PacketType::GET_RES_NS);
}

template<class key_t, class val_t>
GetResponseNS<key_t, val_t>::GetResponseNS(const char * data, uint32_t max_size) {
	COUT_N_EXIT("Invalid invoke of GetResponseNS(char * const data, uint32_t max_size)");
}

template<class key_t, class val_t>
void GetResponseNS<key_t, val_t>::deserialize(const char *data, uint32_t recv_size) {
	COUT_N_EXIT("Invalid invoke of deserialize of GetResponseNS");
}

// PutRequestGS

template<class key_t, class val_t>
PutRequestGS<key_t, val_t>::PutRequestGS(uint8_t thread_id, key_t key, val_t val) 
	: PutRequest<key_t, val_t>::PutRequest(thread_id, key, val)
{	
	this->_type = static_cast<uint8_t>(PacketType::PUT_REQ_GS);
}

template<class key_t, class val_t>
PutRequestGS<key_t, val_t>::PutRequestGS(const char * data, uint32_t recv_size) {
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::PUT_REQ_GS);
}

template<class key_t, class val_t>
uint32_t PutRequestGS<key_t, val_t>::serialize(char * const data, uint32_t max_size) {
	COUT_N_EXIT("Invalid invoke of serialize for PutReqestGS");
}

// PutRequestPS

template<class key_t, class val_t>
PutRequestPS<key_t, val_t>::PutRequestPS(uint8_t thread_id, key_t key, val_t val) 
	: PutRequest<key_t, val_t>::PutRequest(thread_id, key, val)
{	
	this->_type = static_cast<uint8_t>(PacketType::PUT_REQ_PS);
}

template<class key_t, class val_t>
PutRequestPS<key_t, val_t>::PutRequestPS(const char * data, uint32_t recv_size) {
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::PUT_REQ_PS);
}

template<class key_t, class val_t>
uint32_t PutRequestPS<key_t, val_t>::serialize(char * const data, uint32_t max_size) {
	COUT_N_EXIT("Invalid invoke of serialize for PutReqestPS");
}

// PutRequestCase1

template<class key_t, class val_t>
PutRequestCase1<key_t, val_t>::PutRequestCase1(uint8_t thread_id, key_t key, val_t val) 
	: PutRequest<key_t, val_t>::PutRequest(thread_id, key, val)
{	
	this->_type = static_cast<uint8_t>(PacketType::PUT_REQ_CASE1);
}

template<class key_t, class val_t>
PutRequestCase1<key_t, val_t>::PutRequestCase1(const char * data, uint32_t recv_size) {
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::PUT_REQ_CASE1);
}

template<class key_t, class val_t>
uint32_t PutRequestCase1<key_t, val_t>::serialize(char * const data, uint32_t max_size) {
	COUT_N_EXIT("Invalid invoke of serialize for PutReqestCase1");
}

// DelRequestCase1

template<class key_t, class val_t>
DelRequestCase1<key_t, val_t>::DelRequestCase1(uint8_t thread_id, key_t key, val_t val) 
	: PutRequest<key_t, val_t>::PutRequest(thread_id, key, val)
{	
	this->_type = static_cast<uint8_t>(PacketType::DEL_REQ_CASE1);
}

template<class key_t, class val_t>
DelRequestCase1<key_t, val_t>::DelRequestCase1(const char * data, uint32_t recv_size) {
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::DEL_REQ_CASE1);
}

template<class key_t, class val_t>
uint32_t DelRequestCase1<key_t, val_t>::serialize(char * const data, uint32_t max_size) {
	COUT_N_EXIT("Invalid invoke of serialize for DelRequestCase1");
}

// PutRequestGSCase2

template<class key_t, class val_t>
PutRequestGSCase2<key_t, val_t>::PutRequestGSCase2(uint8_t thread_id, key_t key, val_t val) 
	: PutRequest<key_t, val_t>::PutRequest(thread_id, key, val)
{	
	this->_type = static_cast<uint8_t>(PacketType::PUT_REQ_GS_CASE2);
}

template<class key_t, class val_t>
PutRequestGSCase2<key_t, val_t>::PutRequestGSCase2(const char * data, uint32_t recv_size) {
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::PUT_REQ_GS_CASE2);
}

template<class key_t, class val_t>
uint32_t PutRequestGSCase2<key_t, val_t>::serialize(char * const data, uint32_t max_size) {
	COUT_N_EXIT("Invalid invoke of serialize for PutRequestGSCase2");
}

// PutRequestPSCase2

template<class key_t, class val_t>
PutRequestPSCase2<key_t, val_t>::PutRequestPSCase2(uint8_t thread_id, key_t key, val_t val) 
	: PutRequest<key_t, val_t>::PutRequest(thread_id, key, val)
{	
	this->_type = static_cast<uint8_t>(PacketType::PUT_REQ_PS_CASE2);
}

template<class key_t, class val_t>
PutRequestPSCase2<key_t, val_t>::PutRequestPSCase2(const char * data, uint32_t recv_size) {
	this->deserialize(data, recv_size);
	INVARIANT(static_cast<packet_type_t>(this->_type) == PacketType::PUT_REQ_PS_CASE2);
}

template<class key_t, class val_t>
uint32_t PutRequestPSCase2<key_t, val_t>::serialize(char * const data, uint32_t max_size) {
	COUT_N_EXIT("Invalid invoke of serialize for PutRequestPSCase2");
}

// PutRequestCase3

template<class key_t, class val_t>
PutRequestCase3<key_t, val_t>::PutRequestCase3(uint8_t thread_id, key_t key, val_t val) 
	: PutRequest<key_t, val_t>::PutRequest(thread_id, key, val)
{	
	this->_type = static_cast<uint8_t>(PacketType::PUT_REQ_CASE3);
}

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
DelRequestCase3<key_t>::DelRequestCase3(uint8_t thread_id, key_t key) 
	: DelRequest<key_t>::DelRequest(thread_id, key)
{	
	this->_type = static_cast<uint8_t>(PacketType::DEL_REQ_CASE3);
}

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
