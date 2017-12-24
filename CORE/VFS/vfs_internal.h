/**
 * vfs_internal.h
 */
#ifndef FISHY_VFS_INTERNAL_H
#define FISHY_VFS_INTERNAL_H

#include <ios>

namespace vfs {
namespace filters {
class streamfilter;
}
class Path;

/**
 * Open a file
 */
filters::streamfilter *open(const Path &, const std::ios::openmode);

/**
 * Close a file
 * @see #open
 */
bool close(filters::streamfilter *);

} // namespace vfs

#endif
