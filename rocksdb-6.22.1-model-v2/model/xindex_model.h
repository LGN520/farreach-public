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

#include <vector>
#include <cassert>
#include "mkl.h"
#include "mkl_lapacke.h"
#include "model/linear_model_wrapper.h"
#include "include/rocksdb/slice.h"

#if !defined(XINDEX_MODEL_H)
#define XINDEX_MODEL_H

namespace ROCKSDB_NAMESPACE {

class VarlenLinearModel {
  friend class LinearModelWrapper;
  typedef std::vector<double> model_key_t;
 public:
  ~VarlenLinearModel();
  void prepare(const std::vector<std::string> &keys,
               const std::vector<size_t> &positions);
  void prepare(const typename std::vector<std::string>::const_iterator &keys_begin, uint32_t size);
  void prepare_model(const std::vector<double *> &model_key_ptrs,
                     const std::vector<size_t> &positions);
  size_t predict(const rocksdb::Slice &curkey) const;

 private:
  uint32_t get_max_key_len(const std::vector<std::string> &keys);
  uint32_t get_max_key_len(const typename std::vector<std::string>::const_iterator &keys_begin, uint32_t size);
  size_t get_error_bound(const std::vector<std::string> &keys,
                         const std::vector<size_t> &positions);
  //size_t get_error_bound(const std::vector<std::string> &keys,
  //                       const std::vector<size_t> &positions, const uint32_t limit);
  size_t get_error_bound(
      const typename std::vector<std::string>::const_iterator &keys_begin,
      uint32_t size);
  //size_t get_error_bound(
  //    const typename std::vector<std::string>::const_iterator &keys_begin,
  //    uint32_t size, const uint32_t limit);

  size_t error_bound = 0;
  size_t limit = 0;
  double *weights = nullptr; // weights of max_key_len + 1
  uint32_t max_key_len = 0;
};

}  // namespace xindex

#endif  // XINDEX_MODEL_H
