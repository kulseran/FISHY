/**
 * filter_passthrough.h
 *
 * streamfilter that passes through to the underlying delegate
 */
#ifndef FISHY_FILTER_PASSTHROUGH_H
#define FISHY_FILTER_PASSTHROUGH_H

#include <CORE/types.h>
#include <CORE/VFS/vfs_filter.h>

namespace vfs {
namespace filters {

/**
 * Implementation of {@link streamfilter} that passes through to an underlying delegate.
 */
class passthrough : public streamfilter {
  public:
    passthrough(const pos_type low = 0, const pos_type len = std::numeric_limits<s64>::max());
    virtual ~passthrough() {}

    virtual bool chain(streamfilter *, const std::ios::openmode);
    virtual const char *getFilterName() const { return "passthrough"; }

  private:
    pos_type m_seekLow;
    pos_type m_seekLen;
    off_type limitReadOff(const off_type sz) { return std::min(sz, off_type(m_seekLen - m_curPos)); }

  protected:
    virtual int_type overflow(int_type ch);
    virtual int_type underflow();
    virtual std::streamsize xsgetn(char *pBuffer, std::streamsize sz);
    virtual std::streamsize xsputn(const char *pBuffer, std::streamsize sz);
    virtual pos_type seekoff(off_type, std::ios_base::seekdir, std::ios_base::openmode = std::ios_base::in | std::ios_base::out);
    virtual int_type uflow();
};

/**
 *
 */
inline passthrough::passthrough(const pos_type low, const pos_type len)
  : m_seekLow(low),
    m_seekLen(len) {
}

} // namespace filters
} // namespace vfs

#endif
