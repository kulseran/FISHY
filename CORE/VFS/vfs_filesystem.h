/**
 * Interface for creating a filesystem that can be used as a source for a
 * vfs::file
 */
#ifndef FISHY_VFS_FILESYSTEM_H
#define FISHY_VFS_FILESYSTEM_H

#include <CORE/BASE/status.h>
#include <CORE/VFS/FILTERS/filter_passthrough.h>
#include <CORE/VFS/vfs_filter.h>
#include <CORE/VFS/vfs_types.h>
#include <CORE/types.h>

#include <ios>
#include <map>
#include <memory>

namespace vfs {

class iFileSystem;
class Path;

namespace filters {
/**
 * Basic {@link streamfilter} that includes information about the filesystem
 * that was used to connect the filter. This allows the base vfs code to
 * properly close the filter later.
 */
class BaseFsStreamFilter : public passthrough {
  public:
  void setFileSystem(iFileSystem *fileSys) { m_fileSys = fileSys; }
  iFileSystem *getFileSystem() { return m_fileSys; }

  private:
  iFileSystem *m_fileSys;
};

} // namespace filters

/**
 * Interface for filesystem interactions.
 */
class iFileSystem {
  public:
  virtual ~iFileSystem() {}

  /**
   * Here you must provide a function to check the incomming path for a valid
   * mount. If the path is invalid, or anything goes wrong during the mount
   * process, return MOUNTID_INVALID If the path is successfully mounted, return
   * a unique value of type tMountId. This value will later be passed into
   * unmount to unmount the path. The path MAY NOT BE UNIQUE, as the user is
   * allowed to mount the same location multiple times. Be sure to consider
   * this.
   *
   * @param mountId the id that will be passed to unmount to remove this mount
   * point later
   * @param path the physical path to the mount point on the device
   * @param mode the valid openmode for this mount point, attempting to later
   * open a file with mis-matched mode should fail
   */
  virtual Status mount(
      const tMountId mountId,
      const vfs::Path &path,
      const std::ios_base::openmode mode) = 0;

  /**
   * Unmount a path. This should also close anything opened on this mount.
   *
   * @param the mount point to remove. This id was previously passed to mount to
   * create a mount point.
   */
  virtual Status unmount(const tMountId mountId) = 0;

  /**
   * Attempt to open a file. The VFS has determined that "filename" may exist
   * under the mount specified. If you can open the file, return a class
   * deriving from base_fs_streamfilter. Otherwise return nullptr.
   *
   * @param mountId the mountpoint to open the file under
   * @param filename the physical filename to open
   * @param mode the open mode for the file
   * @return a valid {@link BaseFsStreamFilter} pointer on success, or nullptr
   * on failure
   */
  virtual filters::BaseFsStreamFilter *open(
      const tMountId mountId,
      const Path &filename,
      std::ios_base::openmode mode) = 0;

  /**
   * This should close the handle that you returned from open.
   *
   * @param pFile a {@link BaseFsStreamFilter} previously returned from open.
   */
  virtual void close(filters::BaseFsStreamFilter *pFile) = 0;

  /**
   * Remove a file. If secure paths are enabled, this may only be a directory
   * relative to a mount point.
   *
   * @param path the file to remove
   * @param ret the success of the removal
   */
  virtual Status
  remove(const tMountId mountId, const Path &path, bool &ret) = 0;

  /**
   * Get file stats for a file or directory
   *
   * @param path the path to get information for
   * @param stats the stats structure to fill
   */
  virtual Status
  stat(const tMountId mountId, const Path &path, FileStats &stats) = 0;

  /**
   * Create a directory. If secure paths are enabled, this may only be a
   * directory relative to a mount point.
   *
   * @param path the path to create
   * @param ret the success of the directory creation
   */
  virtual Status mkdir(const tMountId mountId, const Path &path, bool &ret) = 0;

  /**
   * Remove a directory. If secure paths are enabled, this may only be a
   * directory relative to a mount point. This may only be called on an empty
   * directory.
   *
   * @param path the path to remove
   * @param ret the success of the directory removal
   */
  virtual Status rmdir(const tMountId mountId, const Path &path, bool &ret) = 0;

  /**
   * Scan over {@code rootPath} and attempt to find all files and directories
   * contained under this path.
   *
   * @param rootPath the path to scan under
   * @param recurse if true, then the scan will emit directory entires followed
   * by their content recursively
   * @return an iterator over the directory content
   */
  virtual DirectoryIterator iterate(
      const tMountId mountId, const Path &rootPath, bool recurse = false) = 0;
};

} // namespace vfs

#endif
