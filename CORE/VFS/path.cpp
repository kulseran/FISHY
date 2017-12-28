#include "path.h"

#include <CORE/BASE/checks.h>

#define SEP_WIN "\\"
#define SEP_LINUX "/"
#define SEP_INTERNAL SEP_LINUX
#define EXTENSION_SEP "."

namespace vfs {

/**
 * Cleans up the seperators in a path to be consistent.
 */
static std::string cleanPath(const std::string &s) {
  std::string rVal = s;
  const std::string::size_type len = rVal.length();
  if (!len) {
    return s;
  }
  for (std::string::size_type x = 0; x < len; ++x) {
    if (rVal[x] == SEP_WIN[0] || rVal[x] == SEP_LINUX[0]) {
      rVal[x] = SEP_INTERNAL[0];
    }
  }

  return rVal;
}

/**
 *
 */
Path::Path() {
}

/**
 *
 */
Path::Path(const Path &other) {
  m_path = other.m_path;
}

/**
 *
 */
Path::Path(const char *s) {
  m_path = cleanPath(s);
}

/**
 *
 */
Path::Path(const std::string &str) {
  m_path = cleanPath(str.c_str());
}

/**
 *
 */
Path Path::operator+(const Path &other) const {
  if (!file().empty()) {
    return Path();
  }
  if (m_path == "./") {
    return other;
  }
  return Path(m_path + other.m_path);
}

/**
 * Walk over the list and return only the final file part of the path
 * (assumed to be the last element after the last SEP)
 */
std::string Path::file(void) const {
  const std::string::size_type iter = m_path.find_last_of(SEP_INTERNAL);
  if (iter != m_path.npos) {
    return m_path.substr(iter + 1, m_path.npos);
  }
  return m_path;
}

/**
 * Return file sans extension.
 */
std::string Path::baseFile(void) const {
  const std::string filename = file();
  const std::string::size_type iter = filename.find_first_of(EXTENSION_SEP);
  if (iter != filename.npos) {
    return filename.substr(0, iter);
  }
  return filename;
}

/**
 * Return file extension.
 */
std::string Path::extension(void) const {
  const std::string filename = file();
  const std::string::size_type iter = filename.find_first_of(EXTENSION_SEP);
  if (iter != filename.npos) {
    return filename.substr(iter + 1, filename.npos);
  }
  return "";
}

/**
 * Walk over the path and return only the directories prefix.
 */
std::string Path::dir(void) const {
  const std::string::size_type iter = m_path.find_last_of(SEP_INTERNAL);
  if (iter != m_path.npos) {
    return m_path.substr(0, iter + 1);
  }
  return "./";
}

/**
 *
 */
bool Path::isParent(const Path &other) const {
  return (other.m_path.find(m_path) == 0);
}

/**
 *
 */
Path Path::stripParent(const Path &other) const {
  ASSERT(other.file().empty());
  ASSERT(other.isParent(*this));

  const std::string::size_type findPos = m_path.find(other.m_path);
  if (findPos != m_path.npos) {
    const std::string::size_type start = findPos + other.m_path.length();
    return Path(m_path.substr(start).c_str());
  }
  return Path();
}

/**
 *
 */
Path Path::resolve(void) const {
  // empty paths are bad
  if (empty()) {
    return *this;
  }

  const std::string olddirs = dir();
  const std::string oldfile = file();
  // don't go through the expensive resolve if there's no ../ or ./
  if (m_path.find("./") == m_path.npos) {
    return *this;
  }

  std::string newdirs;
  std::string::size_type iter = olddirs.length() - 1;
  bool rVal = true;

  size_t skip = 0;
  while (iter != olddirs.npos && iter != 0) {
    const std::string::size_type iterNext =
        olddirs.find_last_of(SEP_INTERNAL, iter - 1);
    std::string curDir = olddirs.substr(iterNext + 1, iter - iterNext);

    iter = iterNext;

    if (iterNext != olddirs.npos) {
      // not the last dir, lets process
      if (curDir == "." SEP_INTERNAL) {
        continue;
      }
      if (curDir == ".." SEP_INTERNAL) {
        ++skip;
        continue;
      }
    }

    if (curDir == ".." SEP_INTERNAL) {
      rVal = false;
    } else if (curDir == "." SEP_INTERNAL) {
      curDir = "";
    } else if (skip) {
      --skip;
      continue;
    }

    newdirs = curDir + newdirs;
  }

  if (!newdirs.length()) {
    newdirs = "";
  }
  for (size_t i = 0; i < skip; ++i) {
    newdirs = ".." SEP_INTERNAL + newdirs;
    rVal = false;
  }

  if (rVal) {
    return Path(newdirs + oldfile);
  }

  return Path();
}

} // namespace vfs
