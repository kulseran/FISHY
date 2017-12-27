#include "filesys_stdio.h"

#include <CORE/BASE/checks.h>
#include <CORE/BASE/logging.h>
#include <CORE/VFS/path.h>
#include <CORE/VFS/vfs.h>

#include <fstream>
#include <sys/stat.h>

#if defined(PLAT_WIN32)
#  include <Windows.h>
#  include <direct.h>
#elif defined(PLAT_LINUX)
#endif

namespace vfs {

/**
 * File handle for stdio files deferes to std::fstream
 */
class StdiosysFileHandle : public filters::BaseFsStreamFilter {
  public:
  StdiosysFileHandle() : _streambuf(nullptr), m_fileLen(0) {}
  std::fstream _stream;

  bool open_base(std::streambuf *sbuf) {
    _streambuf = sbuf;
    m_fileLen = readLength();
    return (sbuf != nullptr);
  }
  void close() {
    if (_streambuf) {
      _streambuf->pubsync();
      _stream.close();
    }
  }

  virtual std::streamoff length() const override { return m_fileLen; }

  std::streamoff readLength() {
    pos_type cur = _streambuf->pubseekoff(0, std::ios::cur);
    pos_type end = _streambuf->pubseekoff(0, std::ios::end);
    pos_type ret = _streambuf->pubseekpos(cur);
    ASSERT(ret == cur);
    return end;
  }

  virtual const char *getFilterName() const override { return "stdiofshandle"; }

  private:
  std::streambuf *_streambuf;
  std::streamoff m_fileLen;

