/**
 * Binary serialization basic sinks
 */
#ifndef FISHY_SERIALIZER_BASESINKS_H
#define FISHY_SERIALIZER_BASESINKS_H

#include <cstring>

namespace core {
namespace base {

/**
 * Sink that is usable to just size a given serialization.
 */
class FakeSink : public iBinarySerializerSink {
  public:
  FakeSink() : m_size(0) {}

  virtual size_t write(const ::core::memory::ConstBlob &b) {
    m_size += b.size();
    return b.size();
  }

  virtual size_t read(::core::memory::Blob &) {
    CHECK_INVALID_OPERATION();
    return 0;
  }

  virtual void seek(const size_t dist) { m_size += dist; }

  virtual size_t avail() { return 0; }

  /**
   * Return the currently serialized size in bytes.
   */
  size_t size() const { return m_size; }

  private:
  size_t m_size;
};

/**
 * Sink that provides a limited range over a parent sink.
 */
class RangeSink : public iBinarySerializerSink {
  public:
  RangeSink(iBinarySerializerSink &sink, size_t size)
      : m_sink(sink), m_size(size) {}

  virtual size_t write(const ::core::memory::ConstBlob &b) {
    const size_t sz = cap(b.size());
    return m_sink.write(::core::memory::ConstBlob(b.data(), sz));
  }

  virtual size_t read(::core::memory::Blob &b) {
    const size_t sz = cap(b.size());
    ::core::memory::Blob tmp(b.data(), sz);
    return m_sink.read(tmp);
  }

  virtual void seek(const size_t dist) {
    const size_t sz = cap(dist);
    m_sink.seek(sz);
  }

  virtual size_t avail() { return m_size; }

  private:
  iBinarySerializerSink &m_sink;
  size_t m_size;

  size_t cap(const size_t in) {
    const size_t ret = std::min(in, m_size);
    m_size -= ret;
    return ret;
  }
};

/**
 * Sink over a writeable {@link Blob}.
 */
class BlobSink : public iBinarySerializerSink {
  public:
  BlobSink(core::memory::Blob &sink) : m_sink(sink), m_pos(0) {}

  virtual size_t write(const ::core::memory::ConstBlob &b) {
    size_t writeable = std::min(b.size(), m_sink.size() - m_pos);
    memcpy(m_sink.data() + m_pos, b.data(), writeable);
    m_pos += writeable;
    return writeable;
  }

  virtual size_t read(::core::memory::Blob &b) {
    size_t readable = std::min(b.size(), m_sink.size() - m_pos);
    memcpy(b.data(), m_sink.data() + m_pos, readable);
    m_pos += readable;
    return readable;
  }

  virtual void seek(const size_t dist) {
    if (m_sink.size() - m_pos < dist) {
      m_pos = m_sink.size();
    } else {
      m_pos = m_pos + dist;
    }
  }

  virtual size_t avail() { return m_sink.size() - m_pos; }

  void reset() { m_pos = 0; }

  /**
   * Return the currently serialized size in bytes.
   */
  size_t size() const { return m_pos; }

  private:
  core::memory::Blob &m_sink;
  size_t m_pos;
};

/**
 * Sink over a read-only {@link ConstBlob}.
 */
class ConstBlobSink : public iBinarySerializerSink {
  public:
  ConstBlobSink(const core::memory::ConstBlob &sink) : m_sink(sink), m_pos(0) {}

  virtual size_t write(const ::core::memory::ConstBlob &b) {
    (void) b;
    CHECK_INVALID_OPERATION();
    return 0;
  }

  virtual size_t read(::core::memory::Blob &b) {
    size_t readable = std::min(b.size(), m_sink.size() - m_pos);
    memcpy(b.data(), m_sink.data() + m_pos, readable);
    m_pos += readable;
    return readable;
  }

  virtual void seek(const size_t dist) {
    if (m_sink.size() - m_pos < dist) {
      m_pos = m_sink.size();
    } else {
      m_pos = m_pos + dist;
    }
  }

  virtual size_t avail() { return m_sink.size() - m_pos; }

  void reset() { m_pos = 0; }

  private:
  const core::memory::ConstBlob &m_sink;
  size_t m_pos;
};

} // namespace base
} // namespace core

#endif
