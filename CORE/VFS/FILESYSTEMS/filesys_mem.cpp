/**
 * filesys_mem.cpp
 */
#include "filesys_mem.h"

#include <CORE/ARCH/timer.h>
#include <CORE/BASE/checks.h>
#include <CORE/BASE/logging.h>
#include <CORE/MATH/math_limit.h>
#include <CORE/VFS/vfs.h>
#include <CORE/VFS/path.h>

#include <fstream>

namespace vfs {

/**
 * File handle for stdio files deferes to std::fstream
 */
class MemFileHandle : public filters::BaseFsStreamFilter {
  public:
    MemFileHandle() {}

    bool open_base(MemFileSystem::FileInfo &fileInfo) {
      m_info = fileInfo;
      m_pos = 0;
      return true;
    }

    void close() {
      // nothing to do
    }

    virtual std::streamoff length() const override { return m_info.m_stats.m_size; }

    virtual const char *getFilterName() const override { return "memfilehandle"; }
  private:
    MemFileSystem::FileInfo m_info;
    std::streamoff m_pos;

  protected:
    virtual int_type overflow(int_type ch) override {
      if (m_pos > m_info.m_readableBlob.size() || m_info.m_readonly) {
        return EOF;
      }
      m_info.m_writeableBlob.data()[m_pos] = (u8) ch;
      return ch;
    }

    virtual std::streamsize showmanyc() override {
      return m_info.m_stats.m_size - m_pos;
    }

    virtual int_type underflow() override {
      if (m_pos > m_info.m_readableBlob.size()) {
        return EOF;
      }
      return m_info.m_readableBlob.data()[m_pos];
    }

    virtual std::streamsize xsgetn(char *pBuffer, std::streamsize sz) override {
      std::streamsize actual = std::min(sz, (std::streamsize) m_info.m_stats.m_size - m_pos);
      if (actual > 0) {
        std::copy(m_info.m_readableBlob.data() + m_pos, m_info.m_readableBlob.data() + m_pos + actual, pBuffer);
      }
      m_pos += actual;
      return actual;
    }

    virtual std::streamsize xsputn(const char *pBuffer, std::streamsize sz) override {
      if (m_info.m_readonly) {
        return 0;
      }
      std::streamsize actual = std::min(sz, m_info.m_readableBlob.size() - m_pos);
      if (actual > 0) {
        std::copy(pBuffer, pBuffer + actual, m_info.m_writeableBlob.data() + m_pos);
      }
      m_pos += actual;
      m_info.m_stats.m_size = std::max((u64) m_pos, m_info.m_stats.m_size);
      return actual;
    }

    virtual pos_type seekoff(off_type off, std::ios_base::seekdir way, std::ios_base::openmode mode) override {
      (void) mode;

      switch (way) {
        case std::ios_base::beg:
          m_pos = 0 + off;
          break;
        case std::ios_base::end:
          m_pos = m_info.m_stats.m_size + off;
          break;
        case std::ios_base::cur:
          m_pos = m_pos + off;
          break;
      }
      m_pos = core::math::clamp(m_pos, (std::streamoff) 0, (std::streamoff) m_info.m_stats.m_size);
      return m_pos;
    }

    virtual pos_type seekpos(pos_type pos, std::ios_base::openmode mode) override {
      return seekoff(pos, std::ios_base::beg, mode);
    }

    virtual int sync() override {
      // nothing to do
      return 0;
    }

