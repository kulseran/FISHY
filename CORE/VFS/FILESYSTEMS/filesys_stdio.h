/**
 * Filesystem wrapper over the default stdio operations.
 */
#ifndef FISHY_FILESYS_STDIO_H
#define FISHY_FILESYS_STDIO_H

#include <CORE/VFS/path.h>
#include <CORE/VFS/vfs_filesystem.h>

#include <map>
#include <memory>

namespace vfs {

/**
 * Implementation of {@link iFileSystem} that delegates to std::fstream
 */
class StdioFileSystem : public iFileSystem {
  public:
  virtual Status mount(
      const tMountId mountId,
      const vfs::Path &dest,
      const std::ios_base::openmode mode) override;
  virtual Status unmount(const tMountId) override;

  virtual Status open(
      filters::BaseFsStreamFilter *&pFile,
      const tMountId mountIdx,
      const Path &filename,
      const std::ios_base::openmode mode) override;
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

  private:
  struct MountInfo {
    std::ios_base::openmode m_mode;
  };
  typedef std::map< tMountId, MountInfo > tMountMap;
  tMountMap m_mounts;
};

extern std::shared_ptr< StdioFileSystem > getStaticStdioFileSystem();

} // namespace vfs

//----------------------------------------------------------------------------
// I M P L E M E N T A T I O N
//----------------------------------------------------------------------------

#endif
