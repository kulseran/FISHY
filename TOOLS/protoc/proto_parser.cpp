#include "proto_parser.h"

#include <CORE/BASE/logging.h>
#include <CORE/UTIL/lexical_cast.h>
#include <CORE/UTIL/lrparser.h>
#include <CORE/UTIL/regex.h>
#include <CORE/UTIL/stringutil.h>
#include <CORE/types.h>

using core::types::EnumDef;
using core::types::FieldDef;
using core::types::MessageDef;
using core::types::ProtoDef;
using core::types::RpcFunctionDef;
using core::types::ServiceDef;
using core::util::parser::RegExPattern;

using core::util::TrimQuotes;

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
    "PACKAGE",    "IMPORT",      "MESSAGE",    "SERVICE",    "ENUM",
    "RPC",        "RETURNS",     "REPEATED",   "DOUBLE",     "FLOAT",
    "INT32",      "INT64",       "UINT32",     "UINT64",     "SINT32",
    "SINT64",     "FIXED32",     "FIXED64",    "SFIXED32",   "SFIXED64",
    "BOOL",       "STRING",      "BYTES",      "IDENT",      "STR",
    "WS",         "NUM",         "EOL",        "EQUALS",     "COMMENT",
    "OPEN_BRACE", "CLOSE_BRACE", "OPEN_PEREN", "CLOSE_PEREN"};

typedef core::util::parser::Tokenizer< eTokenTypes > tTokenizer;
typedef std::vector< typename tTokenizer::Token > tTokenList;
typedef core::util::parser::LRParser< eTokenTypes > tParser;
typedef tParser::Rule tRule;
typedef tParser::RuleOrTokenChain tCRule;

/**
 * Utility to help build out a proto message def
 */
class ProtoDefBuilder {
  public:
  ProtoDefBuilder(ProtoDef &target) : m_target(target) {}

  /**
   *
   */
  bool setPackage(
      tParser::iNode *&unused,
      const tParser::tTokenOrNodeList::const_iterator &begin,
      const tParser::tTokenOrNodeList::const_iterator &end) {
    (void) unused;

    tParser::tTokenOrNodeList::const_iterator itr = begin + 1;

    RET_M(m_target.m_package.empty(), "Too many package defines.");
    m_target.m_package = std::move(itr->m_token.getToken());

    return true;
  }

  /**
   *
   */
  bool addImport(
      tParser::iNode *&unused,
      const tParser::tTokenOrNodeList::const_iterator &begin,
      const tParser::tTokenOrNodeList::const_iterator &end) {
    (void) unused;

    tParser::tTokenOrNodeList::const_iterator itr = begin + 1;
    m_target.m_imports.push_back(TrimQuotes(itr->m_token.getToken()));

    return true;
  }

  /**
   *
   */
  bool addService(
      tParser::iNode *&unused,
      const tParser::tTokenOrNodeList::const_iterator &begin,
      const tParser::tTokenOrNodeList::const_iterator &end) {
    (void) unused;

    tParser::tTokenOrNodeList::const_iterator itr = begin + 1;
    ServiceDef def;
    def.m_name = std::move(itr->m_token.getToken());
    std::advance(itr, 2);
    tParser::tNode< std::vector< RpcFunctionDef > > *pRpcs =
        reinterpret_cast< tParser::tNode< std::vector< RpcFunctionDef > > * >(
            itr->m_pNode);
    def.m_functions = std::move(pRpcs->m_data);
    delete pRpcs;

    m_target.m_services.push_back(def);
    return true;
  }

  /**
   *
   */
  bool addMessage(
      tParser::iNode *&unused,
      const tParser::tTokenOrNodeList::const_iterator &begin,
      const tParser::tTokenOrNodeList::const_iterator &end) {
    (void) unused;

    tParser::tTokenOrNodeList::const_iterator itr = begin + 1;
    tParser::tNode< MessageDef > *pMessage =
        reinterpret_cast< tParser::tNode< MessageDef > * >(itr->m_pNode);
    m_target.m_messages.push_back(std::move(pMessage->m_data));
    delete pMessage;

    return true;
  }

  private:
  ProtoDef &m_target;
};

/**
 *
 */
