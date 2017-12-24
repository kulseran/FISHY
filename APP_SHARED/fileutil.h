/**
 * Utility functions most tools/apps will need when looking at files.
 */
#ifndef FISHY_FILEUTIL_H
#define FISHY_FILEUTIL_H

#include <CORE/BASE/status.h>

namespace vfs {
class Path;
}

namespace appshared {

/**
 * Utility function to parse a proto out of a file.
 *
template < typename tType >
Status parseProtoFromFile(const vfs::Path &path, tType &proto);

/**
 * Utility function to serialize a binary proto out of a file.
 *
template < typename tType >
inline Status serializeProtoFromFile(const vfs::Path &path, tType &proto);

/**
 * Utility function to parse a proto out of a file.
 *
template < typename tType >
Status printProtoToFile(const vfs::Path &path, const tType &proto);

/**
 * Utility function to serialize a binary proto out of a file.
 *
template < typename tType >
inline Status serializeProtoToFile(const vfs::Path &path, const tType &proto);
*/

/**
 * Utility to dump a file to a string.
 */
Status parseFileToString(const vfs::Path &path, std::string &content);

} // namespace appshared

#  include "fileutil.inl"

#endif
