/**
 * Utility types used in the VFS
 */
#ifndef FISHY_VFS_TYPES_H
#define FISHY_VFS_TYPES_H

#include <CORE/UTIL/noncopyable.h>
#include <CORE/types.h>

#include "path.h"
#include "vfs.h"

namespace vfs {

/**
 * Contains information about a file or directory path.
 */
struct FileStats {
  bool m_exists;
  bool m_isDir;
  u64 m_modifiedTime;
  u64 m_size;

  FileStats() : m_exists(false), m_isDir(false), m_modifiedTime(0), m_size(0) {}
};

/**
 * Iterator element class for use with {@link #iterate}
 */
struct DirectoryNode {
  DirectoryNode() : m_mountId(INVALID_MOUNT_ID) {}
  FileStats m_stats;
  Path m_path;
  tMountId m_mountId;

  bool operator==(const DirectoryNode &other) const;
  bool operator!=(const DirectoryNode &other) const;
};

/**
 * Iterator class for use with {@link #iterate}
 */
class DirectoryIterator
    : public std::iterator< std::forward_iterator_tag, DirectoryNode > {
  public:
  class iDirectoryIteratorImpl : core::util::noncopyable {
    public:
    iDirectoryIteratorImpl();
    virtual ~iDirectoryIteratorImpl();

    DirectoryNode get() const;
    virtual bool next() = 0;

    private:
    DirectoryNode m_node;

    protected:
    void setNode(const DirectoryNode &node);
  };

  DirectoryIterator();
  DirectoryIterator(const std::shared_ptr< iDirectoryIteratorImpl > &data);

  DirectoryNode get() const;

  DirectoryIterator &operator++();
  DirectoryIterator operator++(int);
  bool operator==(const DirectoryIterator &other) const;
  bool operator!=(const DirectoryIterator &other) const;

  private:
  std::shared_ptr< iDirectoryIteratorImpl > m_pData;
};

} // namespace vfs

#  include "vfs_types.inl"

#endif