bool genEnumField(
    tParser::iNode *&ret,
    const tParser::tTokenOrNodeList::const_iterator &begin,
    const tParser::tTokenOrNodeList::const_iterator &end) {
  tParser::tTokenOrNodeList::const_iterator itr = begin;
  tParser::tNode< FieldDef > *pDef = new tParser::tNode< FieldDef >;
  pDef->m_data.m_name = itr->m_token.getToken();
  std::advance(itr, 2);
  CHECK(core::util::lexical_cast(
      itr->m_token.getToken(), pDef->m_data.m_fieldNum));
  ret = pDef;
  return true;
}

/**
 *
 */
bool genFieldList(
    tParser::iNode *&ret,
    const tParser::tTokenOrNodeList::const_iterator &begin,
    const tParser::tTokenOrNodeList::const_iterator &end) {
  tParser::tTokenOrNodeList::const_iterator itr = begin;
  tParser::tNode< std::vector< FieldDef > > *pList =
      new tParser::tNode< std::vector< FieldDef > >;
  tParser::tNode< FieldDef > *pDef =
      reinterpret_cast< tParser::tNode< FieldDef > * >(itr->m_pNode);
  pList->m_data.push_back(std::move(pDef->m_data));
  delete pDef;
  ret = pList;
  return true;
}

/**
 *
 */
bool appendFieldList(
    tParser::iNode *&ret,
    const tParser::tTokenOrNodeList::const_iterator &begin,
    const tParser::tTokenOrNodeList::const_iterator &end) {
  tParser::tTokenOrNodeList::const_iterator itr = begin;
  tParser::tNode< std::vector< FieldDef > > *pList =
      reinterpret_cast< tParser::tNode< std::vector< FieldDef > > * >(
          itr->m_pNode);
  ++itr;
  tParser::tNode< FieldDef > *pDef =
      reinterpret_cast< tParser::tNode< FieldDef > * >(itr->m_pNode);
  pList->m_data.push_back(std::move(pDef->m_data));
  delete pDef;
  ret = pList;
  return true;
}

/**
 *
 */
bool genRpc(
    tParser::iNode *&ret,
    const tParser::tTokenOrNodeList::const_iterator &begin,
    const tParser::tTokenOrNodeList::const_iterator &end) {
  tParser::tTokenOrNodeList::const_iterator itr = begin + 1;
  tParser::tNode< RpcFunctionDef > *pDef = new tParser::tNode< RpcFunctionDef >;
  pDef->m_data.m_name = itr->m_token.getToken();
  std::advance(itr, 2);
  pDef->m_data.m_param = itr->m_token.getToken();
  std::advance(itr, 4);
  pDef->m_data.m_return = itr->m_token.getToken();
  ret = pDef;
  return true;
}

/**
 *
 */
bool genRpcList(
    tParser::iNode *&ret,
    const tParser::tTokenOrNodeList::const_iterator &begin,
    const tParser::tTokenOrNodeList::const_iterator &end) {
  tParser::tTokenOrNodeList::const_iterator itr = begin;
  tParser::tNode< std::vector< RpcFunctionDef > > *pList =
      new tParser::tNode< std::vector< RpcFunctionDef > >;
  tParser::tNode< RpcFunctionDef > *pRpc =
      reinterpret_cast< tParser::tNode< RpcFunctionDef > * >(itr->m_pNode);
  pList->m_data.push_back(std::move(pRpc->m_data));
  delete pRpc;
  ret = pList;
  return true;
}

/**
 *
 */
bool appendRpcList(
    tParser::iNode *&ret,
    const tParser::tTokenOrNodeList::const_iterator &begin,
    const tParser::tTokenOrNodeList::const_iterator &end) {
  tParser::tTokenOrNodeList::const_iterator itr = begin;
  tParser::tNode< std::vector< RpcFunctionDef > > *pList =
      reinterpret_cast< tParser::tNode< std::vector< RpcFunctionDef > > * >(
          itr->m_pNode);
  ++itr;
  pList->m_data.push_back(
      reinterpret_cast< tParser::tNode< RpcFunctionDef > * >(itr->m_pNode)
          ->m_data);
  delete itr->m_pNode;
  ret = pList;
  return true;
}

/**
 *
 */
