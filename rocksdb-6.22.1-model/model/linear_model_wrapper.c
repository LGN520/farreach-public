#include "model/linear_model_wrapper.h"

LinearModelWrapper::LinearModelWrapper(const std::vector<Slice> &index_keys,
		const std::vector<std::vector<Slice>> &data_keys_list) {
	data_block_n = data_keys_list.size();
	data_models_ = new linear_model_t[data_block_n];

	vector<size_t> index_positions(index_keys.size());
	for (size_t i = 0; i < index_keys.size(); i++) {
		index_positions[i] = i;
	}
	index_model_.prepare(index_keys, index_positions);

	for (size_t data_block_i = 0; data_block_i < data_keys_list.size(); data_block_i++) {
		vector<size_t> data_positions(data_keys_list[data_block_i].size());
		for (size_t i = 0; i < data_keys_list[data_block_i].size(); i++) {
			data_positions[i] = i;
		}
		data_models_[data_block_i].prepare(data_keys_list[data_block_i], data_positions);
	}
}

uint32_t LinearModelWrapper::index_predict(const Slice &key) {
	return index_model_.predict(key);
}

uint32_t LinearModelWrapper::index_error_bound() const {
	return index_model_.error_bound;
}

uint32_t LinearModelWrapper::data_predict(const Slice &key, const uint32_t &data_idx) {
	return data_models_[data_idx].predict(key);
}

uint32_t LinearModelWrapper::data_error_bound(const uint32_t &data_idx) const {
	return data_models_[data_idx].error_bound;
}

LinearModelWrapper::~LinearModelWrapper() {
	delete data_models_;
}
