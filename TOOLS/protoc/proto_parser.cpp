#include "proto_parser.h"

#include <CORE/types.h>
#include <CORE/BASE/logging.h>
#include <CORE/UTIL/lexical_cast.h>
#include <CORE/UTIL/regex.h>
#include <CORE/UTIL/stringutil.h>
#include <CORE/UTIL/lrparser.h>

using core::util::EnumDef;
using core::util::FieldDef;
using core::util::MessageDef;
using core::util::ProtoDef;
using core::util::RpcFunctionDef;
using core::util::ServiceDef;
using core::util::parser::RegExPattern;

namespace proto {

/**
 * Token Types and tokenizer
 */
struct eTokenTypes {
  enum type {
    PACKAGE,
    IMPORT,
    MESSAGE,
    SERVICE,
    ENUM,
    RPC,
    RETURNS,

    OPTIONAL,
    REPEATED,

    // The order of the following must match FieldDef::eFieldType
    DOUBLE,
    FLOAT,
    INT32,
    INT64,
    UINT32,
    UINT64,
    SINT32,
    SINT64,
    FIXED32,
    FIXED64,
    SFIXED32,
    SFIXED64,
    BOOL,
    STRING,
    BYTES,

    IDENT,
    STR,
    WS,
    NUM,
    EOL,
    EQUALS,
    COMMENT,
    OPEN_BRACE,
    CLOSE_BRACE,
    OPEN_PEREN,
    CLOSE_PEREN,

    COUNT
  };

  static const char *names[COUNT];
};

const char *eTokenTypes::names[eTokenTypes::COUNT] = {
  "PACKAGE",
  "IMPORT",
  "MESSAGE",
  "SERVICE",
  "ENUM",
  "RPC",
  "RETURNS",
  "OPTIONAL",
  "REPEATED",
  "DOUBLE",
  "FLOAT",
  "INT32",
  "INT64",
  "UINT32",
  "UINT64",
  "SINT32",
  "SINT64",
  "FIXED32",
  "FIXED64",
  "SFIXED32",
  "SFIXED64",
  "BOOL",
  "STRING",
  "BYTES",
  "IDENT",
  "STR",
  "WS",
  "NUM",
  "EOL",
  "EQUALS",
  "COMMENT",
  "OPEN_BRACE",
  "CLOSE_BRACE",
  "OPEN_PEREN",
  "CLOSE_PEREN"
};

typedef core::util::parser::Tokenizer<eTokenTypes> tTokenizer;
typedef std::vector< typename tTokenizer::Token > tTokenList;
typedef core::util::parser::LRParser<eTokenTypes> tParser;
typedef tParser::Rule tRule;
typedef tParser::RuleOrTokenChain tCRule;

/**
 * Utility to help build out a proto message def
 */
class ProtoDefBuilder {
  public:
    ProtoDefBuilder(ProtoDef &target)
      : m_target(target) {
    }

    /**
     *
     */
    bool setPackage(void *&unused, const tParser::tTokenOrNodeList::const_iterator &begin, const tParser::tTokenOrNodeList::const_iterator &end) {
      (void) unused;

      tParser::tTokenOrNodeList::const_iterator itr = begin + 1;

      if (!m_target.m_package.empty()) {
        Log(LL::Error) << "Too many package defines." << std::endl;
        return false;
      }
      m_target.m_package = std::move(itr->m_token.getToken());

      return true;
    }

    /**
     *
     */
    bool addImport(void *&unused, const tParser::tTokenOrNodeList::const_iterator &begin, const tParser::tTokenOrNodeList::const_iterator &end) {
      (void) unused;

      tParser::tTokenOrNodeList::const_iterator itr = begin + 1;
      m_target.m_imports.push_back(core::util::trimQuotes(itr->m_token.getToken()));

      return true;
    }

