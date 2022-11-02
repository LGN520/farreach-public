#include "outband_packet_format.h"

SnapshotSignal::SnapshotSignal(int control_type, int snapshotid) {
	this._control_type = control_type;
	this._snapshotid = snapshotid;
}

SnapshotSignal::SnapshotSignal(const char *buf, int recv_size) {
	deserialzie(buf, recv_size);
}

int SnapshotSignal::control_type() const {
	return this._control_type;
}

int SnapshotSignal::snapshotid() const {
	return this._snapshotid;
}

int SnapshotSignal::serialize(char *buf, int max_size) {
	INVARIANT(max_size >= 2*sizeof(int));

	int offset = 0;

	memcpy(buf + offset, &this._control_type, sizeof(int));
	offset += sizeof(int);

	memcpy(buf + offset, &this._snapshotid, sizeof(int));
	offset += sizeof(int);

	return offset;
}

void SnapshotSignal::deserialize(const char* buf, int recv_size) {
	INVARIANT(recv_size >= 2*sizeof(int));

	int offset = 0;

	memcpy(&this._control_type, buf + offset, sizeof(int));
	offset += sizeof(int);

	memcpy(&this._snapshotid, buf + offset, sizeof(int));
	offset += sizeof(int);
}

// Utils

static int dynamic_serialize_snapshot_getdata_ack(dynamic_array_t &dynamic_buf, int control_type, int max_pipelinenum, int max_logicalservernum, uint32_t* perpipeline_recordcnt, uint16_t** perpipeline_serveridxarray, netreach_key_t** perpipeline_keyarray, val_t** perpipeline_valarray, uint32_t** perpipeline_seqarray, bool** perpipeline_statarray, uint64_t* perlogicalserver_specialcase_bwcost) {

	// per-server data
	dynamic_array_t tmp_sendbuf_list[max_logicalservernum]; // per-record data for each server
	for (uint16_t tmp_global_server_logical_idx = 0; tmp_global_server_logical_idx < max_logicalservernum; tmp_global_server_logical_idx++) {
		tmp_sendbuf_list[tmp_global_server_logical_idx].init(MAX_BUFSIZE, MAX_LARGE_BUFSIZE);
	}
	int tmp_send_bytes[max_logicalservernum];
	memset((void *)tmp_send_bytes, 0, max_logicalservernum*sizeof(int)); // bytes of per-record data for each server
	int tmp_record_cnts[max_logicalservernum];
	memset((void *)tmp_record_cnts, 0, max_logicalservernum*sizeof(int));

	// clear intermediate dynamic arrays
	for (size_t tmp_global_server_logical_idx = 0; tmp_global_server_logical_idx < max_logicalservernum; tmp_global_server_logical_idx++) {
		tmp_sendbuf_list[tmp_global_server_logical_idx].clear();
	}

	// FORMAT
	// snapshot data: <int SNAPSHOT_GETDATA_ACK, int32_t total_bytes (including SNAPSHOT_GETDATA_ACK), per-server data>
	// per-server data: <int32_t perserver_bytes, uint16_t serveridx, int32_t recordcnt, uint64_t cur_specialcase_bwcost, per-record data>
	// per-record data: <16B key, uint16_t vallen, value (w/ padding), uint32_t seq, bool stat>
	
	// prepare per-server per-record data
	for (uint32_t tmp_pipeidx = 0; tmp_pipeidx < max_pipelinenum; tmp_pipeidx++) {
		for (uint32_t tmpidx = 0; tmpidx < perpipeline_recordcnt[tmp_pipeidx]; tmpidx++) {
			uint16_t tmp_serveridx = perpipeline_serveridxarray[tmp_pipeidx][tmpidx];
			dynamic_array_t &tmp_sendbuf = tmp_sendbuf_list[tmp_serveridx];

			// 16B key
			uint32_t tmp_keysize = perpipeline_keyarray[tmp_pipeidx][tmpidx].dynamic_serialize(tmp_sendbuf, tmp_send_bytes[tmp_serveridx]);
			tmp_send_bytes[tmp_serveridx] += tmp_keysize;

			// val
			uint32_t tmp_valsize = perpipeline_valarray[tmp_pipeidx][tmpidx].dynamic_serialize(tmp_sendbuf, tmp_send_bytes[tmp_serveridx]);
			tmp_send_bytes[tmp_serveridx] += tmp_valsize;

			// seq
			tmp_sendbuf.dynamic_memcpy(tmp_send_bytes[tmp_serveridx], (char *)&perpipeline_seqarray[tmp_pipeidx][tmpidx], sizeof(uint32_t));
			tmp_send_bytes[tmp_serveridx] += sizeof(uint32_t);

			// stat
			tmp_sendbuf.dynamic_memcpy(tmp_send_bytes[tmp_serveridx], (char *)&perpipeline_statarray[tmp_pipeidx][tmpidx], sizeof(bool));
			tmp_send_bytes[tmp_serveridx] += sizeof(bool);

			tmp_record_cnts[tmp_serveridx] += 1;
		}
	}

	// prepare header of snapshot data and per-server data
	int total_bytes = sizeof(int) + sizeof(int32_t); // leave 8B for SNAPSHOT_GETDATA_ACK and total_bytes
	for (uint16_t tmp_serveridx = 0; tmp_serveridx < max_servernum; tmp_serveridx++) {
		if (tmp_record_cnts[tmp_serveridx] > 0) {
			int32_t tmp_perserver_bytes = tmp_send_bytes[tmp_serveridx] + sizeof(int32_t) + sizeof(uint16_t) + sizeof(int) + sizeof(uint64_t);

			// per-server bytes
			dynamic_buf.dynamic_memcpy(total_bytes, (char *)&tmp_perserver_bytes, sizeof(int32_t));
			total_bytes += sizeof(int32_t);

			// serveridx
			dynamic_buf.dynamic_memcpy(total_bytes, (char *)&tmp_serveridx, sizeof(uint16_t));
			total_bytes += sizeof(uint16_t);

			// recordcnt
			dynamic_buf.dynamic_memcpy(total_bytes, (char *)&tmp_record_cnts[tmp_serveridx], sizeof(int));
			total_bytes += sizeof(int);

			// specialcase bwcost
			dynamic_buf.dynamic_memcpy(total_bytes, (char *)&perlogicalserver_specialcase_bwcost[tmp_serveridx], sizeof(uint64_t));
			total_bytes += sizeof(uint64_t);

			// per-record data of each server
			dynamic_buf.dynamic_memcpy(total_bytes, tmp_sendbuf_list[tmp_serveridx].array(), tmp_send_bytes[tmp_serveridx]);
			total_bytes += tmp_send_bytes[tmp_serveridx];
		}
	}
	dynamic_buf.dynamic_memcpy(0, (char *)&control_type, sizeof(int)); // set 1st 4B as SNAPSHOT_GETDATA_ACK
	dynamic_buf.dynamic_memcpy(0 + sizeof(int), (char *)&total_bytes, sizeof(int32_t)); // set 2nd 4B as total_bytes
	INVARIANT(total_bytes <= MAX_LARGE_BUFSIZE);
	return total_bytes;
}

