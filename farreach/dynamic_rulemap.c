#include "../common/dynamic_rulemap.h"

DynamicRulemap::DynamicRulemap(int periodnum, const char * rulepath) {
	INVARIANT(ruleprefix != NULL);

	mappings.resize(periodnum);
	for (int i = 0; i < periodnum; i++) { // 0, 1, ..., 5
		char tmp_rulefile[256];
		memset(tmp_rulefile, 0, 256);
		sprintf(tmp_rulefile, "%s/%d.out", rulepath, i); // 0, 1, 2, ..., 5

		FILE *tmp_rulefd = fopen(tmp_rulefile, "r");
		if (tmp_rulefd == NULL) {
			printf("No such dynamic rulemap file: %s\n", tmp_rulefile);
			exit(-1);
		}
		
		// format: originalkey + blank space + newkey
		char tmp_ruleline[MAX_BUFSIZE];
		while (fgets(tmp_ruleline, MAX_BUFSIZE, tmp_rulefd) != NULL) {
			char *tmp_originalkeyend = strchr(tmp_ruleline, ' ');
			INVARIANT(tmp_originalkeyend != NULL);
			netreach_key_t tmp_originalkey = extract_key(tmp_ruleline, tmp_originalkeyend - tmp_ruleline);

			char * tmp_newkeybegin = tmp_originalkeyend + 1;
			char * tmp_newkeyend = strchr(tmp_ruleline, '\n');
			INVARIANT(tmp_newkeyend != NULL);
			netreach_key_t tmp_newkey = extract_key(tmp_newkeybegin, tmp_newkeyend - tmp_newkeybegin);

			mappings[i].insert(std::pair<netreach_key_t, netreach_key_t>(tmp_originalkey, tmp_newkey));
		}
	}

	periodidx = -1;
}

void DynamicRulemap::trymap(netreach_key_t &originalkey) {
	INVARIANT(periodidx >= 0 && periodidx < mappings.size());

	o2n_map_t::iterator iter = mappings[periodidx].find(originalkey);
	if (iter != mappings[periodidx].end()) {
		originalkey = iter->second;
	}
	return;
}

bool DynamicRulemap::nextperiod() {
	periodidx += 1;
	if (periodidx >= 0 && periodidx < mappings.size()) {
		return true;
	}
	else {
		return false;
	}
}

DynamicRulemap::~DynamicRulemap() {}
