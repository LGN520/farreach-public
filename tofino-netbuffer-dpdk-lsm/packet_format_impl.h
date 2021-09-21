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
	this->deserialize(data, recv_size);
	INVARIANT(this->_type == packet_type_t::GET_REQ);
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

// PutRequest

template<class key_t, class val_t>
PutRequest<key_t, val_t>::PutRequest()
	: Packet<key_t>(), _val(0)
{
}

template<class key_t, class val_t>
PutRequest<key_t, val_t>::PutRequest(uint32_t thread_id, key_t key, val_t val) 
	: Packet<key_t>(PacketType::PUT_REQ, thread_id, key), _val(val)
{	
}

template<class key_t, class val_t>
PutRequest<key_t, val_t>::PutRequest(const char * data, uint32_t recv_size) {
	this->deserialize(data, recv_size);
	INVARIANT(this->_type == PacketType::PUT_REQ);
}

template<class key_t, class val_t>
val_t PutRequest<key_t, val_t>::val() const {
	return _val;
}

template<class key_t, class val_t>
uint32_t PutRequest<key_t, val_t>::size() {
	return sizeof(packet_type_t) + sizeof(uint32_t) + sizeof(key_t) + sizeof(val_t);
}

template<class key_t, class val_t>
uint32_t PutRequest<key_t, val_t>::serialize(char * const data, uint32_t max_size) {
	uint32_t my_size = this->size();
	INVARIANT(max_size >= my_size);
	char *begin = data;
	memcpy(begin, (void *)&this->_type, sizeof(packet_type_t));
	begin += sizeof(packet_type_t);
	memcpy(begin, (void *)&this->_thread_id, sizeof(uint32_t));
	begin += sizeof(uint32_t);
	memcpy(begin, (void *)&this->_key, sizeof(key_t));
	begin += sizeof(key_t);
	memcpy(begin, (void *)&this->_val, sizeof(val_t));
	return my_size;
}

template<class key_t, class val_t>
void PutRequest<key_t, val_t>::deserialize(const char * data, uint32_t recv_size) {
	uint32_t my_size = this->size();
	INVARIANT(my_size == recv_size);
	const char *begin = data;
	memcpy((void *)&this->_type, begin, sizeof(packet_type_t));
	begin += sizeof(packet_type_t);
	memcpy((void *)&this->_thread_id, begin, sizeof(uint32_t));
	begin += sizeof(uint32_t);
	memcpy((void *)&this->_key, begin, sizeof(key_t));
	begin += sizeof(key_t);
	memcpy((void *)&this->_val, begin, sizeof(val_t));
}

// DelRequest

template<class key_t, class val_t>
DelRequest<key_t, val_t>::DelRequest()
	: Packet<key_t>()
{
}

template<class key_t>
DelRequest<key_t>::DelRequest(uint32_t thread_id, key_t key)
	: Packet<key_t>(packet_type_t::DEL_REQ, thread_id, key)
{
}

template<class key_t>
DelRequest<key_t>::DelRequest(const char * data, uint32_t recv_size) {
	this->deserialize(data, recv_size);
	INVARIANT(this->_type == packet_type_t::DEL_REQ);
}

template<class key_t>
uint32_t DelRequest<key_t>::size() {
	return sizeof(packet_type_t) + sizeof(uint32_t) + sizeof(key_t);
}

