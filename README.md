libsniper is a small, very fast C++ framework for writing highload HTTP services (f.e. RTB services)
------------------------------------------------

Currently libsniper is successfully used by several companies:
- as framework for DSP (Demand Side Platform) at AdSniper / MediaSniper. Serves more than 300K qps.
- as framework for MetaHash Peer Node. Serves more than 150K tps.

#### Features

* Supported HTTP/1.x protocol with HTTP pipelining
* Keep-alive and slow requests handling
* Support WaitGroup inspired by Go
* Client/Server library
* Graceful server shutdown

#### [TechEmpower benchmark](https://github.com/TechEmpower/FrameworkBenchmarks/tree/master/frameworks/C%2B%2B/libsniper)



#### Performance
**CPU**: 2x Intel Xeon CPU E5-2630 v4 @ 2.20GHz

Pipeline mode: 16 requests

```
---------------------------------------------------------
 Concurrency: 1024 for plaintext

 wrk -H 'Host: tfb-server' \
 -H 'Accept: text/plain,text/html;q=0.9,application/xhtml+xml;q=0.9,application/xml;q=0.8,*/*;q=0.7' \
 -H 'Connection: keep-alive' --latency -d 15 -c 1024 --timeout 8 \
 -t 40 http://tfb-server:8090/plaintext -s pipeline.lua -- 16
---------------------------------------------------------

Running 15s test @ http://tfb-server:8090/plaintext
  40 threads and 1024 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency     4.05ms    4.15ms  72.86ms   86.29%
    Req/Sec   142.62k    34.15k  315.64k    73.09%
  Latency Distribution
     50%    2.73ms
     75%    5.72ms
     90%    9.52ms
     99%   18.64ms
  86549168 requests in 15.10s, 12.01GB read
Requests/sec: 5732611.09
Transfer/sec:    814.59MB
```
