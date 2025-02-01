# README of benchmark/results

- In old-exp1-v1.md
  + Fix the timeout issue causing incorrect execution time and missing rotation results
    * Yet not fix the runtime variance issue causing unreasonable thpt/latency
  + val is before fixing the timeout issue
  + <u>val</u> is after fixing the timeout issue
- In old-exp1-v2.md
  + Fix the runtime variance issue causing unreasonable thpt/latency
    * Yet not fix the redundant latency issue causing unreasonable medium latency (too small)
  + val is after fixing the runtime variance issue
  + <u>val</u> is the result from old-exp1-v1.md, which may be re-run later
- In exp1.md (latest)
  + Fix the redundant latency issue causing unreasonable medium latency (too small)
