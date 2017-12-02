#include <TESTS/test_assertions.h>
#include <TESTS/testcase.h>

#include <CORE/BASE/config.h>
#include <CORE/types.h>

using core::config::Flag;
using core::config::ParseFlags;

Flag< int > g_testFlag1("testFlag1", "desc", 0);
Flag< std::string > g_testFlag2("testFlag2", "desc", "defaultvalue");
Flag< std::string > g_testFlag3("testFlag3", "desc", "defaultvalue");

REGISTER_TEST_CASE(testDefaults) {
  const char *defaultArgs[] = {"filename"};
  bool ret = ParseFlags(ARRAY_LENGTH(defaultArgs), defaultArgs);
  TEST(testing::assertTrue(ret));
  TEST(testing::assertEquals(g_testFlag1.get(), 0));
  TEST(testing::assertEquals(g_testFlag2.get(), "defaultvalue"));
}

REGISTER_TEST_CASE(testParseValidEquals) {
  const char *defaultArgs[] = {"filename", "--testFlag1=3", "--testFlag2=hats"};
  bool ret = ParseFlags(ARRAY_LENGTH(defaultArgs), defaultArgs);

  TEST(testing::assertTrue(ret));
  TEST(testing::assertEquals(g_testFlag1.get(), 3));
  TEST(testing::assertEquals(g_testFlag2.get(), "hats"));
}

REGISTER_TEST_CASE(testParseBadValue) {
  const char *defaultArgs[] = {"filename", "testFlag1=3"};
  bool ret = ParseFlags(ARRAY_LENGTH(defaultArgs), defaultArgs);

  TEST(testing::assertFalse(ret));
}

REGISTER_TEST_CASE(testParseBadFlag) {
  const char *defaultArgs[] = {"filename", "unknown_flag=3"};
  bool ret = ParseFlags(ARRAY_LENGTH(defaultArgs), defaultArgs);

  TEST(testing::assertFalse(ret));
}

REGISTER_TEST_CASE(testParseConfig) {
  const char *defaultArgs[] = {
      "filename",
      "--config_file=../../tests/core/base/testdata/config_file_test.txt"};
  bool ret = ParseFlags(ARRAY_LENGTH(defaultArgs), defaultArgs);

  TEST(testing::assertTrue(ret));
  TEST(testing::assertEquals(g_testFlag1.get(), 3));
  TEST(testing::assertEquals(g_testFlag2.get(), "hats"));
  TEST(testing::assertEquals(g_testFlag3.get(), "cats"));
}
