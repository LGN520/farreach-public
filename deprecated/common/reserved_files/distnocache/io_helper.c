#include "io_helper.h"

// server

void get_server_db_path(std::string &db_path, uint16_t workerid) {
	GET_STRING(db_path, "/tmp/distnocache/worker" << workerid << ".db");
	return;
}

void get_server_deletedset_path(std::string &deletedset_path, uint16_t workerid) {
	GET_STRING(deletedset_path, "/tmp/distnocache/worker" << workerid << ".deletedset");
	return;
}

void get_server_snapshotid_path(std::string &snapshotid_path, uint16_t workerid) {
	GET_STRING(snapshotid_path, "/tmp/distnocache/worker" << workerid << ".snapshotid");
	return;
}

void get_server_snapshotdb_path(std::string &snapshotdb_path, uint16_t workerid) {
	GET_STRING(snapshotdb_path, "/tmp/distnocache/worker" << workerid << ".snapshotdb");
	return;
}

void get_server_snapshotdbseq_path(std::string &snapshotdbseq_path, uint16_t workerid, uint32_t snapshotid) {
	GET_STRING(snapshotdbseq_path, "/tmp/distnocache/worker" << workerid << ".snapshotdbseq" << snapshotid);
	return;
}

void get_server_inswitchsnapshot_path(std::string &inswitchsnapshot_path, uint16_t workerid, uint32_t snapshotid) {
	GET_STRING(inswitchsnapshot_path, "/tmp/distnocache/worker" << workerid << ".inswitchsnapshot" << snapshotid);
	return;
}

void get_server_snapshotdeletedset_path(std::string &snapshotdeletedset_path, uint16_t workerid, uint32_t snapshotid) {
	GET_STRING(snapshotdeletedset_path, "/tmp/distnocache/worker" << workerid << ".snapshotdeletedset" << snapshotid);
	return;
}

// controller

void get_controller_snapshotid_path(std::string &snapshotid_path) {
	GET_STRING(snapshotid_path, "/tmp/distnocache/controller.snapshotid");
	return;
}

void get_controller_snapshotdata_path(std::string &snapshotdata_path, int snapshotid) {
	GET_STRING(snapshotdata_path, "/tmp/distnocache/controller.snapshotdata" << snapshotid);
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