  protected:
  virtual int_type overflow(int_type ch) override {
    ASSERT(_streambuf);
    return _streambuf->sputc((char) ch);
  }
  virtual std::streamsize showmanyc() override {
    ASSERT(_streambuf);
    return _streambuf->in_avail();
  }
  virtual int_type underflow() override {
    ASSERT(_streambuf);
    return _streambuf->sgetc();
  }
  virtual std::streamsize xsgetn(char *pBuffer, std::streamsize sz) override {
    ASSERT(_streambuf);
    return _streambuf->sgetn(pBuffer, sz);
  }
  virtual std::streamsize
  xsputn(const char *pBuffer, std::streamsize sz) override {
    ASSERT(_streambuf);
    return _streambuf->sputn(pBuffer, sz);
  }
  virtual pos_type seekoff(
      off_type off,
      std::ios_base::seekdir way,
      std::ios_base::openmode mode) override {
    ASSERT(_streambuf);
    return _streambuf->pubseekoff(off, way, mode);
  }
  virtual pos_type
  seekpos(pos_type pos, std::ios_base::openmode mode) override {
    ASSERT(_streambuf);
    return _streambuf->pubseekpos(pos, mode);
  }
  virtual int sync() override {
    ASSERT(_streambuf);
    return _streambuf->pubsync();
  }
  virtual int uflow() override {
    ASSERT(_streambuf);
    return _streambuf->sbumpc();
  }
};

/**
 *
 */
std::shared_ptr< StdioFileSystem > getStaticStdioFileSystem() {
  static std::shared_ptr< StdioFileSystem > s_filesys(new StdioFileSystem);
  return s_filesys;
}

/**
 *
 */
Status StdioFileSystem::mount(
    const tMountId mountId,
    const vfs::Path &path,
    const std::ios_base::openmode mode) {
  FileStats stats;
  bool ret = stat(INVALID_MOUNT_ID, path, stats);
  if (!ret || !stats.m_exists || !stats.m_isDir) {
    return Status::NOT_FOUND;
  }
  CHECK_M(
      path.dir() == path.str(),
      "stdio would mount directory, however mountpoint path is lacking "
      "trailing seperator.");
  MountInfo info;
  info.m_mode = mode;
  m_mounts[mountId] = info;
  return Status::OK;
}

/**
 *
 */
Status StdioFileSystem::unmount(const tMountId mountId) {
  RET_SM(
      m_mounts.find(mountId) != m_mounts.end(),
      Status::OUT_OF_BOUNDS,
      "Can't unmount non-existant mount!");
  m_mounts.erase(mountId);
  return Status::OK;
}

/**
 *
 */
Status StdioFileSystem::open(
    filters::BaseFsStreamFilter *&pFile,
    const tMountId mountId,
    const Path &filename,
    const std::ios_base::openmode mode) {
  Log(LL::Trace) << "Stdio File opening: " << filename.str();
  pFile = nullptr;

  CHECK(m_mounts.find(mountId) != m_mounts.end());

  if ((m_mounts.find(mountId)->second.m_mode & mode) != mode) {
    return Status::BAD_ARGUMENT;
  }

  StdiosysFileHandle *pStdioFile = new StdiosysFileHandle();
  pStdioFile->_stream.open(filename.c_str(), mode);
  if (!pStdioFile->_stream.is_open()) {
    delete pStdioFile;
    return Status::NOT_FOUND;
  }

  pStdioFile->open_base(pStdioFile->_stream.rdbuf());
  pFile = pStdioFile;
  return Status::OK;
}

/**
 *
 */
void StdioFileSystem::close(filters::BaseFsStreamFilter *pFile) {
  StdiosysFileHandle *pHandle = dynamic_cast< StdiosysFileHandle * >(pFile);
  ASSERT(pHandle);
  pHandle->close();
  delete pFile;
}

/**
 *
 */
Status
StdioFileSystem::remove(const tMountId mountId, const Path &dir, bool &ret) {
  if (dir.empty()) {
    return Status::BAD_ARGUMENT;
  }
  if ((m_mounts.find(mountId)->second.m_mode & std::ios::out)
      != std::ios::out) {
    return Status::BAD_ARGUMENT;
  }

  ret = (::remove(dir.c_str()) == 0);
  return Status::OK;
}

#ifndef S_ISDIR
#define S_ISDIR(S) (((S) &_S_IFDIR) != 0)
#endif

/**
 *
 */
Status StdioFileSystem::stat(
    const tMountId mountId, const Path &path, FileStats &retVal) {
  if (mountId != INVALID_MOUNT_ID
      && (m_mounts.find(mountId)->second.m_mode & std::ios::in)
             != std::ios::in) {
    return Status::BAD_ARGUMENT;
  }

  if (path.empty()) {
    return Status::BAD_ARGUMENT;
  }

  struct stat stats;
  std::string pathStr = path.str();
  if (pathStr.back() == '/') {
    pathStr.pop_back();
  }
  if (::stat(pathStr.c_str(), &stats) < 0) {
    return Status::NOT_FOUND;
  }

  retVal.m_modifiedTime = (u64) stats.st_mtime; // seconds
  retVal.m_exists = true;
  retVal.m_isDir = S_ISDIR(stats.st_mode);
  retVal.m_size = stats.st_size;
  return Status::OK;
}

/**
 *
 */
Status
StdioFileSystem::mkdir(const tMountId mountId, const Path &dir, bool &ret) {
  if (dir.empty()) {
    return Status::BAD_ARGUMENT;
  }
  if ((m_mounts.find(mountId)->second.m_mode & std::ios::out)
      != std::ios::out) {
    return Status::BAD_ARGUMENT;
  }

#if defined(PLAT_WIN32)
  ret = (::_mkdir(dir.c_str()) == 0);
#elif defined(PLAT_LINUX)
  ret = (::mkdir(dir.c_str(), S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IROTH) == 0);
#else
#  error mkdir not supported on this platform
#endif
  return Status::OK;
}

/**
 *
 */
Status
StdioFileSystem::rmdir(const tMountId mountId, const Path &dir, bool &ret) {
  if (dir.empty()) {
    return Status::BAD_ARGUMENT;
  }
  if ((m_mounts.find(mountId)->second.m_mode & std::ios::out)
      != std::ios::out) {
    return Status::BAD_ARGUMENT;
  }

#if defined(PLAT_WIN32)
  ret = (::_rmdir(dir.c_str()) == 0);
#elif defined(PLAT_LINUX)
#else
#  error mkdir not supported on this platform
#endif
  return Status::OK;
}

/**
 * Internal iterator structure for traversing a directory tree.
 */
class StdioDirectoryIterator
    : public DirectoryIterator::iDirectoryIteratorImpl {
  public:
  StdioDirectoryIterator() {}

  StdioDirectoryIterator(const tMountId mountId, const Path &root, bool recurse)
      : m_mountId(mountId),
        m_root(root),
        m_recurse(recurse),
        m_pChild(nullptr) {
#if defined(PLAT_WIN32)
    const std::string rootAll = m_root.dir() + "*";
    m_hFind = FindFirstFile(rootAll.c_str(), &m_findData);
    while (strcmp(m_findData.cFileName, ".") == 0
           || strcmp(m_findData.cFileName, "..") == 0) {
      if (!FindNextFile(m_hFind, &m_findData)) {
        FindClose(m_hFind);
        m_hFind = INVALID_HANDLE_VALUE;
        break;
      }
    }
    next();
#elif defined(PLAT_LINUX)
#else
#  error directory iterator not supported on this platform
#endif
  }

  ~StdioDirectoryIterator() {
#if defined(PLAT_WIN32)
    if (m_hFind != INVALID_HANDLE_VALUE) {
      FindClose(m_hFind);
    }
    if (m_pChild) {
      delete m_pChild;
    }
#elif defined(PLAT_LINUX)
#else
#  error directory iterator not supported on this platform
#endif
  }

  virtual bool next() override {
#if defined(PLAT_WIN32)
    DirectoryNode node;
    if (m_pChild) {
      node = m_pChild->get();
      if (node.m_stats.m_exists) {
        setNode(node);
        m_pChild->next();
        return true;
      }
      delete m_pChild;
      m_pChild = nullptr;
    }
    if (m_hFind == INVALID_HANDLE_VALUE) {
      setNode(DirectoryNode());
      return false;
    }

    node.m_stats.m_exists = true;
    node.m_mountId = m_mountId;
    node.m_stats.m_isDir =
        (m_findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
    node.m_stats.m_modifiedTime =
        ((u64) m_findData.ftLastWriteTime.dwLowDateTime)
        | (((u64) m_findData.ftLastWriteTime.dwHighDateTime) << 32);
    node.m_stats.m_size =
        (u64(m_findData.nFileSizeHigh) << 32) | m_findData.nFileSizeLow;
    if (node.m_stats.m_isDir) {
      node.m_path = Path(m_root.str() + m_findData.cFileName + "/");
      if (m_recurse) {
        m_pChild =
            new StdioDirectoryIterator(m_mountId, node.m_path, m_recurse);
      }
    } else {
      node.m_path = m_root + Path(m_findData.cFileName);
    }
    setNode(node);

    do {
      if (!FindNextFile(m_hFind, &m_findData)) {
        FindClose(m_hFind);
        m_hFind = INVALID_HANDLE_VALUE;
        break;
      }
    } while (strcmp(m_findData.cFileName, ".") == 0
             || strcmp(m_findData.cFileName, "..") == 0);
#elif defined(PLAT_LINUX)
#else
#  error directory iterator not supported on this platform
#endif
    return true;
  }

  private:
  StdioDirectoryIterator *m_pChild;
  tMountId m_mountId;
  Path m_root;
  bool m_recurse;

#if defined(PLAT_WIN32)
  WIN32_FIND_DATA m_findData;
  HANDLE m_hFind;
#elif defined(PLAT_LINUX)
#else
#  error directory iterator not supported on this platform
#endif
};

/**
 *
 */
DirectoryIterator StdioFileSystem::iterate(
    const tMountId mountId, const Path &root, bool recurse) {
  if ((m_mounts.find(mountId)->second.m_mode & std::ios::in) != std::ios::in) {
    return DirectoryIterator();
  }

  return DirectoryIterator(
      std::shared_ptr< DirectoryIterator::iDirectoryIteratorImpl >(
          new StdioDirectoryIterator(mountId, Path(root.dir()), recurse)));
}

} // namespace vfs
