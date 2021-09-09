// NetBuffer

#pragma once
#include <algorithm>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "include/rocksdb/types.h"
#include "db/dbformat.h"

namespace ROCKSDB_NAMESPACE {

class TableReader;
class LinearModelWrapper;

constexpr uint64_t kFileNumberMask = 0x3FFFFFFFFFFFFFFF;

extern uint64_t PackFileNumberAndPathId(uint64_t number, uint64_t path_id);

// A copyable structure contains information needed to read data from an SST
// file. It can contain a pointer to a table reader opened for the file, or
// file number and size, which can be used to create a new table reader for it.
// The behavior is undefined when a copied of the structure is used when the
// file is not in any live version any more.
class FileDescriptor {
public:
  // Table reader in table_reader_handle
  TableReader* table_reader;
  uint64_t packed_number_and_path_id;
  uint64_t file_size;  // File size in bytes
  SequenceNumber smallest_seqno;  // The smallest seqno in this file
  SequenceNumber largest_seqno;   // The largest seqno in this file
  LinearModelWrapper* linear_model_wrapper; //NetBuffer

  FileDescriptor();

  FileDescriptor(uint64_t number, uint32_t path_id, uint64_t _file_size);

  FileDescriptor(uint64_t number, uint32_t path_id, uint64_t _file_size,
                 SequenceNumber _smallest_seqno, SequenceNumber _largest_seqno);

  FileDescriptor(const FileDescriptor& fd);

  FileDescriptor& operator=(const FileDescriptor& fd);

  uint64_t GetNumber() const;
  uint32_t GetPathId() const;
  uint64_t GetFileSize() const; 
};

}  // namespace ROCKSDB_NAMESPACE