static void deserialize_snapshot_getdata_ack(dynamic_array_t &dynamic_buf, const int control_type, int &total_bytes, std::vector<int> &perserver_bytes, std::vector<uint16_t> &perserver_serveridx, std::vector<int> &perserver_recordcnt, std::vector<uint64_t> &perserver_specialcase_bwcost, std::vector<std::vector<netreach_key_t>> &perserver_keyarray, std::vector<std::vector<val_t>> &perserver_valarray, std::vector<std::vector<uint32_t>> &perserver_seqarray, std::vector<std::vector<bool>> &perserver_statarray) {
	deserialize_snapshot_getdata_ack(dynamic_buf.array(), dynamic_buf.size(), control_type, total_bytes, perserver_bytes, perserver_serveridx, perserver_recordcnt, perserver_specialcase_bwcost, perserver_keyarray, perserver_valarray, perserver_seqarray, perserver_statarray);
}

static void deserialize_snapshot_getdata_ack(char *buf, int maxsize, const int control_type, int &total_bytes, std::vector<int> &perserver_bytes, std::vector<uint16_t> &perserver_serveridx, std::vector<int> &perserver_recordcnt, std::vector<uint64_t> &perserver_specialcase_bwcost, std::vector<std::vector<netreach_key_t>> &perserver_keyarray, std::vector<std::vector<val_t>> &perserver_valarray, std::vector<std::vector<uint32_t>> &perserver_seqarray, std::vector<std::vector<bool>> &perserver_statarray) {
	// FORMAT
	// snapshot data: <int SNAPSHOT_GETDATA_ACK, int32_t total_bytes (including SNAPSHOT_GETDATA_ACK), per-server data>
	// per-server data: <int32_t perserver_bytes, uint16_t serveridx, int32_t recordcnt, uint64_t cur_specialcase_bwcost, per-record data>
	// per-record data: <16B key, uint16_t vallen, value (w/ padding), uint32_t seq, bool stat>
	
	int offset = 0;

	int tmp_control_type = 0;
	memcpy(&tmp_control_type, buf, sizeof(int));
	INVARIANT(tmp_control_type == control_type);
	offset += sizeof(int);

	memcpy(&total_bytes, buf + offset, sizeof(int32_t));
	INVARIANT(maxsize == total_bytes);
	offset += sizeof(int32_t);

	while (offset < total_bytes) {
		int tmpserver_bytes = 0;
		memcpy(&tmpserver_bytes, buf + offset, sizeof(int32_t));
		offset += sizeof(int32_t);
		perserver_bytes.push_back(tmpserver_bytes);

		uint16_t tmpserver_serveridx = 0;
		memcpy(&tmpserver_serveridx, buf + offset, sizeof(uint16_t));
		offset += sizeof(uint16_t);
		perserver_serveridx.push_back(tmpserver_serveridx);

		int tmpserver_recordcnt = 0;
		memcpy(&tmpserver_recordcnt, buf + offset, sizeof(int32_t));
		offset += sizeof(int32_t);
		perserver_recordcnt.push_back(tmpserver_recordcnt);

		uint64_t tmpserver_specialcase_bwcost = 0;
		memcpy(&tmpserver_specialcase_bwcost, buf + offset, sizeof(uint64_t));
		offset += sizeof(uint64_t);
		perserver_specialcase_bwcost.push_back(tmpserver_specialcase_bwcost);

		std::vector<netreach_key_t> tmpserver_keyarray;
		std::vector<val_t> tmpserver_valarray;
		std::vector<uint32_t> tmpserver_seqarray;
		std::vector<bool> tmpserver_statarray;
		for (int i = 0; i < tmpserver_recordcnt; i++) {
			netreach_key_t tmpkey;
			int tmpkeysize = tmpkey.deserialize(buf + offset, maxsize - offset);
			offset += tmpkeysize;
			tmpserver_keyarray.push_back(tmpkey);

			val_t tmpval;
			int tmpvalsize = tmpval.deserialize(buf + offset, maxsize - offset);
			offset += tmpvalsize;
			tmpserver_valarray.push_back(tmpval);

			uint32_t tmpseq;
			memcpy(&tmpseq, buf + offset, sizeof(uint32_t));
			offset += sizeof(uint32_t);
			tmpserver_seqarray.push_back(tmpseq);

			bool tmpstat;
			memcpy(&tmpstat, buf + offset, sizeof(bool));
			offset += sizeof(bool);
			tmpserver_statarray.push_back(tmpstat);
		}
		perserver_keyarray.push_back(tmpserver_keyarray);
		perserver_valarray.push_back(tmpserver_valarray);
		perserver_seqarray.push_back(tmpserver_seqarray);
		perserver_statarray.push_back(tmpserver_statarray);
	}
}

