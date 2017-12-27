#include <TESTS/test_assertions.h>
#include <TESTS/testcase.h>

#include <CORE/VFS/FILESYSTEMS/filesys_mem.h>
#include <CORE/types.h>

using vfs::FileStats;
using vfs::MemFileSystem;
using vfs::Path;
using vfs::tMountId;

REGISTER_TEST_CASE(testMemFileMountUnmount) {
  MemFileSystem filesys;

  const vfs::tMountId mountId1 = 1;
  TEST(testing::assertEquals(
      filesys.mount(mountId1, "memfile/", std::ios::in | std::ios::binary)
          .getStatus(),
      Status::OK));
  TEST(
      testing::assertEquals(filesys.unmount(mountId1).getStatus(), Status::OK));

  const vfs::tMountId mountId2 = 2;
  TEST(testing::assertEquals(
      filesys.mount(mountId2, "./", std::ios::in | std::ios::binary)
          .getStatus(),
      Status::BAD_ARGUMENT));
  TEST(testing::assertEquals(
      filesys.unmount(mountId2).getStatus(), Status::OUT_OF_BOUNDS));
}

REGISTER_TEST_CASE(testMemFileOpenBadMode) {
  MemFileSystem filesys;
  const vfs::tMountId mountId = 1;
  TEST(testing::assertEquals(
      filesys.mount(mountId, "memfile/", std::ios::in | std::ios::binary)
          .getStatus(),
      Status::OK));

  const vfs::Path filename("memfile/test.txt");
  vfs::filters::BaseFsStreamFilter *pFile = nullptr;
  TEST(testing::assertEquals(
      filesys.open(pFile, mountId, filename, std::ios::out | std::ios::binary)
          .getStatus(),
      Status::BAD_ARGUMENT));
  TEST(testing::assertNull(pFile));
}

REGISTER_TEST_CASE(testMemFileOpenBadFile) {
  MemFileSystem filesys;
  const vfs::tMountId mountId = 1;
  TEST(testing::assertEquals(
      filesys.mount(mountId, "memfile/", std::ios::in | std::ios::binary)
          .getStatus(),
      Status::OK));

  const vfs::Path filename("memfile/test.txt");
  vfs::filters::BaseFsStreamFilter *pFile = nullptr;
  TEST(testing::assertEquals(
      filesys.open(pFile, mountId, filename, std::ios::in | std::ios::binary)
          .getStatus(),
      Status::NOT_FOUND));
  TEST(testing::assertNull(pFile));
}

REGISTER_TEST_CASE(testMemFileOpenBadFileMode) {
  MemFileSystem filesys;
  const vfs::tMountId mountId = 1;
  TEST(testing::assertEquals(
      filesys
          .mount(
              mountId,
              "memfile/",
              std::ios::in | std::ios::out | std::ios::binary)
          .getStatus(),
      Status::OK));

  const std::string expectedContent = "This is a file.";
  const core::memory::ConstBlob testBlob(expectedContent);
  const vfs::Path filename("memfile/test.txt");
  TEST(testing::assertTrue(filesys.create(filename, testBlob)));

  vfs::filters::BaseFsStreamFilter *pFile = nullptr;
  TEST(testing::assertEquals(
      filesys.open(pFile, mountId, filename, std::ios::out | std::ios::binary)
          .getStatus(),
      Status::BAD_ARGUMENT));
  TEST(testing::assertNull(pFile));
}

REGISTER_TEST_CASE(testMemFileCreateRemove) {
  MemFileSystem filesys;

  std::string expectedContent = "This is a file.";

  const core::memory::ConstBlob testBlob1(expectedContent);
  const vfs::Path filename1("memfile/test1.txt");
  TEST(testing::assertTrue(filesys.create(filename1, testBlob1)));

  const core::memory::Blob testBlob2(expectedContent);
  const vfs::Path filename2("memfile/test2.txt");
  TEST(testing::assertTrue(filesys.create(filename2, testBlob2)));

  TEST(testing::assertFalse(filesys.create(filename1, testBlob1)));
  TEST(testing::assertFalse(filesys.create(filename2, testBlob2)));

  bool ret = false;
  TEST(testing::assertTrue(
      filesys.remove(vfs::INVALID_MOUNT_ID, filename1, ret)));
  TEST(testing::assertTrue(ret));
  TEST(testing::assertTrue(
      filesys.remove(vfs::INVALID_MOUNT_ID, filename1, ret)));
  TEST(testing::assertFalse(ret));
  TEST(testing::assertFalse(
      filesys.remove(vfs::INVALID_MOUNT_ID, vfs::Path(), ret)));
}

