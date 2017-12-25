#include "vfs_internal.h"

#include <CORE/ARCH/intrinsics.h>
#include <CORE/ARCH/timer.h>
#include <CORE/BASE/asserts.h>
#include <CORE/BASE/checks.h>
#include <CORE/BASE/logging.h>
#include <CORE/UTIL/algorithm.h>
#include <CORE/UTIL/lexical_cast.h>
#include <CORE/VFS/FILESYSTEMS/filesys_mem.h>
#include <CORE/VFS/FILESYSTEMS/filesys_stdio.h>
#include <CORE/VFS/FILTERS/filter_passthrough.h>
#include <CORE/VFS/path.h>
#include <CORE/VFS/vfs.h>
#include <CORE/VFS/vfs_filter.h>
#include <EXTERN_LIB/srutil/delegate/delegate.hpp>

#include <algorithm>
#include <atomic>
#include <cstdlib>
#include <fstream>
#include <vector>

namespace vfs {

#define S_ISDIR(S) ((S) &_S_IFDIR)
#define TMP_PATH "tmp/"
#define COPY_BUFFER_SZ 1024

typedef u32 tFileSysId;
typedef std::vector< std::shared_ptr< iFileSystem > > tFileSystemList;

/**
 * Container for information about mount points.
 */
struct MountPoint {
  tMountId m_id;
  tFileSysId m_fileSys;

  Path m_src;
  Path m_dest;
};
typedef std::vector< MountPoint > tMountList;

/**
 * Container for information about the VFS.
 */
class VfsDetail {
  public:
  tMountList m_mountPoints;
  tFileSystemList m_fileSystems;
  u32 m_id;
  bool m_vfsUseSecurePaths;

  std::mutex m_mutex;

  VfsDetail();
  ~VfsDetail();

  /**
   *
   */
  tMountId NextMountId();

  void addFileSys(std::shared_ptr< iFileSystem > &fileSys);
  tMountId mount(
      const Path &src,
      const Path &dest,
      std::ios_base::openmode mode,
      bool userChecks);
  tMountId mountTempFolder();

  bool unmount(const tMountId mountId);
  void unmountAll();
  void setSecurePathing(bool useSecurePaths);

  bool close(filters::streamfilter *filebuf);
  filters::streamfilter *open(const Path &path, const std::ios::openmode mode);

