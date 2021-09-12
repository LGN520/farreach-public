#ifndef LINEAR_MODEL_WRAPPER_H
#define LINEAR_MODEL_WRAPPER_H

#include <stdint.h>
#include <vector>

#include "rocksdb/slice.h"
#include "xindex_model.h"
#include "xindex_model_impl.h"

namespace ROCKSDB_NAMESPACE {

class ModelResult {
	public:
		uint32_t predict_idx;
		uint32_t error_bound;
};

class LinearModelWrapper {
	typedef VarlenLinearModel<Slice> linear_model_t;
	public:
		LinearModelWrapper(const std::vector<Slice> &index_keys, 
				const std::vector<std::vector<Slice>> &data_keys_list,
				uint32_t index_model_n = 1, uint32_t data_model_n = 10);
		~LinearModelWrapper();
		ModelResult index_predict(const Slice &key) const;
		ModelResult data_predict(const Slice &key, const uint32_t &data_idx) const;
		//uint32_t index_predict(const Slice &key) const;
		//uint32_t index_error_bound() const;
		//uint32_t data_predict(const Slice &key, const uint32_t &data_idx) const;
		//uint32_t data_error_bound(const uint32_t &data_idx) const;
	private:
		linear_model_t *index_models_; // # of index_models_ is index_model_n
		linear_model_t **data_models_list_; // # of linear models is data_block_n * data_model_n
		Slice *index_pivots_; // save the smallest key of each index model in index block
		Slice **data_pivots_list_; // save the smallest key of each data model in each data block
		uint32_t data_block_n_;
		uint32_t index_model_n_;
		uint32_t data_model_n_;
};

}

#endif
