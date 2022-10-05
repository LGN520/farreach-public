#ifndef DYNAMIC_RULEMAP_H
#define DYNAMIC_RULEMAP_H

#include <map>
#include <vector>
#include <stdio.h>
#include <atomic>

#include "key.h"
#include "helper.h"

#ifdef USE_YCSB
#include "workloadparser/ycsb_parser.h"
#elif defined USE_SYNTHETIC
#include "workloadparser/synthetic_parser.h"
#endif

class DynamicRulemap {
	typedef std::map<netreach_key_t, netreach_key_t> o2n_map_t; // mapping from original key to new key

	public:
		DynamicRulemap(int periodnum, const char * ruleprefix);
		~DynamicRulemap();

		// try mapping of current period
		void trymap(netreach_key_t &originalkey);
		// move to next period
		bool nextperiod();

	private:
		std::atomic<int> periodidx; // we have one writer (remote_client.main) and multiple readers (remote_client.workers) on periodidx
		std::vector<o2n_map_t> mappings; // after loading dynamic rules, mappings will not be changed
};

typedef DynamicRulemap dynamic_rulemap_t;

#endif