bool genEnum(
    tParser::iNode *&ret,
    const tParser::tTokenOrNodeList::const_iterator &begin,
    const tParser::tTokenOrNodeList::const_iterator &end) {
  tParser::tTokenOrNodeList::const_iterator itr = begin + 1;
  tParser::tNode< EnumDef > *pDef = new tParser::tNode< EnumDef >;
  pDef->m_data.m_name = itr->m_token.getToken();
  std::advance(itr, 2);
  tParser::tNode< std::vector< FieldDef > > *pValues =
      reinterpret_cast< tParser::tNode< std::vector< FieldDef > > * >(
          itr->m_pNode);
  pDef->m_data.m_values = std::move(pValues->m_data);
  delete pValues;
  ret = pDef;
  return true;
}

/**
 *
 */
bool genField(
    tParser::iNode *&ret,
    const tParser::tTokenOrNodeList::const_iterator &begin,
    const tParser::tTokenOrNodeList::const_iterator &end) {
  tParser::tTokenOrNodeList::const_iterator itr = begin;
  tParser::tNode< FieldDef > *pDef = new tParser::tNode< FieldDef >;
  if (itr->m_token.getId() == eTokenTypes::IDENT) {
    pDef->m_data.m_type = FieldDef::FIELD_MSG;
    pDef->m_data.m_msgType = itr->m_token.getToken();
  } else {
    pDef->m_data.m_type =
        FieldDef::eFieldType(itr->m_token.getId() - eTokenTypes::DOUBLE);
  }
  ++itr;
  pDef->m_data.m_name = itr->m_token.getToken();
  ++itr;
  if (itr->m_token.getId() == eTokenTypes::REPEATED) {
    pDef->m_data.m_repeated = true;
    std::advance(itr, 2);
  } else {
    pDef->m_data.m_repeated = false;
    ++itr;
  }
  CHECK(core::util::lexical_cast(
      itr->m_token.getToken(), pDef->m_data.m_fieldNum));
  ret = pDef;
  return true;
}

/**
 *
 */
bool finalizeMessage(
    tParser::iNode *&ret,
    const tParser::tTokenOrNodeList::const_iterator &begin,
    const tParser::tTokenOrNodeList::const_iterator &end) {
  tParser::tTokenOrNodeList::const_iterator itr = begin + 1;
  const std::string name = itr->m_token.getToken();
  std::advance(itr, 2);
  tParser::tNode< MessageDef > *pDef =
      reinterpret_cast< tParser::tNode< MessageDef > * >(itr->m_pNode);
  pDef->m_data.m_name = std::move(name);
  ret = pDef;
  return true;
}

/**
 *
 */
bool genMessageBodyMessage(
    tParser::iNode *&ret,
    const tParser::tTokenOrNodeList::const_iterator &begin,
    const tParser::tTokenOrNodeList::const_iterator &end) {
  tParser::tTokenOrNodeList::const_iterator itr = begin;
  tParser::tNode< MessageDef > *pDef =
      reinterpret_cast< tParser::tNode< MessageDef > * >(itr->m_pNode);
  ++itr;
  tParser::tNode< MessageDef > *pDefChild =
      reinterpret_cast< tParser::tNode< MessageDef > * >(itr->m_pNode);
  pDef->m_data.m_messages.push_back(std::move(pDefChild->m_data));
  delete pDefChild;
  ret = pDef;
  return true;
}

/**
 *
 */
bool genMessageMessageBody(
    tParser::iNode *&ret,
    const tParser::tTokenOrNodeList::const_iterator &begin,
    const tParser::tTokenOrNodeList::const_iterator &end) {
  tParser::tTokenOrNodeList::const_iterator itr = begin;
  tParser::tNode< MessageDef > *pDefChild =
      reinterpret_cast< tParser::tNode< MessageDef > * >(itr->m_pNode);
  ++itr;
  tParser::tNode< MessageDef > *pDef =
      reinterpret_cast< tParser::tNode< MessageDef > * >(itr->m_pNode);
  pDef->m_data.m_messages.push_back(std::move(pDefChild->m_data));
  delete pDefChild;
  ret = pDef;
  return true;
}

/**
 *
 */