  FileStats stat(const Path &path);
};

static const tFileSysId INVALID_FILESYS_ID = (tFileSysId) -1;

/**
 *
 */
VfsDetail &GetVFS() {
  static VfsDetail s_detail;
  return s_detail;
}

/**
 *
 */
VfsDetail::VfsDetail() {
  Trace();
  std::lock_guard< std::mutex > lock(m_mutex);
  m_fileSystems.push_back(getStaticStdioFileSystem());
  m_fileSystems.push_back(getStaticMemFileSystem());
}

/**
 *
 */
VfsDetail::~VfsDetail() {
  Trace();
  std::lock_guard< std::mutex > lock(m_mutex);
  ASSERT(m_mountPoints.size() == 0);
  m_fileSystems.clear();
}

/**
 *
 */
tMountId VfsDetail::NextMountId() {
  ASSERT(m_id < std::numeric_limits< u32 >::max());
  return ((tMountId) m_id++);
}

/**
 *
 */
void AddFileSys(std::shared_ptr< iFileSystem > &fileSys) {
  GetVFS().addFileSys(fileSys);
}

/**
 *
 */
void VfsDetail::addFileSys(std::shared_ptr< iFileSystem > &fileSys) {
  std::lock_guard< std::mutex > lock(m_mutex);
  m_fileSystems.push_back(fileSys);
}

/**
 *
 */
tMountId
Mount(const Path &src, const Path &dest, std::ios_base::openmode mode) {
  return GetVFS().mount(src, dest, mode, true);
}

/**
 *
 */
tMountId MountTempFolder() {
  return GetVFS().mountTempFolder();
}

/**
 *
 */
tMountId VfsDetail::mountTempFolder() {
  std::lock_guard< std::mutex > lock(m_mutex);

  static const char *OSTMP = "ostmp/";

  const char *pTempDir = nullptr;
  pTempDir = core::util::first_non_null(pTempDir, getenv("TMPDIR"));
  pTempDir = core::util::first_non_null(pTempDir, getenv("TMP"));
  pTempDir = core::util::first_non_null(pTempDir, getenv("TEMP"));
  pTempDir = core::util::first_non_null(pTempDir, getenv("TEMPDIR"));
  pTempDir = core::util::first_non_null(pTempDir, getenv("USERPROFILE"));
  if (pTempDir == nullptr) {
    return INVALID_MOUNT_ID;
  }

  vfs::Path osRootTempPath(vfs::Path(pTempDir).resolve().str() + "/");
  const tMountId tempMount = mount(
      osRootTempPath, OSTMP, std::ios_base::binary | std::ios_base::out, false);
  if (tempMount == INVALID_MOUNT_ID) {
    return INVALID_MOUNT_ID;
  }
  if (!vfs::Stat(vfs::Path(OSTMP) + vfs::Path("fishy/")).m_exists) {
    if (!vfs::util::MkDir(vfs::Path(OSTMP) + vfs::Path("fishy/"))) {
      Unmount(tempMount);
      return INVALID_MOUNT_ID;
    }
  }

  vfs::Path tempPath;
  do {
    std::string ticks = "007";
    core::util::lexical_cast(core::timer::GetSystemTicks(), ticks)
        .ignoreErrors();
    const std::string tempDir = "fishy/" + ticks + "/";
    tempPath = Path(tempDir);
  } while (vfs::Stat(vfs::Path(OSTMP) + tempPath).m_exists);

  if (!vfs::util::MkDir(vfs::Path(OSTMP) + tempPath)) {
    Unmount(tempMount);
    return INVALID_MOUNT_ID;
  }
  Unmount(tempMount);

  return mount(
      osRootTempPath + tempPath,
      TMP_PATH,
      std::ios_base::binary | std::ios_base::in | std::ios_base::out,
      false);
}

/**
 *
 */
tMountId VfsDetail::mount(
    const Path &src,
    const Path &dest,
    std::ios_base::openmode mode,
    bool userChecks) {
  Log(LL::Trace) << "vfs::Mount(\"" << src.str() << "\", \"" << dest.str()
                 << "\")";

  std::lock_guard< std::mutex > lock(m_mutex);
  Path resolvedPath = src;
  if (m_vfsUseSecurePaths) {
    resolvedPath = resolvedPath.resolve();
    if (resolvedPath.empty()) {
      return INVALID_MOUNT_ID;
    }
  }
  CHECK_M(!userChecks || dest.str() != TMP_PATH, "Use '#MountTempFolder' API.");

  const tMountId mountId = NextMountId();
  tFileSysId validFileSysId = INVALID_FILESYS_ID;

  for (tFileSystemList::iterator filesys = m_fileSystems.begin();
       filesys != m_fileSystems.end();
       ++filesys) {
    if ((*filesys)->mount(mountId, resolvedPath, mode)) {
      validFileSysId = static_cast< tFileSysId >(
          std::distance(m_fileSystems.begin(), filesys));
      break;
    }
  }

  if (validFileSysId == INVALID_FILESYS_ID) {
    return INVALID_MOUNT_ID;
  }

  MountPoint mountPoint;
  mountPoint.m_src = resolvedPath;
  mountPoint.m_dest = dest.resolve();
  mountPoint.m_id = mountId;
  mountPoint.m_fileSys = validFileSysId;
  m_mountPoints.push_back(mountPoint);
  Log(LL::Info) << "Adding mount \"" << mountPoint.m_src.str() << "\" -> \""
                << mountPoint.m_dest.str() << "\"";
  return mountPoint.m_id;
}

/**
 *
 */
bool Unmount(const tMountId mountId) {
  return GetVFS().unmount(mountId);
}

/**
 *
 */
bool VfsDetail::unmount(const tMountId mountId) {
  std::lock_guard< std::mutex > lock(m_mutex);

  if (mountId == INVALID_MOUNT_ID) {
    return true;
  }

  for (tMountList::iterator itr = m_mountPoints.begin();
       itr != m_mountPoints.end();) {
    if (itr->m_id == mountId) {
      Log(LL::Info) << "Removing mount \"" << itr->m_src.str() << "\" -> \""
                    << itr->m_dest.str() << "\"";
      if (m_fileSystems[itr->m_fileSys]->unmount(itr->m_id)) {
        itr = m_mountPoints.erase(itr);
        return true;
      }
    } else {
      ++itr;
    }
  }
  return false;
}

/**
 *
 */
void UnmountAll() {
  GetVFS().unmountAll();
}

/**
 *
 */
void VfsDetail::unmountAll() {
  std::lock_guard< std::mutex > lock(m_mutex);

  for (tMountList::iterator itr = m_mountPoints.begin();
       itr != m_mountPoints.end();
       ++itr) {
    Log(LL::Info) << "Removing mount \"" << itr->m_src.str() << "\" -> \""
                  << itr->m_dest.str() << "\"";
    m_fileSystems[itr->m_fileSys]->unmount(itr->m_id).ignoreErrors();
  }
  m_mountPoints.clear();
}

/**
 *
 */
bool close(filters::streamfilter *filebuf) {
  return GetVFS().close(filebuf);
}

/**
 *
 */
bool VfsDetail::close(filters::streamfilter *filebuf) {
  std::lock_guard< std::mutex > lock(m_mutex);
  filters::BaseFsStreamFilter *pFile =
      dynamic_cast< filters::BaseFsStreamFilter * >(filebuf);
  ASSERT(pFile->getFileSystem());
  pFile->getFileSystem()->close(pFile);
  return true;
}

/**
 *
 */
template < typename tRetVal >
class FileSysVisitor {
  public:
  typedef typename srutil::delegate< bool(
      tMountId, const Path &, tRetVal &, iFileSystem *) >
      tFunc;

