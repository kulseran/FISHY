/**
 * Memory blobs for data storage and data passing
 */
#ifndef FISHY_BLOB_H
#define FISHY_BLOB_H

#include <CORE/types.h>

#include <string>

namespace core {
namespace memory {

/**
 * Container for pointing to a constant memory buffer.
 */
class ConstBlob {
  public:
  ConstBlob();
  ConstBlob(const u8 *const data, const size_t size);
  ConstBlob(const std::string &data);

  /**
   * Get a pointer to the contained data.
   */
  const u8 *const data() const;

  /**
   * Access a single byte of the contained data.
   */
  u8 data(const size_t index) const;

  /**
   * Return the size of the contained data.
   */
  size_t size() const;

  private:
  const size_t m_size;
  const u8 *const m_pData;
};

/**
 * Container for pointing to a mutable memory buffer.
 */
class Blob {
  public:
  Blob();
  Blob(u8 *data, const size_t size);
  Blob(std::string &data);

  /**
   * Get a pointer to the contained data.
   */
  u8 *data();

  /**
   * Get a pointer to the contained data.
   */
  const u8 *data() const;

  /**
   * Access a single byte of the contained data.
   */
  u8 &data(const size_t index);

  /**
   * Access a single byte of the contained data.
   */
  u8 data(const size_t index) const;

  /**
   * Return the size of the contained data.
   */
  size_t size() const;

  /**
   * Decrease the indicated size of the contained data.
   */
  void trimSize(const size_t size);

  /**
   * Convert to a {@link ConstBlob}
   */
  operator ConstBlob() const;

  private:
  size_t m_size;
  u8 *const m_pData;
};

} // namespace memory
} // namespace core

#  include "blob.inl"

#endif // FISHY_BLOB_H