static int dynamic_serialize_snapshot_senddata(dynamic_array_t &dynamic_buf, int control_type, int snapshotid, uint16_t tmpserver_serveridx, int tmpserver_recordcnt, std::vector<netreach_key_t> &tmpserver_keyarray, std::vector<val_t> &tmpserver_valarray, std::vector<uint32_t> &tmpserver_seqarray, std::vector<bool> &tmpserver_statarray) {
	// per-server snapshot data: <int SNAPSHOT_SENDDATA, int snapshotid, int32_t perserver_bytes (including SNAPSHOT_SENDDATA), uint16_t serveridx, int32_t record_cnt, per-record data>
	// per-record data: <16B key, uint16_t vallen, value (w/ padding), uint32_t seq, bool stat>

	int offset = 0;

	// SNAPSHOT_SENDDATA
	dynamic_buf.dynamic_memcpy(offset, (char *)&control_type, sizeof(int));
	offset += sizeof(int);

	// snapshotid
	dynamic_buf.dynamic_memcpy(offset, (char *)&snapshotid, sizeof(int));
	offset += sizeof(int);

	// skip perserver_bytes temporarily
	offset += sizeof(int32_t);

	// serveridx
	dynamic_buf.dynamic_memcpy(offset, (char *)&tmpserver_serveridx, sizeof(uint16_t));
	offset += sizeof(uint16_t);

	// recordcnt
	dynamic_buf.dynamic_memcpy(offset, (char *)&tmpserver_recordcnt, sizeof(int));
	offset += sizeof(int);

	for (int i = 0; i < tmpserver_keyarray.size(); i++) {
		// key
		int tmpkeysize = tmpserver_keyarray[i].dynamic_serialize(dynamic_buf, offset);
		offset += tmpkeysize;

		// value
		int tmpvalsize = tmpserver_valarray[i].dynamic_serialize(dynamic_buf, offset);
		offset += tmpvalsize;

		// seq
		dynamic_buf.dynamic_memcpy(offset, (char *)&tmpserver_seqarray[i], sizeof(uint32_t));
		offset += sizeof(uint32_t);

		// stat
		dynamic_buf.dynamic_memcpy(offset, (char *)&tmpserver_statarray[i], sizeof(bool));
		offset += sizeof(bool);
	}

	// go back to update perserver_bytes
	dynamic_buf.dynamic_memcpy(sizeof(int) + sizeof(int), (char *)&offset, sizeof(int32_t));

	return offset;
}

