// NetBuffer

#include "db/file_descriptor.h"

namespace ROCKSDB_NAMESPACE {
  
FileDescriptor::FileDescriptor() : FileDescriptor(0, 0, 0) {}

FileDescriptor::FileDescriptor(uint64_t number, uint32_t path_id, uint64_t _file_size)
      : FileDescriptor(number, path_id, _file_size, kMaxSequenceNumber, 0) {}

FileDescriptor::FileDescriptor(uint64_t number, uint32_t path_id, uint64_t _file_size,
                 SequenceNumber _smallest_seqno, SequenceNumber _largest_seqno)
      : table_reader(nullptr),
        packed_number_and_path_id(PackFileNumberAndPathId(number, path_id)),
        file_size(_file_size),
        smallest_seqno(_smallest_seqno),
        largest_seqno(_largest_seqno),
   		linear_model_wrapper_(nullptr) {}

FileDescriptor::FileDescriptor(const FileDescriptor& fd) { *this = fd; }

FileDescriptor& FileDescriptor::operator=(const FileDescriptor& fd) {
    table_reader = fd.table_reader;
    packed_number_and_path_id = fd.packed_number_and_path_id;
    file_size = fd.file_size;
    smallest_seqno = fd.smallest_seqno;
    largest_seqno = fd.largest_seqno;
	linear_model_wrapper_ = fd.linear_model_wrapper_;
    return *this;
  }

uint64_t FileDescriptor::GetNumber() const {
    return packed_number_and_path_id & kFileNumberMask;
  }
  
uint32_t FileDescriptor::GetPathId() const {
    return static_cast<uint32_t>(
        packed_number_and_path_id / (kFileNumberMask + 1));
  }
  
uint64_t FileDescriptor::GetFileSize() const { return file_size; }

}  // namespace ROCKSDB_NAMESPACE
