#ifndef IO_HELPER_H
#define IO_HELPER_H

#include <string>
#include <stdio.h> // remove
#include <ftw.h> // nftw
#include <error.h> // errno
#include <fcntl.h> // open
#include <unistd.h> // access
#include <sys/mman.h> // mmap
// fstat
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "helper.h"

void get_server_db_path(method_t methodid, std::string &db_path, uint16_t workerid);
void get_server_deletedset_path(method_t methodid, std::string &deletedset_path, uint16_t workerid);
void get_server_snapshotid_path(method_t methodid, std::string &snapshotid_path, uint16_t workerid);
void get_server_snapshotdb_path(method_t methodid, std::string &snapshotdb_path, uint16_t workerid); // at most one snapshotdb for server-side recovery
void get_server_snapshotdbseq_path(method_t methodid, std::string &snapshotdbseq_path, uint16_t workerid, uint32_t snapshotid);
void get_server_snapshotdeletedset_path(method_t methodid, std::string &snapshotdeletedset_path, uint16_t workerid, uint32_t snapshotid);
void get_server_inswitchsnapshot_path(method_t methodid, std::string &inswitchsnapshot_path, uint16_t workerid, uint32_t snapshotid);
void get_server_snapshotmaxseq_path(method_t methodid, std::string &snapshotmaxseq_path, uint16_t workerid, uint32_t snapshotid);
void get_server_latestmaxseq_path(method_t methodid, std::string &latestmaxseq_path, uint16_t workerid);

void get_controller_snapshotid_path(method_t methodid, std::string &snapshotid_path);
void get_controller_snapshotdata_path(method_t methodid, std::string &snapshotdata_path, int snapshotid);
void get_controller_spinesnapshotdata_path(method_t methodid, std::string &snapshotdata_path, int snapshotid);
void get_controller_leafsnapshotdata_path(method_t methodid, std::string &snapshotdata_path, int snapshotid);

bool isexist(std::string path);
uint32_t get_filesize(std::string path);
char *readonly_mmap(std::string path, uint32_t offset, uint32_t size);
int rmfile(const char *pathname, const struct stat *sbuf, int type, struct FTW *ftwb);
void rmfiles(const char *pathname);
void load_snapshotid(int &snapshotid, std::string snapshotid_path);
void store_snapshotid(int snapshotid, std::string snapshotid_path);
void load_snapshotdbseq(uint64_t &snapshotdbseq, std::string snapshotdbseq_path);
void store_snapshotdbseq(uint64_t snapshotdbseq, std::string snapshotdbseq_path);
void load_maxseq(uint32_t &maxseq, std::string maxseq_path);
void store_maxseq(uint32_t maxseq, std::string maxseq_path);
void store_buf(const char * buf, uint32_t bufsize, std::string path);

// for switchos
void get_controller_snapshotid_path(method_t methodid, char *snapshotid_path, int len);
void get_controller_snapshotdata_path(method_t methodid, char *snapshotdata_path, int len, int snapshotid);
void get_controller_spinesnapshotdata_path(method_t methodid, char *snapshotdata_path, int len, int snapshotid);
void get_controller_leafsnapshotdata_path(method_t methodid, char *snapshotdata_path, int len, int snapshotid);
bool isexist(const char *path);
void load_snapshotid(int &snapshotid, const char* snapshotid_path);
uint32_t get_filesize(const char* path);
char *readonly_mmap(const char *path, uint32_t offset, uint32_t size);

#endif
