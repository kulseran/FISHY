/**
 * Sinks to and from iostreams.
 */
#ifndef FISHY_SERIALIZER_STREAMSINK_H
#define FISHY_SERIALIZER_STREAMSINK_H

#include "serializer.h"

#include <iostream>

namespace core {
namespace base {

class InStreamSink : public iBinarySerializerSink {
  public:
  InStreamSink(std::istream &sink) : m_sink(sink), m_size(sizeStream(sink)) {}

  virtual size_t write(const ::core::memory::ConstBlob &b) {
    (void) b;
    CHECK_INVALID_OPERATION();
    return 0;
  }

  virtual size_t read(::core::memory::Blob &b) {
    const std::streamoff start = m_sink.tellg();
    m_sink.read(reinterpret_cast< char * >(b.data()), b.size());
    const std::streamoff end = m_sink.tellg();
    return (end - start);
  }

  virtual void seek(const size_t dist) { m_sink.seekg(dist, std::ios::cur); }

  virtual size_t avail() { return (m_size - m_sink.tellg()); }

  private:
  std::istream &m_sink;
  const size_t m_size;

  size_t sizeStream(std::istream &file) {
    const std::streamoff pos = file.tellg();
    file.seekg(0, std::ios::end);
    const std::streamoff end = file.tellg();
    file.seekg(pos, std::ios::beg);
    return (end - pos);
  }
};

class OutStreamSink : public iBinarySerializerSink {
  public:
  OutStreamSink(std::ostream &sink) : m_sink(sink) {}

  virtual size_t write(const ::core::memory::ConstBlob &b) {
    const std::streamoff before = m_sink.tellp();
    m_sink.write(reinterpret_cast< const char * >(b.data()), b.size());
    const std::streamoff after = m_sink.tellp();
    return (size_t)(after - before);
  }

  virtual size_t read(::core::memory::Blob &b) {
    (void) b;
    CHECK_INVALID_OPERATION();
    return 0;
  }

  virtual void seek(const size_t dist) { m_sink.seekp(dist, std::ios::cur); }

  virtual size_t avail() { return std::numeric_limits< size_t >::max(); }

  private:
  std::ostream &m_sink;
};

} // namespace base
} // namespace core

#endif
