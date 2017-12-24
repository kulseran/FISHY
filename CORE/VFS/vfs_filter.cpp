/**
 * vfs_filter.cpp
 */
#include <CORE/BASE/asserts.h>
#include <CORE/VFS/vfs_filter.h>

namespace vfs {

namespace filters {

/**
 *
 */
streamfilter::streamfilter()
  : m_buf(nullptr) {
}

/**
 *
 */
streamfilter::~streamfilter() {
}

/**
 *
 */
bool streamfilter::chain(streamfilter *buf, std::ios_base::openmode mode) {
  m_buf = buf;
  m_iomode = mode;
  return (m_buf != nullptr);
}

/**
 *
 */
void streamfilter::close() {
  sync();
}

/**
 *
 */
streamfilter::int_type streamfilter::sync() {
  ASSERT(m_buf);
  // Flush our buffers
  int_type ofrval = overflow(EOF);

  // Flush the next buffer in the chain
  int_type psynrval = m_buf->pubsync();

  // Report any errors
  if (ofrval == EOF || psynrval == EOF) {
    return EOF;
  }

  return psynrval;
}

/**
 *
 */
std::streamsize streamfilter::showmanyc() {
  ASSERT(m_buf);
  return m_buf->in_avail();
}

/**
 *
 */
streamfilter::pos_type streamfilter::seekoff(streamfilter::off_type delta, std::ios_base::seekdir dir, std::ios_base::openmode mode) {
  ASSERT(m_buf);

  // out+seek => sync
  if (mode & std::ios::out) {
    sync();
  }
  setp(nullptr, nullptr);

  // do the seeking
  pos_type oldPos = m_curPos;
  m_curPos = doseekoff(delta, dir, mode);

  // Seek could invalidate the pointers....
  off_type actualDelta = m_curPos - oldPos;

  if (actualDelta != 0) {
    if (gptr()) {
      char *newGetPos = gptr() + (int)actualDelta;
      if (newGetPos >= eback() && newGetPos < egptr()) {
        setg(eback(), newGetPos, egptr());
      } else {
        setg(nullptr, nullptr, nullptr);
      }
    } else {
      setg(nullptr, nullptr, nullptr);
    }
  }

  return m_curPos;
}

#if defined(PLAT_WIN32)
/**
 * MS compiler's 'safe' features break the xsgetn overload resulting in amazingly slow performance.
 */
std::streamsize streamfilter::_Xsgetn_s(char *pBuffer, size_t ptrlen, std::streamsize sz) {
  std::streamsize realSz = std::min(size_t(sz), ptrlen);
  return xsgetn(pBuffer, realSz);
}

/**
* MS compiler's 'safe' features break the xsgetn overload resulting in amazingly slow performance.
*/
std::streamsize streamfilter::_Xsputn_s(char *pBuffer, size_t ptrlen, std::streamsize sz) {
  std::streamsize realSz = std::min(size_t(sz), ptrlen);
  return xsputn(pBuffer, realSz);
}
#endif

/**
 *
 */
streamfilter::pos_type streamfilter::seekpos(streamfilter::pos_type pos, std::ios_base::openmode mode) {
  return seekoff(pos, std::ios::beg, mode);
}

/**
 *
 */
streamfilter::pos_type streamfilter::doseekoff(off_type delta, std::ios_base::seekdir dir, std::ios_base::openmode mode) const {
  pos_type start;
  switch (dir) {
    case std::ios_base::beg:
      start = 0;
      break;
    case std::ios_base::end:
      start = length();
      break;
    case std::ios_base::cur:
      start = m_curPos;
      break;
    default:
      ASSERT(0);
      break;
  }

  if (mode & std::ios::in) {
    if (start + delta < 0) {
      return -1;
    }
    if (start + delta > length()) {
      return -1;
    }
  } else if (mode & std::ios::out) {
    if (start + delta < 0) {
      return -1;
    }
  }
  return (start + delta);
}

} // namespace filters
} // namespace vfs