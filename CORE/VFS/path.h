/**
 * Routines to handle paths to files and folders.
 * Provides a way to seperate paths into drives, path, and
 * files. It also gives a way to easily compare and append
 * paths.
 */
#ifndef FISHY_PATH_H
#define FISHY_PATH_H

#include <string>

namespace vfs {

/**
 * Platform independant path handling.
 */
class Path {
  public:
  Path();
  Path(const Path &);
  Path(const char *);
  explicit Path(const std::string &);

  /**
   * Resolve relative path to an absolute one.
   * @return newly resolved path
   */
  Path resolve(void) const;

  /**
   * @return true if the path is empty.
   */
  bool empty(void) const { return m_path.empty(); }

  /**
   * @return the file portion of the path.
   */
  std::string file() const;

  /**
   * @return the file portion, sans extension.
   */
  std::string baseFile() const;

  /**
   * @return the file extension.
   */
  std::string extension() const;

  /**
   * @return the directory portion of the path.
   */
  std::string dir() const;

  std::string str() const { return m_path; }
  const char *c_str() const { return m_path.c_str(); }

  /**
   * Appends two paths.
   * @return result of appending a path to this one, or an empty path on error
   */
  Path operator+(const Path &) const;

  /**
   * Considers exact matches of prefix as parent.
   * eg. "foo".isParent("foo/bar") -> true
   *
   * @return true if {@code p} is a parent of this path
   */
  bool isParent(const Path &p) const;

  /**
   * @see #isParent
   * If p is a parent of this directory, returns the remaining
   * relative path to that parent.
   * eg. "foo/bar/baz".stripParent("foo/") -> "bar/baz"
   */
  Path stripParent(const Path &p) const;

  protected:
  std::string m_path;
};

} // namespace vfs

#endif
