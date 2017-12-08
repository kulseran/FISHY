#include <TESTS/test_assertions.h>
#include <TESTS/testcase.h>

#include <CORE/UTIL/regex.h>
#include <CORE/types.h>

using namespace core::util::parser;

REGISTER_TEST_CASE(testLiteralMatch) {
  RegExPattern pattern("ab");
  const std::string input = "ab";
  TEST(testing::assertTrue(pattern.match(input.begin(), input.end())));
}

REGISTER_TEST_CASE(testAnyMatch) {
  RegExPattern pattern(".");
  const std::string input = "a";
  TEST(testing::assertTrue(pattern.match(input.begin(), input.end())));
}

REGISTER_TEST_CASE(testKleinStar) {
  RegExPattern pattern("a*b");
  const std::string input = "ab";
  const std::string input2 = "aaaab";
  const std::string input3 = "b";
  TEST(testing::assertTrue(pattern.match(input.begin(), input.end())));
  TEST(testing::assertTrue(pattern.match(input2.begin(), input2.end())));
  TEST(testing::assertTrue(pattern.match(input3.begin(), input3.end())));
}

REGISTER_TEST_CASE(testZeroOrOne) {
  RegExPattern pattern("a?b");
  const std::string input = "b";
  const std::string input2 = "ab";
  const std::string input3 = "aab";
  TEST(testing::assertTrue(pattern.match(input.begin(), input.end())));
  TEST(testing::assertTrue(pattern.match(input2.begin(), input2.end())));
  TEST(testing::assertFalse(pattern.match(input3.begin(), input3.end())));
}

REGISTER_TEST_CASE(testOneOrMore) {
  RegExPattern pattern("a+b");
  const std::string input = "ab";
  const std::string input2 = "aaaab";
  const std::string input3 = "b";
  TEST(testing::assertTrue(pattern.match(input.begin(), input.end())));
  TEST(testing::assertTrue(pattern.match(input2.begin(), input2.end())));
  TEST(testing::assertFalse(pattern.match(input3.begin(), input3.end())));
}

REGISTER_TEST_CASE(testGroup) {
  RegExPattern pattern("(ab)+c");
  const std::string input = "abc";
  const std::string input2 = "ababc";
  const std::string input3 = "c";
  TEST(testing::assertTrue(pattern.match(input.begin(), input.end())));
  TEST(testing::assertTrue(pattern.match(input2.begin(), input2.end())));
  TEST(testing::assertFalse(pattern.match(input3.begin(), input3.end())));
}

REGISTER_TEST_CASE(testRange) {
  RegExPattern pattern("[ab]c");
  const std::string input = "ac";
  const std::string input2 = "bc";
  const std::string input3 = "dc";
  TEST(testing::assertTrue(pattern.match(input.begin(), input.end())));
  TEST(testing::assertTrue(pattern.match(input2.begin(), input2.end())));
  TEST(testing::assertFalse(pattern.match(input3.begin(), input3.end())));
}

REGISTER_TEST_CASE(testRangeNegate) {
  RegExPattern pattern("[^a]c");
  const std::string input = "ac";
  const std::string input2 = "bc";
  TEST(testing::assertFalse(pattern.match(input.begin(), input.end())));
  TEST(testing::assertTrue(pattern.match(input2.begin(), input2.end())));
}

REGISTER_TEST_CASE(testRangeAlpha) {
  RegExPattern pattern("[\\a]+");
  const std::string input =
      "qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM";
  const std::string input2 = "1234567890";
  TEST(testing::assertTrue(pattern.match(input.begin(), input.end())));
  TEST(testing::assertFalse(pattern.match(input2.begin(), input2.end())));
}

REGISTER_TEST_CASE(testRangeDigit) {
  RegExPattern pattern("[\\d]+");
  const std::string input = "1234567890";
  const std::string input2 =
      "qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM";
  TEST(testing::assertTrue(pattern.match(input.begin(), input.end())));
  TEST(testing::assertFalse(pattern.match(input2.begin(), input2.end())));
}

REGISTER_TEST_CASE(testRangeWord) {
  RegExPattern pattern("[\\w]+");
  const std::string input =
      "1234567890_qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM";
  const std::string input2 = "[];'";
  TEST(testing::assertTrue(pattern.match(input.begin(), input.end())));
  TEST(testing::assertFalse(pattern.match(input2.begin(), input2.end())));
}

REGISTER_TEST_CASE(testRangeWhitespace) {
  RegExPattern pattern("[\\s]+");
  const std::string input = " \t\n\r";
  const std::string input2 = "[];'";
  TEST(testing::assertTrue(pattern.match(input.begin(), input.end())));
  TEST(testing::assertFalse(pattern.match(input2.begin(), input2.end())));
}

REGISTER_TEST_CASE(testFloatNumeric) {
  RegExPattern pattern("[-\\+]?\\d+\\.?\\d*");
  const std::string input = "10";
  const std::string input2 = "0.10";
  const std::string input3 = "10.10";
  const std::string input4 = "-10.10";
  TEST(testing::assertTrue(pattern.match(input.begin(), input.end())));
  TEST(testing::assertTrue(pattern.match(input2.begin(), input2.end())));
  TEST(testing::assertTrue(pattern.match(input3.begin(), input3.end())));
  TEST(testing::assertTrue(pattern.match(input4.begin(), input4.end())));
}
