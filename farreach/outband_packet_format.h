#ifndef OUTBAND_PACKET_FORMAT_H
#define OUTBAND_PACKET_FORMAT_H

#include "../common/helper.h"
#include "../common/key.h"
#include "../common/val.h"
#include "../common/io_helper.h"
#include <vector>
#include <arpa/inet.h> // htonl
#include <dirent.h>
#include <stdio.h>

class SnapshotSignal {
	public:
		SnapshotSignal(int control_type, int snapshotid);
		SnapshotSignal(const char *buf, int recv_size);

		int control_type() const;
		int snapshotid() const;

		int serialize(char *buf, int max_size);
	private:
		int _control_type;
		int _snapshotid;

		void deserialize(const char* buf, int recv_size);
};

typedef SnapshotSignal snapshot_signal_t;

// Utils

// SNAPSHOT_GETDATA_ACK
int dynamic_serialize_snapshot_getdata_ack(dynamic_array_t &dynamic_buf, int control_type, int max_pipelinenum, int max_logicalservernum, uint32_t* perpipeline_recordcnt, uint16_t** perpipeline_serveridxarray, netreach_key_t** perpipeline_keyarray, val_t** perpipeline_valarray, uint32_t** perpipeline_seqarray, bool** perpipeline_statarray, uint64_t* perlogicalserver_specialcase_bwcost);

void deserialize_snapshot_getdata_ack(dynamic_array_t &dynamic_buf, const int control_type, int &total_bytes, std::vector<int> &perserver_bytes, std::vector<uint16_t> &perserver_serveridx, std::vector<int> &perserver_recordcnt, std::vector<uint64_t> &perserver_specialcase_bwcost, std::vector<std::vector<netreach_key_t>> &perserver_keyarray, std::vector<std::vector<val_t>> &perserver_valarray, std::vector<std::vector<uint32_t>> &perserver_seqarray, std::vector<std::vector<bool>> &perserver_statarray);
void deserialize_snapshot_getdata_ack(char *buf, int maxsize, const int control_type, int &total_bytes, std::vector<int> &perserver_bytes, std::vector<uint16_t> &perserver_serveridx, std::vector<int> &perserver_recordcnt, std::vector<uint64_t> &perserver_specialcase_bwcost, std::vector<std::vector<netreach_key_t>> &perserver_keyarray, std::vector<std::vector<val_t>> &perserver_valarray, std::vector<std::vector<uint32_t>> &perserver_seqarray, std::vector<std::vector<bool>> &perserver_statarray);

// SNAPSHOT_SENDDATA
int dynamic_serialize_snapshot_senddata(dynamic_array_t &dynamic_buf, int control_type, int snapshotid, uint16_t tmpserver_serveridx, int tmpserver_recordcnt, std::vector<netreach_key_t> &tmpserver_keyarray, std::vector<val_t> &tmpserver_valarray, std::vector<uint32_t> &tmpserver_seqarray, std::vector<bool> &tmpserver_statarray);

void deserialize_snapshot_senddata(dynamic_array_t &dynamic_buf, int control_type, int &snapshotid, int &tmpserver_bytes, uint16_t &tmpserver_serveridx, int &tmpserver_recordcnt, std::vector<netreach_key_t> &tmpserver_keyarray, std::vector<val_t> &tmpserver_valarray, std::vector<uint32_t> &tmpserver_seqarray, std::vector<bool> &tmpserver_statarray);

// UPSTREAM_BACKUP_NOTIFICATION
int dynamic_serialize_upstream_backup_notification(dynamic_array_t &dynamic_buf, std::vector<std::vector<netreach_key_t>> &perserver_keyarray, std::vector<std::vector<uint32_t>> &perserver_seqarray);

// file content of upstream backups
void deserialize_upstream_backup_content(char *buf, int maxsize, std::vector<netreach_key_t> &keyarray, std::vector<val_t> &valarray, std::vector<uint32_t> &seqarray, std::vector<bool> &statarray);
void deserialize_perclient_upstream_backup_files(char *dirname, std::vector<std::vector<netreach_key_t>> &perclient_keyarray, std::vector<std::vector<val_t>> &perclient_valarray, std::vector<std::vector<uint32_t>> &perclient_seqarray, std::vector<std::vector<bool>> &perclient_statarray);

// per-server maxseq
void deserialize_perserver_maxseq_files(char *dirname, std::vector<uint32_t> maxseqarray);

#endif
