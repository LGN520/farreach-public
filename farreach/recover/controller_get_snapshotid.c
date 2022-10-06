#include "../../common/helper.h"
#include "../../common/io_helper.h"

#include "../common_impl.h"

int main(int argc, char **argv) {
	std::string snapshotid_path;
	get_controller_snapshotid_path(CURMETHOD_ID, snapshotid_path);
	int snapshotid;
	load_snapshotid(snapshotid, snapshotid_path);
	printf("controller latest snapshotid: %d\n", snapshotid);
	return 0;
}
