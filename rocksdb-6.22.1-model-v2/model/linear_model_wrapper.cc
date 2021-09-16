#include "model/linear_model_wrapper.h"

namespace ROCKSDB_NAMESPACE {

LinearModelWrapper::LinearModelWrapper(const std::vector<std::string> &index_keys,
		const std::vector<std::vector<std::string>> &data_keys_list) {
	data_block_n_ = data_keys_list.size();

	data_models_ = new linear_model_t[data_block_n_];

	// Train index models
	std::vector<size_t> index_positions(index_keys.size());
	for (size_t i = 0; i < index_keys.size(); i++) {
		index_positions[i] = i;
	}
	index_model_.prepare(index_keys, index_positions);
	//printf("index_model.error_bound = %u, limit: %u\n", uint32_t(i), uint32_t(index_model_.error_bound), uint32_t(index_model_.limit));

	// Train data models
	for (size_t data_block_i = 0; data_block_i < data_keys_list.size(); data_block_i++) {
		std::vector<size_t> data_positions(data_keys_list[data_block_i].size());
		for (size_t i = 0; i < data_keys_list[data_block_i].size(); i++) {
			data_positions[i] = i;
		}
		data_models_[data_block_i].prepare(data_keys_list[data_block_i], data_positions);
		//printf("data_models_[%u].error_bound = %u, limit: %u\n", uint32_t(data_block_i), uint32_t(data_models_[data_block_i].error_bound), uint32_t(data_models_[data_block_i].limit));
	}
}

ModelResult LinearModelWrapper::index_predict(const Slice &key) const {
	ModelResult result;
	result.predict_idx = index_model_.predict(key);
	result.error_bound = index_model_.error_bound;
	return result;
} 

ModelResult LinearModelWrapper::data_predict(const Slice &key, const uint32_t &data_idx) const {
	if (unlikely(data_idx >= data_block_n_)) {
		printf("[LinearModelWrapper::data_predict] Invalid data idx %u >= # of data block %u\n", data_idx, data_block_n_);
		exit(-1);
	}

	ModelResult result;
	result.predict_idx = data_models_[data_idx].predict(key);
	result.error_bound = data_models_[data_idx].error_bound;
	return result;
}


LinearModelWrapper::~LinearModelWrapper() {
	delete data_models_;
}

}
