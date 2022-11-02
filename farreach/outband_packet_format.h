#ifndef OUTBAND_PACKET_FORMAT_H
#define OUTBAND_PACKET_FORMAT_H

#include "../common/helper.h"
#include <vector>

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
static int dynamic_serialize_snapshot_getdata_ack(dynamic_array_t &dynamic_buf, int max_pipelinenum, int max_logicalservernum, uint32_t* perpipeline_recordcnt, uint16_t** perpipeline_serveridxarray, netreach_key_t** perpipeline_keyarray, val_t** perpipeline_valarray, uint32_t** perpipeline_seqarray, bool** perpipeline_statarray, uint64_t* perlogicalserver_specialcase_bwcost);

static void deserialize_snapshot_getdata_ack(dynamic_array_t &dynamic_buf, const int control_type, int &total_bytes, std::vector<int> &perserver_bytes, std::vector<uint16_t> &perserver_serveridx, std::vector<int> &perserver_recordcnt, std::vector<uint64_t> &perserver_specialcase_bwcost, std::vector<std::vector<netreach_key_t>> &perserver_keyarray, std::vector<std::vector<val_t>> &perserver_valarray, std::vector<std::vector<uint32_t>> &perserver_seqarray, std::vector<std::vector<bool>> &perserver_statarray);
static void deserialize_snapshot_getdata_ack(char *buf, int maxsize, const int control_type, int &total_bytes, std::vector<int> &perserver_bytes, std::vector<uint16_t> &perserver_serveridx, std::vector<int> &perserver_recordcnt, std::vector<uint64_t> &perserver_specialcase_bwcost, std::vector<std::vector<netreach_key_t>> &perserver_keyarray, std::vector<std::vector<val_t>> &perserver_valarray, std::vector<std::vector<uint32_t>> &perserver_seqarray, std::vector<std::vector<bool>> &perserver_statarray);

// SNAPSHOT_SENDDATA
static int dynamic_serialize_snapshot_senddata(dynamic_array_t &dynamic_buf, int snapshotid, int tmpserver_bytes, uint16_t tmpserver_serveridx, int tmpserver_recordcnt, std::vector<netreach_key_t> &tmpserver_keyarray, std::vector<val_t> &tmpserver_valarray, std::vector<uint32_t> &tmpserver_seqarray, std::vector<bool> &tmpserver_statarray);

static int deserialize_snapshot_senddata(dynamic_array_t &dynamic_buf, int &snapshotid, int &tmpserver_bytes, uint16_t &tmpserver_serveridx, int &tmpserver_recordcnt, std::vector<netreach_key_t> &tmpserver_keyarray, std::vector<val_t> &tmpserver_valarray, std::vector<uint32_t> &tmpserver_seqarray, std::vector<bool> &tmpserver_statarray);

// UPSTREAM_BACKUP_NOTIFICATION (TODO: END HERE)
// FORMAT: <int num>, <Key k0, int seq0>, <Key k1, int seq1>, ...
static int serialize_upstream_backup_notification(dynamic_array_t &dynamic_buf, int max_pipelinenum, netreach_key_t **perpipeline_keyarray, uint32_t **perpipeline_seqarray);

#endif
