#include <TESTS/test_assertions.h>
#include <TESTS/testcase.h>

#include <CORE/UTIL/lrparser.h>
#include <CORE/UTIL/tokenizer.h>

using namespace core::util::parser;

struct eTokenNames {
  enum type { TOKEN_IDENT, TOKEN_OBRACE, TOKEN_CBRACE, TOKEN_WS, COUNT };
  static const char *names[COUNT];
};
const char *eTokenNames::names[] = {
    "TOKEN_IDENT",
    "TOKEN_OBRACE",
    "TOKEN_CBRACE",
    "TOKEN_WS",
};

typedef LRParser< eTokenNames > tParser;
typedef Tokenizer< eTokenNames > tTokenizer;
typedef tParser::Rule Rule;

/**
 *
 */
static bool HandleField2(
    tParser::iNode *&ret,
    const tParser::tTokenOrNodeList::const_iterator &begin,
    const tParser::tTokenOrNodeList::const_iterator &end) {
  TEST(testing::assertEquals(2, std::distance(begin, end)));
  return true;
}

/**
 *
 */
static bool HandleField3(
    tParser::iNode *&ret,
    const tParser::tTokenOrNodeList::const_iterator &begin,
    const tParser::tTokenOrNodeList::const_iterator &end) {
  TEST(testing::assertEquals(3, std::distance(begin, end)));
  tParser::tTokenOrNodeList::const_iterator itr = begin;
  itr++;
  TEST(testing::assertEquals(itr->m_type, tParser::TokenOrNode::RULE_NODE));
  return true;
}

static std::vector< std::string > g_errorMsg;
/**
 *
 */
static void HandleError(const std::string &msg) {
  g_errorMsg.push_back(msg);
}

/**
 *
 */
static const tTokenizer &CreateTokenizer() {
  tTokenizer::tTokenizerList tokenTypes;
  tokenTypes.push_back(
      tTokenizer::tTokenizer(eTokenNames::TOKEN_OBRACE, RegExPattern("{")));
  tokenTypes.push_back(
      tTokenizer::tTokenizer(eTokenNames::TOKEN_CBRACE, RegExPattern("}")));
  tokenTypes.push_back(
      tTokenizer::tTokenizer(eTokenNames::TOKEN_IDENT, RegExPattern("\\w+")));
  tokenTypes.push_back(
      tTokenizer::tTokenizer(eTokenNames::TOKEN_WS, RegExPattern("\\s+")));
  static tTokenizer tokenizer(tokenTypes);
  return tokenizer;
}

/**
 *
 */
static tParser CreateParser() {
  tParser parser(CreateTokenizer());

  Rule &objBody = parser.addRule("objectbody");
  Rule &object = parser.addRule("object");

  objBody |=
      tParser::RuleOrTokenChain(
          tParser::RuleOrTokenChain::tProc::from_function< HandleField2 >())
      & eTokenNames::TOKEN_OBRACE & eTokenNames::TOKEN_CBRACE;
  objBody |=
      tParser::RuleOrTokenChain(
          tParser::RuleOrTokenChain::tProc::from_function< HandleField3 >())
      & eTokenNames::TOKEN_OBRACE & object & eTokenNames::TOKEN_CBRACE;

  object |= tParser::RuleOrTokenChain() & eTokenNames::TOKEN_IDENT & objBody;

  g_errorMsg.clear();
  return parser;
}

REGISTER_TEST_CASE(testSuccessfulLRParse) {
  const std::string testFile = "testmessage {\n"
                               "  testmessage2 {\n"
                               "    \n"
                               "  }\n"
                               "}\n";
  tParser parser = CreateParser();
  TEST(testing::assertTrue(
      parser.parse(eTokenNames::TOKEN_WS, testFile.begin(), testFile.end())));
}

REGISTER_TEST_CASE(testLRParseErrorOnMissingEnd) {
  const std::string testFile = "testmessage {\n"
                               "  testmessage2 {\n"
                               "    \n"
                               "  }\n"
                               "\n";
  tParser parser = CreateParser();
  TEST(testing::assertFalse(parser.parse(
      eTokenNames::TOKEN_WS,
      testFile.begin(),
      testFile.end(),
      tParser::tErrorCB::from_function< HandleError >())));
  TEST(testing::assertEquals(g_errorMsg.size(), 2));
  TEST(testing::assertEquals(
      g_errorMsg[1], "Unexpected <EOF> while parsing objectbody on line 0"));
}

REGISTER_TEST_CASE(testLRParseErrorOnMissingFirstToken) {
  const std::string testFile = "testmessage {\n"
                               "  {\n"
                               "    \n"
                               "  }\n"
                               "}\n";
  tParser parser = CreateParser();
  TEST(testing::assertFalse(parser.parse(
      eTokenNames::TOKEN_WS,
      testFile.begin(),
      testFile.end(),
      tParser::tErrorCB::from_function< HandleError >())));
  TEST(testing::assertEquals(g_errorMsg.size(), 4));
  TEST(testing::assertEquals(
      g_errorMsg[0], "Unexpected TOKEN_OBRACE while parsing object on line 0"));
}