REGISTER_TEST_CASE(testMemMkdirRmdir) {
  MemFileSystem filesys;

  const vfs::Path path("some/path/");

  bool ret = false;
  TEST(testing::assertTrue(filesys.mkdir(vfs::INVALID_MOUNT_ID, path, ret)));
  TEST(testing::assertTrue(ret));

  ret = false;
  TEST(testing::assertFalse(
      filesys.mkdir(vfs::INVALID_MOUNT_ID, vfs::Path(), ret)));
  TEST(testing::assertFalse(ret));

  ret = false;
  TEST(testing::assertTrue(filesys.rmdir(vfs::INVALID_MOUNT_ID, path, ret)));
  TEST(testing::assertTrue(ret));

  ret = false;
  TEST(testing::assertFalse(
      filesys.rmdir(vfs::INVALID_MOUNT_ID, vfs::Path(), ret)));
  TEST(testing::assertFalse(ret));
}

REGISTER_TEST_CASE(testMemStat) {
  MemFileSystem filesys;

  FileStats stats;
  TEST(testing::assertFalse(
      filesys.stat(vfs::INVALID_MOUNT_ID, vfs::Path(), stats)));
  TEST(testing::assertEquals(
      filesys.stat(vfs::INVALID_MOUNT_ID, "memfile/test.txt", stats)
          .getStatus(),
      Status::OK));
  TEST(testing::assertFalse(stats.m_exists));

  const std::string expectedContent = "This is a file.";
  const core::memory::ConstBlob testBlob(expectedContent);
  const vfs::Path filename("memfile/test.txt");
  TEST(testing::assertTrue(filesys.create(filename, testBlob)));

  TEST(testing::assertEquals(
      filesys.stat(vfs::INVALID_MOUNT_ID, "memfile/test.txt", stats)
          .getStatus(),
      Status::OK));
  TEST(testing::assertTrue(stats.m_exists));
  TEST(testing::assertFalse(stats.m_isDir));
  TEST(testing::assertEquals(stats.m_size, 15));
  TEST(testing::assertTrue(stats.m_modifiedTime > 0));
}

REGISTER_TEST_CASE(testMemConstFileEndToEnd) {
  MemFileSystem filesys;
  const vfs::tMountId mountId = 1;
  TEST(testing::assertEquals(
      filesys.mount(mountId, "memfile/", std::ios::in | std::ios::binary)
          .getStatus(),
      Status::OK));

  std::string expectedContent = "badfoodbadfoodbadfoodbadfood";
  core::memory::Blob testBlob(expectedContent);
  const vfs::Path filename("memfile/test.txt");

  TEST(testing::assertTrue(filesys.create(filename, testBlob)));
  vfs::filters::BaseFsStreamFilter *pFile = nullptr;
  TEST(testing::assertEquals(
      filesys.open(pFile, mountId, filename, std::ios::in | std::ios::binary)
          .getStatus(),
      Status::OK));
  TEST(testing::assertNotNull(pFile));
  TEST(testing::assertEquals(
      std::string(pFile->getFilterName()), std::string("memfilehandle")));
  TEST(testing::assertEquals(pFile->length(), 0));

  pFile->sputn("Hello world", 11);
  pFile->pubseekpos(0, std::ios::beg);
  char actualContent[16] = {0};
  pFile->sgetn(actualContent, 15);
  const std::string expectedNewContent = "Hello world";
  TEST(testing::assertEquals(actualContent, expectedNewContent));
  filesys.close(pFile);

  bool ret = false;
  TEST(testing::assertEquals(
      filesys.remove(mountId, filename, ret).getStatus(), Status::OK));
  TEST(testing::assertTrue(ret));
  TEST(testing::assertEquals(filesys.unmount(mountId).getStatus(), Status::OK));
}
