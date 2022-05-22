#include <string.h>
#include <string>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "parser.h"
//#include "utf8.h"

#define MINIMUM_WORKLOAD_FILESIZE

#define MAX_LINE_SIZE 256

int ParserIterator::load_batch_size = 1 * 1000 * 1000; // 1M records per batch

ParserIterator::ParserIterator(const char* filename) {
	_fd = open(filename, O_RDONLY);
	if (_fd == -1) {
		printf("No such file: %s\n", filename);
		exit(-1);
	}

	struct stat statbuf;
	int fstat_res = fstat(_fd, &statbuf);
	if (fstat_res < 0) {
		printf("Cannot get stat of %s\n", filename);
		exit(-1);
	}
	_filesize = statbuf.st_size;

	_keys = new Key[load_batch_size];
	_vals = new Val[load_batch_size];
	_types = new uint8_t[load_batch_size];
	_lines = new std::string[load_batch_size];

	/*bool next_res = next();
	if (!next_res) {
		printf("Empty file: %s\n", filename);
		exit(-1);
	}*/
}

ParserIterator::ParserIterator(const ParserIterator& other) {
	printf("[ParserIterator] no copy constructor\n");
	exit(-1);
}

ParserIterator::~ParserIterator() {
	unmap_content();
	closeiter();

	if (_keys != NULL) {
		delete [] _keys;
		_keys = NULL;
	}
	if (_vals != NULL) {
		delete [] _vals;
		_vals = NULL;
	}
	if (_types != NULL) {
		delete [] _types;
		_types = NULL;
	}
	if (_lines != NULL) {
		delete [] _lines;
		_lines = NULL;
	}
}

Key ParserIterator::key() {
	INVARIANT(_idx >= 0 && _idx <= _maxidx);
	return _keys[_idx];
}

Val ParserIterator::val() {
	INVARIANT(_idx >= 0 && _idx <= _maxidx);
	return _vals[_idx];
}

uint8_t ParserIterator::type() {
	INVARIANT(_idx >= 0 && _idx <= _maxidx);
	return _types[_idx];
}

std::string ParserIterator::line() {
	INVARIANT(_idx >= 0 && _idx <= _maxidx);
	return _lines[_idx];
}

bool ParserIterator::next() {
	if (_idx == -1 || _idx == _maxidx) {
		return next_batch(); // set _idx = 0; update _maxidx;
	}
	else {
		_idx += 1;
	}

	INVARIANT(_idx >= 0 && _idx <= _maxidx);
	return true;
}

void ParserIterator::closeiter() {
	if (_fd != -1) {
		close(_fd);
		_fd = -1;
	}
}

Key *ParserIterator::keys() {
	INVARIANT(_keys != NULL);
	return _keys;
}

Val *ParserIterator::vals() {
	INVARIANT(_vals != NULL);
	return _vals;
}

uint8_t *ParserIterator::types() {
	INVARIANT(_types != NULL);
	return _types;
}

int ParserIterator::maxidx() {
	INVARIANT(_maxidx != -1);
	return _maxidx;
}

bool ParserIterator::next_batch() {
	INVARIANT(_content == NULL);

	if (_maxidx < load_batch_size-1) {
		return false;
	}

	map_content(); // map file into content

	// load at most load_batch_size records
	_idx = 0;
	_maxidx = -1;

	if (_content == NULL) {
		return false;
	}

	char *line = _content + _fileoffset;
	while (true) {
		// NOTE: we cannot use strlen, which count # of chars until '\0'
		char *nextline = strchr(line, '\n');
		if (nextline == NULL) {
			nextline = _content + _filesize;
		}
		else {
			nextline += 1;
		}
		int linelen = nextline - line;

		if (strncmp(line, "INSERT", 6) == 0 || strncmp(line, "UPDATE", 6) == 0) {
			INVARIANT(_types != NULL);
			_types[_maxidx+1] = int8_t(packet_type_t::PUTREQ);
			if (!parsekv(line, linelen)) {
				printf("No KV after INSERT: %s\n", line);
				exit(-1);
			}
			_maxidx += 1;
		}
		else if (strncmp(line, "READ", 4) == 0) {
			_types[_maxidx+1] = int8_t(packet_type_t::GETREQ);
			if (!parsekey(line, linelen)) {
				printf("No key after READ: %s\n", line);
				exit(-1);
			}
			_maxidx += 1;
		}
		else if (strncmp(line, "SCAN", 4) == 0) {
			_types[_maxidx+1] = int8_t(packet_type_t::SCANREQ);
			if (!parsekey(line, linelen)) {
				printf("No key after SCAN: %s\n", line);
				exit(-1);
			}
			_maxidx += 1;
		}
		else if (strncmp(line, "DELETE", 6) == 0) {
			_types[_maxidx+1] = int8_t(packet_type_t::DELREQ);
			if (!parsekey(line, linelen)) {
				printf("No key after DELETE: %s\n", line);
				exit(-1);
			}
			_maxidx += 1;
		}

		// switch to next line
		_fileoffset += linelen;
		//COUT_VAR(std::string(line, linelen));
		//printf("linelen: %d, _fileoffset: %d, _filesize: %d, _maxidx: %d, load_batch_size: %d\n", linelen, _fileoffset, _filesize, _maxidx, load_batch_size);
		line = nextline;

		INVARIANT(_maxidx < load_batch_size);
		if (_maxidx == load_batch_size-1) {
			break;
		}

		if (_fileoffset >= _filesize) {
			break;
		}
	}

	unmap_content(); // umap to free allocated memory

	if (_maxidx == -1) {
		return false;
	}
	else {
		return true;
	}
}