    /**
     *
     */
    bool addService(void *&unused, const tParser::tTokenOrNodeList::const_iterator &begin, const tParser::tTokenOrNodeList::const_iterator &end) {
      (void) unused;

      tParser::tTokenOrNodeList::const_iterator itr = begin + 1;
      ServiceDef def;
      def.m_name = std::move(itr->m_token.getToken());
      std::advance(itr, 2);
      std::vector< RpcFunctionDef > *pRpcs = reinterpret_cast< std::vector< RpcFunctionDef > * >(itr->m_pNode);
      def.m_functions = std::move(*pRpcs);
      delete pRpcs;

      m_target.m_services.push_back(def);
      return true;
    }

    /**
     *
     */
    bool addMessage(void *&unused, const tParser::tTokenOrNodeList::const_iterator &begin, const tParser::tTokenOrNodeList::const_iterator &end) {
      (void) unused;

      tParser::tTokenOrNodeList::const_iterator itr = begin + 1;
      MessageDef *pMessage = reinterpret_cast< MessageDef * >(itr->m_pNode);
      m_target.m_messages.push_back(std::move(*pMessage));
      delete pMessage;

      return true;
    }

  private:
    ProtoDef &m_target;
};

/**
 *
 */
bool genEnumField(void *&ret, const tParser::tTokenOrNodeList::const_iterator &begin, const tParser::tTokenOrNodeList::const_iterator &end) {
  tParser::tTokenOrNodeList::const_iterator itr = begin;
  FieldDef *pDef = new FieldDef;
  pDef->m_name = itr->m_token.getToken();
  std::advance(itr, 2);
  CHECK(core::util::lexical_cast(itr->m_token.getToken(), pDef->m_fieldNum));
  ret = pDef;
  return true;
}

/**
 *
 */
bool genFieldList(void *&ret, const tParser::tTokenOrNodeList::const_iterator &begin, const tParser::tTokenOrNodeList::const_iterator &end) {
  tParser::tTokenOrNodeList::const_iterator itr = begin;
  std::vector< FieldDef > *pList = new std::vector< FieldDef >();
  FieldDef *pDef = reinterpret_cast< FieldDef * >(itr->m_pNode);
  pList->push_back(std::move(*pDef));
  delete pDef;
  ret = pList;
  return true;
}

/**
 *
 */
bool appendFieldList(void *&ret, const tParser::tTokenOrNodeList::const_iterator &begin, const tParser::tTokenOrNodeList::const_iterator &end) {
  tParser::tTokenOrNodeList::const_iterator itr = begin;
  std::vector< FieldDef > *pList = reinterpret_cast< std::vector< FieldDef > *>(itr->m_pNode);
  ++itr;
  FieldDef *pDef = reinterpret_cast< FieldDef * >(itr->m_pNode);
  pList->push_back(std::move(*pDef));
  delete pDef;
  ret = pList;
  return true;
}

/**
 *
 */
bool genRpc(void *&ret, const tParser::tTokenOrNodeList::const_iterator &begin, const tParser::tTokenOrNodeList::const_iterator &end) {
  tParser::tTokenOrNodeList::const_iterator itr = begin + 1;
  RpcFunctionDef *pDef = new RpcFunctionDef;
  pDef->m_name = itr->m_token.getToken();
  std::advance(itr, 2);
  pDef->m_param = itr->m_token.getToken();
  std::advance(itr, 4);
  pDef->m_return = itr->m_token.getToken();
  ret = pDef;
  return true;
}

/**
 *
 */
bool genRpcList(void *&ret, const tParser::tTokenOrNodeList::const_iterator &begin, const tParser::tTokenOrNodeList::const_iterator &end) {
  tParser::tTokenOrNodeList::const_iterator itr = begin;
  std::vector< RpcFunctionDef > *pList = new std::vector< RpcFunctionDef >();
  RpcFunctionDef *pRpc = reinterpret_cast< RpcFunctionDef * >(itr->m_pNode);
  pList->push_back(std::move(*pRpc));
  delete pRpc;
  ret = pList;
  return true;
}

/**
 *
 */
bool appendRpcList(void *&ret, const tParser::tTokenOrNodeList::const_iterator &begin, const tParser::tTokenOrNodeList::const_iterator &end) {
  tParser::tTokenOrNodeList::const_iterator itr = begin;
  std::vector< RpcFunctionDef > *pList = reinterpret_cast< std::vector< RpcFunctionDef > *>(itr->m_pNode);
  ++itr;
  pList->push_back(*reinterpret_cast< RpcFunctionDef * >(itr->m_pNode));
  delete itr->m_pNode;
  ret = pList;
  return true;
}

/**
 *
 */
bool genEnum(void *&ret, const tParser::tTokenOrNodeList::const_iterator &begin, const tParser::tTokenOrNodeList::const_iterator &end) {
  tParser::tTokenOrNodeList::const_iterator itr = begin + 1;
  EnumDef *pDef = new EnumDef;
  pDef->m_name = itr->m_token.getToken();
  std::advance(itr, 2);
  std::vector< FieldDef > *pValues = reinterpret_cast< std::vector< FieldDef > * >(itr->m_pNode);
  pDef->m_values = std::move(*pValues);
  delete pValues;
  ret = pDef;
  return true;
}

/**
 *
 */
bool genField(void *&ret, const tParser::tTokenOrNodeList::const_iterator &begin, const tParser::tTokenOrNodeList::const_iterator &end) {
  tParser::tTokenOrNodeList::const_iterator itr = begin;
  FieldDef *pDef = new FieldDef;
  pDef->m_repeated = itr->m_token.getId() == eTokenTypes::REPEATED;
  ++itr;
  if (itr->m_token.getId() == eTokenTypes::IDENT) {
    pDef->m_type = FieldDef::FIELD_MSG;
    pDef->m_msgType = itr->m_token.getToken();
  } else {
    pDef->m_type = FieldDef::eFieldType(itr->m_token.getId() - eTokenTypes::DOUBLE);
  }
  ++itr;
  pDef->m_name = itr->m_token.getToken();
  std::advance(itr, 2);
  CHECK(core::util::lexical_cast(itr->m_token.getToken(), pDef->m_fieldNum));
  ret = pDef;
  return true;
}

/**
 *
 */
bool finalizeMessage(void *&ret, const tParser::tTokenOrNodeList::const_iterator &begin, const tParser::tTokenOrNodeList::const_iterator &end) {
  tParser::tTokenOrNodeList::const_iterator itr = begin + 1;
  const std::string name = itr->m_token.getToken();
  std::advance(itr, 2);
  MessageDef *pDef = reinterpret_cast< MessageDef * >(itr->m_pNode);
  pDef->m_name = std::move(name);
  ret = pDef;
  return true;
}

/**
 *
 */
bool genMessageBodyMessage(void *&ret, const tParser::tTokenOrNodeList::const_iterator &begin, const tParser::tTokenOrNodeList::const_iterator &end) {
  tParser::tTokenOrNodeList::const_iterator itr = begin;
  MessageDef *pDef = reinterpret_cast< MessageDef * >(itr->m_pNode);
  ++itr;
  MessageDef *pDefChild = reinterpret_cast< MessageDef * >(itr->m_pNode);
  pDef->m_messages.push_back(std::move(*pDefChild));
  delete pDefChild;
  ret = pDef;
  return true;
}

/**
 *
 */
bool genMessageMessageBody(void *&ret, const tParser::tTokenOrNodeList::const_iterator &begin, const tParser::tTokenOrNodeList::const_iterator &end) {
  tParser::tTokenOrNodeList::const_iterator itr = begin;
  MessageDef *pDefChild = reinterpret_cast< MessageDef * >(itr->m_pNode);
  ++itr;
  MessageDef *pDef = reinterpret_cast< MessageDef * >(itr->m_pNode);
  pDef->m_messages.push_back(std::move(*pDefChild));
  delete pDefChild;
  ret = pDef;
  return true;
}

/**
 *
 */
bool genMessageEnum(void *&ret, const tParser::tTokenOrNodeList::const_iterator &begin, const tParser::tTokenOrNodeList::const_iterator &end) {
  tParser::tTokenOrNodeList::const_iterator itr = begin;
  MessageDef *pDef;
  if (itr + 1 == end) {
    pDef = new MessageDef;
  } else {
    pDef = reinterpret_cast< MessageDef * >(itr->m_pNode);
    ++itr;
  }
  EnumDef *pEnumDef = reinterpret_cast< EnumDef * >(itr->m_pNode);
  pDef->m_enums.push_back(std::move(*pEnumDef));
  delete pEnumDef;
  ret = pDef;
  return true;
}

/**
 *
 */
bool genMessageField(void *&ret, const tParser::tTokenOrNodeList::const_iterator &begin, const tParser::tTokenOrNodeList::const_iterator &end) {
  tParser::tTokenOrNodeList::const_iterator itr = begin;
  MessageDef *pDef;
  if (itr + 1 == end) {
    pDef = new MessageDef;
  } else {
    pDef = reinterpret_cast< MessageDef * >(itr->m_pNode);
    ++itr;
  }
  FieldDef *pField = reinterpret_cast< FieldDef * >(itr->m_pNode);
  pDef->m_fields.push_back(std::move(*pField));
  delete pField;
  ret = pDef;
  return true;
}

/**
 *
 */
bool parse(ProtoDef &def, const std::string &fileData) {
  tTokenizer::tTokenizerList tokenRegexs;
  tokenRegexs.push_back(tTokenizer::tTokenizer(eTokenTypes::WS, RegExPattern("([ \t\n\r]*)|(//.*\n)")));
  tokenRegexs.push_back(tTokenizer::tTokenizer(eTokenTypes::STR, RegExPattern("\"[^\"]*\"")));
  tokenRegexs.push_back(tTokenizer::tTokenizer(eTokenTypes::EOL, RegExPattern(";")));
  tokenRegexs.push_back(tTokenizer::tTokenizer(eTokenTypes::EQUALS, RegExPattern("=")));
  tokenRegexs.push_back(tTokenizer::tTokenizer(eTokenTypes::OPEN_BRACE, RegExPattern("{")));
  tokenRegexs.push_back(tTokenizer::tTokenizer(eTokenTypes::CLOSE_BRACE, RegExPattern("}")));
  tokenRegexs.push_back(tTokenizer::tTokenizer(eTokenTypes::OPEN_PEREN, RegExPattern("\\(")));
  tokenRegexs.push_back(tTokenizer::tTokenizer(eTokenTypes::CLOSE_PEREN, RegExPattern("\\)")));
  tokenRegexs.push_back(tTokenizer::tTokenizer(eTokenTypes::PACKAGE, RegExPattern("package ")));
  tokenRegexs.push_back(tTokenizer::tTokenizer(eTokenTypes::IMPORT, RegExPattern("import ")));
  tokenRegexs.push_back(tTokenizer::tTokenizer(eTokenTypes::MESSAGE, RegExPattern("message ")));
  tokenRegexs.push_back(tTokenizer::tTokenizer(eTokenTypes::SERVICE, RegExPattern("service ")));
  tokenRegexs.push_back(tTokenizer::tTokenizer(eTokenTypes::ENUM, RegExPattern("enum ")));
  tokenRegexs.push_back(tTokenizer::tTokenizer(eTokenTypes::RPC, RegExPattern("rpc ")));
  tokenRegexs.push_back(tTokenizer::tTokenizer(eTokenTypes::RETURNS, RegExPattern("returns ")));
  tokenRegexs.push_back(tTokenizer::tTokenizer(eTokenTypes::REPEATED, RegExPattern("repeated ")));
  tokenRegexs.push_back(tTokenizer::tTokenizer(eTokenTypes::OPTIONAL, RegExPattern("optional ")));
  tokenRegexs.push_back(tTokenizer::tTokenizer(eTokenTypes::DOUBLE, RegExPattern("double ")));
  tokenRegexs.push_back(tTokenizer::tTokenizer(eTokenTypes::FLOAT, RegExPattern("float ")));
  tokenRegexs.push_back(tTokenizer::tTokenizer(eTokenTypes::INT32, RegExPattern("int32 ")));
  tokenRegexs.push_back(tTokenizer::tTokenizer(eTokenTypes::INT64, RegExPattern("int64 ")));
  tokenRegexs.push_back(tTokenizer::tTokenizer(eTokenTypes::UINT32, RegExPattern("uint32 ")));
  tokenRegexs.push_back(tTokenizer::tTokenizer(eTokenTypes::UINT64, RegExPattern("uint64 ")));
  tokenRegexs.push_back(tTokenizer::tTokenizer(eTokenTypes::SINT32, RegExPattern("sint32 ")));
  tokenRegexs.push_back(tTokenizer::tTokenizer(eTokenTypes::SINT64, RegExPattern("sint64 ")));
  tokenRegexs.push_back(tTokenizer::tTokenizer(eTokenTypes::FIXED32, RegExPattern("fixed32 ")));
  tokenRegexs.push_back(tTokenizer::tTokenizer(eTokenTypes::FIXED64, RegExPattern("fixed64 ")));
  tokenRegexs.push_back(tTokenizer::tTokenizer(eTokenTypes::SFIXED32, RegExPattern("sfixed32 ")));
  tokenRegexs.push_back(tTokenizer::tTokenizer(eTokenTypes::SFIXED64, RegExPattern("sfixed64 ")));
  tokenRegexs.push_back(tTokenizer::tTokenizer(eTokenTypes::BOOL, RegExPattern("bool ")));
  tokenRegexs.push_back(tTokenizer::tTokenizer(eTokenTypes::STRING, RegExPattern("string ")));
  tokenRegexs.push_back(tTokenizer::tTokenizer(eTokenTypes::BYTES, RegExPattern("bytes ")));
  tokenRegexs.push_back(tTokenizer::tTokenizer(eTokenTypes::NUM, RegExPattern("\\d*(\\.\\d+)?")));
  tokenRegexs.push_back(tTokenizer::tTokenizer(eTokenTypes::IDENT, RegExPattern("\\a[\\.\\w]*")));

  tTokenizer tokenizer(tokenRegexs);
  tParser parser(tokenizer);

  ProtoDefBuilder builder(def);

  tRule &packageRule = parser.addRule("package");
  packageRule |= tCRule(tCRule::tProc::from_method<ProtoDefBuilder, &ProtoDefBuilder::setPackage>(&builder)) & eTokenTypes::PACKAGE & eTokenTypes::IDENT & eTokenTypes::EOL;

  tRule &importRule = parser.addRule("import");
  importRule |= tCRule(tCRule::tProc::from_method<ProtoDefBuilder, &ProtoDefBuilder::addImport>(&builder)) & eTokenTypes::IMPORT & eTokenTypes::STR & eTokenTypes::EOL;

  tRule &fieldRule = parser.addRule("field");
  for (u32 fieldType = eTokenTypes::DOUBLE; fieldType <= eTokenTypes::BYTES; ++fieldType) {
    fieldRule |= tCRule(tCRule::tProc::from_function<genField>()) & eTokenTypes::OPTIONAL & eTokenTypes::type(fieldType) & eTokenTypes::IDENT & eTokenTypes::EQUALS & eTokenTypes::NUM & eTokenTypes::EOL;
  }
  for (u32 fieldType = eTokenTypes::DOUBLE; fieldType <= eTokenTypes::BYTES; ++fieldType) {
    fieldRule |= tCRule(tCRule::tProc::from_function<genField>()) & eTokenTypes::REPEATED & eTokenTypes::type(fieldType) & eTokenTypes::IDENT & eTokenTypes::EQUALS & eTokenTypes::NUM & eTokenTypes::EOL;
  }
  fieldRule |= tCRule(tCRule::tProc::from_function<genField>()) & eTokenTypes::OPTIONAL & eTokenTypes::IDENT & eTokenTypes::IDENT & eTokenTypes::EQUALS & eTokenTypes::NUM & eTokenTypes::EOL;
  fieldRule |= tCRule(tCRule::tProc::from_function<genField>()) & eTokenTypes::REPEATED & eTokenTypes::IDENT & eTokenTypes::IDENT & eTokenTypes::EQUALS & eTokenTypes::NUM & eTokenTypes::EOL;

  tRule &enumFieldRule = parser.addRule("enumfield");
  enumFieldRule |= tCRule(tCRule::tProc::from_function<genEnumField>()) & eTokenTypes::IDENT & eTokenTypes::EQUALS & eTokenTypes::NUM & eTokenTypes::EOL;

  tRule &enumFieldListRule = parser.addRule("enumfieldList");
  enumFieldListRule |= tCRule(tCRule::tProc::from_function<appendFieldList>()) & enumFieldListRule & enumFieldRule;
  enumFieldListRule |= tCRule(tCRule::tProc::from_function<genFieldList>()) & enumFieldRule;

  tRule &enumRule = parser.addRule("enum");
  enumRule |= tCRule(tCRule::tProc::from_function<genEnum>()) & eTokenTypes::ENUM & eTokenTypes::IDENT & eTokenTypes::OPEN_BRACE & enumFieldListRule & eTokenTypes::CLOSE_BRACE;

  tRule &messageBodyRule = parser.addRule("messageBody");
  tRule &messageRule = parser.addRule("message");
  messageRule |= tCRule(tCRule::tProc::from_function<finalizeMessage>()) & eTokenTypes::MESSAGE & eTokenTypes::IDENT & eTokenTypes::OPEN_BRACE & messageBodyRule & eTokenTypes::CLOSE_BRACE;

  messageBodyRule |= tCRule(tCRule::tProc::from_function<genMessageMessageBody>()) & messageRule & messageBodyRule;
  messageBodyRule |= tCRule(tCRule::tProc::from_function<genMessageBodyMessage>()) & messageBodyRule & messageRule;
  messageBodyRule |= tCRule(tCRule::tProc::from_function<genMessageEnum>()) & messageBodyRule & enumRule;
  messageBodyRule |= tCRule(tCRule::tProc::from_function<genMessageField>()) & messageBodyRule & fieldRule;
  messageBodyRule |= tCRule(tCRule::tProc::from_function<genMessageEnum>()) & enumRule;
  messageBodyRule |= tCRule(tCRule::tProc::from_function<genMessageField>()) & fieldRule;

  tRule &serviceRpc = parser.addRule("serviceRpc");
  serviceRpc |= tCRule(tCRule::tProc::from_function<genRpc>()) & eTokenTypes::RPC & eTokenTypes::IDENT & eTokenTypes::OPEN_PEREN & eTokenTypes::IDENT & eTokenTypes::CLOSE_PEREN & eTokenTypes::RETURNS & eTokenTypes::OPEN_PEREN & eTokenTypes::IDENT & eTokenTypes::CLOSE_PEREN & eTokenTypes::EOL;

  tRule &serviceRpcListRule = parser.addRule("serviceRpcList");
  serviceRpcListRule |= tCRule(tCRule::tProc::from_function<appendRpcList>()) & serviceRpcListRule & serviceRpc;
  serviceRpcListRule |= tCRule(tCRule::tProc::from_function<genRpcList>()) & serviceRpc;

  tRule &serviceRule = parser.addRule("service");
  serviceRule |= tCRule(tCRule::tProc::from_method<ProtoDefBuilder, &ProtoDefBuilder::addService>(&builder)) & eTokenTypes::SERVICE & eTokenTypes::IDENT & eTokenTypes::OPEN_BRACE & serviceRpcListRule & eTokenTypes::CLOSE_BRACE;

  tRule &fileRule = parser.addRule("file");
  fileRule |= tCRule() & packageRule;
  fileRule |= tCRule() & fileRule & importRule;
  fileRule |= tCRule() & fileRule & serviceRule;
  fileRule |= tCRule(tCRule::tProc::from_method<ProtoDefBuilder, &ProtoDefBuilder::addMessage>(&builder)) & fileRule & messageRule;

  return parser.parse(eTokenTypes::WS, fileData.begin(), fileData.end());
}

} // namespace proto
