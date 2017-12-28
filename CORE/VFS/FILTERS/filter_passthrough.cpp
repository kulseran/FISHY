#include "filter_passthrough.h"

#include <CORE/BASE/asserts.h>

namespace vfs {
namespace filters {

/**
 *
 */
bool passthrough::chain(streamfilter *filter, const std::ios::openmode mode) {
  if (!streamfilter::chain(filter, mode)) {
    return false;
  }

  m_seekLen =
      std::min(m_seekLen, (pos_type)(((pos_type) length()) - m_seekLow));
  return true;
}

/**
 * Just push the character out if there is room. (SLOW! this happens one char at
 * a time)
 */
passthrough::int_type passthrough::overflow(passthrough::int_type ch) {
  ASSERT(m_buf);
  // Did someone flush a valid char?
  if (ch != EOF) {
    // Push it down
    int_type rVal = m_buf->sputc((char) ch);
    if (rVal != EOF) {
      m_curPos += 1;
    }
    return rVal;
  }
  return 0;
}

/**
 * The default uflow assumes that we specified a buffer. But, we specifically
 * haven't. So, we need to do a slower version that will actually give us a
 * result without crash. This results in a slow read of a single char at a time.
 */
passthrough::int_type passthrough::uflow() {
  if (m_curPos >= length()) {
    return EOF;
  }
  return m_buf->sbumpc();
}

/**
 * Default "underflow"
 * Just read in a char. (SLOW! this happens one char at a time!)
 */
passthrough::int_type passthrough::underflow() {
  ASSERT(m_buf);
  if (m_curPos >= length()) {
    return EOF;
  }
  return m_buf->sgetc();
}

/**
 * Get a bunch of data from the stream
 */
std::streamsize passthrough::xsgetn(char *pBuffer, std::streamsize sz) {
  ASSERT(m_buf);
  std::streamsize realSz = limitReadOff(sz);
  std::streamsize bytesRead = m_buf->sgetn(pBuffer, realSz);
  m_curPos += bytesRead;
  return bytesRead;
}

/**
 * Put a bunch of data into the stream
 */
std::streamsize passthrough::xsputn(const char *pBuffer, std::streamsize sz) {
  ASSERT(m_buf);
  std::streamsize bytesWritten = m_buf->sputn(pBuffer, sz);
  m_curPos += bytesWritten;
  return bytesWritten;
}

/**
 * Seek the lower stream, and make sure our local position matches
 */
passthrough::pos_type passthrough::seekoff(
    passthrough::off_type delta,
    std::ios_base::seekdir dir,
    std::ios_base::openmode mode) {
  // Find out where we are on the lower stream
  pos_type realCurPos = m_buf->pubseekoff(0, std::ios::cur, mode);
  if (realCurPos == (pos_type) -1) {
    return realCurPos;
  }

  // Assume we got here by filling up our buffer
  if (mode & std::ios::in) {
    ptrdiff_t buffSz = egptr() - eback();
    ptrdiff_t buffFwd = gptr() - eback();
    off_type back_track = buffSz - buffFwd;
    realCurPos -= back_track;
  }

  // Do our local seek
  m_curPos = (m_seekLow > realCurPos) ? (0) : (realCurPos - m_seekLow);
  streamfilter::seekoff(delta, dir, mode);
  if (m_curPos == (pos_type) -1) {
    return m_curPos;
  }

  // Reseek the lower stream
  realCurPos = m_buf->pubseekpos(m_curPos + m_seekLow, mode);
  if (realCurPos == (pos_type) -1) {
    return realCurPos;
  }

  // Recalc our current from what we got back
  m_curPos = (m_seekLow > realCurPos) ? (0) : (realCurPos - m_seekLow);
  return m_curPos;
}

} // namespace filters
} // namespace vfs
