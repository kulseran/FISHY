/**
 * Implemention of file io as a replacement to std::fstream
 */
#ifndef FISHY_VFS_FILE_H
#define FISHY_VFS_FILE_H

#include "path.h"

#include <CORE/BASE/status.h>
#include <CORE/UTIL/noncopyable.h>

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

  Status
  open(const Path &, std::ios_base::openmode = std::ios::in | std::ios::binary);
  bool is_open() const;

  void flush();
  void close();

  std::streamoff getFileLen() const;

  filters::streamfilter *popFilter();
  bool pushFilter(filters::streamfilter *);

  private:
  int m_filterDepth;
};

/**
 * Output File Stream
 */
class ofstream : public std::ostream, core::util::noncopyable {
  public:
  ofstream(
      const Path &, std::ios_base::openmode = std::ios::out | std::ios::binary);
  ~ofstream();

  Status open(
      const Path &, std::ios_base::openmode = std::ios::out | std::ios::binary);
  bool is_open() const;

  void flush();
  void close();

  bool pushFilter(filters::streamfilter *);
  filters::streamfilter *popFilter();

  private:
  int m_filterDepth;
};

} // namespace vfs

#endif
