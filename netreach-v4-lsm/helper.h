#include <cassert>
#include <chrono>
#include <cstring>
#include <iostream>
#include <sstream>
#include <time.h>
#include <execinfo.h> // backtrace
#include <unistd.h>

#if !defined(HELPER_H)
#define HELPER_H

#define WARMUP_RAW_WORKLOAD(buf, workload) \
	sprintf(buf, "%s-warmup.out", workload)

#define LOAD_RAW_WORKLOAD(buf, workload) \
	sprintf(buf, "%s-load.out", workload)

#define RUN_RAW_WORKLOAD(buf, workload) \
	sprintf(buf, "%s-run.out", workload)

#define LOAD_SPLIT_DIR(buf, workload, threadnum) \
	sprintf(buf, "%s-load-%d", workload, threadnum)

#define RUN_SPLIT_DIR(buf, workload, threadnum) \
	sprintf(buf, "%s-run-%d", workload, threadnum)

#define LOAD_SPLIT_WORKLOAD(buf, dir, threadid, loaderid) \
	sprintf(buf, "%s/%u-%u.out", dir, threadid, loaderid);

#define RUN_SPLIT_WORKLOAD(buf, dir, threadid) \
	sprintf(buf, "%s/%u.out", dir, threadid);

#define GET_DATAPATH(buf, workload_name, group_idx) \
	sprintf(buf, "/tmp/netbuffer/%s/group%d.db", workload_name, group_idx)

#define GET_BUFFERPATH(buf, workload_name, group_idx, buffer_id) \
	sprintf(buf, "/tmp/netbuffer/%s/buffer%d-%d.db", workload_name, group_idx, buffer_id)

#define GET_ROOT(buf, workload_name)  \
	sprintf(buf, "%s-root.out", workload_name)

/***** Workload *****/

//#define USE_YCSB
#define USE_SYNTHETIC
//#define USE_TPCC

/***** Workload *****/

//#define USE_HASH
#define USE_RANGE

/***** Timing *****/

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

/***** Dumping *****/

#define COUT_THIS(this) std::cout << this << std::endl;
#define COUT_VAR(this) std::cout << #this << ": " << this << std::endl;
#define COUT_POS() COUT_THIS("at " << __FILE__ << ":" << __LINE__)
#define COUT_N_EXIT(msg) \
  COUT_THIS(msg);        \
  COUT_POS();            \
  abort();

static inline void dump_buf(char *buf, uint32_t bufsize)
{
	for (uint32_t byteidx = 0; byteidx < bufsize; byteidx++) {
		printf("0x%02x ", uint8_t(buf[byteidx]));
	}
	printf("\n");
}

/***** Debuging *****/

//#define DEBUG_SNAPSHOT

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
	print_stacktrace(); \
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
#if defined(NDEBUGGING)
#define DEBUG_THIS(this)
#else
#define DEBUG_THIS(this) std::cerr << this << std::endl
#endif

// 0: print information of main thread
// 1: not print
#if 1
#define NDEBUGGING_LOG
#endif
#if defined(NDEBUGGING_LOG)
#define FDEBUG_THIS(ofs, this)
#else
#define FDEBUG_THIS(ofs, this) ofs << this << std::endl
#endif

/***** Others *****/

#define GET_STRING(v, this) { \
	std::stringstream ss; \
 	ss << this; \
	v = ss.str(); \
}

#define UNUSED(var) ((void)var)

#define CACHELINE_SIZE (1 << 6)

#define PACKED __attribute__((packed))

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

#define MAX_BUFSIZE 40960 // 40KB; TODO: increase for large value / more threads
#define MAX_LARGE_BUFSIZE 8388608 // 8MB

#define UDP_DEFAULT_RCVBUFSIZE 212992 // 208KB used in linux by default
#define UDP_LARGE_RCVBUFSIZE 8388608 // 8MB (used by socket to receive large data; client_num + 1 for controller + server_num)

#define MQ_SIZE 40960 // in-memory message queue
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

static inline void print_stacktrace()
{
    int size = 16;
    void * array[size];
    int stack_num = backtrace(array, size);
    char ** stacktrace = backtrace_symbols(array, stack_num);
    for (int i = 0; i < stack_num; ++i)
    {
        printf("%s\n", stacktrace[i]);
    }
    free(stacktrace);
}

#endif  // HELPER_H