  static bool
  findFileSysOperation(const Path &path, tRetVal &ret, tFunc &func) {
    Path inPath = path.resolve();
    if (inPath.empty()) {
      if (GetVFS().m_vfsUseSecurePaths) {
        Log(LL::Error) << "Unable to resolve path: " << path.str();
        return false;
      } else {
        inPath = path;
      }
    }

    for (tMountList::reverse_iterator itr = GetVFS().m_mountPoints.rbegin();
         itr != GetVFS().m_mountPoints.rend();
         ++itr) {
      iFileSystem *pFileSys = GetVFS().m_fileSystems[itr->m_fileSys].get();
      if (!GetVFS().m_vfsUseSecurePaths
          && dynamic_cast< StdioFileSystem * >(pFileSys) != nullptr) {
        const Path resolvedPath = inPath;
        if (func(itr->m_id, resolvedPath, ret, pFileSys)) {
          return true;
        }
      }

      if (itr->m_dest.isParent(inPath)) {
        const Path resolvedPath = itr->m_src + inPath.stripParent(itr->m_dest);
        if (func(itr->m_id, resolvedPath, ret, pFileSys)) {
          return true;
        }
      }
    }
    return false;
  }
};

/**
 *
 */
class OpenVisitor {
  public:
  OpenVisitor(const std::ios::openmode mode) : m_mode(mode) {}

  bool doOpen(
      tMountId mountId,
      const Path &resolvedPath,
      filters::streamfilter *&out,
      iFileSystem *pFileSys) {
    filters::BaseFsStreamFilter *pFile =
        pFileSys->open(mountId, resolvedPath, m_mode);
    if (pFile != nullptr) {
      pFile->setFileSystem(pFileSys);
      out = pFile;
      return true;
    }
    return false;
  }

