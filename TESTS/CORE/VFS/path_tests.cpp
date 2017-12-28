#include <TESTS/testcase.h>
#include <TESTS/test_assertions.h>

#include <CORE/types.h>
#include <CORE/VFS/path.h>

using vfs::Path;

REGISTER_TEST_CASE(testSetPath) {
  Path result1 = Path("foo/") + Path("baz");
  Path result2 = Path("foo\\") + Path("baz");
  std::string expected = "foo/baz";

  TEST(testing::assertEquals(expected, result1.str()));
  TEST(testing::assertEquals(expected, result2.str()));
}

REGISTER_TEST_CASE(testPathFile) {
  std::string result1 = Path("foo/bar").file();
  std::string expected1 = "bar";
  TEST(testing::assertEquals(expected1, result1));

  std::string result2 = Path("foo/bar.exe").file();
  std::string expected2 = "bar.exe";
  TEST(testing::assertEquals(expected2, result2));

  std::string result3 = Path("foo/bar/").file();
  std::string expected3 = "";
  TEST(testing::assertEquals(expected3, result3));
}

REGISTER_TEST_CASE(testPathBaseFile) {
  std::string result1 = Path("foo/bar").baseFile();
  std::string expected1 = "bar";
  TEST(testing::assertEquals(expected1, result1));

  std::string result2 = Path("foo/bar.exe").baseFile();
  std::string expected2 = "bar";
  TEST(testing::assertEquals(expected2, result2));

  std::string result3 = Path("foo/bar/").baseFile();
  std::string expected3 = "";
  TEST(testing::assertEquals(expected3, result3));
}

REGISTER_TEST_CASE(testPathExtension) {
  std::string result1 = Path("foo/bar").extension();
  std::string expected1 = "";
  TEST(testing::assertEquals(expected1, result1));

  std::string result2 = Path("foo/bar.exe").extension();
  std::string expected2 = "exe";
  TEST(testing::assertEquals(expected2, result2));

  std::string result3 = Path("foo/bar/").extension();
  std::string expected3 = "";
  TEST(testing::assertEquals(expected3, result3));
}

REGISTER_TEST_CASE(testPathDir) {
  std::string result1 = Path("foo/bar").dir();
  std::string expected1 = "foo/";
  TEST(testing::assertEquals(expected1, result1));

  std::string result2 = Path("foo/bar/").dir();
  std::string expected2 = "foo/bar/";
  TEST(testing::assertEquals(expected2, result2));

  std::string result3 = Path("foo").dir();
  std::string expected3 = "./";
  TEST(testing::assertEquals(expected3, result3));
}

REGISTER_TEST_CASE(testPathResolve) {
  Path result1 = Path().resolve();
  TEST(testing::assertTrue(result1.empty()));

  Path result2 = Path("./bar").resolve();
  std::string expected2 = "bar";
  TEST(testing::assertEquals(expected2, result2.str()));

  Path result3 = Path("../bar").resolve();
  TEST(testing::assertTrue(result3.empty()));

  Path result4 = Path("baz/../bar").resolve();
  std::string expected4 = "bar";
  TEST(testing::assertEquals(expected4, result4.str()));

  Path result5 = Path("baz/./bar").resolve();
  std::string expected5 = "baz/bar";
  TEST(testing::assertEquals(expected5, result5.str()));
}

REGISTER_TEST_CASE(testPathParent) {
  TEST(testing::assertTrue(Path("foo").isParent(Path("foo/bar"))));
  TEST(testing::assertFalse(Path("foo").isParent(Path("baz/bar"))));
}

REGISTER_TEST_CASE(testPathStripParent) {
  const Path root("foo/bar");

  Path result1 = root.stripParent(Path("foo/"));
  std::string expected1 = "bar";
  TEST(testing::assertEquals(expected1, result1.str()));
}
