replace(){
	grep -r 'include "helper.h"' -l | xargs sed -i  's!include "helper.h"!include "../common/helper.h"!g'
	grep -r 'include "../helper.h"' -l | xargs sed -i  's!include "../helper.h"!include "../../common/helper.h"!g'
	grep -r 'include "key.h"' -l | xargs sed -i  's!include "key.h"!include "../common/key.h"!g'
	grep -r 'include "../key.h"' -l | xargs sed -i  's!include "../key.h"!include "../../common/key.h"!g'
	grep -r 'include "val.h"' -l | xargs sed -i  's!include "val.h"!include "../common/val.h"!g'
	grep -r 'include "dynamic_array.h"' -l | xargs sed -i  's!include "dynamic_array.h"!include "../common/dynamic_array.h"!g'
	grep -r 'include "dynamic_rulemap.h"' -l | xargs sed -i  's!include "dynamic_rulemap.h"!include "../common/dynamic_rulemap.h"!g'
	grep -r 'include "latency_helper.h"' -l | xargs sed -i  's!include "latency_helper.h"!include "../common/latency_helper.h"!g'
	grep -r 'include "io_helper.h"' -l | xargs sed -i  's!include "io_helper.h"!include "../common/io_helper.h"!g'
	grep -r 'include "../io_helper.h"' -l | xargs sed -i  's!include "../io_helper.h"!include "../../common/io_helper.h"!g'
	grep -r 'include "socket_helper.h"' -l | xargs sed -i  's!include "socket_helper.h"!include "../common/socket_helper.h"!g'
	grep -r 'include "pkt_ring_buffer.h"' -l | xargs sed -i  's!include "pkt_ring_buffer.h"!include "../common/pkt_ring_buffer.h"!g'
	grep -r 'include "snapshot_record.h"' -l | xargs sed -i  's!include "snapshot_record.h"!include "../common/snapshot_record.h"!g'
	grep -r 'include "special_case.h"' -l | xargs sed -i  's!include "special_case.h"!include "../common/special_case.h"!g'
	grep -r 'include "workloadparser/' -l | xargs sed -i  's!include "workloadparser/!include "../common/workloadparser/!g'
	grep -r 'include "iniparser/' -l | xargs sed -i  's!include "iniparser/!include "../common/iniparser/!g'
	grep -r 'include "../iniparser/' -l | xargs sed -i  's!include "../iniparser/!include "../../common/iniparser/!g'
	grep -r 'include "packet_format.h"' -l | xargs sed -i  's!include "packet_format.h"!include "../common/packet_format.h"!g'
	grep -r 'include "packet_format_impl.h"' -l | xargs sed -i  's!include "packet_format_impl.h"!include "../common/packet_format_impl.h"!g'
	grep -r 'include "deleted_set_impl.h"' -l | xargs sed -i  's!include "deleted_set_impl.h"!include "../common/deleted_set_impl.h"!g'
	grep -r 'include "rocksdb_wrapper.h"' -l | xargs sed -i  's!include "rocksdb_wrapper.h"!include "../common/rocksdb_wrapper.h"!g'
	grep -r 'include "../rocksdb_wrapper.h"' -l | xargs sed -i  's!include "../rocksdb_wrapper.h"!include "../../common/rocksdb_wrapper.h"!g'
}

#cd farreach
#replace
#cd ..

#cd nocache
#replace
#cd ..

#cd netcache
#replace
#cd ..

#cd distfarreach
#replace
#cd ..

#cd distcache
#replace
#cd ..

cd distnocache
replace
cd ..
