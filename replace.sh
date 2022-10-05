replace(){
	grep -r 'include "helper.h"' -l | xargs sed -i  's!include "helper.h"!include "../common/helper.h"!g'
	grep -r 'include "../helper.h"' -l | xargs sed -i  's!include "../helper.h"!include "../../common/helper.h"!g'
	grep -r 'include "key.h"' -l | xargs sed -i  's!include "key.h"!include "../common/key.h"!g'
	grep -r 'include "../key.h"' -l | xargs sed -i  's!include "../key.h"!include "../../common/key.h"!g'
	grep -r 'include "val.h"' -l | xargs sed -i  's!include "val.h"!include "../common/val.h"!g'
	grep -r 'include "dynamic_array.h"' -l | xargs sed -i  's!include "dynamic_array.h"!include "../common/dynamic_array.h"!g'
	#grep -r 'include "dynamic_rulemap.h"' -l | xargs sed -i  's!include "dynamic_rulemap.h"!include "../common/dynamic_rulemap.h"!g'
	grep -r 'include "latency_helper.h"' -l | xargs sed -i  's!include "latency_helper.h"!include "../common/latency_helper.h"!g'
	#grep -r 'include "pkt_ring_buffer.h"' -l | xargs sed -i  's!include "pkt_ring_buffer.h"!include "../common/pkt_ring_buffer.h"!g'
	grep -r 'include "snapshot_record.h"' -l | xargs sed -i  's!include "snapshot_record.h"!include "../common/snapshot_record.h"!g'
	grep -r 'include "special_case.h"' -l | xargs sed -i  's!include "special_case.h"!include "../common/special_case.h"!g'
	#grep -r 'include "workloadparser/' -l | xargs sed -i  's!include "workloadparser/!include "../common/workloadparser/!g'
	grep -r 'include "iniparser/' -l | xargs sed -i  's!include "iniparser/!include "../common/iniparser/!g'
	grep -r 'include "../iniparser/' -l | xargs sed -i  's!include "../iniparser/!include "../../common/iniparser/!g'
}

cd farreach
replace
cd ..