static void deserialize_snapshot_senddata(dynamic_array_t &dynamic_buf, const int control_type, int &snapshotid, int &tmpserver_bytes, uint16_t &tmpserver_serveridx, int &tmpserver_recordcnt, std::vector<netreach_key_t> &tmpserver_keyarray, std::vector<val_t> &tmpserver_valarray, std::vector<uint32_t> &tmpserver_seqarray, std::vector<bool> &tmpserver_statarray) {
	// per-server snapshot data: <int SNAPSHOT_SENDDATA, int snapshotid, int32_t perserver_bytes (including SNAPSHOT_SENDDATA), uint16_t serveridx, int32_t record_cnt, per-record data>
	// per-record data: <16B key, uint16_t vallen, value (w/ padding), uint32_t seq, bool stat>

	int offset = 0;

	// SNAPSHOT_SENDDATA
	int tmp_control_type = 0;
	memcpy(&tmp_control_type, dynamic_buf.array() + offset, sizeof(int));
	INVARIANT(tmp_control_type == control_type);
	offset += sizeof(int);

	// snapshotid
	memcpy(&snapshotid, dynamic_buf.array() + offset, sizeof(int));
	offset += sizeof(int);

	// perserver_bytes
	memcpy(&tmpserver_bytes, dynamic_buf.array() + offset, sizeof(int32_t));
	offset += sizeof(int32_t);

	// serveridx
	memcpy(&tmpserver_serveridx, dynamic_buf.array() + offset, sizeof(uint16_t));
	offset += sizeof(uint16_t);

	// recordcnt
	memcpy(&tmpserver_recordcnt, dynamic_buf.array() + offset, sizeof(int32_t));
	offset += sizeof(int32_t);

	for (int i = 0; i < tmpserver_recordcnt; i++) {
		// key
		netreach_key_t tmpkey;
		int tmpkeysize = tmpkey.deserialize(dynamic_buf.array() + offset, dynamic_buf.size() - offset);
		tmpserver_keyarray.push_back(tmpkey);
		offset += tmpkeysize;

		// value
		val_t tmpval;
		int tmpvalsize = tmpval.deserialize(dynamic_buf.array() + offset, dynamic_buf.size() - offset);
		tmpserver_valarray.push_back(tmpval);
		offset += tmpvalsize;

		// seq
		uint32_t tmpseq = 0;
		memcpy(&tmpseq, dynamic_buf.array() + offset, sizeof(uint32_t));
		tmpserver_seqarray.push_back(tmpseq);
		offset += sizeof(uint32_t);

		// stat
		bool tmpstat = false;
		memcpy(&tmpstat, dynamic_buf.array() + offset, sizeof(bool));
		tmpserver_statarray.push_back(tmpstat);
		offset += sizeof(bool);
	}
}

// FORMAT: <int num>, <Key k0, int seq0>, <Key k1, int seq1>, ...
static int dynamic_serialize_upstream_backup_notification(dynamic_array_t &dynamic_buf, std::vector<std::vector<netreach_key_t>> &perserver_keyarray, std::vector<std::vector<uint32_t>> &perserver_seqarray) {
	int offset = 0;

	// skip num
	offset += sizeof(int);

	int tmpnum = 0;
	for (int i = 0; i < perserver_keyarray.size(); i++) {
		for (int j = 0; j < perserver_keyarray[i].size(); j++) {
			netreach_key_t tmpkey = perserver_keyarray[i][j];
			uint32_t tmpseq = perserver_seqarray[i][j];

			// key
			int tmpkeysize = tmpkey.dynamic_serialize(dynamic_buf, offset);
			offset += tmpkeysize;

			// seq
			dynamic_buf.dynamic_memcpy(offset, (char *)&tmpseq, sizeof(uint32_t));
			offset += sizeof(uint32_t);

			tmpnum++;
		}
	}

	// go back to update num
	dynamic_buf.dynamic_memcpy(0, (char *)&tmpnum, sizeof(int));

	return offset;
}
