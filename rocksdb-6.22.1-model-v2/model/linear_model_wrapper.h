// Only for one model

#ifndef LINEAR_MODEL_WRAPPER_H
#define LINEAR_MODEL_WRAPPER_H

#include <stdint.h>
#include <vector>

#include "rocksdb/slice.h"
#include "xindex_model.h"

namespace ROCKSDB_NAMESPACE {

class ModelResult {
	public:
		uint32_t predict_idx;
		uint32_t error_bound;
};

class LinearModelWrapper {
	typedef VarlenLinearModel linear_model_t;
	public:
		LinearModelWrapper(const std::vector<std::string> &index_keys, 
				const std::vector<std::vector<std::string>> &data_keys_list);
		~LinearModelWrapper();
		ModelResult index_predict(const Slice &key) const;
		ModelResult data_predict(const Slice &key, const uint32_t &data_idx) const;
	private:
		linear_model_t index_model_; // # of index_models_ is index_model_n
		linear_model_t *data_models_; // # of linear models is data_block_n * data_model_n
		uint32_t data_block_n_;
};

}

#endif
