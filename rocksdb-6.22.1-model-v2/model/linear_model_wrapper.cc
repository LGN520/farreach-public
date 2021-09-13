#include "model/linear_model_wrapper.h"

namespace ROCKSDB_NAMESPACE {

LinearModelWrapper::LinearModelWrapper(const std::vector<Slice> &index_keys,
		const std::vector<std::vector<Slice>> &data_keys_list,
		uint32_t index_model_n, uint32_t data_model_n) {
	data_block_n_ = data_keys_list.size();
	index_model_n_ = index_model_n;
	data_model_n_ = data_model_n;

	index_models_ = new linear_model_t[index_model_n_]; // index_model_n_
	index_pivots_ = new Slice[index_model_n_];
	data_models_list_ = new linear_model_t*[data_block_n_];
	data_pivots_list_ = new Slice*[data_block_n_];
	for (size_t i = 0; i < data_block_n_; i++) {
		data_models_list_[i] = new linear_model_t[data_model_n_];
		data_pivots_list_[i] = new Slice[data_model_n_];
	} // data_block_n_ * data_model_n_

	// Train index models
	uint32_t per_index_keys_n = index_keys.size() / index_model_n_;
	uint32_t tail_index_keys_n = per_index_keys_n + index_keys.size() % index_model_n_;
	std::vector<Slice>::const_iterator index_keys_iter = index_keys.begin();
	for (size_t i = 0; i < index_model_n_; i++) {
		index_pivots_[i] = *index_keys_iter;
		if (i != index_model_n_ - 1) {
			index_models_[i].prepare(index_keys_iter, per_index_keys_n);
			index_keys_iter += per_index_keys_n;
		}
		else {
			index_models_[i].prepare(index_keys_iter, tail_index_keys_n);
		}
		//printf("index_models[%u].error_bound = %u\n", uint32_t(i), uint32_t(index_models_[i].error_bound));
	}
	
	// Train data models
	for (size_t data_block_i = 0; data_block_i < data_block_n_; data_block_i++) {
		const std::vector<Slice> &data_keys = data_keys_list[data_block_i];
		linear_model_t *data_models_ = data_models_list_[data_block_i];
		Slice *data_pivots_ = data_pivots_list_[data_block_i];
		uint32_t per_data_keys_n = data_keys.size() / data_model_n_;
		uint32_t tail_data_keys_n = per_data_keys_n + data_keys.size() % data_model_n_;
		std::vector<Slice>::const_iterator data_keys_iter = data_keys.begin();
		for (size_t i = 0; i < data_model_n_; i++) {
			data_pivots_[i] = *data_keys_iter;
			if (i != data_model_n_ - 1) {
				data_models_[i].prepare(data_keys_iter, per_data_keys_n);
				data_keys_iter += per_data_keys_n;
			}
			else {
				data_models_[i].prepare(data_keys_iter, tail_data_keys_n);
			}
			//printf("data_models_list[%u][%u].error_bound = %u\n", uint32_t(data_block_i), uint32_t(i), uint32_t(data_models_[i].error_bound));
		}
	}

	// Legacy code for single model

	/*std::vector<size_t> index_positions(index_keys.size());
	for (size_t i = 0; i < index_keys.size(); i++) {
		index_positions[i] = i;
	}
	index_model_.prepare(index_keys, index_positions);*/

	/*for (size_t data_block_i = 0; data_block_i < data_keys_list.size(); data_block_i++) {
		std::vector<size_t> data_positions(data_keys_list[data_block_i].size());
		for (size_t i = 0; i < data_keys_list[data_block_i].size(); i++) {
			data_positions[i] = i;
		}
		data_models_[data_block_i].prepare(data_keys_list[data_block_i], data_positions);
	}*/
}

ModelResult LinearModelWrapper::index_predict(const Slice &key) const {
	ModelResult result;
	uint32_t cur_i = 0;
	uint32_t prev_i = 0;
	uint32_t index_model_i = 0;
	while (index_model_n_ > 1) {
		if (cur_i >= index_model_n_) {
			index_model_i = prev_i;
			break;
		}
		int cmp = key.compare(index_pivots_[cur_i]);
		if (cmp > 0) {
			prev_i = cur_i;
			cur_i++;
		}
		else if (cmp < 0) {
			index_model_i = prev_i;
			break;
		}
		else {
			index_model_i = cur_i;
			break;
		}
	}
	result.predict_idx = index_models_[index_model_i].predict(key);
	result.error_bound = index_models_[index_model_i].error_bound;
	return result;
} 

ModelResult LinearModelWrapper::data_predict(const Slice &key, const uint32_t &data_idx) const {
	if (unlikely(data_idx >= data_block_n_)) {
		printf("[LinearModelWrapper::data_predict] Invalid data idx %u >= # of data block %u\n", data_idx, data_block_n_);
		exit(-1);
	}
	ModelResult result;
	uint32_t cur_i = 0;
	uint32_t prev_i = 0;
	uint32_t data_model_i = 0;
	Slice *data_pivots_ = data_pivots_list_[data_idx];
	while (data_model_n_ > 1) {
		if (cur_i >= data_model_n_) {
			data_model_i = prev_i;
			break;
		}
		int cmp = key.compare(data_pivots_[cur_i]);
		if (cmp > 0) {
			prev_i = cur_i;
			cur_i++;
		}
		else if (cmp < 0) {
			data_model_i = prev_i;
			break;
		}
		else {
			data_model_i = cur_i;
			break;
		}
	}
	result.predict_idx = data_models_list_[data_idx][data_model_i].predict(key);
	result.error_bound = data_models_list_[data_idx][data_model_i].error_bound;
	return result;
}


LinearModelWrapper::~LinearModelWrapper() {
	delete index_models_;
	delete index_pivots_;
	for (uint32_t i = 0; i < data_block_n_; i++) {
		delete data_models_list_[i];
		delete data_pivots_list_[i];
	}
	delete data_models_list_;
	delete data_pivots_list_;
}

/*uint32_t LinearModelWrapper::index_predict(const Slice &key) const {
	size_t result = index_model_.predict(key);
	return uint32_t(result);
}

uint32_t LinearModelWrapper::index_error_bound() const {
	size_t result = index_model_.error_bound;
	return uint32_t(result);
}

uint32_t LinearModelWrapper::data_predict(const Slice &key, const uint32_t &data_idx) const {
	size_t result = data_models_[data_idx].predict(key);
	return uint32_t(result);
}

uint32_t LinearModelWrapper::data_error_bound(const uint32_t &data_idx) const {
	size_t result = data_models_[data_idx].error_bound;
	return uint32_t(result);
}

LinearModelWrapper::~LinearModelWrapper() {
	delete data_models_;
}*/

}
