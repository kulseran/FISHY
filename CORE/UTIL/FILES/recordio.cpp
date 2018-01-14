#include "recordio.h"

#include <CORE/ARCH/endian.h>

namespace core {
namespace util {
namespace files {

/**
 * Input stream adapter for a Record based file.
 */
InRecordIOStream::InRecordIOStream(std::istream &file) : m_file(file) {
}

/**
 *
 */
Status InRecordIOStream::readNextRecord(core::memory::Blob &record) {
  u32 recordSz = 0;
  m_file.read(reinterpret_cast< char * >(&recordSz), sizeof(u32));
  if (m_file.gcount() != sizeof(u32) || m_file.fail()) {
    return Status::NOT_FOUND;
  }
  recordSz = core::endian::little(recordSz);
  if (record.size() < recordSz) {
    return Status::OUT_OF_BOUNDS;
  }
  m_file.read(reinterpret_cast< char * >(record.data()), recordSz);
  if (m_file.gcount() == recordSz || m_file.fail()) {
    record = core::memory::Blob(record.data(), recordSz);
    return Status::OK;
  }
  return Status::NOT_FOUND;
}

/**
 *
 */
Status InRecordIOStream::sizeNextRecord(u32 &sz) {
  const std::istream::pos_type pos = m_file.tellg();
  m_file.read(reinterpret_cast< char * >(&sz), sizeof(u32));
  if (m_file.gcount() != sizeof(u32) || m_file.fail()) {
    return Status::NOT_FOUND;
  }
  m_file.seekg(pos);

  sz = core::endian::little(sz);
  return Status::OK;
}

/**
 *
 */
Status InRecordIOStream::skipForward(const u32 recordCount) {
  for (u32 i = 0; i < recordCount; ++i) {
    u32 recordSz = 0;
    m_file.read(reinterpret_cast< char * >(&recordSz), sizeof(u32));
    if (m_file.gcount() != sizeof(u32) || m_file.fail()) {
      return Status::NOT_FOUND;
    }
    recordSz = core::endian::little(recordSz);
    m_file.seekg(recordSz, std::ios::cur);
  }
  return Status::OK;
}

/**
 *
 */
void InRecordIOStream::reset() {
  m_file.seekg(0, std::ios::beg);
}

/**
 *
 */
OutRecordIOStream::OutRecordIOStream(std::ostream &file) : m_file(file) {
}

/**
 *
 */
Status OutRecordIOStream::appendRecord(const core::memory::ConstBlob &record) {
  if (record.size() >= std::numeric_limits< u32 >::max()) {
    return Status::BAD_ARGUMENT;
  }

  const u32 szOut = static_cast< u32 >(core::endian::little(record.size()));
  m_file.write(reinterpret_cast< const char * >(&szOut), sizeof(u32));
  if (m_file.fail()) {
    return Status::GENERIC_ERROR;
  }
  m_file.write(
      reinterpret_cast< const char *const >(record.data()), record.size());
  if (m_file.fail()) {
    return Status::GENERIC_ERROR;
  }
  return Status::OK;
}

} // namespace files
} // namespace util
} // namespace core