bool genMessageEnum(
    tParser::iNode *&ret,
    const tParser::tTokenOrNodeList::const_iterator &begin,
    const tParser::tTokenOrNodeList::const_iterator &end) {
  tParser::tTokenOrNodeList::const_iterator itr = begin;
  tParser::tNode< MessageDef > *pDef;
  if (itr + 1 == end) {
    pDef = new tParser::tNode< MessageDef >;
  } else {
    pDef = reinterpret_cast< tParser::tNode< MessageDef > * >(itr->m_pNode);
    ++itr;
  }
  tParser::tNode< EnumDef > *pEnumDef =
      reinterpret_cast< tParser::tNode< EnumDef > * >(itr->m_pNode);
  pDef->m_data.m_enums.push_back(std::move(pEnumDef->m_data));
  delete pEnumDef;
  ret = pDef;
  return true;
}

/**
 *
 */
bool genMessageField(
    tParser::iNode *&ret,
    const tParser::tTokenOrNodeList::const_iterator &begin,
    const tParser::tTokenOrNodeList::const_iterator &end) {
  tParser::tTokenOrNodeList::const_iterator itr = begin;
  tParser::tNode< MessageDef > *pDef;
  if (itr + 1 == end) {
    pDef = new tParser::tNode< MessageDef >;
  } else {
    pDef = reinterpret_cast< tParser::tNode< MessageDef > * >(itr->m_pNode);
    ++itr;
  }
  tParser::tNode< FieldDef > *pField =
      reinterpret_cast< tParser::tNode< FieldDef > * >(itr->m_pNode);
  pDef->m_data.m_fields.push_back(std::move(pField->m_data));
  delete pField;
  ret = pDef;
  return true;
}

/**
 *
 */
