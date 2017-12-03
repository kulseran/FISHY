#include <TESTS/testcase.h>
#include <TESTS/test_assertions.h>

#include <CORE/types.h>
#include <CORE/UTIL/lrparser.h>
#include <CORE/UTIL/tokenizer.h>

#include <exception>

using namespace core::util::parser;


struct eTokenNames {
  enum type {
    TOKEN_MESSAGE,
    TOKEN_IDENT,
    TOKEN_OBRACE,
    TOKEN_CBRACE,
    TOKEN_OPTIONAL,
    TOKEN_INT32,
    TOKEN_NUMERIC,
    TOKEN_EOL,
    TOKEN_ASSIGN,
    TOKEN_WS,
  };
};
typedef LRParser<eTokenNames> tParser;
typedef tParser::Rule Rule;

bool handleField(void *&ret, const tParser::tTokenOrNodeList::const_iterator &begin, const tParser::tTokenOrNodeList::const_iterator &end) {
  TEST(testing::assertEquals(6, std::distance(begin, end)));
  return true;
}

REGISTER_TEST_CASE(testSuccessfulLRParse) {

  const std::string testFile = "message testmessage {\n"
                               "  message testmessage2 {\n"
                               "    optional int32 field1 = 1;\n"
                               "  }\n"
                               "}\n";
  Tokenizer<eTokenNames>::tTokenizerList tokenTypes;
  tokenTypes.push_back(Tokenizer<eTokenNames>::tTokenizer(eTokenNames::TOKEN_MESSAGE, RegExPattern("message")));
  tokenTypes.push_back(Tokenizer<eTokenNames>::tTokenizer(eTokenNames::TOKEN_OPTIONAL, RegExPattern("optional")));
  tokenTypes.push_back(Tokenizer<eTokenNames>::tTokenizer(eTokenNames::TOKEN_INT32, RegExPattern("int32")));
  tokenTypes.push_back(Tokenizer<eTokenNames>::tTokenizer(eTokenNames::TOKEN_NUMERIC, RegExPattern("\\d+")));
  tokenTypes.push_back(Tokenizer<eTokenNames>::tTokenizer(eTokenNames::TOKEN_OBRACE, RegExPattern("{")));
  tokenTypes.push_back(Tokenizer<eTokenNames>::tTokenizer(eTokenNames::TOKEN_CBRACE, RegExPattern("}")));
  tokenTypes.push_back(Tokenizer<eTokenNames>::tTokenizer(eTokenNames::TOKEN_EOL, RegExPattern(";")));
  tokenTypes.push_back(Tokenizer<eTokenNames>::tTokenizer(eTokenNames::TOKEN_ASSIGN, RegExPattern("=")));
  tokenTypes.push_back(Tokenizer<eTokenNames>::tTokenizer(eTokenNames::TOKEN_IDENT, RegExPattern("\\w+")));
  tokenTypes.push_back(Tokenizer<eTokenNames>::tTokenizer(eTokenNames::TOKEN_WS, RegExPattern("\\s+")));
  Tokenizer<eTokenNames> tokenizer(tokenTypes);

  LRParser< eTokenNames > parser(tokenizer);
  /*
  Rule &field = parser.addRule("field") |= LRParser< eTokenNames >::RuleOrTokenChain(TOKEN_OPTIONAL) & TOKEN_INT32 & TOKEN_IDENT & TOKEN_ASSIGN & TOKEN_NUMERIC & TOKEN_EOL;
  Rule &fieldList = parser.addRule("fieldList");
  Rule &message = parser.addRule("message");
  Rule &messageList = parser.addRule("messageList");

  field.setCallback(LRParser< eTokenNames >::Rule::tProc::from_function<handleField>());

  fieldList |= fieldList & field;
  fieldList |= field;

  message |= LRParser< eTokenNames >::RuleOrTokenChain(TOKEN_MESSAGE) & TOKEN_IDENT & TOKEN_OBRACE & fieldList & TOKEN_CBRACE;
  message |= LRParser< eTokenNames >::RuleOrTokenChain(TOKEN_MESSAGE) & TOKEN_IDENT & TOKEN_OBRACE & messageList & TOKEN_CBRACE;
  message |= LRParser< eTokenNames >::RuleOrTokenChain(TOKEN_MESSAGE) & TOKEN_IDENT & TOKEN_OBRACE & messageList & fieldList & TOKEN_CBRACE;

  messageList |= messageList & message;
  messageList |= message;

  TEST(testing::assertTrue(parser.parse(TOKEN_WS, testFile.begin(), testFile.end())));*/
  TEST(testing::fail("not implemented"));
}
