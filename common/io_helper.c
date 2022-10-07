#include "io_helper.h"

// server

void get_server_db_path(method_t methodid, std::string &db_path, uint16_t workerid) {
	const char *methodname = get_methodname_byid(methodid);
	GET_STRING(db_path, "/tmp/" << std::string(methodname) << "/worker" << workerid << ".db");
	return;
}

void get_server_deletedset_path(method_t methodid, std::string &deletedset_path, uint16_t workerid) {
	const char *methodname = get_methodname_byid(methodid);
	GET_STRING(deletedset_path, "/tmp/" << std::string(methodname) << "/worker" << workerid << ".deletedset");
	return;
}

void get_server_snapshotid_path(method_t methodid, std::string &snapshotid_path, uint16_t workerid) {
	const char *methodname = get_methodname_byid(methodid);
	GET_STRING(snapshotid_path, "/tmp/" << std::string(methodname) << "/worker" << workerid << ".snapshotid");
	return;
}

void get_server_snapshotdb_path(method_t methodid, std::string &snapshotdb_path, uint16_t workerid) {
	const char *methodname = get_methodname_byid(methodid);
	GET_STRING(snapshotdb_path, "/tmp/" << std::string(methodname) << "/worker" << workerid << ".snapshotdb");
	return;
}

void get_server_snapshotdbseq_path(method_t methodid, std::string &snapshotdbseq_path, uint16_t workerid, uint32_t snapshotid) {
	const char *methodname = get_methodname_byid(methodid);
	GET_STRING(snapshotdbseq_path, "/tmp/" << std::string(methodname) << "/worker" << workerid << ".snapshotdbseq" << snapshotid);
	return;
}

void get_server_inswitchsnapshot_path(method_t methodid, std::string &inswitchsnapshot_path, uint16_t workerid, uint32_t snapshotid) {
	const char *methodname = get_methodname_byid(methodid);
	GET_STRING(inswitchsnapshot_path, "/tmp/" << std::string(methodname) << "/worker" << workerid << ".inswitchsnapshot" << snapshotid);
	return;
}

void get_server_snapshotdeletedset_path(method_t methodid, std::string &snapshotdeletedset_path, uint16_t workerid, uint32_t snapshotid) {
	const char *methodname = get_methodname_byid(methodid);
	GET_STRING(snapshotdeletedset_path, "/tmp/" << std::string(methodname) << "/worker" << workerid << ".snapshotdeletedset" << snapshotid);
	return;
}

// controller

void get_controller_snapshotid_path(method_t methodid, std::string &snapshotid_path) {
	const char *methodname = get_methodname_byid(methodid);
	GET_STRING(snapshotid_path, "/tmp/" << std::string(methodname) << "/controller.snapshotid");
	return;
}

void get_controller_snapshotdata_path(method_t methodid, std::string &snapshotdata_path, int snapshotid) {
	const char *methodname = get_methodname_byid(methodid);
	GET_STRING(snapshotdata_path, "/tmp/" << std::string(methodname) << "/controller.snapshotdata" << snapshotid);
	return;
}

void get_controller_spinesnapshotdata_path(method_t methodid, std::string &snapshotdata_path, int snapshotid) {
	const char *methodname = get_methodname_byid(methodid);
	GET_STRING(snapshotdata_path, "/tmp/" << std::string(methodname) << "/controller.spinesnapshotdata" << snapshotid);
	return;
}

void get_controller_leafsnapshotdata_path(method_t methodid, std::string &snapshotdata_path, int snapshotid) {
	const char *methodname = get_methodname_byid(methodid);
	GET_STRING(snapshotdata_path, "/tmp/" << std::string(methodname) << "/controller.leafsnapshotdata" << snapshotid);
	return;
}

// utilities

bool isexist(std::string path) {
	return access(path.c_str(), F_OK) == 0;
}

uint32_t get_filesize(std::string path) {
	if (isexist(path)) {
		int fd = open(path.c_str(), O_RDONLY);
		INVARIANT(fd != -1);

		struct stat statbuf;
		int fstat_res = fstat(fd, &statbuf);
		if (fstat_res < 0) {
			printf("Cannot get stat of %s\n", path.c_str());
			exit(-1);
		}
		uint32_t filesize = statbuf.st_size;

		close(fd);
		return filesize;
	}
	return 0;
}

char *readonly_mmap(std::string path, uint32_t offset, uint32_t size) {
	if (isexist(path)) {
		int fd = open(path.c_str(), O_RDONLY);
		INVARIANT(fd != -1);

		char * content = (char *)(mmap(NULL, size, PROT_READ, MAP_SHARED, fd, offset)); // NOTE: the last argument must be page-size-aligned
		if (content == MAP_FAILED) {
			printf("mmap fails: errno = %d\n", errno);
			exit(-1);
		}
		INVARIANT(content != NULL);

		close(fd);
		return content;
	}
	return NULL;
}

int rmfile(const char *pathname, const struct stat *sbuf, int type, struct FTW *ftwb) {
	if (remove(pathname) < 0) {
		printf("fail to remove %s; errno: %d\n", pathname, int(errno));
		return -1;
	}
	return 0;
}

void rmfiles(const char *pathname) {
	if (access(pathname, F_OK) == 0) {
		if (nftw(pathname, rmfile, 10, FTW_DEPTH | FTW_MOUNT | FTW_PHYS) < 0)
		{
			printf("fail to walk file tree in %s; errno: %d\n", pathname, int(errno));
			exit(1);
		}
	}
	return;
}

