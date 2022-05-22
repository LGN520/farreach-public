#ifndef DELETED_SET_IMPL_H
#define DELETED_SET_IMPL_H

#include "deleted_set.h"

template<class key_t, class seq_t>
DeletedSet<key_t, seq_t>::DeletedSet(uint32_t c, uint32_t rtt) {
	// Conservative estimation: 100 Mops / c*rtt us = 100 opus * c*rtt us = 100c*rtt
	max_size = 100 * c * rtt;
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
		INVARIANT(tmpres.second == true);
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

#endif
