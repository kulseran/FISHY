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
  open(path, mode).ignoreErrors();
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
Status ifstream::open(const Path &path, std::ios_base::openmode mode) {
  CHECK_M(!is_open(), "Attempted to re-open file without closing");

  // Sanity check. This should have the 'in' flag on it.
  const bool hasRequiredMode = mode & std::ios::in;
  ASSERT(hasRequiredMode);
  if (!hasRequiredMode) {
    return Status::BAD_ARGUMENT;
  }

  // Sanity check. Many modes aren't supported!
  const bool hasNoInvalidModes =
      (mode & (std::ios::out | std::ios::app | std::ios::ate | std::ios::trunc))
      == 0;
  ASSERT(hasNoInvalidModes);
  if (!hasNoInvalidModes) {
    return Status::BAD_ARGUMENT;
  }

  // Attempt open
  filters::streamfilter *filebuf = vfs::open(path, mode);
  if (!filebuf) {
    return Status::NOT_FOUND;
  }

  // Make sure we can actually connect this through.
  filters::passthrough *handle =
      dynamic_cast< filters::passthrough * >(rdbuf());
  if (!handle->chain(filebuf, mode)) {
    vfs::close(filebuf);
    return Status::GENERIC_ERROR;
  }

  return Status::OK;
}

/**
 *
 */
bool ifstream::is_open() const {
  filters::streamfilter *handle =
      dynamic_cast< filters::streamfilter * >(rdbuf());
  return handle->getNext();
}

/**
 *
 */
void ifstream::close() {
  if (!is_open()) {
    return;
  }

  CHECK_M(m_filterDepth == 0, "Filters still attached to file!");

  filters::streamfilter *handle =
      dynamic_cast< filters::streamfilter * >(rdbuf());
  filters::streamfilter *filebuf =
      dynamic_cast< filters::streamfilter * >(handle->getNext());
  CHECK(vfs::close(filebuf));
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

  if (!filter->chain(handle, std::ios::in)) {
    return false;
  }

  rdbuf(filter);
  m_filterDepth++;
  return true;
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
  m_filterDepth--;

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
  open(path, mode).ignoreErrors();
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
Status ofstream::open(const Path &path, std::ios_base::openmode mode) {
  CHECK_M(!is_open(), "Attempted to re-open file without closing");

  // Sanity check. This should have the 'in' flag on it.
  const bool hasRequiredMode = mode & std::ios::out;
  ASSERT(hasRequiredMode);
  if (!hasRequiredMode) {
    return Status::BAD_ARGUMENT;
  }

  // Sanity check. Many modes aren't supported!
  const bool hasNoInvalidModes =
      (mode & (std::ios::in | std::ios::app | std::ios::ate | std::ios::trunc))
      == 0;
  ASSERT(hasNoInvalidModes);
  if (!hasNoInvalidModes) {
    return Status::BAD_ARGUMENT;
  }

  // Attempt open
  filters::streamfilter *filebuf = vfs::open(path, mode);
  if (!filebuf) {
    return Status::NOT_FOUND;
  }

  // Make sure we can actually connect this through.
  filters::passthrough *handle =
      dynamic_cast< filters::passthrough * >(rdbuf());
  if (!handle->chain(filebuf, mode)) {
    CHECK(vfs::close(filebuf));
    return Status::GENERIC_ERROR;
  }

  return Status::OK;
}

/**
 *
 */
bool ofstream::is_open() const {
  filters::streamfilter *handle =
      dynamic_cast< filters::streamfilter * >(rdbuf());
  return handle->getNext();
}

/**
 *
 */
void ofstream::close() {
  if (!is_open()) {
    return;
  }

  CHECK_M(m_filterDepth == 0, "Filters still attached to file!");

  filters::streamfilter *handle =
      dynamic_cast< filters::streamfilter * >(rdbuf());
  filters::streamfilter *filebuf =
      dynamic_cast< filters::streamfilter * >(handle->getNext());
  CHECK(vfs::close(filebuf));
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

  if (!filter->chain(handle, std::ios::in)) {
    return false;
  }

  rdbuf(filter);
  m_filterDepth++;

  return true;
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
  m_filterDepth--;

  filters::streamfilter *handle =
      dynamic_cast< filters::streamfilter * >(rdbuf());
  rdbuf(handle->getNext());

  handle->close();

  return handle;
}

} // namespace vfs
