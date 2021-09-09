#ifndef LINEAR_MODEL_WRAPPER_H
#define LINEAR_MODEL_WRAPPER_H

#include <stdint.h>
#include <vector>

#include "rocksdb/slice.h"
#include "xindex_model.h"
#include "xindex_model_impl.h"

namespace ROCKSDB_NAMESPACE {

class LinearModelWrapper {
	typedef VarlenLinearModel<Slice> linear_model_t;
	public:
		LinearModelWrapper(const std::vector<Slice> &index_keys, 
				const std::vector<std::vector<Slice>> &data_keys_list);
		~LinearModelWrapper();
		uint32_t index_predict(const Slice &key) const;
		uint32_t index_error_bound() const;
		uint32_t data_predict(const Slice &key, const uint32_t &data_idx) const;
		uint32_t data_error_bound(const uint32_t &data_idx) const;
	private:
		linear_model_t index_model_;	
		linear_model_t *data_models_; // linear models of data_block_n
		uint32_t data_block_n;
};

}

#endif