    virtual int uflow() override {
      if (m_pos + 1 > m_info.m_readableBlob.size()) {
        return EOF;
      }
      return m_info.m_readableBlob.data()[m_pos++];
    }
};

/**
 *
 */
std::shared_ptr<MemFileSystem> getStaticMemFileSystem() {
  static std::shared_ptr<MemFileSystem> s_filesys(new MemFileSystem());
  return s_filesys;
}

/**
 *
 */
bool MemFileSystem::mount(const tMountId mountId, const vfs::Path &path, const std::ios_base::openmode mode) {
  LOCK_MUTEX(m_lock);
  static vfs::Path rootPath("memfile/");
  if (!rootPath.isParent(path)) {
    return false;
  }

  MountInfo info;
  info.m_mode = mode;
  m_mounts[mountId] = info;
  return true;
}

/**
 *
 */
bool MemFileSystem::unmount(const tMountId mountId) {
  LOCK_MUTEX(m_lock);
  RET_M(m_mounts.find(mountId) != m_mounts.end(), "Can't unmount non-existant mount!");
  m_mounts.erase(mountId);
  return true;
}

/**
 *
 */
filters::BaseFsStreamFilter *MemFileSystem::open(const tMountId mountId, const Path &filename, std::ios_base::openmode mode) {
  TraceMsg("Memfile File opening: " << filename.str());
  LOCK_MUTEX(m_lock);
  if (m_mounts.find(mountId) == m_mounts.end()) {
    Log(LL::Error) << "Can't open file on non-existant mount." << std::endl;
    return nullptr;
  }
  if ((m_mounts.find(mountId)->second.m_mode & mode) != mode) {
    return nullptr;
  }

  tFileMap::iterator itr = m_files.find(filename.str());
  if (itr == m_files.end()) {
    return nullptr;
  }

  MemFileHandle *pFile = new MemFileHandle();
  pFile->open_base(itr->second);
  return pFile;
}

/**
 *
 */
void MemFileSystem::close(filters::BaseFsStreamFilter *pFile) {
  MemFileHandle *pHandle = dynamic_cast<MemFileHandle *>(pFile);
  ASSERT(pHandle);
  pHandle->close();
  delete pFile;
}

/**
 *
 */
bool MemFileSystem::remove(const tMountId mountId, const Path &dir, bool &ret) {
  (void) mountId;
  LOCK_MUTEX(m_lock);

  if (dir.empty()) {
    return false;
  }

  tFileMap::const_iterator itr = m_files.find(dir.str());
  if (itr == m_files.end()) {
    ret = false;
  } else {
    ret = true;
    m_files.erase(itr);
  }
  return true;
}

/**
 *
 */
bool MemFileSystem::stat(const tMountId mountId, const Path &path, FileStats &ret) {
  (void) mountId;
  LOCK_MUTEX(m_lock);

  if (path.empty()) {
    return false;
  }

  tFileMap::const_iterator itr = m_files.find(path.str());
  if (itr != m_files.end()) {
    ret = itr->second.m_stats;
  }
  return true;
}

/**
 *
 */
bool MemFileSystem::mkdir(const tMountId mountId, const Path &dir, bool &ret) {
  (void) mountId;

  if (dir.empty()) {
    return false;
  }
  // Directories don't actually exist in this filesystem.
  ret = true;
  return true;
}

/**
 *
 */
bool MemFileSystem::rmdir(const tMountId mountId, const Path &dir, bool &ret) {
  (void) mountId;

  if (dir.empty()) {
    return false;
  }
  // Directories don't actually exist in this filesystem.
  ret = true;
  return true;
}

/**
 *
 */
iFileSystem::DirectoryIterator MemFileSystem::iterate(const tMountId mountId, const Path &root, bool recurse) {
  return iFileSystem::DirectoryIterator();
}


bool MemFileSystem::create(const Path &path, const core::memory::ConstBlob &blob) {
  LOCK_MUTEX(m_lock);
  tFileMap::const_iterator itr = m_files.find(path.str());
  if (itr != m_files.end()) {
    return false;
  }

  FileInfo info;
  info.m_readableBlob = blob;
  info.m_readonly = true;
  info.m_stats.m_exists = true;
  info.m_stats.m_isDir = false;
  info.m_stats.m_modifiedTime = core::timer::GetTicks();
  info.m_stats.m_size = blob.size();
  m_files[path.str()] = info;
  return true;
}

bool MemFileSystem::create(const Path &path, core::memory::Blob &blob) {
  LOCK_MUTEX(m_lock);
  tFileMap::const_iterator itr = m_files.find(path.str());
  if (itr != m_files.end()) {
    return false;
  }

  FileInfo info;
  info.m_writeableBlob = blob;
  info.m_readableBlob = blob;
  info.m_readonly = false;
  info.m_stats.m_exists = true;
  info.m_stats.m_isDir = false;
  info.m_stats.m_modifiedTime = core::timer::GetTicks();
  info.m_stats.m_size = 0;
  m_files[path.str()] = info;
  return true;
}

} // namespace vfs