  private:
  const std::ios::openmode m_mode;
};

/**
 *
 */
filters::streamfilter *open(const Path &path, const std::ios::openmode mode) {
  return GetVFS().open(path, mode);
}

/**
 *
 */
filters::streamfilter *
VfsDetail::open(const Path &path, const std::ios::openmode mode) {
  std::lock_guard< std::mutex > lock(m_mutex);
  filters::streamfilter *retVal = nullptr;
  OpenVisitor visitor(mode);
  FileSysVisitor< filters::streamfilter * >::tFunc visitorFunc =
      FileSysVisitor< filters::streamfilter * >::tFunc::
          from_method< OpenVisitor, &OpenVisitor::doOpen >(&visitor);
  FileSysVisitor< filters::streamfilter * >::findFileSysOperation(
      path, retVal, visitorFunc);
  return retVal;
}

/**
 *
 */
class StatVisitor {
  public:
  bool doStat(
      tMountId mountId,
      const Path &resolvedPath,
      FileStats &out,
      iFileSystem *pFileSys) {
    return pFileSys->stat(mountId, resolvedPath, out);
  }
};

/**
 *
 */
FileStats Stat(const Path &path) {
  return GetVFS().stat(path);
}

/**
 *
 */
FileStats VfsDetail::stat(const Path &path) {
  std::lock_guard< std::mutex > lock(m_mutex);
  FileStats retVal;
  StatVisitor visitor;
  FileSysVisitor< FileStats >::tFunc visitorFunc = FileSysVisitor< FileStats >::
      tFunc::from_method< StatVisitor, &StatVisitor::doStat >(&visitor);
  FileSysVisitor< FileStats >::findFileSysOperation(path, retVal, visitorFunc);
  return retVal;
}

namespace util {

/**
 *
 */
bool Copy(const Path &pathIn, const Path &pathOut) {
  Trace();
  Log(LL::Trace) << "Copying '" << pathIn.str() << "' to '" << pathOut.str()
                 << "'";
  filters::streamfilter *in = open(pathIn, std::ios::in | std::ios::binary);
  if (!in) {
    Log(LL::Error) << "Unable to open input for copy operation.";
    return false;
  }

  filters::streamfilter *out = open(pathOut, std::ios::out | std::ios::binary);
  if (!out) {
    Log(LL::Error) << "Unable to open output for copy operation.";
    close(in);
    return false;
  }

  bool rVal = true;
  char buffer[COPY_BUFFER_SZ];
  do {
    const size_t read = in->sgetn(buffer, COPY_BUFFER_SZ);
    if (read == 0) {
      break;
    }
    const size_t written = out->sputn(buffer, read);
    if (read != written) {
      rVal = false;
      break;
    }
  } while (true);

  close(in);
  close(out);
  return rVal;
}

/**
 *
 */
class MkDirVisitor {
  public:
  bool doMkdir(
      tMountId mountId,
      const Path &resolvedPath,
      bool &out,
      iFileSystem *pFileSys) {
    return pFileSys->mkdir(mountId, resolvedPath, out);
  }
};

/**
 *
 */
bool MkDir(const Path &path) {
  std::lock_guard< std::mutex > lock(GetVFS().m_mutex);
  FileStats stats = Stat(path);
  if (!stats.m_exists) {
    bool retVal = false;
    MkDirVisitor visitor;
    FileSysVisitor< bool >::tFunc visitorFunc = FileSysVisitor< bool >::tFunc::
        from_method< MkDirVisitor, &MkDirVisitor::doMkdir >(&visitor);
    FileSysVisitor< bool >::findFileSysOperation(path, retVal, visitorFunc);
    return retVal;
  } else {
    return stats.m_isDir;
  }
}

/**
 *
 */
class RmDirVisitor {
  public:
  bool doRmdir(
      tMountId mountId,
      const Path &resolvedPath,
      bool &out,
      iFileSystem *pFileSys) {
    return pFileSys->rmdir(mountId, resolvedPath, out);
  }
};

/**
 *
 */
bool RmDir(const Path &path) {
  std::lock_guard< std::mutex > lock(GetVFS().m_mutex);
  bool retVal = false;
  RmDirVisitor visitor;
  FileSysVisitor< bool >::tFunc visitorFunc = FileSysVisitor< bool >::tFunc::
      from_method< RmDirVisitor, &RmDirVisitor::doRmdir >(&visitor);
  FileSysVisitor< bool >::findFileSysOperation(path, retVal, visitorFunc);
  return retVal;
}

/**
 *
 */
class RemoveVisitor {
  public:
  bool doRemove(
      tMountId mountId,
      const Path &resolvedPath,
      bool &out,
      iFileSystem *pFileSys) {
    return pFileSys->remove(mountId, resolvedPath, out);
  }
};

/**
 *
 */
bool Remove(const Path &path) {
  std::lock_guard< std::mutex > lock(GetVFS().m_mutex);
  bool retVal = false;
  RemoveVisitor visitor;
  FileSysVisitor< bool >::tFunc visitorFunc = FileSysVisitor< bool >::tFunc::
      from_method< RemoveVisitor, &RemoveVisitor::doRemove >(&visitor);
  FileSysVisitor< bool >::findFileSysOperation(path, retVal, visitorFunc);
  return retVal;
}

/**
 * Internal iterator class that will walk through all the mount points in
 * reverse order, and output any iterations that it can find.
 */
class VFSDirectoryIterator
    : public iFileSystem::DirectoryIterator::iDirectoryIteratorImpl {
  public:
  /**
   *
   */
  VFSDirectoryIterator(const Path &root, bool recurse)
      : m_root(resolveOrDie(root)),
        m_recurse(recurse),
        m_lastMount(INVALID_MOUNT_ID) {
    findNextMount();
  }

  /**
   * Implements iteration over both files and possible mount points.
   */
  virtual bool next() override {
    m_child++;
    while (m_child.get() == iFileSystem::DirectoryNode()) {
      if (!findNextMount()) {
        return false;
      }
    }
    setTranslatedNode(m_child.get());
    return true;
  }

  private:
  const Path m_root;
  const bool m_recurse;
  tMountId m_lastMount;
  iFileSystem::DirectoryIterator m_child;

  /**
   * Resolves the input path for iteration.
   */
  static Path resolveOrDie(const Path &path) {
    Path inPath = path.resolve();
    if (path.empty()) {
      CHECK_M(
          !GetVFS().m_vfsUseSecurePaths, "Attempted to iterate invalid path.");
      inPath = path;
    }
    return inPath;
  }

  /**
   * Sets the output of this iterator.
   * Before doing so, translates the path from file system space into vfs space.
   */
  void setTranslatedNode(const iFileSystem::DirectoryNode &node) {
    if (!node.m_stats.m_exists) {
      setNode(node);
    }
    iFileSystem::DirectoryNode translatedNode = node;
    for (tMountList::iterator itr = GetVFS().m_mountPoints.begin();
         itr != GetVFS().m_mountPoints.end();
         ++itr) {
      if (itr->m_id == node.m_mountId) {
        translatedNode.m_path =
            itr->m_dest + node.m_path.stripParent(itr->m_src);
        setNode(translatedNode);
        return;
      }
    }
    CHECK_UNREACHABLE();
  }

  /**
   * Iterate through to the next mount point that matches the input path.
   */
  bool findNextMount() {
    tMountList::reverse_iterator itr = GetVFS().m_mountPoints.rbegin();
    if (m_lastMount != INVALID_MOUNT_ID) {
      while (itr != GetVFS().m_mountPoints.rend() && itr->m_id != m_lastMount) {
        ++itr;
      }
      if (itr != GetVFS().m_mountPoints.rend()) {
        ++itr;
      }
    }

    // Find all mount points with path as a parrent
    for (; itr != GetVFS().m_mountPoints.rend(); ++itr) {
      iFileSystem *pFileSys = GetVFS().m_fileSystems[itr->m_fileSys].get();
      Path resolvedPath;
      if (itr->m_dest.isParent(m_root)) {
        resolvedPath = itr->m_src + m_root.stripParent(itr->m_dest);
      } else if (m_recurse && m_root.isParent(itr->m_dest)) {
        resolvedPath = itr->m_src + m_root;
      }

      if (!resolvedPath.empty()) {
        m_lastMount = itr->m_id;
        m_child = pFileSys->iterate(itr->m_id, resolvedPath, m_recurse);
        if (m_child.get() != iFileSystem::DirectoryNode()) {
          setTranslatedNode(m_child.get());
          return m_child.get().m_stats.m_exists;
        }
      }
    }
    return false;
  }
};

/**
 *
 */
iFileSystem::DirectoryIterator List(const Path &path, bool recurse) {
  return iFileSystem::DirectoryIterator(
      std::shared_ptr< iFileSystem::DirectoryIterator::iDirectoryIteratorImpl >(
          new VFSDirectoryIterator(path, recurse)));
}

/**
 *
 */
Path GetTempDir() {
  return Path(TMP_PATH);
}

/**
 *
 */
Path GetTempFile() {
  static std::atomic_int32_t uniqueFileCounter = 0;
  std::string fileName;
  core::util::lexical_cast(uniqueFileCounter++, fileName).ignoreErrors();
  return Path("tmp/" + fileName);
}

} // namespace util

/**
 *
 */
void SetSecurePathing(bool useSecurePaths) {
  GetVFS().setSecurePathing(useSecurePaths);
}

/**
 *
 */
void VfsDetail::setSecurePathing(bool useSecurePaths) {
  std::lock_guard< std::mutex > lock(m_mutex);
  m_vfsUseSecurePaths = useSecurePaths;
}

} // namespace vfs
