/**
 * Virtual File System operations
 */
#ifndef FISHY_VFS_H
#define FISHY_VFS_H

#include "path.h"
#include "vfs_filesystem.h"

#include <memory>

namespace vfs {

/**
 * Must be called before all other vfs operations
 */
void Init();

/**
 * Should be called before application exit
 */
void Dest();

/**
 * @return true if {@link Init} has been called
 */
bool IsInitalized();

/**
 * Enable/Disable secure path checking.
 * When enabled, mount points must be children of the current working directory
 * of the application. And all file paths opened on a mount point must be
 * children of that mountpoint. When disabled, mount points may be anywhere on
 * the host computer and file paths may be relative to a mount point such that
 * they are a parent of the mount point.
 *
 * Should be called after all initial "unsafe" mount points are connected so
 * that malicious input data can't be used to set a file or mount point that
 * points to random data on the computer.
 */
void SetSecurePathing(bool = true);

/**
 * Add a {@link iFileSystem} handler.
 */
void AddFileSys(std::shared_ptr< iFileSystem > &fileSys);

/**
 * Add a mount point.
 * All the registred file systems will be tried in lifo order to open the {@code
 * src} path. If the {@code src} path is handled, the {@code dest} path is
 * registered with the system. Following calls to open files use the {@code
 * dest} path as their root. eg. vfs::Mount("./data12341.zip", "./data");
 *     vfs::ifile("./data/foo.txt");
 *
 * @param src the physical source path to mount
 * @param dest the logical destination path to create
 * @param mode the open mode to restrict the open modes of files opened on this
 * mount point
 * @return a tMountId to identify this mount point. May be passed to Unmount to
 * remove the mount point
 */
tMountId Mount(
    const Path &src,
    const Path &dest,
    std::ios_base::openmode = std::ios_base::binary | std::ios_base::in);

/**
 * Add a mount point for the storage of temporary files.
 * This uses the underlying system's temporary file functionality to generate a
 * temp folder. The mount point is accessable via {@link util::GetTempDir} and
 * {@link util::GetTempFile}.
 */
tMountId MountTempFolder();

/**
 * Removes a mount point previously mounted with Mount.  Will close all handles
 * to files in this mount point.
 *
 * @param id the tMountId returned from a previous call to Mount
 */
bool Unmount(const tMountId id);

/**
 * Remove all mount points.
 * @see Unmount
 */
void UnmountAll();

/**
 * @param src a logical file path
 * @return stats structure for the given file
 */
FileStats Stat(const Path &src);

namespace util {

/**
 * Perform a copy of {@code src} to {@code dst}
 * @param src a logical file path to an existing file
 * @param dst a logical file path to a non-existing file
 * @return true if the copy was a success
 */
bool Copy(const Path &src, const Path &dst);

/**
 * Create a directory.
 */
bool MkDir(const Path &src);

/**
 * Remove a directory.
 */
bool RmDir(const Path &src);

/**
 * Removes a file
 */
bool Remove(const Path &src);

/**
 * List the content of a directory, optionally recursing through the whole
 * directory tree.
 */
iFileSystem::DirectoryIterator List(const Path &root, bool recurse = false);

/**
 * Generates a temporary directory
 */
Path GetTempDir();

/**
 * Generates a temporary file
 */
Path GetTempFile();

} // namespace util

} // namespace vfs

#endif
