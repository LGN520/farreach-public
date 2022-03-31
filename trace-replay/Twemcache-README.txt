## Anonymized Cache Request Traces from Twitter Production

### Trace Overview
This repository describes the traces from Twitter's in-memory caching ([Twemcache](https://github.com/twitter/twemcache)/[Pelikan](https://github.com/twitter/pelikan)) clusters. The current traces were collected from 54 clusters in Mar 2020. The traces are one-week-long. 
More details are described in the following paper. 
* [Juncheng Yang, Yao Yue, Rashmi Vinayak, A large scale analysis of hundreds of in-memory cache clusters at Twitter. _14th USENIX Symposium on Operating Systems Design and Implementation (OSDI 20)_, 2020](https://www.usenix.org/conference/osdi20/presentation/yang). 

---

### Trace Format 
The traces are compressed with [zstd](https://github.com/facebook/zstd), to decompress run `zstd -d /path/file`. 
The decompressed traces are plain text structured as comma-separated columns. Each row represents one request in the following format.


  * `timestamp`: the time when the cache receives the request, in sec 
  * `anonymized key`: the original key with anonymization 
  * `key size`: the size of key in bytes 
  * `value size`: the size of value in bytes 
  * `client id`: the anonymized clients (frontend service) who sends the request
  * `operation`: one of get/gets/set/add/replace/cas/append/prepend/delete/incr/decr 
  * `TTL`: the time-to-live (TTL) of the object set by the client, it is 0 when the request is not a write request.  


Note that during key anonymization, we preserve the namespaces, for example, if the anonymized key is `nz:u:eeW511W3dcH3de3d15ec`, the first two fields `nz` and `u` are namespaces, note that the namespaces are not necessarily delimited by `:`, different workloads use different delimiters with different number of namespaces. 


---

### Choice of traces for different evaluations 
For different evaluation purposes, we recommend the following clusters/workloads 

* **miss ratio related (admission, eviction)**: cluster52, cluster17 (low miss ratio), cluster18 (low miss ratio), cluster24, cluster44, cluster45, cluster29. 


* **write-heavy workloads**: cluster12, cluster15, cluster31, cluster37. 


* **TTL-related**: mix of small and large TTLs: cluster 52, cluster22, cluster25, cluster11; small TTLs only: cluster18, cluster19, cluster6, cluster7. 


---


### More information about each workload can be found at https://github.com/twitter/cache-trace 


### License
![Creative Commons CC-BY license](https://i.creativecommons.org/l/by/4.0/88x31.png)
The data and trace documentation are made available under the
[CC-BY](https://creativecommons.org/licenses/by/4.0/) license.
By downloading it or using them, you agree to the terms of this license.

- NOTES
	+ Cluster12: write-intensive (32M lines; 79% write -> 88% small write; 25M writes, 192 subsequent, 2 blocked)