void ParserIterator::unmap_content() {
	if (_content != NULL) {
		int res = munmap(_content, _filesize);
		if (res != 0) {
			printf("munmap fails: %d\n", res);
			exit(-1);
		}
		_content = NULL;
	}
}

void ParserIterator::map_content() {
	INVARIANT(_content == NULL);
	if (_fileoffset < _filesize) {
		_content = (char *)(mmap(NULL, _filesize, PROT_READ, MAP_SHARED, _fd, 0)); // NOTE: the last argument must be page-size-aligned
		if (_content == MAP_FAILED) {
			printf("mmap fails: errno = %d\n", errno);
			exit(-1);
		}
		INVARIANT(_content != NULL);
	}
}

bool ParserIterator::parsekv(const char* line, int linelen) {
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
	uint64_t tmpkey = std::stoull(keystr);
	uint32_t keyhilo = 0;
	uint32_t keyhihi = 0;
	memcpy((void *)&keyhilo, (void *)&tmpkey, sizeof(uint32_t)); // lowest 4B -> keyhilo
	memcpy((void *)&keyhihi, ((char *)&tmpkey)+4, sizeof(uint32_t)); // highest 4B -> keyhihi
	_keys[_maxidx+1] = Key(0, 0, keyhilo, keyhihi);

#ifdef MINIMUM_WORKLOAD_FILESIZE
	char val_buf[Val::SWITCH_MAX_VALLEN];
	memset(val_buf, 0x11, Val::SWITCH_MAX_VALLEN);
	_vals[_maxidx+1] = Val(val_buf, Val::SWITCH_MAX_VALLEN);
#else
	val_begin = strstr(line, "[ field0=");
	if (unlikely(val_begin == nullptr)) return false;
	val_begin += 9; // Skip "[ field0="
	val_end = line + linelen - 3; // The last substr is " ]\n"
	if (unlikely(strncmp(val_end, " ]\n", 3) != 0)) return false;
	if (unlikely(val_begin >= val_end)) return false;
	uint32_t val_len = uint32_t(val_end - val_begin); // # of bytes
	char val_buf[val_len];
	memset(val_buf, '\0', val_len);
	memcpy(val_buf, val_begin, val_end - val_begin);
	_vals[_maxidx+1] = Val(val_buf, val_len);
#endif

	_lines[_maxidx+1] = std::string(line, linelen);
	return true;
}

bool ParserIterator::parsekey(const char* line, int linelen) {
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
	uint64_t tmpkey = std::stoull(keystr);
	uint32_t keyhilo = 0;
	uint32_t keyhihi = 0;
	memcpy((void *)&keyhilo, (void *)&tmpkey, sizeof(uint32_t)); // lowest 4B -> keyhilo
	memcpy((void *)&keyhihi, ((char *)&tmpkey)+4, sizeof(uint32_t)); // highest 4B -> keyhihi
	_keys[_maxidx+1] = Key(0, 0, keyhilo, keyhihi);

	_vals[_maxidx+1] = Val();
	_lines[_maxidx+1] = std::string(line, linelen);
	return true;
}
