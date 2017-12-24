/**
 * Binary serialization functions
 */
#ifndef FISHY_SERIALIZER_H
#define FISHY_SERIALIZER_H

#include <CORE/BASE/checks.h>
#include <CORE/MEMORY/blob.h>

#include <algorithm>

namespace core {
namespace base {

/**
 * Serialization sink interface.
 */
class iBinarySerializerSink {
  public:
  iBinarySerializerSink() : m_fail(false) {}
  virtual ~iBinarySerializerSink() {}

  /**
   * Implementations of this should dump the blob to the output.
   * @return the number of bytes actuall written
   */
  virtual size_t write(const ::core::memory::ConstBlob &) = 0;

  /**
   * Implementations of this should pull {@link Blob#size()}
   * bytes from the source, and insert them into the Blob.
   * @return the number of bytes actually read
   */
  virtual size_t read(::core::memory::Blob &) = 0;

  /**
   * Should seek forward in the input stream.
   */
  virtual void seek(const size_t) = 0;

  /**
   * @return the number of bytes remaining in the stream.
   */
  virtual size_t avail() = 0;

  /**
   * Puts the stream into an error state.
   * If in an error state, {@link #read} and {@link #write} are expected to
   * return 0 bytes read or written.
   */
  void set_fail() { m_fail = true; }

  /**
   * Check if the stream is in an error state.
   */
  bool fail() const { return m_fail; }

  private:
  bool m_fail;
};

} // namespace base
} // namespace core

/**
 * Macro for defining output serializers
 */
#  define OSERIALIZE(tType)                                 \
    inline ::core::base::iBinarySerializerSink &operator<<( \
        ::core::base::iBinarySerializerSink &buff, const tType &obj)

/**
 * Macro for defining input serializers
 */
#  define ISERIALIZE(tType)                                 \
    inline ::core::base::iBinarySerializerSink &operator>>( \
        ::core::base::iBinarySerializerSink &buff, tType &obj)

#  include "serializer_basesinks.h"

#endif
