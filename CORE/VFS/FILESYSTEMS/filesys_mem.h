/**
 * filesys_mem.h
 */
#ifndef FISHY_FILESYS_MEM_H
#define FISHY_FILESYS_MEM_H

#include <CORE/ARCH/mutex.h>
#include <CORE/MEMORY/blob.h>
#include <CORE/VFS/vfs_filesystem.h>

namespace vfs {

/**
 * Implementation of {@link iFileSystem} that can open registered blobs
 * at file paths.
 */
class MemFileSystem : public iFileSystem {
  public:
    virtual bool mount(const tMountId mountId, const vfs::Path &dest, const std::ios_base::openmode mode);
    virtual bool unmount(const tMountId);

    virtual filters::BaseFsStreamFilter *open(const tMountId mountIdx, const Path &filename, std::ios_base::openmode mode);
    virtual void close(filters::BaseFsStreamFilter *file);

    virtual bool remove(const tMountId mountId, const Path &src, bool &ret);
    virtual bool stat(const tMountId mountId, const Path &src, FileStats &statsOut);
    virtual bool mkdir(const tMountId mountId, const Path &dir, bool &ret);
    virtual bool rmdir(const tMountId mountId, const Path &dir, bool &ret);

    virtual DirectoryIterator iterate(const tMountId mountId, const Path &rootPath, bool recurse = false);

    bool create(const Path &path, const core::memory::ConstBlob &);
    bool create(const Path &path, core::memory::Blob &);

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
    typedef std::map<tMountId, MountInfo> tMountMap;
    tMountMap m_mounts;

    typedef std::map<std::string, FileInfo> tFileMap;
    tFileMap m_files;

    std::mutex m_lock;
};

extern std::shared_ptr<MemFileSystem> getStaticMemFileSystem();

} // namespace vfs

//----------------------------------------------------------------------------
// I M P L E M E N T A T I O N
//----------------------------------------------------------------------------

#endif
