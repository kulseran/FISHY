/**
 * Filesystem for supporting temporary in-memory blobs.
 */
#ifndef FISHY_FILESYS_MEM_H
#define FISHY_FILESYS_MEM_H

#include <CORE/MEMORY/blob.h>
#include <CORE/VFS/vfs_filesystem.h>

#include <mutex>

namespace vfs {

/**
 * Implementation of {@link iFileSystem} that can open registered blobs
 * at file paths.
 */
class MemFileSystem : public iFileSystem {
  public:
  virtual Status mount(
      const tMountId mountId,
      const vfs::Path &dest,
      const std::ios_base::openmode mode) override;
  virtual Status unmount(const tMountId) override;

  virtual filters::BaseFsStreamFilter *open(
      const tMountId mountIdx,
      const Path &filename,
      std::ios_base::openmode mode) override;
  virtual void close(filters::BaseFsStreamFilter *file) override;

  virtual Status
  remove(const tMountId mountId, const Path &src, bool &ret) override;
  virtual Status
  stat(const tMountId mountId, const Path &src, FileStats &statsOut) override;
  virtual Status
  mkdir(const tMountId mountId, const Path &dir, bool &ret) override;
  virtual Status
  rmdir(const tMountId mountId, const Path &dir, bool &ret) override;

  virtual DirectoryIterator iterate(
      const tMountId mountId,
      const Path &rootPath,
      bool recurse = false) override;

  Status create(const Path &path, const core::memory::ConstBlob &);
  Status create(const Path &path, core::memory::Blob &);

  struct FileInfo {
    core::memory::Blob m_writeableBlob;
    core::memory::ConstBlob m_readableBlob;
    FileStats m_stats;
    bool m_readonly;
  };

  private:
  struct MountInfo {
    std::ios_base::openmode m_mode;
  };
  typedef std::map< tMountId, MountInfo > tMountMap;
  tMountMap m_mounts;

  typedef std::map< std::string, FileInfo > tFileMap;
  tFileMap m_files;

  std::mutex m_lock;
};

extern std::shared_ptr< MemFileSystem > getStaticMemFileSystem();

} // namespace vfs

//----------------------------------------------------------------------------
// I M P L E M E N T A T I O N
//----------------------------------------------------------------------------

#endif