template<class key_t>
uint32_t DelRequest<key_t>::serialize(char * const data, uint32_t max_size) {
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
void DelRequest<key_t>::deserialize(const char * data, uint32_t recv_size) {
	uint32_t my_size = this->size();
	INVARIANT(my_size == recv_size);
	const char *begin = data;
	memcpy((void *)&this->_type, begin, sizeof(packet_type_t));
	begin += sizeof(packet_type_t);
	memcpy((void *)&this->_thread_id, begin, sizeof(uint32_t));
	begin += sizeof(uint32_t);
	memcpy((void *)&this->_key, begin, sizeof(key_t));
}

// ScanRequest
template<class key_t>
ScanRequest<key_t>::ScanRequest(uint32_t thread_id, key_t key, uint32_t num)
	: Packet<key_t>(packet_type_t::SCAN_REQ, thread_id, key), _num(num)
{
}

template<class key_t>
ScanRequest<key_t>::ScanRequest(const char * data, uint32_t recv_size) {
	this->deserialize(data, recv_size);
	INVARIANT(this->_type == packet_type_t::SCAN_REQ);
}

template<class key_t>
uint32_t ScanRequest<key_t>::num() const {
	return this->_num;
}

template<class key_t>
uint32_t ScanRequest<key_t>::size() {
	return sizeof(packet_type_t) + sizeof(uint32_t) + sizeof(key_t) + sizeof(uint32_t);
}

template<class key_t>
uint32_t ScanRequest<key_t>::serialize(char * const data, uint32_t max_size) {
	uint32_t my_size = this->size();
	INVARIANT(max_size >= my_size);
	char *begin = data;
	memcpy(begin, (void *)&this->_type, sizeof(packet_type_t));
	begin += sizeof(packet_type_t);
	memcpy(begin, (void *)&this->_thread_id, sizeof(uint32_t));
	begin += sizeof(uint32_t);
	memcpy(begin, (void *)&this->_key, sizeof(key_t));
	begin += sizeof(key_t);
	memcpy(begin, (void *)&this->_num, sizeof(uint32_t));
	return my_size;
}

template<class key_t>
void ScanRequest<key_t>::deserialize(const char * data, uint32_t recv_size) {
	uint32_t my_size = this->size();
	INVARIANT(my_size == recv_size);
	const char *begin = data;
	memcpy((void *)&this->_type, begin, sizeof(packet_type_t));
	begin += sizeof(packet_type_t);
	memcpy((void *)&this->_thread_id, begin, sizeof(uint32_t));
	begin += sizeof(uint32_t);
	memcpy((void *)&this->_key, begin, sizeof(key_t));
	begin += sizeof(key_t);
	memcpy((void *)&this->_num, begin, sizeof(uint32_t));
}


// GetResponse

template<class key_t, class val_t>
GetResponse<key_t, val_t>::GetResponse(uint32_t thread_id, key_t key, val_t val) 
	: Packet<key_t>(PacketType::GET_RES, thread_id, key), _val(val)
{	
}

template<class key_t, class val_t>
GetResponse<key_t, val_t>::GetResponse(const char * data, uint32_t recv_size) {
	this->deserialize(data, recv_size);
	INVARIANT(this->_type == PacketType::GET_RES);
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
	begin += sizeof(key_t);
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
	begin += sizeof(key_t);
	memcpy((void *)&this->_val, begin, sizeof(val_t));
}

// PutResponse

template<class key_t>
PutResponse<key_t>::PutResponse(uint32_t thread_id, key_t key, bool stat) 
	: Packet<key_t>(PacketType::PUT_RES, thread_id, key), _stat(stat)
{	
}

template<class key_t>
PutResponse<key_t>::PutResponse(const char * data, uint32_t recv_size) {
	this->deserialize(data, recv_size);
	INVARIANT(this->_type == PacketType::PUT_RES);
}

template<class key_t>
bool PutResponse<key_t>::stat() const {
	return _stat;
}

template<class key_t>
uint32_t PutResponse<key_t>::size() {
	return sizeof(packet_type_t) + sizeof(uint32_t) + sizeof(key_t) + sizeof(bool);
}

template<class key_t>
uint32_t PutResponse<key_t>::serialize(char * const data, uint32_t max_size) {
	uint32_t my_size = this->size();
	INVARIANT(max_size >= my_size);
	char *begin = data;
	memcpy(begin, (void *)&this->_type, sizeof(packet_type_t));
	begin += sizeof(packet_type_t);
	memcpy(begin, (void *)&this->_thread_id, sizeof(uint32_t));
	begin += sizeof(uint32_t);
	memcpy(begin, (void *)&this->_key, sizeof(key_t));
	begin += sizeof(key_t);
	memcpy(begin, (void *)&this->_stat, sizeof(bool));
	return my_size;
}

template<class key_t>
void PutResponse<key_t>::deserialize(const char * data, uint32_t recv_size) {
	uint32_t my_size = this->size();
	INVARIANT(my_size == recv_size);
	const char *begin = data;
	memcpy((void *)&this->_type, begin, sizeof(packet_type_t));
	begin += sizeof(packet_type_t);
	memcpy((void *)&this->_thread_id, begin, sizeof(uint32_t));
	begin += sizeof(uint32_t);
	memcpy((void *)&this->_key, begin, sizeof(key_t));
	begin += sizeof(key_t);
	memcpy((void *)&this->_stat, begin, sizeof(bool));
}

// DelResponse

template<class key_t>
DelResponse<key_t>::DelResponse(uint32_t thread_id, key_t key, bool stat) 
	: Packet<key_t>(PacketType::DEL_RES, thread_id, key), _stat(stat)
{	
}

template<class key_t>
DelResponse<key_t>::DelResponse(const char * data, uint32_t recv_size) {
	this->deserialize(data, recv_size);
	INVARIANT(this->_type == PacketType::DEL_RES);
}

template<class key_t>
bool DelResponse<key_t>::stat() const {
	return _stat;
}

template<class key_t>
uint32_t DelResponse<key_t>::size() {
	return sizeof(packet_type_t) + sizeof(uint32_t) + sizeof(key_t) + sizeof(bool);
}

template<class key_t>
uint32_t DelResponse<key_t>::serialize(char * const data, uint32_t max_size) {
	uint32_t my_size = this->size();
	INVARIANT(max_size >= my_size);
	char *begin = data;
	memcpy(begin, (void *)&this->_type, sizeof(packet_type_t));
	begin += sizeof(packet_type_t);
	memcpy(begin, (void *)&this->_thread_id, sizeof(uint32_t));
	begin += sizeof(uint32_t);
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
	memcpy((void *)&this->_type, begin, sizeof(packet_type_t));
	begin += sizeof(packet_type_t);
	memcpy((void *)&this->_thread_id, begin, sizeof(uint32_t));
	begin += sizeof(uint32_t);
	memcpy((void *)&this->_key, begin, sizeof(key_t));
	begin += sizeof(key_t);
	memcpy((void *)&this->_stat, begin, sizeof(bool));
}

// ScanResponse

template<class key_t, class val_t>
ScanResponse<key_t, val_t>::ScanResponse(uint32_t thread_id, key_t key, uint32_t num, std::vector<std::pair<key_t, val_t>> pairs) 
	: Packet<key_t>(PacketType::SCAN_RES, thread_id, key), _num(num)
{	
	INVARIANT(pairs.size() == num);
	this->_pairs.assign(pairs.begin(), pairs.end());
}

template<class key_t, class val_t>
ScanResponse<key_t, val_t>::ScanResponse(const char * data, uint32_t recv_size) {
	this->deserialize(data, recv_size);
	INVARIANT(this->_type == PacketType::SCAN_RES);
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
	return sizeof(packet_type_t) + sizeof(uint32_t) + sizeof(key_t) + sizeof(uint32_t) + this->_num*(sizeof(key_t) + sizeof(val_t));
}

template<class key_t, class val_t>
uint32_t ScanResponse<key_t, val_t>::serialize(char * const data, uint32_t max_size) {
	uint32_t my_size = this->size();
	INVARIANT(max_size >= my_size);
	char *begin = data;
	memcpy(begin, (void *)&this->_type, sizeof(packet_type_t));
	begin += sizeof(packet_type_t);
	memcpy(begin, (void *)&this->_thread_id, sizeof(uint32_t));
	begin += sizeof(uint32_t);
	memcpy(begin, (void *)&this->_key, sizeof(key_t));
	begin += sizeof(key_t);
	memcpy(begin, (void *)&this->_num, sizeof(val_t));
	begin += sizeof(uint32_t);
	for (uint32_t pair_i = 0; pair_i < this->_pairs.size(); pair_i++) {
		memcpy(begin, (void *)&this->_pairs[pair_i].first, sizeof(key_t));
		begin += sizeof(key_t);
		memcpy(begin, (void *)&this->_pairs[pair_i].second, sizeof(val_t));
		if (pair_i != this->_pairs.size() - 1) {
			begin += sizeof(val_t);
		}
	}
	return my_size;
}

template<class key_t, class val_t>
void ScanResponse<key_t, val_t>::deserialize(const char * data, uint32_t recv_size) {
	INVARIANT(recv_size >= (sizeof(packet_type_t) + sizeof(uint32_t) + sizeof(key_t) + sizeof(uint32_t)));
	const char *begin = data;
	memcpy((void *)&this->_type, begin, sizeof(packet_type_t));
	begin += sizeof(packet_type_t);
	memcpy((void *)&this->_thread_id, begin, sizeof(uint32_t));
	begin += sizeof(uint32_t);
	memcpy((void *)&this->_key, begin, sizeof(key_t));
	begin += sizeof(key_t);
	memcpy((void *)&this->_num, begin, sizeof(uint32_t));
	begin += sizeof(uint32_t);

	uint32_t my_size = this->size();
	INVARIANT(recv_size == my_size);
	key_t tmp_key;
	val_t tmp_val;
	this->_pairs.reserve(this->_num);
	for (uint32_t pair_i = 0; pair_i < this->_num; pair_i++) {
		memcpy((void *)&tmp_key, begin, sizeof(key_t));
		begin += sizeof(key_t);
		memcpy((void *)&tmp_val, begin, sizeof(val_t));
		this->_pairs.push_back(std::pair<key_t, val_t>(tmp_key, tmp_val));
		if (pair_i != uint32_t(this->_num - 1)) {
			begin += sizeof(val_t);
		}
	}
}

// PutRequestS

template<class key_t, class val_t>
PutRequestS<key_t, val_t>::PutRequestS(uint32_t thread_id, key_t key, val_t val) 
	: PutRequest<key_t, val_t>::PutRequest(thread_id, key, val)
{	
	this->type = PacketType::PUT_REQ_S;
}

template<class key_t, class val_t>
PutRequestS<key_t, val_t>::PutRequestS(const char * data, uint32_t recv_size) {
	this->deserialize(data, recv_size);
	INVARIANT(this->_type == PacketType::PUT_REQ_S);
}

template<class key_t, class val_t>
uint32_t PutRequestS<key_t, val_t>::serialize(char * const data, uint32_t max_size) {
	COUT_N_EXIT("Invalid invoke of serialize for PutReqestS");
}

// DelRequestS

template<class key_t, class val_t>
DelRequestS<key_t, val_t>::DelRequestS(uint32_t thread_id, key_t key) 
	: DelRequest<key_t, val_t>::DelRequest(thread_id, key)
{	
	this->type = PacketType::DEL_REQ_S;
}

template<class key_t, class val_t>
DelRequestS<key_t, val_t>::DelRequestS(const char * data, uint32_t recv_size) {
	this->deserialize(data, recv_size);
	INVARIANT(this->_type == PacketType::DEL_REQ_S);
}

template<class key_t, class val_t>
uint32_t DelRequestS<key_t, val_t>::serialize(char * const data, uint32_t max_size) {
	COUT_N_EXIT("Invalid invoke of serialize for DelReqestS");
}

// APIs
packet_type_t get_packet_type(const char * data, uint32_t recv_size) {
	INVARIANT(recv_size >= sizeof(packet_type_t));
	packet_type_t tmp;
	memcpy((void *)&tmp, data, sizeof(packet_type_t));
	return tmp;
}
