#include <TESTS/test_assertions.h>
#include <TESTS/testcase.h>

#include <CORE/UTIL/stringutil.h>

using core::util::CountLines;
using core::util::Escape;
using core::util::IdentifierSafe;
using core::util::Joiner;
using core::util::PrettySize;
using core::util::ReplaceStr;
using core::util::Splitter;
using core::util::TrimQuotes;
using core::util::TrimWhitespace;
using core::util::Unescape;

static const std::string testString1 = "a, b, c, d";

REGISTER_TEST_CASE(testTrimWs) {
  const std::string testString = "   value   e   ";
  std::string result = TrimWhitespace(testString);
  std::string expected = "value   e";

  TEST(testing::assertEquals(result, expected));
}

REGISTER_TEST_CASE(testTrimWsEmpty) {
  TEST(testing::assertEquals(TrimWhitespace(""), ""));
}

REGISTER_TEST_CASE(testTrimQuotes) {
  const std::string testString = "\"value\"";
  std::string result = TrimQuotes(testString);
  std::string expected = "value";

  TEST(testing::assertEquals(result, expected));
}

REGISTER_TEST_CASE(testTrimQuotesEmpty) {
  TEST(testing::assertEquals(TrimQuotes(""), ""));
}

REGISTER_TEST_CASE(testSplitDefault) {
  std::vector< std::string > result = Splitter().split(testString1);
  // Splits on ' ', expect values with commas.
  std::vector< std::string > expected;
  expected.push_back("a,");
  expected.push_back("b,");
  expected.push_back("c,");
  expected.push_back("d");

  TEST(testing::assertEquals(result, expected));
}

REGISTER_TEST_CASE(testSplitLimit) {
  std::vector< std::string > result = Splitter().split(testString1, 2);
  // Split count 2, expect only 2 strings
  std::vector< std::string > expected;
  expected.push_back("a,");
  expected.push_back("b, c, d");

  TEST(testing::assertEquals(result, expected));
}

REGISTER_TEST_CASE(testSplitCustomSep) {
  std::vector< std::string > result = Splitter().on(',').split(testString1);
  // Split on ',', expect strings with whitespace
  std::vector< std::string > expected;
  expected.push_back("a");
  expected.push_back(" b");
  expected.push_back(" c");
  expected.push_back(" d");

  TEST(testing::assertEquals(result, expected));
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

  TEST(testing::assertEquals(result, expected));
}

REGISTER_TEST_CASE(testJoinDefault) {
  std::vector< std::string > list =
      Splitter().on(',').trimWhitespace().split(testString1);
  std::string result = Joiner().join(list.begin(), list.end());
  // Joining with ", " should should result in original string
  std::string expected = testString1;

  TEST(testing::assertEquals(result, expected));
}

REGISTER_TEST_CASE(testJoinCustomSep) {
  std::vector< std::string > list =
      Splitter().on(',').trimWhitespace().split(testString1);
  std::string result = Joiner().on("-").join(list.begin(), list.end());
  // Using different join character, expects replace ', ' between letters
  std::string expected = "a-b-c-d";

  TEST(testing::assertEquals(result, expected));
}

REGISTER_TEST_CASE(testJoinEmpty) {
  std::vector< std::string > list;
  std::string result = Joiner().on("-").join(list.begin(), list.end());
  std::string expected = "";

  TEST(testing::assertEquals(result, expected));
}

REGISTER_TEST_CASE(testEscape) {
  const std::string input = "noescapes\n\r\t\"\\";
  const std::string expected = "noescapes\\n\\r\\t\\\"\\\\";

  const std::string result = Escape(input);
  TEST(testing::assertEquals(result, expected));
}

REGISTER_TEST_CASE(testEscapeEmpty) {
  TEST(testing::assertEquals(Escape(""), ""));
}

REGISTER_TEST_CASE(testUnescape) {
  const std::string input = "noescapes\\b\\n\\r\\t\\\"\\\\";
  const std::string expected = "noescapes\\b\n\r\t\"\\";

  const std::string result = Unescape(input);
  TEST(testing::assertEquals(result, expected));
}

REGISTER_TEST_CASE(testUnescapeEmpty) {
  TEST(testing::assertEquals(Unescape(""), ""));
}

REGISTER_TEST_CASE(testEscapeUnescapeNonprintable) {
  const std::string input = "\xFF";
  const std::string expected = "\\255";

  const std::string result = Escape(input);
  TEST(testing::assertEquals(result, expected));
  const std::string result2 = Unescape(result);
  TEST(testing::assertEquals(result2, input));
}

REGISTER_TEST_CASE(testReplaceEmpty) {
  const std::string source = "foobarfoobazfoobizfoo";
  const std::string match = "foo";
  const std::string replace = "";
  const std::string expected = "barbazbiz";
  TEST(testing::assertEquals(ReplaceStr(source, match, replace), expected));
}

REGISTER_TEST_CASE(testReplaceLarger) {
  const std::string source = "foobarfoobazfoobizfoo";
  const std::string match = "foo";
  const std::string replace = "fizzbiz";
  const std::string expected = "fizzbizbarfizzbizbazfizzbizbizfizzbiz";
  TEST(testing::assertEquals(ReplaceStr(source, match, replace), expected));
}

REGISTER_TEST_CASE(testReplaceNotFound) {
  const std::string source = "foobarfoobazfoobizfoo";
  const std::string match = "zaz";
  const std::string replace = "fizzbiz";
  const std::string expected = source;
  TEST(testing::assertEquals(ReplaceStr(source, match, replace), expected));
}

REGISTER_TEST_CASE(testIdentiferSafe) {
  const std::string source = "9a0d9 asc[p.c ad][a 01293asdf";
  const std::string expected = "_a0d9_asc_p_c_ad__a_01293asdf";
  TEST(testing::assertEquals(IdentifierSafe(source), expected));
}

REGISTER_TEST_CASE(testIdentiferSafeEmpty) {
  TEST(testing::assertEquals(IdentifierSafe(""), ""));
}

REGISTER_TEST_CASE(testPretySize) {
  TEST(testing::assertEquals(PrettySize(1ull), "1 B"));
  TEST(testing::assertEquals(PrettySize(1024ull), "1 KiB"));
  TEST(testing::assertEquals(PrettySize(1024ull * 1024), "1 MiB"));
  TEST(testing::assertEquals(PrettySize(1024ull * 1024 * 1024), "1 GiB"));
  TEST(
      testing::assertEquals(PrettySize(1024ull * 1024 * 1024 * 1024), "1 TiB"));
  TEST(testing::assertEquals(
      PrettySize(1024ull * 1024 * 1024 * 1024 * 1024), "1 PiB"));
}

REGISTER_TEST_CASE(testCountLines) {
  const std::string source = "0\n1\n2\n3";
  TEST(testing::assertEquals(CountLines(source.begin(), source.end()), 3));
}
