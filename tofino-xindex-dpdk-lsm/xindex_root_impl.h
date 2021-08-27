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
#include <unordered_map>

#include "xindex_root.h"

#if !defined(XINDEX_ROOT_IMPL_H)
#define XINDEX_ROOT_IMPL_H

namespace xindex {

template <class key_t, class val_t, bool seq>
Root<key_t, val_t, seq>::~Root() {
	for (size_t group_i = 0; group_i < group_n; group_i++) {
		delete groups[group_i].second;
	}
}

template <class key_t, class val_t, bool seq>
void Root<key_t, val_t, seq>::init(const std::vector<key_t> &keys,
                                   const std::vector<val_t> &vals, uint32_t group_n) {
  INVARIANT(seq == false);

  size_t record_n = keys.size();

  // use the found group_n_trial to initialize groups
  this->group_n = group_n;
  groups = std::make_unique<std::pair<key_t, group_t *volatile>[]>(group_n);
  size_t records_per_group = record_n / group_n;
  size_t trailing_record_n = record_n - records_per_group * group_n;
  size_t previous_end_i = 0;
  for (size_t group_i = 0; group_i < group_n; group_i++) {
    size_t begin_i = previous_end_i;
    size_t end_i = previous_end_i + records_per_group;
    end_i = end_i > record_n ? record_n : end_i;
    if (trailing_record_n > 0) {
      end_i++;
      trailing_record_n--;
    }
    previous_end_i = end_i;
    INVARIANT((group_i == group_n - 1 && end_i == record_n) ||
              group_i < group_n - 1);

    groups[group_i].first = keys[begin_i];
    groups[group_i].second = new group_t();
    groups[group_i].second->init(keys.begin() + begin_i, vals.begin() + begin_i,
                                 end_i - begin_i, group_i);
  }

  // Disable OS page cache
  //system("hdparm -W 0 /tmp/netbuffer");

  // then decide # of 2nd stage model of root RMI
  adjust_rmi();

  DEBUG_THIS("--- [root] final XIndex Paramater: group_n = "
             << group_n << ", rmi_2nd_stage_model_n=" << rmi_2nd_stage_model_n);
}

/*
 * Root::calculate_err
 */
template <class key_t, class val_t, bool seq>
void Root<key_t, val_t, seq>::calculate_err(const std::vector<key_t> &keys,
                                            const std::vector<val_t> &vals,
                                            size_t group_n_trial,
                                            double &err_at_percentile,
                                            double &max_err, double &avg_err) {
  double access_percentage = 0.9;
  size_t record_n = keys.size();
  avg_err = 0;
  err_at_percentile = 0;
  max_err = 0;

  std::vector<double> errors;

  size_t records_per_group = record_n / group_n_trial;
  size_t trailing_record_n = record_n - records_per_group * group_n_trial;
  size_t previous_end_i = 0;
  for (size_t group_i = 0; group_i < group_n_trial; group_i++) {
    size_t begin_i = previous_end_i;
    size_t end_i = previous_end_i + records_per_group;
    end_i = end_i > record_n ? record_n : end_i;
    if (trailing_record_n > 0) {
      end_i++;
      trailing_record_n--;
    }
    previous_end_i = end_i;
    INVARIANT((group_i == group_n_trial - 1 && end_i == record_n) ||
              group_i < group_n_trial - 1);

    linear_model_t model;
    model.prepare(keys.begin() + begin_i, end_i - begin_i);
    double e = model.get_error_bound(keys.begin() + begin_i, end_i - begin_i);
    errors.push_back(e);
    avg_err += e;
  }
  avg_err /= group_n_trial;

  // check whether the erros satisfy the specified performance requirement
  std::sort(errors.begin(), errors.end());
  err_at_percentile = errors[(size_t)(errors.size() * access_percentage)];
  max_err = errors[errors.size() - 1];
}

/*
 * Root::get
 */
template <class key_t, class val_t, bool seq>
inline result_t Root<key_t, val_t, seq>::get(const key_t &key, val_t &val) {
  return locate_group(key)->get(key, val);
}

/*
 * Root::put
 */
template <class key_t, class val_t, bool seq>
inline result_t Root<key_t, val_t, seq>::put(const key_t &key, const val_t &val) {
  return locate_group(key)->put(key, val);
}

/*
 * Root::remove
 */
template <class key_t, class val_t, bool seq>
inline result_t Root<key_t, val_t, seq>::remove(const key_t &key) {
  return locate_group(key)->remove(key);
}

template <class key_t, class val_t, bool seq>
inline size_t Root<key_t, val_t, seq>::scan(
    const key_t &begin, const size_t n,
    std::vector<std::pair<key_t, val_t>> &result) {
  /*size_t remaining = n;
  result.clear();
  result.reserve(n);
  key_t next_begin = begin;
  key_t latest_group_pivot = key_t::min();  // for cross-slot chained groups

  int group_i;
  group_t *group = locate_group_pt2(begin, locate_group_pt1(begin, group_i));
  while (remaining && group_i < (int)group_n) {
    while (remaining && group &&
           group->get_pivot() > latest_group_pivot) { // avoid re-entry
      size_t done = group->scan(next_begin, remaining, result);
      assert(done <= remaining);
      remaining -= done;
      latest_group_pivot = group->get_pivot();
      next_begin = key_t::min();  // though don't know the exact begin
      group = group->next;
    }
    group_i++;
    group = groups[group_i].second;
  }

  return n - remaining;*/

  // TODO
  return 0;
}

template <class key_t, class val_t, bool seq>
inline size_t Root<key_t, val_t, seq>::range_scan(
    const key_t &begin, const key_t &end,
    std::vector<std::pair<key_t, val_t>> &result) {
  COUT_N_EXIT("not implemented yet");
}

template <class key_t, class val_t, bool seq>
void *Root<key_t, val_t, seq>::do_adjustment(void *args) {
  volatile bool &should_update_array = ((BGInfo *)args)->should_update_array;
  std::atomic<bool> &started = ((BGInfo *)args)->started;
  std::atomic<bool> &finished = ((BGInfo *)args)->finished;
  size_t bg_i = (((BGInfo *)args)->bg_i);
  size_t bg_num = (((BGInfo *)args)->bg_n);
  volatile bool &running = ((BGInfo *)args)->running;

  while (running) {
    sleep(1);
    if (started) {
      started = false;

      // read the current root ptr and bg thread's responsible range
      Root &root = **(Root * volatile *)(((BGInfo *)args)->root_ptr);
      size_t begin_group_i = bg_i * root.group_n / bg_num;
      size_t end_group_i = bg_i == bg_num - 1
                               ? root.group_n
                               : (bg_i + 1) * root.group_n / bg_num;

      // iterate through the array, and do maintenance
      size_t compact = 0;
      size_t buf_size = 0, cnt = 0;
      for (size_t group_i = begin_group_i; group_i < end_group_i; group_i++) {
        if (group_i % 50000 == 0) {
          DEBUG_THIS("----- [structure update] doing group_i=" << group_i);
        }

        group_t *volatile *group = &(root.groups[group_i].second);
        while (*group != nullptr) {
          // check for group split/merge, if not, do compaction
		  size_t buffer_size = (*group)->buffer_size;
          buf_size += buffer_size;
          cnt++;
          group_t *old_group = (*group);
          if (buffer_size > config.buffer_compact_threshold) {
            // DEBUG_THIS("------ [compaction], buf_size="
            //            << buffer_size << ", group_i=" << group_i);

			// TODO: Compact
            /*group_t *new_group = old_group->compact_phase_1();
            *group = new_group;
            memory_fence();
            rcu_barrier();
            compact++;
            new_group->compact_phase_2();
            memory_fence();
            rcu_barrier();  // make sure no one is accessing the old data
            old_group->free_data();
            old_group->free_buffer();
            delete old_group;*/
          }

          // do next (in the chain)
          group = &((*group)->next);
        }
      }

      finished = true;
      DEBUG_THIS("------ [structure update] compact_n: " << compact);
      DEBUG_THIS("------ [structure update] buf_size/cnt: " << 1.0 * buf_size /
                                                                   cnt);
      DEBUG_THIS("------ [structure update] done with "
                 << begin_group_i << "-" << end_group_i << " (" << cnt << ")");
    }
  }
  return nullptr;
}

template <class key_t, class val_t, bool seq>
Root<key_t, val_t, seq> *Root<key_t, val_t, seq>::create_new_root() {
  Root *new_root = new Root();

  size_t new_group_n = 0;
  for (size_t group_i = 0; group_i < group_n; group_i++) {
    group_t *group = groups[group_i].second;
    while (group != nullptr) {
      new_group_n++;
      group = group->next;
    }
  }

  DEBUG_THIS("--- [root] update root array. old_group_n="
             << group_n << ", new_group_n=" << new_group_n);
  new_root->group_n = new_group_n;
  new_root->groups = std::make_unique<std::pair<key_t, group_t *volatile>[]>(
      new_root->group_n);

  size_t new_group_i = 0;
  for (size_t group_i = 0; group_i < group_n; group_i++) {
    group_t *group = groups[group_i].second;
    while (group != nullptr) {
      new_root->groups[new_group_i].first = group->get_pivot();
      new_root->groups[new_group_i].second = group;
      group = group->next;
      new_group_i++;
    }
  }

  for (size_t group_i = 0; group_i < new_root->group_n - 1; group_i++) {
    assert(new_root->groups[group_i].first <
           new_root->groups[group_i + 1].first);
  }

  new_root->rmi_1st_stage = rmi_1st_stage;
  new_root->rmi_2nd_stage = rmi_2nd_stage;
  new_root->rmi_2nd_stage_model_n = rmi_2nd_stage_model_n;
  new_root->adjust_rmi();

  return new_root;
}

template <class key_t, class val_t, bool seq>
void Root<key_t, val_t, seq>::trim_root() {
  for (size_t group_i = 0; group_i < group_n; group_i++) {
    group_t *group = groups[group_i].second;
    if (group_i != group_n - 1) {
      assert(group->next ? group->next == groups[group_i + 1].second : true);
    }
    group->next = nullptr;
  }
}

template <class key_t, class val_t, bool seq>
void Root<key_t, val_t, seq>::adjust_rmi() {
  size_t max_model_n = config.root_memory_constraint / sizeof(linear_model_t);
  size_t max_trial_n = 10;

  size_t model_n_trial = rmi_2nd_stage_model_n;
  if (model_n_trial == 0) {
    max_trial_n = 100;
    const size_t group_n_per_model_per_rmi_error_experience_factor = 4;
    model_n_trial = std::min(
        max_model_n,         // do not exceed memory constraint
        std::max((size_t)1,  // do not decrease to zero
                 (size_t)(group_n / config.root_error_bound /
                          group_n_per_model_per_rmi_error_experience_factor)));
  }

  train_rmi(model_n_trial);
  size_t model_n_trial_prev_prev = 0;
  size_t model_n_trial_prev = model_n_trial;

  size_t trial_i = 0;
  double mean_error = 0;
  for (; trial_i < max_trial_n; trial_i++) {
    std::vector<double> errors;
    for (size_t group_i = 0; group_i < group_n; group_i++) {
      errors.push_back(
          std::abs((double)group_i - predict(groups[group_i].first)) + 1);
    }
    mean_error =
        std::accumulate(errors.begin(), errors.end(), 0.0) / errors.size();

    if (mean_error > config.root_error_bound) {
      if (rmi_2nd_stage_model_n == max_model_n) {
        break;
      }
      model_n_trial = std::min(
          max_model_n,  // do not exceed memory constraint
          std::max(rmi_2nd_stage_model_n + 1,  // but at least increase by 1
                   (size_t)(rmi_2nd_stage_model_n * mean_error /
                            config.root_error_bound)));
    } else if (mean_error < config.root_error_bound / 2) {
      if (rmi_2nd_stage_model_n == 1) {
        break;
      }
      model_n_trial = std::max(
          (size_t)1,                           // do not decrease to zero
          std::min(rmi_2nd_stage_model_n - 1,  // but at least decrease by 1
                   (size_t)(rmi_2nd_stage_model_n * mean_error /
                            (config.root_error_bound / 2))));
    } else {
      break;
    }

    train_rmi(model_n_trial);
    if (model_n_trial == model_n_trial_prev_prev) {
      break;
    }
    model_n_trial_prev_prev = model_n_trial_prev;
    model_n_trial_prev = model_n_trial;
  }

  DEBUG_THIS("--- [root] final rmi size: "
             << rmi_2nd_stage_model_n << " (error=" << mean_error << "), after "
             << trial_i << " trial(s)");
}

template <class key_t, class val_t, bool seq>
inline void Root<key_t, val_t, seq>::train_rmi(size_t rmi_2nd_stage_model_n) {
  this->rmi_2nd_stage_model_n = rmi_2nd_stage_model_n;
  delete[] rmi_2nd_stage;
  rmi_2nd_stage = new linear_model_t[rmi_2nd_stage_model_n]();

  // train 1st stage
  std::vector<key_t> keys(group_n);
  std::vector<size_t> positions(group_n);
  for (size_t group_i = 0; group_i < group_n; group_i++) {
    keys[group_i] = groups[group_i].first;
    positions[group_i] = group_i;
  }

  rmi_1st_stage.prepare(keys, positions);

  // train 2nd stage
  std::vector<std::vector<key_t>> keys_dispatched(rmi_2nd_stage_model_n);
  std::vector<std::vector<size_t>> positions_dispatched(rmi_2nd_stage_model_n);

  for (size_t key_i = 0; key_i < keys.size(); ++key_i) {
    size_t group_i_pred = rmi_1st_stage.predict(keys[key_i]);
    size_t next_stage_model_i = pick_next_stage_model(group_i_pred);
    keys_dispatched[next_stage_model_i].push_back(keys[key_i]);
    positions_dispatched[next_stage_model_i].push_back(positions[key_i]);
  }

  for (size_t model_i = 0; model_i < rmi_2nd_stage_model_n; ++model_i) {
    std::vector<key_t> &keys = keys_dispatched[model_i];
    std::vector<size_t> &positions = positions_dispatched[model_i];
    rmi_2nd_stage[model_i].prepare(keys, positions);
  }
}

template <class key_t, class val_t, bool seq>
size_t Root<key_t, val_t, seq>::pick_next_stage_model(size_t group_i_pred) {
  size_t second_stage_model_i;
  second_stage_model_i = group_i_pred * rmi_2nd_stage_model_n / group_n;

  if (second_stage_model_i >= rmi_2nd_stage_model_n) {
    second_stage_model_i = rmi_2nd_stage_model_n - 1;
  }

  return second_stage_model_i;
}

template <class key_t, class val_t, bool seq>
inline size_t Root<key_t, val_t, seq>::predict(const key_t &key) {
  size_t pos_pred = rmi_1st_stage.predict(key);
  size_t next_stage_model_i = pick_next_stage_model(pos_pred);
  return rmi_2nd_stage[next_stage_model_i].predict(key);
}

/*
 * Root::locate_group
 */
template <class key_t, class val_t, bool seq>
inline typename Root<key_t, val_t, seq>::group_t *
Root<key_t, val_t, seq>::locate_group(const key_t &key) {
  int group_i;  // unused
  group_t *head = locate_group_pt1(key, group_i);
  return locate_group_pt2(key, head);
}

template <class key_t, class val_t, bool seq>
inline typename Root<key_t, val_t, seq>::group_t *
Root<key_t, val_t, seq>::locate_group_pt1(const key_t &key, int &group_i) {
  group_i = predict(key);
  group_i = group_i > (int)group_n - 1 ? group_n - 1 : group_i;
  group_i = group_i < 0 ? 0 : group_i;

  // exponential search
  int begin_group_i, end_group_i;
  if (groups[group_i].first <= key) {
    size_t step = 1;
    begin_group_i = group_i;
    end_group_i = begin_group_i + step;
    while (end_group_i < (int)group_n && groups[end_group_i].first <= key) {
      step = step * 2;
      begin_group_i = end_group_i;
      end_group_i = begin_group_i + step;
    }  // after this while loop, end_group_i might be >= group_n
    if (end_group_i > (int)group_n - 1) {
      end_group_i = group_n - 1;
    }
  } else {
    size_t step = 1;
    end_group_i = group_i;
    begin_group_i = end_group_i - step;
    while (begin_group_i >= 0 && groups[begin_group_i].first > key) {
      step = step * 2;
      end_group_i = begin_group_i;
      begin_group_i = end_group_i - step;
    }  // after this while loop, begin_group_i might be < 0
    if (begin_group_i < 0) {
      begin_group_i = -1;
    }
  }

  // now group[begin].pivot <= key && group[end + 1].pivot > key
  // in loop body, the valid search range is actually [begin + 1, end]
  // (inclusive range), thus the +1 term in mid is a must
  // this algorithm produces index to the last element that is <= key
  while (end_group_i != begin_group_i) {
    // the "+2" term actually should be a "+1" after "/2", this is due to the
    // rounding in c++ when the first operant of "/" operator is negative
    int mid = (end_group_i + begin_group_i + 2) / 2;
    if (groups[mid].first <= key) {
      begin_group_i = mid;
    } else {
      end_group_i = mid - 1;
    }
  }
  // the result falls in [-1, group_n - 1]
  // now we ensure the pointer is not null
  group_i = end_group_i < 0 ? 0 : end_group_i;
  group_t *group = groups[group_i].second;
  while (group_i > 0 && group == nullptr) {
    group_i--;
    group = groups[group_i].second;
  }
  // however, we treat the pivot key of the 1st group as -inf, thus we return
  // 0 when the search result is -1
  assert(groups[0].second != nullptr);

  return group;
}

template <class key_t, class val_t, bool seq>
inline typename Root<key_t, val_t, seq>::group_t *
Root<key_t, val_t, seq>::locate_group_pt2(const key_t &key, group_t *begin) {
  group_t *group = begin;
  group_t *next = group->next;
  while (next != nullptr && next->get_pivot() <= key) {
    group = next;
    next = group->next;
  }
  return group;
}

}  // namespace xindex

#endif  // XINDEX_ROOT_IMPL_H
