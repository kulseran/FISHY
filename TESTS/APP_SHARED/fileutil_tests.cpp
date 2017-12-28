#include <TESTS/test_assertions.h>
#include <TESTS/testcase.h>

#include <APP_SHARED/fileutil.h>

// #include "testproto.pb.h"

using appshared::parseFileToString;

/*
REGISTER_TEST_CASE(testParseProtoFromFile) {
  test::TestProto data;
  bool ret =
appshared::parseProtoFromFile(vfs::Path("./TESTDATA/fileutil_test1.pb"), data);

  TEST(testing::assertTrue(ret));
  TEST(testing::assertEquals(10, data.get_a_int()));
  TEST(testing::assertEquals(std::string("hello world"), data.get_a_string()));
}
*/

REGISTER_TEST_CASE(testParseFileToString) {
  const vfs::Path filename = "./TESTS/APP_SHARED/testdata/testfile.txt";
  const std::string expected =
      "a file\nfilled with text\nthat takes up 3 lines\n";

  std::string result;
  TEST(testing::assertEquals(
      parseFileToString(filename, result).getStatus(), Status::OK));
  TEST(testing::assertEquals(result, expected));
}
