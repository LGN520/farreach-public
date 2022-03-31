IBM runs one of the world's leading object store cloud based service - IBM Cloud Object Storage.  We have taken anonymized traces from the IBM Cloud Object Storage service on the IBM Cloud and used them to study data flows in and out of the object store.  This insight we gained from the traces was pivotal in our research into large scale cache systems, some of which was published in HotStorage 2020 under the title "It's Time to Revisit LRU vs. FIFO" (https://www.usenix.org/system/files/hotstorage20_paper_eytan.pdf).


The data set is composed of 98 traces containing around 1.6 Billion requests for 342 Million unique objects.  The traces themselves are about 88 GB in size.  Each trace contains the REST operations issued against a single bucket in IBM Cloud Object Storage during a single week in 2019.  All the traces were collected during the same week in 2019.  The traces are names IBMObjectStoreTrace<trace_number>Part<part_number> where most traces are contained in a single file, and therefore have just one part, and four traces are broken into multiple parts. 


Each trace was selected based on a single criteria - that it contain some read (i.e. GET OBJECT) requests.  Each trace contains GET OBJECT, PUT OBJECT, HEAD OBJECT, DELETE OBJECT requests taken over a week long period, where each request includes a time stamp,  the request type, the object id, a starting offset and an ending offset, and the total object size.  Only successful requests (i.e. those requests that returned a return code of 200) are listed.  This data set is intended to enable studying cache behavior, and therefore requests that were not served are of no interest.  


Bucket names are omitted, and objects are represented as ids generated through a one-way hash function.  The format of the each trace record is <time stamp of request>, <request type>, <object ID>, <optional: size of object>, <optional: beginning offset>, <optional: ending offset>.  The timestamp is the number of milliseconds from the point where we began collecting the traces. For example:


1219008 REST.PUT.OBJECT 8d4fcda3d675bac9 1056
1221974 REST.HEAD.OBJECT 39d177fb735ac5df 528
1232437 REST.HEAD.OBJECT 3b8255e0609a700d 1456
1232488 REST.GET.OBJECT 95d363d3fbdc0b03 1168 0 1167
1234545 REST.GET.OBJECT bfc07f9981aa6a5a 528 0 527
1256364 REST.HEAD.OBJECT c27efddbeef2b638 12752
1256491 REST.HEAD.OBJECT 13943e909692962f 9760
1256556 REST.GET.OBJECT 884ba9b0c6d1fe97 23872 0 23871
1256584 REST.HEAD.OBJECT d86b7bfefc63995d 12592

- Subtraces
	+ Trace000: read-intensive (45M lines; 0.1% write -> 0.06% small write)
	+ Trace001: write-intensive (1.5M lines; 40% write -> 1.2% small write; 0.6M writes, 5 subsequent, 5 blocked)
	+ Trace002: read-intensive (0.38M lines; 0.03% write -> 5.5% small write)
	+ Trace003: read-intensive (1.5M lines; 0.8% write -> 46% small write)
	+ Trace004: read-intensive (10M lines; 0.2% write -> 0% small write)
	+ Trace005-part0: read-intensive (53M lines; 0% write -> 0% small write)