void load_snapshotid(int &snapshotid, std::string snapshotid_path) {
	FILE *snapshotid_fd = fopen(snapshotid_path.c_str(), "rb");
	fread(&snapshotid, sizeof(int), 1, snapshotid_fd);
	fclose(snapshotid_fd);
	return;
}

void store_snapshotid(int snapshotid, std::string snapshotid_path) {
	FILE *snapshotid_fd = fopen(snapshotid_path.c_str(), "wb+");
	fwrite(&snapshotid, sizeof(int), 1, snapshotid_fd);
	fclose(snapshotid_fd);
	return;
}

void load_snapshotdbseq(uint64_t &snapshotdbseq, std::string snapshotdbseq_path) {
	FILE *snapshotdbseq_fd = fopen(snapshotdbseq_path.c_str(), "rb");
	fread(&snapshotdbseq, sizeof(uint64_t), 1, snapshotdbseq_fd);
	fclose(snapshotdbseq_fd);
	return;
}

void store_snapshotdbseq(uint64_t snapshotdbseq, std::string snapshotdbseq_path) {
	FILE *snapshotdbseq_fd = fopen(snapshotdbseq_path.c_str(), "wb+");
	fwrite(&snapshotdbseq, sizeof(uint64_t), 1, snapshotdbseq_fd);
	fclose(snapshotdbseq_fd);
	return;
}

void store_buf(const char * buf, uint32_t bufsize, std::string path) {
	FILE *fd = fopen(path.c_str(), "wb+");
	fwrite(buf, sizeof(char), bufsize, fd);
	fclose(fd);
	return;
}

// for switchos

void get_controller_snapshotid_path(method_t methodid, char *snapshotid_path, int len) {
	const char *methodname = get_methodname_byid(methodid);
	char tmpstr[256];
	memset(tmpstr, '\0', 256);
	sprintf(tmpstr, "/tmp/%s/controller.snapshotid", methodname);
	INVARIANT(snapshotid_path != NULL && len >= strlen(tmpstr));
	memset(snapshotid_path, '\0', len);
	memcpy(snapshotid_path, tmpstr, strlen(tmpstr));
	return;
}

void get_controller_snapshotdata_path(method_t methodid, char *snapshotdata_path, int len, int snapshotid) {
	const char *methodname = get_methodname_byid(methodid);
	char tmpstr[256];
	memset(tmpstr, '\0', 256);
	sprintf(tmpstr, "/tmp/%s/controller.snapshotdata", methodname);
	INVARIANT(snapshotdata_path != NULL && len >= strlen(tmpstr));
	memset(snapshotdata_path, '\0', len);
	memcpy(snapshotdata_path, tmpstr, strlen(tmpstr));
	return;
}

void get_controller_spinesnapshotdata_path(method_t methodid, char *snapshotdata_path, int len, int snapshotid) {
	const char *methodname = get_methodname_byid(methodid);
	char tmpstr[256];
	memset(tmpstr, '\0', 256);
	sprintf(tmpstr, "/tmp/%s/controller.spinesnapshotdata", methodname);
	INVARIANT(snapshotdata_path != NULL && len >= strlen(tmpstr));
	memset(snapshotdata_path, '\0', len);
	memcpy(snapshotdata_path, tmpstr, strlen(tmpstr));
	return;
}

void get_controller_leafsnapshotdata_path(method_t methodid, char *snapshotdata_path, int len, int snapshotid) {
	const char *methodname = get_methodname_byid(methodid);
	char tmpstr[256];
	memset(tmpstr, '\0', 256);
	sprintf(tmpstr, "/tmp/%s/controller.leafsnapshotdata", methodname);
	INVARIANT(snapshotdata_path != NULL && len >= strlen(tmpstr));
	memset(snapshotdata_path, '\0', len);
	memcpy(snapshotdata_path, tmpstr, strlen(tmpstr));
	return;
}

bool isexist(const char *path) {
	return access(path, F_OK) == 0;
}

void load_snapshotid(int &snapshotid, const char *snapshotid_path) {
	FILE *snapshotid_fd = fopen(snapshotid_path, "rb");
	fread(&snapshotid, sizeof(int), 1, snapshotid_fd);
	fclose(snapshotid_fd);
	return;
}

uint32_t get_filesize(const char *path) {
	if (isexist(path)) {
		int fd = open(path, O_RDONLY);
		INVARIANT(fd != -1);

		struct stat statbuf;
		int fstat_res = fstat(fd, &statbuf);
		if (fstat_res < 0) {
			printf("Cannot get stat of %s\n", path);
			exit(-1);
		}
		uint32_t filesize = statbuf.st_size;

		close(fd);
		return filesize;
	}
	return 0;
}

char *readonly_mmap(const char *path, uint32_t offset, uint32_t size) {
	if (isexist(path)) {
		int fd = open(path, O_RDONLY);
		INVARIANT(fd != -1);

		char * content = (char *)(mmap(NULL, size, PROT_READ, MAP_SHARED, fd, offset)); // NOTE: the last argument must be page-size-aligned
		if (content == MAP_FAILED) {
			printf("mmap fails: errno = %d\n", errno);
			exit(-1);
		}
		INVARIANT(content != NULL);

		close(fd);
		return content;
	}
	return NULL;
}
