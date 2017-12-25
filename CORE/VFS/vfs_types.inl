#ifndef FISHY_VFS_FILESYSTEM_INL
#define FISHY_VFS_FILESYSTEM_INL

#include <CORE/UTIL/comparisonchain.h>

namespace vfs {

/**
 *
 */
inline bool DirectoryNode::operator==(const DirectoryNode &other) const {
  return core::util::ComparisonChain()
      .andOne(m_path.str(), other.m_path.str())
      .andOne(m_mountId, other.m_mountId)
      .buildEqual();
}

/**
 *
 */
inline bool DirectoryNode::operator!=(const DirectoryNode &other) const {
  return !(*this == other);
}

/**
 *
 */
inline DirectoryIterator::iDirectoryIteratorImpl::iDirectoryIteratorImpl() {
}

/**
 *
 */
inline DirectoryIterator::iDirectoryIteratorImpl::~iDirectoryIteratorImpl() {
}

/**
 *
 */
inline DirectoryNode DirectoryIterator::iDirectoryIteratorImpl::get() const {
  return m_node;
}

/**
 *
 */
inline void
DirectoryIterator::iDirectoryIteratorImpl::setNode(const DirectoryNode &node) {
  m_node = node;
}

/**
 *
 */
inline DirectoryIterator::DirectoryIterator() {
}

/**
 *
 */
inline DirectoryIterator::DirectoryIterator(
    const std::shared_ptr< iDirectoryIteratorImpl > &data)
    : m_pData(data) {
}

/**
 *
 */
inline DirectoryNode DirectoryIterator::get() const {
  if (m_pData) {
    return m_pData->get();
  }
  return DirectoryNode();
}

/**
 *
 */
inline DirectoryIterator &DirectoryIterator::operator++() {
  if (m_pData) {
    if (!m_pData->next()) {
      m_pData.reset();
    }
  }
  return *this;
}

/**
 *
 */
inline DirectoryIterator DirectoryIterator::operator++(int) {
  DirectoryIterator tmp(*this);
  ++(*this);
  return tmp;
}

/**
 *
 */
inline bool DirectoryIterator::
operator==(const DirectoryIterator &other) const {
  if (m_pData == other.m_pData) {
    return true;
  } else if (m_pData == nullptr || other.m_pData == nullptr) {
    return false;
  }
  return m_pData->get() == other.m_pData->get();
}

/**
 *
 */
inline bool DirectoryIterator::
operator!=(const DirectoryIterator &other) const {
  return !(*this == other);
}

} // namespace vfs

#endif
