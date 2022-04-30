/*
 * The code is part of the XIndex project.
 *
 *    Copyright (C) 2020 Institute of Parallel and Distributed Systems (IPADS), Shanghai Jiao Tong University.
 *    All rights reserved.
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

#include <cassert>
#include <chrono>
#include <cstring>
#include <iostream>
#include <sstream>
#include <time.h>

#if !defined(HELPER_H)
#define HELPER_H

#define GET_STRING(v, this) { \
	std::stringstream ss; \
 	ss << this; \
	v = ss.str(); \
}

#define LOAD_RAW_WORKLOAD(buf, workload) \
	sprintf(buf, "%s-load.out", workload)

#define RUN_RAW_WORKLOAD(buf, workload) \
	sprintf(buf, "%s-run.out", workload)

#define LOAD_SPLIT_DIR(buf, workload, threadnum) \
	sprintf(buf, "%s-load-%d", workload, threadnum)

#define RUN_SPLIT_DIR(buf, workload, threadnum) \
	sprintf(buf, "%s-run-%d", workload, threadnum)

#define GET_SPLIT_WORKLOAD(buf, dir, threadid) \
	sprintf(buf, "%s/%u.out", dir, threadid);

#define GET_DATAPATH(buf, workload_name, group_idx) \
	sprintf(buf, "/tmp/netbuffer/%s/group%d.db", workload_name, group_idx)

#define GET_BUFFERPATH(buf, workload_name, group_idx, buffer_id) \
	sprintf(buf, "/tmp/netbuffer/%s/buffer%d-%d.db", workload_name, group_idx, buffer_id)

#define GET_ROOT(buf, workload_name)  \
	sprintf(buf, "%s-root.out", workload_name)

// us
//#define CUR_TIME() (double) clock() / CLOCKS_PER_SEC * 1000.0 * 1000.0

// s + ns
#define CUR_TIME(t) clock_gettime(CLOCK_REALTIME, &t)
// t3 = t1 - t2: timespecsub(t1, t2, t3)
#define DELTA_TIME(t1, t2, t3) \
	do { \
		(t3).tv_sec = (t1).tv_sec - (t2).tv_sec; \
		(t3).tv_nsec = (t1).tv_nsec - (t2).tv_nsec; \
		if ((t3).tv_nsec < 0) { \
			(t3).tv_sec--; \
			(t3).tv_nsec += 1000000000L; \
		} \
	} while (0)
// t3 = t1 + t2: timespecadd(t1, t2, t3)
#define SUM_TIME(t1, t2, t3) \
	do { \
		(t3).tv_sec = (t1).tv_sec + (t2).tv_sec;\
		(t3).tv_nsec = (t1).tv_nsec + (t2).tv_nsec;\
		if ((t3).tv_nsec >= 1000000000L) {\
			(t3).tv_sec++;\
			(t3).tv_nsec -= 1000000000L;\
		}\
	} while (0)
// s + ns -> us
#define GET_MICROSECOND(t) t.tv_sec * 1000 * 1000 + double(t.tv_nsec) / 1000.0

#define COUT_THIS(this) std::cout << this << std::endl;
#define COUT_VAR(this) std::cout << #this << ": " << this << std::endl;
#define COUT_POS() COUT_THIS("at " << __FILE__ << ":" << __LINE__)
#define COUT_N_EXIT(msg) \
  COUT_THIS(msg);        \
  COUT_POS();            \
  abort();

// 1: assert
// 0: ignore assert
#if 1
#define DEBUG_ASSERT
#endif
#ifdef DEBUG_ASSERT
#define INVARIANT(cond)            \
  if (!(cond)) {                   \
    COUT_THIS(#cond << " failed"); \
    COUT_POS();                    \
    abort();                       \
  }
#else
#define INVARIANT(cond)
#endif

// 0: print information of background retraining thread
// 1: not print
#if 1
#define NDEBUGGING
#endif

// 0: print information of main thread
// 1: not print
#if 1
#define NDEBUGGING_LOG
#endif

// 1: test dpdk polling time to reduce it from normal RTT
// 0: not test
#if 0
#define TEST_DPDK_POLLING
#endif

// 1: test aggregate throughput
// 0: not test
#if 1
#define TEST_AGG_THPT
#endif

// 1: original xindex with fixed 8B value and no snapshot
// 0: extended version with variable length value and snapshot
#if 0
#define ORIGINAL_XINDEX
#endif

#ifndef ORIGINAL_XINDEX
// v1: extended_xindex: max memory for varlen value + multi-versioning snapshot

// v2: extended_xindex_dynamic: extended_xindex + dynamic memory for varlen value (xindex_util.h)
// 1: use dynamic memory for variable-length value to save space by read-write locking
// 0: use max memory for vaiable-length value to trade space for performance by optimistic locking
#if 1
#define DYNAMIC_MEMORY
#endif

// v3: extended_xindex_dynamic_seq: extended_xindex_dynamic + seq mechanism for serializability (xindex*.h)
// 1: use seq mechanism for serizability under potential packet loss
#ifdef DYNAMIC_MEMORY
#if 1
#define SEQ_MECHANISM
#endif
#endif

// For each version: affect extended_xindex/xindex_group*.h + xindex_root*.h
// 1: use bloom filter to optimize operation on newly inserted data in buffer
#if 0
#define BF_OPTIMIZATION
#endif
#endif

#if defined(NDEBUGGING)
#define DEBUG_THIS(this)
#else
#define DEBUG_THIS(this) std::cerr << this << std::endl
#endif

#if defined(NDEBUGGING_LOG)
#define FDEBUG_THIS(ofs, this)
#else
#define FDEBUG_THIS(ofs, this) ofs << this << std::endl
#endif

#define UNUSED(var) ((void)var)

#define CACHELINE_SIZE (1 << 6)

#define PACKED __attribute__((packed))

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

#define MAX_BUFSIZE 4096 // 4KB; TODO: increase for large value
#define MAX_LARGE_BUFSIZE 8388608 // 8MB
#define MAX_LCORE_NUM 72

#define MQ_SIZE 256 // in-memory message queue
#define SINGLE_MQ_SIZE 2 // single-message message queue

inline void memory_fence() { asm volatile("mfence" : : : "memory"); }

/** @brief Compiler fence.
 * Prevents reordering of loads and stores by the compiler. Not intended to
 * synchronize the processor's caches. */
inline void fence() { asm volatile("" : : : "memory"); }

inline uint64_t cmpxchg(uint64_t *object, uint64_t expected,
                               uint64_t desired) {
  asm volatile("lock; cmpxchgq %2,%1"
               : "+a"(expected), "+m"(*object)
               : "r"(desired)
               : "cc");
  fence();
  return expected;
}

inline uint8_t cmpxchgb(uint8_t *object, uint8_t expected,
                               uint8_t desired) {
  asm volatile("lock; cmpxchgb %2,%1"
               : "+a"(expected), "+m"(*object)
               : "r"(desired)
               : "cc");
  fence();
  return expected;
}

#endif  // HELPER_H
