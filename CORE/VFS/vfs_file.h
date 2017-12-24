/**
 * vfs_file.h
 *
 * Implemention of file io as a replacement to std::fstream
 */
#ifndef FISHY_VFS_FILE_H
#define FISHY_VFS_FILE_H

#include "path.h"

#include <CORE/UTIL/noncopyable.h>
#include <CORE/types.h>

// Disable C4250, see
// https://connect.microsoft.com/VisualStudio/feedback/details/733720/
// #pragma warning(push)
// #pragma warning(disable : 4250)

namespace vfs {

namespace filters {
class streamfilter;
}

/**
 * Input File Stream
 */
class ifstream : public std::istream, core::util::noncopyable {
  public:
  ifstream(
      const Path &, std::ios_base::openmode = std::ios::in | std::ios::binary);
  ~ifstream();

  bool
  open(const Path &, std::ios_base::openmode = std::ios::in | std::ios::binary);
  bool is_open() const;

  void flush();
  void close();

  std::streamoff getFileLen() const;

  filters::streamfilter *popFilter();
  bool pushFilter(filters::streamfilter *);

  private:
  u32 m_filterDepth;
};

/**
 * Output File Stream
 */
class ofstream : public std::ostream, core::util::noncopyable {
  public:
  ofstream(
      const Path &, std::ios_base::openmode = std::ios::out | std::ios::binary);
  ~ofstream();

  bool open(
      const Path &, std::ios_base::openmode = std::ios::out | std::ios::binary);
  bool is_open() const;

  void flush();
  void close();

  bool pushFilter(filters::streamfilter *);
  filters::streamfilter *popFilter();

  private:
  u32 m_filterDepth;
};

} // namespace vfs

// #  pragma warning(pop)

#endif
