/*
 * The code is part of the XIndex project.
 *
 *    Copyright (C) 2020 Institute of Parallel and Distributed Systems (IPADS),
 * Shanghai Jiao Tong University. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * For more about XIndex, visit:
 *     https://ppopp20.sigplan.org/details/PPoPP-2020-papers/13/XIndex-A-Scalable-Learned-Index-for-Multicore-Data-Storage
 */

#include "xindex_model.h"

#if !defined(XINDEX_MODEL_IMPL_H)
#define XINDEX_MODEL_IMPL_H

namespace ROCKSDB_NAMESPACE {

template <class key_t>
VarlenLinearModel<key_t>::~VarlenLinearModel() {
	delete weights;
}

template <class key_t>
void VarlenLinearModel<key_t>::prepare(const std::vector<key_t> &keys,
                                 const std::vector<size_t> &positions) {
  assert(keys.size() == positions.size());
  if (keys.size() == 0) return;

  max_key_len = get_max_key_len(keys);

  std::vector<model_key_t> model_keys(keys.size());
  std::vector<double *> key_ptrs(keys.size());
  for (uint32_t i = 0; i < keys.size(); i++) {
	model_keys[i].resize(max_key_len);
    keys[i].to_model_key(model_keys[i].data(), max_key_len);
    key_ptrs[i] = model_keys[i].data();
  }

  weights = new double[max_key_len + 1];

  prepare_model(key_ptrs, positions);
  error_bound = get_error_bound(keys, positions);
}

template <class key_t>
void VarlenLinearModel<key_t>::prepare(
    const typename std::vector<key_t>::const_iterator &keys_begin,
    uint32_t size) {
  if (size == 0) return;

  max_key_len = get_max_key_len(keys_begin, size);

  std::vector<model_key_t> model_keys(size);
  std::vector<double *> key_ptrs(size);
  std::vector<size_t> positions(size);
  for (size_t i = 0; i < size; i++) {
	model_keys[i].resize(max_key_len);
    (keys_begin + i)->to_model_key(model_keys[i].data(), max_key_len);
    key_ptrs[i] = model_keys[i].data();
    positions[i] = i;
  }

  weights = new double[max_key_len + 1];

  prepare_model(key_ptrs, positions);
  error_bound = get_error_bound(keys_begin, size);
}

template <class key_t>
void VarlenLinearModel<key_t>::prepare_model(
    const std::vector<double *> &model_key_ptrs,
    const std::vector<size_t> &positions) {
  if (positions.size() == 0) return;
  if (positions.size() == 1) {
    VarlenLinearModel<key_t>::weights[max_key_len] = positions[0];
    return;
  }

  if (max_key_len == 1) {  // use multiple dimension LR when running tpc-c
    double x_expected = 0, y_expected = 0, xy_expected = 0,
           x_square_expected = 0;
    for (size_t key_i = 0; key_i < positions.size(); key_i++) {
      double key = model_key_ptrs[key_i][0];
      x_expected += key;
      y_expected += positions[key_i];
      x_square_expected += key * key;
      xy_expected += key * positions[key_i];
    }
    x_expected /= positions.size();
    y_expected /= positions.size();
    x_square_expected /= positions.size();
    xy_expected /= positions.size();

    weights[0] = (xy_expected - x_expected * y_expected) /
                 (x_square_expected - x_expected * x_expected);
    weights[1] = (x_square_expected * y_expected - x_expected * xy_expected) /
                 (x_square_expected - x_expected * x_expected);
    return;
  }

  // trim down samples to avoid large memory usage
  size_t step = 1;
  /*size_t desired_training_key_n = 10000;
  if (model_key_ptrs.size() > desired_training_key_n) {
    step = model_key_ptrs.size() / desired_training_key_n;
  }*/

  std::vector<size_t> useful_feat_index;
  for (size_t feat_i = 0; feat_i < max_key_len; feat_i++) {
    double first_val = model_key_ptrs[0][feat_i];
    for (size_t key_i = 0; key_i < model_key_ptrs.size(); key_i += step) {
      if (model_key_ptrs[key_i][feat_i] != first_val) {
        useful_feat_index.push_back(feat_i);
        break;
      }
    }
  }
  if (model_key_ptrs.size() != 1 && useful_feat_index.size() == 0) {
	printf("all features are the same");
  }
  size_t useful_feat_n = useful_feat_index.size();
  bool use_bias = true;

  // we may need multiple runs to avoid "not full rank" error
  int fitting_res = -1;
  while (fitting_res != 0) {
    // use LAPACK to solve least square problem, i.e., to minimize ||b-Ax||_2
    // where b is the actual positions, A is inputmodel_keys
    int m = model_key_ptrs.size() / step;                  // number of samples
    int n = use_bias ? useful_feat_n + 1 : useful_feat_n;  // number of features
    double *a = (double *)malloc(m * n * sizeof(double));
    double *b = (double *)malloc(std::max(m, n) * sizeof(double));
    if (a == nullptr || b == nullptr) {
      printf("cannot allocate memory for matrix a or b");
	  exit(-1);
    }

    for (int sample_i = 0; sample_i < m; ++sample_i) {
      // we only fit with useful features
      for (size_t useful_feat_i = 0; useful_feat_i < useful_feat_n;
           useful_feat_i++) {
        a[sample_i * n + useful_feat_i] =
            model_key_ptrs[sample_i * step][useful_feat_index[useful_feat_i]];
      }
      if (use_bias) {
        a[sample_i * n + useful_feat_n] = 1;  // the extra 1
      }
      b[sample_i] = positions[sample_i * step];
      assert(sample_i * step < model_key_ptrs.size());
    }

    // fill the rest of b when m < n, otherwise nan value will cause failure
    for (int b_i = m; b_i < n; b_i++) {
      b[b_i] = 0;
    }

    fitting_res = LAPACKE_dgels(LAPACK_ROW_MAJOR, 'N', m, n, 1 /* nrhs */, a,
                                n /* lda */, b, 1 /* ldb, i.e. nrhs */);

    if (fitting_res > 0) {
      // now we need to remove one column in matrix a
      // note that fitting_res indexes starting with 1
      if ((size_t)fitting_res > useful_feat_index.size()) {
        use_bias = false;
      } else {
        size_t feat_i = fitting_res - 1;
        useful_feat_index.erase(useful_feat_index.begin() + feat_i);
        useful_feat_n = useful_feat_index.size();
      }

      if (useful_feat_index.size() == 0 && use_bias == false) {
        printf(
            "impossible! cannot fail when there is only 1 bias column in "
            "matrix a");
		exit(-1);
      }
    } else if (fitting_res < 0) {
      printf("%i-th parameter had an illegal value\n", -fitting_res);
      exit(-2);
    }

    // set weights to all zero
    for (size_t weight_i = 0; weight_i < (max_key_len+1); weight_i++) {
      weights[weight_i] = 0;
    }
    // set weights of useful features
    for (size_t useful_feat_i = 0; useful_feat_i < useful_feat_index.size();
         useful_feat_i++) {
      weights[useful_feat_index[useful_feat_i]] = b[useful_feat_i];
    }
    // set bias
    if (use_bias) {
      weights[max_key_len] = b[n - 1];
    }

    free(a);
    free(b);
  }
  assert(fitting_res == 0);
}

template <class key_t>
size_t VarlenLinearModel<key_t>::predict(const key_t &curkey) const {
  model_key_t model_key;
  model_key.resize(max_key_len);

  double *model_key_ptr = model_key.data();
  curkey.to_model_key(model_key_ptr, max_key_len);

  if (max_key_len == 1) {
    double res = weights[0] * *model_key_ptr + weights[1];
    return res > 0 ? res : 0;
  } else {
    double res = 0;
    for (size_t feat_i = 0; feat_i < max_key_len; feat_i++) {
      res += weights[feat_i] * model_key_ptr[feat_i];
    }
    res += weights[max_key_len];  // the bias term
    return res > 0 ? res : 0;
  }

  printf("Should not arrive here!");
  exit(-1);
  return 0;
}

template <class key_t>
uint32_t VarlenLinearModel<key_t>::get_max_key_len(
		const std::vector<key_t> &keys) {
	uint32_t result = 0;
	uint32_t tmp_len = 0;
	for (size_t key_i = 0; key_i < keys.size(); key_i++) {
		tmp_len = keys[key_i].get_key_len();
		if (tmp_len > result) {
			result = tmp_len;
		}
	}
	return result;
}

template <class key_t>
uint32_t VarlenLinearModel<key_t>::get_max_key_len(
    const typename std::vector<key_t>::const_iterator &keys_begin,
	uint32_t size) {
	typename std::vector<key_t>::const_iterator iter = keys_begin;
	uint32_t result = 0;
	uint32_t tmp_len = 0;
	for (size_t i = 0; i < size; i++) {
		tmp_len = iter->get_key_len();
		if (tmp_len > result) {
			result = tmp_len;
		}
		iter++;
	}
	return result;
}

template <class key_t>
size_t VarlenLinearModel<key_t>::get_error_bound(
    const std::vector<key_t> &keys, const std::vector<size_t> &positions) {
  int max = 0;

  for (size_t key_i = 0; key_i < keys.size(); ++key_i) {
    long long int pos_actual = positions[key_i];
    long long int pos_pred = predict(keys[key_i]);
    int error = std::abs(pos_actual - pos_pred);

    if (error > max) {
      max = error;
    }
  }

  return max;
}

template <class key_t>
size_t VarlenLinearModel<key_t>::get_error_bound(
    const typename std::vector<key_t>::const_iterator &keys_begin,
    uint32_t size) {
  int max = 0;

  for (size_t key_i = 0; key_i < size; ++key_i) {
    long long int pos_actual = key_i;
    long long int pos_pred = predict(*(keys_begin + key_i));
    int error = std::abs(pos_actual - pos_pred);

    if (error > max) {
      max = error;
    }
  }

  return max;
}

}  // namespace xindex

#endif  // XINDEX_MODEL_IMPL_H
