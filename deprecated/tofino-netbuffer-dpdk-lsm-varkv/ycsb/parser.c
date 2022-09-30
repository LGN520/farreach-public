#include <string.h>
#include <string>

#include "parser.h"
//#include "utf8.h"

#define VAL_STRLEN 96

ParserIterator::ParserIterator(const char* filename) {
	fp = fopen(filename, "r");
	if (fp == nullptr) {
		printf("No such file: %s\n", filename);
		exit(-1);
	}
	bool res = next();
	if (!res) {
		printf("Empty file: %s\n", filename);
		exit(-1);
	}
}

ParserIterator::ParserIterator(const ParserIterator& other) {
	_key = other._key;
	_val = other._val;
	_type = other._type;
	_line = other._line;
	fp = other.fp;
}

ParserIterator::~ParserIterator() {
	if (fp != nullptr) {
		fclose(fp);
		fp = nullptr;
	}
}

Key ParserIterator::key() {
	return _key;
}

Val ParserIterator::val() {
	return _val;
}

uint8_t ParserIterator::type() {
	return _type;
}

std::string ParserIterator::line() {
	return _line;
}

bool ParserIterator::next() {
	char *line = nullptr;	
	size_t len = 0;
	ssize_t read_status = 0;
	bool result = false;
	/* 
	 * NOTE: after the first run of getline, line is not nullptr (with the size of len);
	 * If len is large enough, line will not be reallocated and len will also not be updated.
	 * NOTE: len is the length of allocated memory for line (minimum is 120), which is not
	 * the length of line string.
	 */
	while ((read_status = getline(&line, &len, fp)) != -1) {
		INVARIANT(strlen(line) <= len);
		if (strncmp(line, "INSERT", 6) == 0 || strncmp(line, "UPDATE", 6) == 0) {
			_type = uint8_t(packet_type_t::PUT_REQ);
			if (!parsekv(line)) {
				printf("No KV after INSERT: %s\n", line);
				exit(-1);
			}
			result = true;
			break;
		}
		else if (strncmp(line, "READ", 4) == 0) {
			_type = uint8_t(packet_type_t::GET_REQ);
			if (!parsekey(line)) {
				printf("No key after READ: %s\n", line);
				exit(-1);
			}
			result = true;
			break;
		}
		else if (strncmp(line, "SCAN", 4) == 0) {
			_type = uint8_t(packet_type_t::SCAN_REQ);
			if (!parsekey(line)) {
				printf("No key after SCAN: %s\n", line);
				exit(-1);
			}
			result = true;
			break;
		}
	}


	if (line != nullptr) {
		delete line;
		line = nullptr;
	}
	return result;
}

void ParserIterator::close() {
	if (fp != nullptr) {
		fclose(fp);
		fp = nullptr;
	}
}

bool ParserIterator::parsekv(const char* line) {
	const char* key_begin = nullptr;
	const char* key_end = nullptr;
	const char* val_begin = nullptr;
	const char* val_end = nullptr;

	key_begin = strstr(line, "user");
	if (unlikely(key_begin == nullptr)) return false;
	key_begin += 4; // Skip the first usertable
	key_begin = strstr(key_begin, "user");
	if (unlikely(key_begin == nullptr)) return false;
	key_begin += 4; // At the first character of key
	key_end = strchr(key_begin, ' '); // At the end of key
	if (unlikely(key_end == nullptr)) return false;
	std::string keystr(key_begin, key_end - key_begin);
	uint64_t keylo = std::stoull(keystr);
	uint64_t keyhi = 0;
	_key = Key(keylo, keyhi);

	val_begin = strstr(line, "[ field0=");
	if (unlikely(val_begin == nullptr)) return false;
	val_begin += 9; // Skip "[ field0="
	val_end = line + strlen(line) - 3; // The last substr is " ]\n"
	if (unlikely(strncmp(val_end, " ]\n", 3) != 0)) return false;
	if (unlikely(val_begin >= val_end)) return false;
	uint8_t val_len = uint8_t(((val_end - val_begin) + 7) / 8);
	char val_buf[val_len * 8];
	memset(val_buf, '\0', val_len * 8);
	memcpy(val_buf, val_begin, VAL_STRLEN);
	_val = Val((uint64_t*)val_buf, val_len);

	_line = std::string(line, strlen(line));
	return true;
}

bool ParserIterator::parsekey(const char* line) {
	const char* key_begin = nullptr;
	const char* key_end = nullptr;

	key_begin = strstr(line, "user");
	if (unlikely(key_begin == nullptr)) return false;
	key_begin += 4; // Skip the first usertable
	key_begin = strstr(key_begin, "user");
	if (unlikely(key_begin == nullptr)) return false;
	key_begin += 4; // At the first character of key
	key_end = strchr(key_begin, ' '); // At the end of key
	if (unlikely(key_end == nullptr)) return false;
	std::string keystr(key_begin, key_end - key_begin);
	uint64_t keylo = std::stoull(keystr);
	uint64_t keyhi = 0;
	_key = Key(keylo, keyhi);

	_val = Val();
	_line = std::string(line, strlen(line));
	return true;
}

Parser::Parser(const char* filename) {
	if (strlen(filename) >= 256) {
		printf("Filename is too long: %s\n", filename);
		exit(-1);
	}
	memcpy(this->filename, filename, strlen(filename));
	this->filename[strlen(filename)] = '\0';
}

ParserIterator Parser::begin() {
	ParserIterator iter(filename);
	return iter;
}
