/**
 * streamfilter class for chaining data transforms during file read/write.
 */
#ifndef FISHY_VFS_FILTER_H
#define FISHY_VFS_FILTER_H

#include <CORE/ARCH/platform.h>
#include <CORE/UTIL/noncopyable.h>

#include <ios>
#include <iostream>

namespace vfs {
namespace filters {

/**
 * Class to provide a filter stack over a streambuf object.
 */
class streamfilter : public std::streambuf, core::util::noncopyable {
  public:
  streamfilter();
  virtual ~streamfilter();

  /**
   * Set the next filter in the chain.
   *
   * @param pFilter the filter to replace the current next filter.
   */
  virtual bool chain(streamfilter *pFilter, const std::ios::openmode);

  /**
   *
   */
  virtual void close();

  /**
   * Get the current next filter in the chain.
   */
  streamfilter *getNext() { return m_buf; }

  /**
   * Get the current next filter in the chain.
   */
  const streamfilter *getNext() const { return m_buf; }

  /**
   * Report the total available bytes on the stream.
   */
  virtual std::streamoff length() const { return m_buf->length(); }

  /**
   * Get the debug name for the filter.
   */
  virtual const char *getFilterName() const = 0;

  protected:
  virtual int_type overflow(int_type ch) = 0;
  virtual int_type underflow() = 0;

  virtual pos_type
      seekoff(off_type, std::ios_base::seekdir, std::ios_base::openmode);
  virtual pos_type seekpos(pos_type, std::ios_base::openmode);
  virtual std::streamsize showmanyc();
  virtual int sync();

#if defined(PLAT_WIN32)
  // _Xsgetn_s for some stupid reason doesn't wrap xsgetn by default
  virtual std::streamsize
  _Xsgetn_s(char *pBuffer, size_t ptrlen, std::streamsize sz);
  virtual std::streamsize
  _Xsputn_s(char *pBuffer, size_t ptrlen, std::streamsize sz);
#endif

  /**
   * Util functions to calculate the new "_cur_pos" value of a seek operation.
   */
  pos_type doseekoff(
      off_type, std::ios_base::seekdir, std::ios_base::openmode) const;

  // Internal bookkeepping
  streamfilter *m_buf;
  pos_type m_curPos;

  std::ios_base::openmode m_iomode;
};

} // namespace filters
} // namespace vfs

#endif