bool parse(ProtoDef &def, const std::string &fileData) {
  tTokenizer::tTokenizerList tokenRegexs;
  tokenRegexs.push_back(tTokenizer::tTokenizer(
      eTokenTypes::WS, RegExPattern("([ \t\n\r]*)|(//.*\n)")));
  tokenRegexs.push_back(
      tTokenizer::tTokenizer(eTokenTypes::STR, RegExPattern("\"[^\"]*\"")));
  tokenRegexs.push_back(
      tTokenizer::tTokenizer(eTokenTypes::EOL, RegExPattern(";")));
  tokenRegexs.push_back(
      tTokenizer::tTokenizer(eTokenTypes::EQUALS, RegExPattern("=")));
  tokenRegexs.push_back(
      tTokenizer::tTokenizer(eTokenTypes::OPEN_BRACE, RegExPattern("{")));
  tokenRegexs.push_back(
      tTokenizer::tTokenizer(eTokenTypes::CLOSE_BRACE, RegExPattern("}")));
  tokenRegexs.push_back(
      tTokenizer::tTokenizer(eTokenTypes::OPEN_PEREN, RegExPattern("\\(")));
  tokenRegexs.push_back(
      tTokenizer::tTokenizer(eTokenTypes::CLOSE_PEREN, RegExPattern("\\)")));
  tokenRegexs.push_back(
      tTokenizer::tTokenizer(eTokenTypes::PACKAGE, RegExPattern("package ")));
  tokenRegexs.push_back(
      tTokenizer::tTokenizer(eTokenTypes::IMPORT, RegExPattern("import ")));
  tokenRegexs.push_back(
      tTokenizer::tTokenizer(eTokenTypes::MESSAGE, RegExPattern("message ")));
  tokenRegexs.push_back(
      tTokenizer::tTokenizer(eTokenTypes::SERVICE, RegExPattern("service ")));
  tokenRegexs.push_back(
      tTokenizer::tTokenizer(eTokenTypes::ENUM, RegExPattern("enum ")));
  tokenRegexs.push_back(
      tTokenizer::tTokenizer(eTokenTypes::RPC, RegExPattern("rpc ")));
  tokenRegexs.push_back(
      tTokenizer::tTokenizer(eTokenTypes::RETURNS, RegExPattern("returns ")));
  tokenRegexs.push_back(
      tTokenizer::tTokenizer(eTokenTypes::REPEATED, RegExPattern("\\[\\] ")));
  tokenRegexs.push_back(
      tTokenizer::tTokenizer(eTokenTypes::DOUBLE, RegExPattern("double ")));
  tokenRegexs.push_back(
      tTokenizer::tTokenizer(eTokenTypes::FLOAT, RegExPattern("float ")));
  tokenRegexs.push_back(
      tTokenizer::tTokenizer(eTokenTypes::INT32, RegExPattern("int32 ")));
  tokenRegexs.push_back(
      tTokenizer::tTokenizer(eTokenTypes::INT64, RegExPattern("int64 ")));
  tokenRegexs.push_back(
      tTokenizer::tTokenizer(eTokenTypes::UINT32, RegExPattern("uint32 ")));
  tokenRegexs.push_back(
      tTokenizer::tTokenizer(eTokenTypes::UINT64, RegExPattern("uint64 ")));
  tokenRegexs.push_back(
      tTokenizer::tTokenizer(eTokenTypes::SINT32, RegExPattern("sint32 ")));
  tokenRegexs.push_back(
      tTokenizer::tTokenizer(eTokenTypes::SINT64, RegExPattern("sint64 ")));
  tokenRegexs.push_back(
      tTokenizer::tTokenizer(eTokenTypes::FIXED32, RegExPattern("fixed32 ")));
  tokenRegexs.push_back(
      tTokenizer::tTokenizer(eTokenTypes::FIXED64, RegExPattern("fixed64 ")));
  tokenRegexs.push_back(
      tTokenizer::tTokenizer(eTokenTypes::SFIXED32, RegExPattern("sfixed32 ")));
  tokenRegexs.push_back(
      tTokenizer::tTokenizer(eTokenTypes::SFIXED64, RegExPattern("sfixed64 ")));
  tokenRegexs.push_back(
      tTokenizer::tTokenizer(eTokenTypes::BOOL, RegExPattern("bool ")));
  tokenRegexs.push_back(
      tTokenizer::tTokenizer(eTokenTypes::STRING, RegExPattern("string ")));
  tokenRegexs.push_back(
      tTokenizer::tTokenizer(eTokenTypes::BYTES, RegExPattern("bytes ")));
  tokenRegexs.push_back(
      tTokenizer::tTokenizer(eTokenTypes::NUM, RegExPattern("\\d*(\\.\\d+)?")));
  tokenRegexs.push_back(
      tTokenizer::tTokenizer(eTokenTypes::IDENT, RegExPattern("\\a[\\.\\w]*")));

  tTokenizer tokenizer(tokenRegexs);
  tParser parser(tokenizer);

  ProtoDefBuilder builder(def);

  tRule &packageRule = parser.addRule("package");
  packageRule |=
      tCRule(tCRule::tProc::
                 from_method< ProtoDefBuilder, &ProtoDefBuilder::setPackage >(
                     &builder))
      & eTokenTypes::PACKAGE & eTokenTypes::IDENT & eTokenTypes::EOL;

  tRule &importRule = parser.addRule("import");
  importRule |=
      tCRule(tCRule::tProc::
                 from_method< ProtoDefBuilder, &ProtoDefBuilder::addImport >(
                     &builder))
      & eTokenTypes::IMPORT & eTokenTypes::STR & eTokenTypes::EOL;

  tRule &fieldRule = parser.addRule("field");
  for (u32 fieldType = eTokenTypes::DOUBLE; fieldType <= eTokenTypes::BYTES;
       ++fieldType) {
    fieldRule |= tCRule(tCRule::tProc::from_function< genField >())
                 & eTokenTypes::type(fieldType) & eTokenTypes::IDENT
                 & eTokenTypes::EQUALS & eTokenTypes::NUM & eTokenTypes::EOL;
  }
  for (u32 fieldType = eTokenTypes::DOUBLE; fieldType <= eTokenTypes::BYTES;
       ++fieldType) {
    fieldRule |= tCRule(tCRule::tProc::from_function< genField >())
                 & eTokenTypes::type(fieldType) & eTokenTypes::IDENT
                 & eTokenTypes::REPEATED & eTokenTypes::EQUALS
                 & eTokenTypes::NUM & eTokenTypes::EOL;
  }
  fieldRule |= tCRule(tCRule::tProc::from_function< genField >())
               & eTokenTypes::IDENT & eTokenTypes::IDENT & eTokenTypes::EQUALS
               & eTokenTypes::NUM & eTokenTypes::EOL;
  fieldRule |= tCRule(tCRule::tProc::from_function< genField >())
               & eTokenTypes::IDENT & eTokenTypes::IDENT & eTokenTypes::REPEATED
               & eTokenTypes::EQUALS & eTokenTypes::NUM & eTokenTypes::EOL;

  tRule &enumFieldRule = parser.addRule("enumfield");
  enumFieldRule |= tCRule(tCRule::tProc::from_function< genEnumField >())
                   & eTokenTypes::IDENT & eTokenTypes::EQUALS & eTokenTypes::NUM
                   & eTokenTypes::EOL;

  tRule &enumFieldListRule = parser.addRule("enumfieldList");
  enumFieldListRule |= tCRule(tCRule::tProc::from_function< appendFieldList >())
                       & enumFieldListRule & enumFieldRule;
  enumFieldListRule |=
      tCRule(tCRule::tProc::from_function< genFieldList >()) & enumFieldRule;

  tRule &enumRule = parser.addRule("enum");
  enumRule |= tCRule(tCRule::tProc::from_function< genEnum >())
              & eTokenTypes::ENUM & eTokenTypes::IDENT & eTokenTypes::OPEN_BRACE
              & enumFieldListRule & eTokenTypes::CLOSE_BRACE;

  tRule &messageBodyRule = parser.addRule("messageBody");
  tRule &messageRule = parser.addRule("message");
  messageRule |= tCRule(tCRule::tProc::from_function< finalizeMessage >())
                 & eTokenTypes::MESSAGE & eTokenTypes::IDENT
                 & eTokenTypes::OPEN_BRACE & messageBodyRule
                 & eTokenTypes::CLOSE_BRACE;

  messageBodyRule |=
      tCRule(tCRule::tProc::from_function< genMessageMessageBody >())
      & messageRule & messageBodyRule;
  messageBodyRule |=
      tCRule(tCRule::tProc::from_function< genMessageBodyMessage >())
      & messageBodyRule & messageRule;
  messageBodyRule |= tCRule(tCRule::tProc::from_function< genMessageEnum >())
                     & messageBodyRule & enumRule;
  messageBodyRule |= tCRule(tCRule::tProc::from_function< genMessageField >())
                     & messageBodyRule & fieldRule;
  messageBodyRule |=
      tCRule(tCRule::tProc::from_function< genMessageEnum >()) & enumRule;
  messageBodyRule |=
      tCRule(tCRule::tProc::from_function< genMessageField >()) & fieldRule;

  tRule &serviceRpc = parser.addRule("serviceRpc");
  serviceRpc |= tCRule(tCRule::tProc::from_function< genRpc >())
                & eTokenTypes::RPC & eTokenTypes::IDENT
                & eTokenTypes::OPEN_PEREN & eTokenTypes::IDENT
                & eTokenTypes::CLOSE_PEREN & eTokenTypes::RETURNS
                & eTokenTypes::OPEN_PEREN & eTokenTypes::IDENT
                & eTokenTypes::CLOSE_PEREN & eTokenTypes::EOL;

  tRule &serviceRpcListRule = parser.addRule("serviceRpcList");
  serviceRpcListRule |= tCRule(tCRule::tProc::from_function< appendRpcList >())
                        & serviceRpcListRule & serviceRpc;
  serviceRpcListRule |=
      tCRule(tCRule::tProc::from_function< genRpcList >()) & serviceRpc;

  tRule &serviceRule = parser.addRule("service");
  serviceRule |=
      tCRule(tCRule::tProc::
                 from_method< ProtoDefBuilder, &ProtoDefBuilder::addService >(
                     &builder))
      & eTokenTypes::SERVICE & eTokenTypes::IDENT & eTokenTypes::OPEN_BRACE
      & serviceRpcListRule & eTokenTypes::CLOSE_BRACE;

  tRule &fileRule = parser.addRule("file");
  fileRule |= tCRule() & packageRule;
  fileRule |= tCRule() & fileRule & importRule;
  fileRule |= tCRule() & fileRule & serviceRule;
  fileRule |=
      tCRule(tCRule::tProc::
                 from_method< ProtoDefBuilder, &ProtoDefBuilder::addMessage >(
                     &builder))
      & fileRule & messageRule;

  return parser.parse(eTokenTypes::WS, fileData.begin(), fileData.end());
}

} // namespace proto
