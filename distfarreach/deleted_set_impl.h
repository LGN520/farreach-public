#ifndef DELETED_SET_IMPL_H
#define DELETED_SET_IMPL_H

#include <fcntl.h> // open
#include <sys/mman.h> // mmap, unmap

#include "deleted_set.h"

template<class key_t, class seq_t>
DeletedSet<key_t, seq_t>::DeletedSet(uint32_t c, uint32_t rtt) {
	// Conservative estimation: constant factor * thpt c opus
	max_size = CAPACITY_FACTOR * c * rtt;
}

template<class key_t, class seq_t>
DeletedSet<key_t, seq_t>& DeletedSet<key_t, seq_t>::operator=(const DeletedSet& other) {
	records_sorted_bykey = other.records_sorted_bykey;
	records_sorted_byseq = other.records_sorted_byseq;
	return *this;
}

template<class key_t, class seq_t>
int DeletedSet<key_t, seq_t>::size() const {
	INVARIANT(records_sorted_bykey.size() == records_sorted_byseq.size());
	return records_sorted_bykey.size();
}

template<class key_t, class seq_t>
void DeletedSet<key_t, seq_t>::add(key_t key, seq_t seq) {
	/*while (true) {
		if (mutex_lock.try_lock()) break;
	}*/

	std::pair<iterator_bykey_t, bool> res = records_sorted_bykey.insert(std::pair<key_t, seq_t>(key, seq));
	if (res.second == true) { // successful insertion
		std::pair<iterator_byseq_t, bool> tmpres = records_sorted_byseq.insert(std::pair<seq_t, key_t>(seq, key));
		UNUSED(tmpres);
		//INVARIANT(tmpres.second == true); // uncomment for debug (many evcited data w/ seq=0 during debug)
		if (records_sorted_bykey.size() > max_size) { // For memory efficiency
			iterator_byseq_t minseq_iter = records_sorted_byseq.begin();
			key_t minseq_key = minseq_iter->second;
			records_sorted_byseq.erase(minseq_iter);
			INVARIANT(records_sorted_bykey.find(minseq_key) != records_sorted_bykey.end());
			records_sorted_bykey.erase(minseq_key);
		}
	}
	else { // key already exists
		seq_t oldseq = res.first->second;
		if (likely(oldseq < seq)) {
			res.first->second = seq;
			INVARIANT(records_sorted_byseq.find(oldseq) != records_sorted_byseq.end());
			records_sorted_byseq.erase(oldseq);
			std::pair<iterator_byseq_t, bool> tmpres = records_sorted_byseq.insert(std::pair<seq_t, key_t>(seq, key));
			UNUSED(tmpres);
			INVARIANT(tmpres.second == true);
		}
	}

	//mutex_lock.unlock();
}

/*template<class key_t, class seq_t>
bool DeletedSet<key_t, seq_t>::getseq(key_t key, seq_t &seq) {
	iterator_bykey_t iter = records_sorted_bykey.find(key);
	if (iter == records_sorted_bykey.end()) {
		return false;
	}
	else {
		seq = iter->second;
		return true;
	}
}*/

template<class key_t, class seq_t>
bool DeletedSet<key_t, seq_t>::check_and_remove(key_t key, seq_t seq, seq_t *deleted_seq_ptr) {
	/*while (true) {
		if (mutex_lock.try_lock()) break;
	}*/

	bool isdeleted = false;
	iterator_bykey_t iter = records_sorted_bykey.find(key);

	if (iter == records_sorted_bykey.end()) {
		//mutex_lock.unlock();
		return isdeleted;
	}

	seq_t oldseq = iter->second;
	if (deleted_seq_ptr != NULL) {
		*deleted_seq_ptr = oldseq;
	}
	INVARIANT(oldseq != seq);
	if (oldseq < seq) {
		records_sorted_bykey.erase(iter);
		INVARIANT(records_sorted_byseq.find(oldseq) != records_sorted_byseq.end());
		records_sorted_byseq.erase(oldseq);
	}
	else {
		isdeleted = true;
	}

	//mutex_lock.unlock();
	return isdeleted;
}

template<class key_t, class seq_t>
size_t DeletedSet<key_t, seq_t>::range_scan(key_t startkey, key_t endkey, std::vector<std::pair<key_t, snapshot_record_t>> &results) {
	results.clear();
	typename std::map<key_t, seq_t>::iterator iter = records_sorted_bykey.lower_bound(startkey);
	for (; iter != records_sorted_bykey.end() && iter->first <= endkey; iter++) {
		snapshot_record_t tmprecord;
		tmprecord.seq = iter->second;
		tmprecord.stat = false;

		results.push_back(std::pair<key_t, snapshot_record_t>(iter->first, tmprecord));
	}
	return results.size();
}

template<class key_t, class seq_t>
void DeletedSet<key_t, seq_t>::clear() {
	records_sorted_bykey.clear();
	records_sorted_byseq.clear();
}

template<class key_t, class seq_t>
void DeletedSet<key_t, seq_t>::load(std::string &path) {
	uint32_t filesize = get_filesize(path);
	INVARIANT(filesize > 0); // at least load 4B recordcnt

	char *content = readonly_mmap(path, 0, filesize);
	INVARIANT(content != NULL);

	// load # of records
	char * curptr = content;
	uint32_t remainsize = filesize;
	uint32_t recordcnt = *((uint32_t *)curptr);
	curptr += sizeof(uint32_t);
	remainsize -= sizeof(uint32_t);

	// load <key, seq> pairs
	key_t tmpkey;
	seq_t tmpseq;
	clear();
	for (uint32_t i = 0; i < recordcnt; i++) {
		uint32_t tmp_keysize = tmpkey.deserialize(curptr, remainsize);
		curptr += tmp_keysize;
		remainsize -= tmp_keysize;
		memcpy(&tmpseq, curptr, sizeof(seq_t));
		curptr += sizeof(seq_t);
		remainsize -= sizeof(seq_t);
		INVARIANT(remainsize >= 0 && remainsize <= filesize);

		records_sorted_bykey.insert(std::pair<key_t, seq_t>(tmpkey, tmpseq));
		records_sorted_byseq.insert(std::pair<seq_t, key_t>(tmpseq, tmpkey));
	}

	int res = munmap(content, filesize);
	if (res != 0) {
		printf("munmap fails: %d\n", res);
		exit(-1);
	}
	content = NULL;
	return;
}

template<class key_t, class seq_t>
void DeletedSet<key_t, seq_t>::store(std::string &path) {
	if (access(path.c_str(), F_OK) == 0) {
		printf("[ERROR] deletedset snapshot already exist: %s\n", path.c_str());
		exit(-1);
	}
	int fd = open(path.c_str(), O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
	INVARIANT(fd != -1);

	// store # of records
	uint32_t recordcnt = records_sorted_bykey.size();
	write(fd, &recordcnt, sizeof(uint32_t));

	// store <key, seq> pairs
	char buf[MAX_BUFSIZE];
	for (typename std::map<key_t, seq_t>::iterator iter = records_sorted_bykey.begin(); iter != records_sorted_bykey.end(); iter++) {
		uint32_t tmp_keysize = iter->first.serialize(buf, MAX_BUFSIZE);
		memcpy(buf + tmp_keysize, &iter->second, sizeof(seq_t));
		write(fd, buf, tmp_keysize + sizeof(seq_t));
	}

	close(fd);
	return;
}

#endif
