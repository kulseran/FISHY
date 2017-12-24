#ifndef FISHY_BLOB_INL
#define FISHY_BLOB_INL

namespace core {
namespace memory {

/**
 *
 */
inline ConstBlob::ConstBlob() : m_size(0), m_pData(nullptr) {
}

/**
 *
 */
inline ConstBlob::ConstBlob(const u8 *const data, const size_t size)
    : m_size(size), m_pData(data) {
}

/**
 *
 */
inline ConstBlob::ConstBlob(const std::string &data)
    : m_size(data.size()), m_pData((const u8 *const) data.data()) {
}

/**
 *
 */
inline size_t ConstBlob::size() const {
  return m_size;
}

/**
 *
 */
inline const u8 *const ConstBlob::data() const {
  return m_pData;
}

/**
 *
 */
inline u8 ConstBlob::data(const size_t index) const {
  ASSERT(index < m_size);
  return m_pData[index];
}

/**
 *
 */
inline Blob::Blob() : m_size(0), m_pData(nullptr) {
}

/**
 *
 */
inline Blob::Blob(u8 *data, const size_t size) : m_size(size), m_pData(data) {
}

/**
 *
 */
inline Blob::Blob(std::string &data)
    : m_size(data.size()), m_pData((u8 *) data.data()) {
}

/**
 *
 */
inline size_t Blob::size() const {
  return m_size;
}

/**
 *
 */
inline u8 *Blob::data() {
  return m_pData;
}

/**
 *
 */
inline const u8 *Blob::data() const {
  return m_pData;
}

/**
 *
 */
inline u8 Blob::data(const size_t index) const {
  ASSERT(index < m_size);
  return m_pData[index];
}

/**
 *
 */
inline u8 &Blob::data(const size_t index) {
  ASSERT(index < m_size);
  return m_pData[index];
}

/**
 *
 */
inline void Blob::trimSize(const size_t size) {
  ASSERT(size <= m_size);
  m_size = size;
}

/**
 *
 */
inline Blob::operator ConstBlob() const {
  return ConstBlob(m_pData, m_size);
}

} // namespace memory
} // namespace core

#endif
