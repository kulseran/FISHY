/**
 * filesys_stdio.h
 */
#ifndef FISHY_FILESYS_STDIO_H
#define FISHY_FILESYS_STDIO_H

#include <CORE/VFS/vfs_filesystem.h>
#include <CORE/VFS/path.h>

#include <memory>

namespace vfs {

/**
 * Implementation of {@link iFileSystem} that delegates to std::fstream
 */
class StdioFileSystem : public iFileSystem {
  public:
    virtual bool mount(const tMountId mountId, const vfs::Path &dest, const std::ios_base::openmode mode) override;
    virtual bool unmount(const tMountId) override;

    virtual filters::BaseFsStreamFilter *open(const tMountId mountIdx, const Path &filename, std::ios_base::openmode mode) override;
    virtual void close(filters::BaseFsStreamFilter *file) override;

    virtual bool remove(const tMountId mountId, const Path &src, bool &ret) override;
    virtual bool stat(const tMountId mountId, const Path &src, FileStats &statsOut) override;
    virtual bool mkdir(const tMountId mountId, const Path &dir, bool &ret) override;
    virtual bool rmdir(const tMountId mountId, const Path &dir, bool &ret) override;

    virtual DirectoryIterator iterate(const tMountId mountId, const Path &rootPath, bool recurse = false) override;

  private:
    struct MountInfo {
      std::ios_base::openmode m_mode;
    };
    typedef std::map<tMountId, MountInfo> tMountMap;
    tMountMap m_mounts;
};

extern std::shared_ptr<StdioFileSystem> getStaticStdioFileSystem();

} // namespace vfs

//----------------------------------------------------------------------------
// I M P L E M E N T A T I O N
//----------------------------------------------------------------------------

#endif
