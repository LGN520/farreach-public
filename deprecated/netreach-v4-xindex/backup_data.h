#ifndef BACKUP_DATA_H
#define BACKUP_DATA_H

#include <map>
#include <stdint.h>

#include "helper.h"
#include "key.h"
#include "val.h"

class BackupData {
	public:
		BackupData();

		std::map<Key, Val> _kvmap;
		std::map<unsigned short, Key> _idxmap;
};

#endif
