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
  // The referenced path existed at the time it was stat'd
  bool m_exists;
  // The referenced path is a directory
  bool m_isDir;
  // The last modified time of the path
  u64 m_modifiedTime;
  // If the path references a file, the size of the file.
  u64 m_size;

  FileStats() : m_exists(false), m_isDir(false), m_modifiedTime(0), m_size(0) {}
};

/**
 * Iterator element class for use with {@link #iterate}
 */
struct DirectoryNode {
  // Stats about the directory or file
  FileStats m_stats;
  // The path to the directory or file
  Path m_path;
  // The mount ID under which this directory of file exists
  tMountId m_mountId;

  DirectoryNode() : m_mountId(INVALID_MOUNT_ID) {}
  bool operator==(const DirectoryNode &other) const;
  bool operator!=(const DirectoryNode &other) const;
};

/**
 * Iterator class for use with {@link #iterate}
 */
class DirectoryIterator
    : public std::iterator< std::forward_iterator_tag, DirectoryNode > {
  public:
  /**
   * Interface for filesystems to provide their own way of getting the
   * next file or directory.
   */
  class iDirectoryIteratorImpl : core::util::noncopyable {
    public:
    iDirectoryIteratorImpl();
    virtual ~iDirectoryIteratorImpl();

    DirectoryNode get() const;

    /**
     * Implementers should override this function.
     * The default {@link #setNode} should be called to apply the result of the
     * iteration to the next object. A default instance of {@link DirectoryNode}
     * should be used when there's no additonal items to iterate over.
     */
    virtual bool next() = 0;

    private:
    DirectoryNode m_node;

    protected:
    void setNode(const DirectoryNode &node);
  };

  DirectoryIterator();
  DirectoryIterator(const std::shared_ptr< iDirectoryIteratorImpl > &data);

  /**
   * @return the current {@link DirectoryNode} this iterator points to
   *     or a default instance if there's no items left to iterate over.
   */
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
