#include <TESTS/test_assertions.h>
#include <TESTS/testcase.h>

#include <CORE/UTIL/tokenizer.h>
#include <CORE/types.h>

using namespace core::util::parser;

REGISTER_TEST_CASE(testSuccessfulParse) {
  struct eTokenNames {
    enum type {
      TOKEN_IDENT,
      TOKEN_OPERATOR_ASSIGN,
      TOKEN_NUMERIC,
      TOKEN_END_STATEMENT,
      TOKEN_WS
    };
  };

  Tokenizer< eTokenNames >::tTokenizerList tokenTypes;
  tokenTypes.push_back(Tokenizer< eTokenNames >::tTokenizer(
      eTokenNames::TOKEN_IDENT, RegExPattern("[\\a][\\w]*")));
  tokenTypes.push_back(Tokenizer< eTokenNames >::tTokenizer(
      eTokenNames::TOKEN_OPERATOR_ASSIGN, RegExPattern("=")));
  tokenTypes.push_back(Tokenizer< eTokenNames >::tTokenizer(
      eTokenNames::TOKEN_NUMERIC, RegExPattern("[\\d]*(\\.[\\d]+)?")));
  tokenTypes.push_back(Tokenizer< eTokenNames >::tTokenizer(
      eTokenNames::TOKEN_END_STATEMENT, RegExPattern(";")));
  tokenTypes.push_back(Tokenizer< eTokenNames >::tTokenizer(
      eTokenNames::TOKEN_WS, RegExPattern("[\\s]*")));
  Tokenizer< eTokenNames > tokenizer(tokenTypes);

  const std::string input = "int x = 5;";

  std::vector< eTokenNames::type > tokens;
  for (Tokenizer< eTokenNames >::const_iterator itr = tokenizer.begin(input);
       itr != tokenizer.end();
       ++itr) {
    tokens.push_back(itr->getId());
  }

  std::vector< eTokenNames::type > expected;
  expected.push_back(eTokenNames::TOKEN_IDENT);
  expected.push_back(eTokenNames::TOKEN_WS);
  expected.push_back(eTokenNames::TOKEN_IDENT);
  expected.push_back(eTokenNames::TOKEN_WS);
  expected.push_back(eTokenNames::TOKEN_OPERATOR_ASSIGN);
  expected.push_back(eTokenNames::TOKEN_WS);
  expected.push_back(eTokenNames::TOKEN_NUMERIC);
  expected.push_back(eTokenNames::TOKEN_END_STATEMENT);
  TEST(testing::assertEquals(expected, tokens));
}
