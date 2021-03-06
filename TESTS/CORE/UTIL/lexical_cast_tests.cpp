#include <TESTS/test_assertions.h>
#include <TESTS/testcase.h>

#include <CORE/UTIL/lexical_cast.h>

#include <vector>

using namespace core::util;

REGISTER_TEST_CASE(testValidCastsFromChar) {
  const std::string source = "1.1";
  f32 floatOut = 0.0f;
  TEST(testing::assertTrue(lexical_cast(source, floatOut)));
  TEST(testing::assertEquals(floatOut, 1.1f));

  f64 doubleOut = 0.0;
  TEST(testing::assertTrue(lexical_cast(source, doubleOut)));
  TEST(testing::assertEquals(doubleOut, 1.1));

  u32 u32Out = 0;
  TEST(testing::assertTrue(lexical_cast(source, u32Out)));
  TEST(testing::assertEquals(u32Out, 1u));

  s32 s32Out = 0;
  TEST(testing::assertTrue(lexical_cast(source, s32Out)));
  TEST(testing::assertEquals(s32Out, 1));
}

REGISTER_TEST_CASE(testValidCastsToString) {
  std::string floatOut;
  TEST(testing::assertTrue(lexical_cast(1.1f, floatOut)));
  TEST(testing::assertEquals(floatOut, "1.1"));

  std::string doubleOut;
  TEST(testing::assertTrue(lexical_cast(1.1, doubleOut)));
  TEST(testing::assertEquals(doubleOut, "1.1"));

  std::string u32Out;
  TEST(testing::assertTrue(lexical_cast(1u, u32Out)));
  TEST(testing::assertEquals(u32Out, "1"));

  std::string s32Out;
  TEST(testing::assertTrue(lexical_cast(1, s32Out)));
  TEST(testing::assertEquals(s32Out, "1"));
}

REGISTER_TEST_CASE(testInvalidCasts) {
  std::vector< int > aVec;
  int anInt;
  TEST(testing::assertFalse(lexical_cast(aVec, anInt)));
  TEST(testing::assertFalse(lexical_cast(anInt, aVec)));
}
