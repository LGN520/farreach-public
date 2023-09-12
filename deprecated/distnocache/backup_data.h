#ifndef BACKUP_DATA_H
#define BACKUP_DATA_H

#include <map>
#include <stdint.h>

#include "../common/helper.h"
#include "../common/key.h"
#include "../common/val.h"

class BackupData {
	public:
		BackupData();

		std::map<Key, Val> _kvmap;
		std::map<unsigned short, Key> _idxmap;
};

#endif
