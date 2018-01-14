/**
 * Utility for reading and writing files containing a repeated set of records.
 * Each record is uniquely sized up to u32::max() bytes.
 *
 * (u32) record length
 * (u8) * record length bits of data
 * (u32) record lengh
 * (u8) * record length bits of data
 * ....
 *
 */
#ifndef FISHY_RECORDIO_H
#define FISHY_RECORDIO_H

#include <CORE/BASE/status.h>
#include <CORE/MEMORY/blob.h>
#include <CORE/types.h>

#include <iosfwd>
#include <string>

namespace core {
namespace util {
namespace files {

/**
 * Input stream adapter for a Record based file.
 */
class InRecordIOStream {
  public:
  InRecordIOStream(std::istream &);

  /**
   * Reads the next record from the file, advancing the read pointer.
   *
   * @param record the output record data
   * @returns Status::OK if a record was read
   */
  Status readNextRecord(core::memory::Blob &record);

  /**
   * Read the next record's size from the file, reseting the read pointer.
   *
   * @param records the size, if any
   * @returns Status::OK if a record existed
   */
  Status sizeNextRecord(u32 &sz);

  /**
   * Seeks forward the indicated number of records.
   *
   * @param recordCount the number of records to skip
   */
  Status skipForward(const u32 recordCount);

  /**
   * Resets the file pointer to the begining of the file.
   */
  void reset();

  private:
  std::istream &m_file;
};

/**
 * Output stream adapter for a Record based file.
 */
class OutRecordIOStream {
  public:
  OutRecordIOStream(std::ostream &file);

  /**
   * Appends the record content to the end of the file.
   *
   * @param record the bytes to write out
   * @return false on errors
   */
  Status appendRecord(const core::memory::ConstBlob &record);

  private:
  std::ostream &m_file;
};

} // namespace files
} // namespace util
} // namespace core

#endif
