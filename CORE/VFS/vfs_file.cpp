/**
 * vfs_file.cpp
 */
#include <CORE/BASE/checks.h>
#include <CORE/VFS/FILTERS/filter_passthrough.h>
#include <CORE/VFS/vfs_file.h>
#include <CORE/VFS/vfs_filter.h>
#include <CORE/VFS/vfs_internal.h>

namespace vfs {

/**
 *
 */
ifstream::ifstream(const Path &path, std::ios_base::openmode mode)
    : std::istream(new filters::passthrough()), m_filterDepth(0) {
  (void) mode;

  open(path);
}

/**
 *
 */
ifstream::~ifstream() {
  close();
}

/**
 *
 */
bool ifstream::open(const Path &path, std::ios_base::openmode mode) {
  CHECK_M(!is_open(), "Attempted to re-open file without closing");

  // Sanity check. This should have the 'in' flag on it.
  ASSERT(mode & std::ios::in);
  if (!(mode & std::ios::in)) {
    return false;
  }

  // Sanity check. Many modes aren't supported!
  ASSERT(
      (mode & (std::ios::out | std::ios::app | std::ios::ate | std::ios::trunc))
      == 0);
  if ((mode & (std::ios::out | std::ios::app | std::ios::ate | std::ios::trunc))
      != 0) {
    return false;
  }

  //_______________
  filters::streamfilter *filebuf;
  filters::passthrough *handle;

  // Attempt open
  filebuf = vfs::open(path, mode);
  if (!filebuf) {
    return false;
  }

  // Make sure we can actually connect this through.
  handle = dynamic_cast< filters::passthrough * >(rdbuf());
  if (!handle->chain(filebuf, mode)) {
    vfs::close(filebuf);
    return false;
  }

  return true;
}

/**
 *
 */
bool ifstream::is_open() const {
  filters::streamfilter *handle =
      dynamic_cast< filters::streamfilter * >(rdbuf());
  if (!handle->getNext()) {
    return false;
  }
  return true;
}

/**
 *
 */
void ifstream::close() {
  if (!is_open()) {
    return;
  }

  if (m_filterDepth != 0) {
    throw std::runtime_error("vfs: Filters still attached to file!");
  }

  filters::streamfilter *handle =
      dynamic_cast< filters::streamfilter * >(rdbuf());
  filters::streamfilter *filebuf =
      dynamic_cast< filters::streamfilter * >(handle->getNext());
  vfs::close(filebuf);
  handle->chain(nullptr, std::ios::in);
}

/**
 *
 */
void ifstream::flush() {
  if (!is_open()) {
    return;
  }

  filters::streamfilter *handle =
      dynamic_cast< filters::streamfilter * >(rdbuf());
  handle->pubsync();
}

/**
 *
 */
bool ifstream::pushFilter(filters::streamfilter *filter) {
  if (!is_open()) {
    return false;
  }

  filters::streamfilter *handle =
      dynamic_cast< filters::streamfilter * >(rdbuf());

  if (filter->chain(handle, std::ios::in)) {
    rdbuf(filter);
    ++m_filterDepth;

    return true;
  }

  return false;
}

/**
 *
 */
filters::streamfilter *ifstream::popFilter() {
  if (!is_open()) {
    return nullptr;
  }

  ASSERT(m_filterDepth > 0);
  if (!m_filterDepth) {
    return nullptr;
  }
  --m_filterDepth;

  filters::streamfilter *handle =
      dynamic_cast< filters::streamfilter * >(rdbuf());
  rdbuf(handle->getNext());

  handle->close();

  return handle;
}

/**
 *
 */
std::streamoff ifstream::getFileLen() const {
  if (!is_open()) {
    return 0;
  }

  filters::streamfilter *handle =
      dynamic_cast< filters::streamfilter * >(rdbuf());
  return handle->length();
}

/**
 *
 */
ofstream::ofstream(const Path &path, std::ios_base::openmode mode)
    : std::ostream(new filters::passthrough()), m_filterDepth(0) {
  (void) mode;

  open(path);
}

/**
 *
 */
ofstream::~ofstream() {
  close();
}

/**
 *
 */
bool ofstream::open(const Path &path, std::ios_base::openmode mode) {
  CHECK_M(!is_open(), "Attempted to re-open file without closing");

  // Sanity check. This should have the 'in' flag on it.
  ASSERT(mode & std::ios::out);
  if (!(mode & std::ios::out)) {
    return false;
  }

  // Sanity check. Many modes aren't supported!
  ASSERT(
      (mode & (std::ios::in | std::ios::app | std::ios::ate | std::ios::trunc))
      == 0);
  if ((mode & (std::ios::in | std::ios::app | std::ios::ate | std::ios::trunc))
      != 0) {
    return false;
  }

  //_______________
  filters::streamfilter *filebuf;
  filters::passthrough *handle;

  // Attempt open
  filebuf = vfs::open(path, mode);
  if (!filebuf) {
    return false;
  }

  // Make sure we can actually connect this through.
  handle = dynamic_cast< filters::passthrough * >(rdbuf());
  if (!handle->chain(filebuf, mode)) {
    vfs::close(filebuf);
    return false;
  }

  return true;
}

/**
 *
 */
bool ofstream::is_open() const {
  filters::streamfilter *handle =
      dynamic_cast< filters::streamfilter * >(rdbuf());
  if (!handle->getNext()) {
    return false;
  }
  return true;
}

/**
 *
 */
void ofstream::close() {
  if (!is_open()) {
    return;
  }

  if (m_filterDepth != 0) {
    throw std::runtime_error("vfs: Filters still attached to file!");
  }

  filters::streamfilter *handle =
      dynamic_cast< filters::streamfilter * >(rdbuf());
  filters::streamfilter *filebuf =
      dynamic_cast< filters::streamfilter * >(handle->getNext());
  vfs::close(filebuf);
  handle->chain(nullptr, std::ios::in);
}

/**
 *
 */
void ofstream::flush() {
  if (!is_open()) {
    return;
  }

  filters::streamfilter *handle =
      dynamic_cast< filters::streamfilter * >(rdbuf());
  handle->pubsync();
}

/**
 *
 */
bool ofstream::pushFilter(filters::streamfilter *filter) {
  if (!is_open()) {
    return false;
  }

  filters::streamfilter *handle =
      dynamic_cast< filters::streamfilter * >(rdbuf());

  if (filter->chain(handle, std::ios::in)) {
    rdbuf(filter);
    ++m_filterDepth;

    return true;
  }

  return false;
}

/**
 *
 */
filters::streamfilter *ofstream::popFilter() {
  if (!is_open()) {
    return nullptr;
  }

  ASSERT(m_filterDepth > 0);
  if (!m_filterDepth) {
    return nullptr;
  }
  --m_filterDepth;

  filters::streamfilter *handle =
      dynamic_cast< filters::streamfilter * >(rdbuf());
  rdbuf(handle->getNext());

  handle->close();

  return handle;
}

} // namespace vfs
