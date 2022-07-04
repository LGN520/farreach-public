

#include "config.h"
#include <iostream>
#include "bfn_pd_rpc_server.h"
#include <thrift/processor/TMultiplexedProcessor.h>

using namespace ::apache::thrift;

#ifdef HAVE_THRIFT_STDCXX_H
#include <thrift/stdcxx.h>
namespace stdcxx = ::apache::thrift::stdcxx;
#else
namespace stdcxx = boost;
#endif

using ::stdcxx::shared_ptr;

#include "p4_pd_rpc_server.ipp"

// processor needs to be of type TMultiplexedProcessor,
// I am keeping a void * pointer for 
// now, in case this function is called from C code
int add_to_rpc_server(void *processor) {
  std::cerr << "Adding Thrift service for P4 program netbufferv4 to server\n";

  shared_ptr<netbufferv4Handler> netbufferv4_handler(new netbufferv4Handler());

  TMultiplexedProcessor *processor_ = (TMultiplexedProcessor *) processor;
  processor_->registerProcessor(
    "netbufferv4",
    shared_ptr<TProcessor>(new netbufferv4Processor(netbufferv4_handler))
  );

  return 0;
}
int rmv_from_rpc_server(void *processor) {
  std::cerr << "Removing Thrift service for P4 program netbufferv4 from server\n";

  TMultiplexedProcessor *processor_ = (TMultiplexedProcessor *) processor;
  processor_->registerProcessor(
    "netbufferv4",
    shared_ptr<TProcessor>()
  );

  return 0;
}
