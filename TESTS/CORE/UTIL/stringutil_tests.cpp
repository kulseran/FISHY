#include <TESTS/test_assertions.h>
#include <TESTS/testcase.h>

#include <CORE/UTIL/stringutil.h>
#include <CORE/types.h>

using core::util::escape;
using core::util::identifierSafe;
using core::util::Joiner;
using core::util::prettySize;
using core::util::replaceStr;
using core::util::Splitter;
using core::util::trimQuotes;
using core::util::trimWhitespace;
using core::util::unescape;

static const std::string testString1 = "a, b, c, d";

REGISTER_TEST_CASE(testTrimWs) {
  const std::string testString = "   value   e   ";
  std::string result = trimWhitespace(testString);
  std::string expected = "value   e";

  TEST(testing::assertEquals(expected, result));
}

REGISTER_TEST_CASE(testTrimQuotes) {
  const std::string testString = "\"value\"";
  std::string result = trimQuotes(testString);
  std::string expected = "value";

  TEST(testing::assertEquals(expected, result));
}

REGISTER_TEST_CASE(testSplitDefault) {
  std::vector< std::string > result = Splitter().split(testString1);
  // Splits on ' ', expect values with commas.
  std::vector< std::string > expected;
  expected.push_back("a,");
  expected.push_back("b,");
  expected.push_back("c,");
  expected.push_back("d");

  TEST(testing::assertEquals(expected, result));
}

REGISTER_TEST_CASE(testSplitLimit) {
  std::vector< std::string > result = Splitter().split(testString1, 2);
  // Split count 2, expect only 2 strings
  std::vector< std::string > expected;
  expected.push_back("a,");
  expected.push_back("b, c, d");

  TEST(testing::assertEquals(expected, result));
}

REGISTER_TEST_CASE(testSplitCustomSep) {
  std::vector< std::string > result = Splitter().on(',').split(testString1);
  // Split on ',', expect strings with whitespace
  std::vector< std::string > expected;
  expected.push_back("a");
  expected.push_back(" b");
  expected.push_back(" c");
  expected.push_back(" d");

  TEST(testing::assertEquals(expected, result));
}

REGISTER_TEST_CASE(testSplitTrim) {
  std::vector< std::string > result =
      Splitter().on(',').trimWhitespace().split(testString1);
  // Split on ',', trimming should result in no whitespace
  std::vector< std::string > expected;
  expected.push_back("a");
  expected.push_back("b");
  expected.push_back("c");
  expected.push_back("d");

  TEST(testing::assertEquals(expected, result));
}

REGISTER_TEST_CASE(testJoinDefault) {
  std::vector< std::string > list =
      Splitter().on(',').trimWhitespace().split(testString1);
  std::string result = Joiner().join(list.begin(), list.end());
  // Joining with ", " should should result in original string
  std::string expected = testString1;

  TEST(testing::assertEquals(expected, result));
}

REGISTER_TEST_CASE(testJoinCustomSep) {
  std::vector< std::string > list =
      Splitter().on(',').trimWhitespace().split(testString1);
  std::string result = Joiner().on("-").join(list.begin(), list.end());
  // Using different join character, expects replace ', ' between letters
  std::string expected = "a-b-c-d";

  TEST(testing::assertEquals(expected, result));
}

REGISTER_TEST_CASE(testEscape) {
  const std::string input = "\n\r\t\"\\";
  const std::string expected = "\\n\\r\\t\\\"\\\\";

  const std::string result = escape(input);
  TEST(testing::assertEquals(expected, result));
}

REGISTER_TEST_CASE(testUnescape) {
  const std::string input = "\\b\\n\\r\\t\\\"\\\\";
  const std::string expected = "\\b\n\r\t\"\\";

  const std::string result = unescape(input);
  TEST(testing::assertEquals(expected, result));
}

REGISTER_TEST_CASE(testEscapeUnescapeNonprintable) {
  const std::string input = "\xFF";
  const std::string expected = "\\255";

  const std::string result = escape(input);
  TEST(testing::assertEquals(expected, result));
  const std::string result2 = unescape(result);
  TEST(testing::assertEquals(input, result2));
}

REGISTER_TEST_CASE(testReplaceEmpty) {
  const std::string source = "foobarfoobazfoobizfoo";
  const std::string match = "foo";
  const std::string replace = "";
  const std::string expected = "barbazbiz";
  TEST(testing::assertEquals(expected, replaceStr(source, match, replace)));
}

REGISTER_TEST_CASE(testReplaceLarger) {
  const std::string source = "foobarfoobazfoobizfoo";
  const std::string match = "foo";
  const std::string replace = "fizzbiz";
  const std::string expected = "fizzbizbarfizzbizbazfizzbizbizfizzbiz";
  TEST(testing::assertEquals(expected, replaceStr(source, match, replace)));
}

REGISTER_TEST_CASE(testIdentiferSafe) {
  const std::string source = "9a0d9 asc[p.c ad][a 01293asdf";
  const std::string expected = "_a0d9_asc_p_c_ad__a_01293asdf";
  TEST(testing::assertEquals(expected, identifierSafe(source)));
}

REGISTER_TEST_CASE(testPretySize) {
  TEST(testing::assertEquals("1 B", prettySize(1ull)));
  TEST(testing::assertEquals("1 KiB", prettySize(1024ull)));
  TEST(testing::assertEquals("1 MiB", prettySize(1024ull * 1024)));
  TEST(testing::assertEquals("1 GiB", prettySize(1024ull * 1024 * 1024)));
  TEST(
      testing::assertEquals("1 TiB", prettySize(1024ull * 1024 * 1024 * 1024)));
  TEST(testing::assertEquals(
      "1 PiB", prettySize(1024ull * 1024 * 1024 * 1024 * 1024)));
}
