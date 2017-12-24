/**
 * vfs_filesystem.inl
 *
 *
 */
#ifndef FISHY_VFS_FILESYSTEM_INL
#define FISHY_VFS_FILESYSTEM_INL

#include <CORE/UTIL/comparisonchain.h>

namespace vfs {

/**
 *
 */
inline bool iFileSystem::DirectoryNode::
operator==(const DirectoryNode &other) const {
  return core::util::ComparisonChain()
      .andOne(m_path.str(), other.m_path.str())
      .andOne(m_mountId, other.m_mountId)
      .buildEqual();
}

inline bool iFileSystem::DirectoryNode::
operator!=(const DirectoryNode &other) const {
  return !(*this == other);
}

/**
 *
 */
inline iFileSystem::DirectoryIterator::iDirectoryIteratorImpl::
    iDirectoryIteratorImpl() {
}

/**
 *
 */
inline iFileSystem::DirectoryIterator::iDirectoryIteratorImpl::
    ~iDirectoryIteratorImpl() {
}

/**
 *
 */
inline iFileSystem::DirectoryNode
iFileSystem::DirectoryIterator::iDirectoryIteratorImpl::get() const {
  return m_node;
}

/**
 *
 */
inline void iFileSystem::DirectoryIterator::iDirectoryIteratorImpl::setNode(
    const DirectoryNode &node) {
  m_node = node;
}

/**
 *
 */
inline iFileSystem::DirectoryIterator::DirectoryIterator() {
}

/**
 *
 */
inline iFileSystem::DirectoryIterator::DirectoryIterator(
    const std::shared_ptr< iDirectoryIteratorImpl > &data)
    : m_pData(data) {
}

/**
 *
 */
inline iFileSystem::DirectoryNode iFileSystem::DirectoryIterator::get() const {
  if (m_pData) {
    return m_pData->get();
  }
  return iFileSystem::DirectoryNode();
}

/**
 *
 */
inline iFileSystem::DirectoryIterator &iFileSystem::DirectoryIterator::
operator++() {
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
inline iFileSystem::DirectoryIterator iFileSystem::DirectoryIterator::
operator++(int) {
  DirectoryIterator tmp(*this);
  ++(*this);
  return tmp;
}

/**
 *
 */
inline bool iFileSystem::DirectoryIterator::
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
inline bool iFileSystem::DirectoryIterator::
operator!=(const DirectoryIterator &other) const {
  return !(*this == other);
}

} // namespace vfs

#endif
