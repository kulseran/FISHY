#include <TESTS/testcase.h>
#include <TESTS/test_assertions.h>

#include <CORE/types.h>
#include <CORE/VFS/FILESYSTEMS/filesys_stdio.h>

//*******
#include <CORE/ARCH/timer.h>
#include <CORE/VFS/vfs.h>
#include <CORE/UTIL/stringutil.h>

#include <iomanip>
//*******

using vfs::tMountId;
using vfs::StdioFileSystem;
using vfs::Path;

REGISTER_TEST_CASE(testStdioDirectoryIterator) {
  /*
    vfs::tMountId id = 1;
    StdioFileSystem fs;
    fs.mount(id, "../TESTS/", std::ios::in);
    vfs::iFileSystem::DirectoryIterator itr = fs.iterate(id, Path("../TESTS/"), true);
    do {
      std::cout << "F:" << itr.get().m_path.str() << std::endl;
    } while (++itr != vfs::iFileSystem::DirectoryIterator());
    */
  /*
  vfs::iFileSystem::DirectoryIterator itr = vfs::util::List(Path("./"), true);
  do {
    const vfs::iFileSystem::DirectoryNode node = itr.get();
    std::cout << "M:" << node.m_mountId << " D:" << node.m_stats.m_isDir << " SZ:" << std::setw(8) << core::util::pretySize(node.m_stats.m_size) << " T:" << node.m_stats.m_modifiedTime << " F: " << node.m_path.c_str() << std::endl;
  } while (++itr != vfs::iFileSystem::DirectoryIterator());
  */
}

// TEST_CASE_DEBUG_SCOPE;
