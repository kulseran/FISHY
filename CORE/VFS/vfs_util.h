#ifndef FISHY_VFS_UTIL_H
#define FISHY_VFS_UTIL_H

#include "path.h"
#include "vfs_types.h"

namespace vfs {
namespace util {

static const char *TMP_PATH = "tmp/";

/**
 * @param src a logical file path
 * @return stats structure for the given file
 */
FileStats Stat(const Path &src);

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
DirectoryIterator List(const Path &root, bool recurse = false);

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
