#include <TESTS/test_assertions.h>
#include <TESTS/testcase.h>

#include <CORE/VFS/FILESYSTEMS/filesys_stdio.h>
#include <CORE/types.h>

using vfs::FileStats;
using vfs::Path;
using vfs::StdioFileSystem;
using vfs::tMountId;

REGISTER_TEST_CASE(testStdioFileMountUnmount) {
  StdioFileSystem filesys;

  const vfs::tMountId mountId = 1;
  TEST(testing::assertEquals(
      filesys.mount(mountId, "./", std::ios::in | std::ios::binary).getStatus(),
      Status::OK));
  TEST(testing::assertEquals(filesys.unmount(mountId).getStatus(), Status::OK));
}

REGISTER_TEST_CASE(testStdioFileOpenBadMode) {
  StdioFileSystem filesys;

  const vfs::tMountId mountId = 1;
  TEST(testing::assertEquals(
      filesys.mount(mountId, "./", std::ios::in | std::ios::binary).getStatus(),
      Status::OK));

  const vfs::Path filename("./test.txt");
  vfs::filters::BaseFsStreamFilter *pFile = nullptr;
  TEST(testing::assertEquals(
      filesys.open(pFile, mountId, filename, std::ios::out | std::ios::binary)
          .getStatus(),
      Status::BAD_ARGUMENT));
  TEST(testing::assertNull(pFile));
}

REGISTER_TEST_CASE(testStdioFileOpenBadFile) {
  StdioFileSystem filesys;
  const vfs::tMountId mountId = 1;
  TEST(testing::assertEquals(
      filesys.mount(mountId, "./", std::ios::in | std::ios::binary).getStatus(),
      Status::OK));

  const vfs::Path filename("./test.txt");
  vfs::filters::BaseFsStreamFilter *pFile = nullptr;
  TEST(testing::assertEquals(
      filesys.open(pFile, mountId, filename, std::ios::in | std::ios::binary)
          .getStatus(),
      Status::NOT_FOUND));
  TEST(testing::assertNull(pFile));
}

REGISTER_TEST_CASE(testStdioStat) {
  StdioFileSystem filesys;

  const tMountId mountId = 1;
  TEST(testing::assertEquals(
      filesys.mount(mountId, "./", std::ios::in | std::ios::binary).getStatus(),
      Status::OK));

  FileStats stats;
  TEST(testing::assertFalse(filesys.stat(mountId, vfs::Path(), stats)));
  TEST(testing::assertEquals(
      filesys.stat(mountId, "./NOEXIST/", stats).getStatus(),
      Status::NOT_FOUND));
  TEST(testing::assertFalse(stats.m_exists));

  TEST(testing::assertEquals(
      filesys.stat(mountId, "TESTS/CORE/VFS/FILESYSTEMS/testdata/", stats)
          .getStatus(),
      Status::OK));
  TEST(testing::assertTrue(stats.m_exists));
  TEST(testing::assertTrue(stats.m_isDir));
  TEST(testing::assertEquals(stats.m_size, 0));
  TEST(testing::assertTrue(stats.m_modifiedTime > 0));

  TEST(testing::assertEquals(
      filesys
          .stat(mountId, "TESTS/CORE/VFS/FILESYSTEMS/testdata/test.txt", stats)
          .getStatus(),
      Status::OK));
  TEST(testing::assertTrue(stats.m_exists));
  TEST(testing::assertFalse(stats.m_isDir));
  TEST(testing::assertEquals(stats.m_size, 14));
  TEST(testing::assertTrue(stats.m_modifiedTime > 0));
}
