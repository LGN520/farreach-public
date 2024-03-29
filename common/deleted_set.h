#ifndef DELETED_SET_H
#define DELETED_SET_H

#include <map>
#include <mutex>

#include "helper.h"
#include "snapshot_record.h"
#include "io_helper.h"

#define CAPACITY_FACTOR 10

template <class key_t, class seq_t>
class DeletedSet {
	// tell compiler that map::iterator is a type instead of a static member
	typedef typename std::map<key_t, seq_t>::iterator iterator_bykey_t;
	typedef typename std::map<seq_t, key_t>::iterator iterator_byseq_t;
	public:
		// We always keep the recently deleted record set (within c*rtt period) to avoid incorrect overwrites caused by evicted packet sent from switch OS (if with packet loss)
		// Very conservative setting: c=10 (>0.02opus*100threads), rtt=100us (>~50us)
		DeletedSet(uint32_t c=10, uint32_t rtt=100);

		DeletedSet<key_t, seq_t>& operator=(const DeletedSet& other);

		int size() const;

		void add(key_t key, seq_t seq);
		//bool getseq(key_t key, seq_t &seq);
		bool check_and_remove(key_t key, seq_t seq, seq_t *deleted_seq_ptr=NULL); // check if isdeleted
		size_t range_scan(key_t startkey, key_t endkey, std::vector<std::pair<key_t, snapshot_record_t>> &results);
		void clear();

		// I/O on disk
		void load(std::string &path);
		void store(std::string &path);

	private:
		std::map<key_t, seq_t> records_sorted_bykey;
		std::map<seq_t, key_t> records_sorted_byseq;
		// NOTE: we do not need lock as we use per-server DeletedSet in main file
		//std::mutex mutex_lock; // Mutex is ok since DeletedSet is a lightweight component compared with underlying in-memory KVS
		uint32_t max_size;
};

#endif
