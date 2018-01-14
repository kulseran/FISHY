/**
 * xml_parser.cpp
 */
#include "xml_parser.h"

#include <CORE/UTIL/lrparser.h>
#include <CORE/UTIL/regex.h>
#include <CORE/UTIL/stringutil.h>
#include <CORE/UTIL/tokenizer.h>

#include <memory>
#include <stack>

using core::util::TrimQuotes;
using core::util::parser::LRParser;
using core::util::parser::RegExPattern;
using core::util::parser::Tokenizer;

typedef std::map< std::string, std::string > tAttributeMap;
typedef std::pair< std::string, std::string > tAttribute;
typedef std::vector< core::util::files::XmlNode > tNodeList;
typedef std::pair< std::string, tAttributeMap > tXmlTag;

/**
 * Parsing tokens
 */
struct eTokens {
  enum type {
    IDENTIFIER,
    WS,
    EQUAL,
    NUMERIC,
    STRING,
    DEFINE,
    OCARROT,
    CCARROT,
    TERMINATOR,
    MISC,

    COUNT
  };

  static const char *names[eTokens::COUNT];
};

/**
 * Token names for error logging
 */
const char *eTokens::names[eTokens::COUNT] = {"IDENTIFIER",
                                              "WS",
                                              "EQUAL",
                                              "NUMERIC",
                                              "STRING",
                                              "DEFINE",
                                              "OCARROT",
                                              "CCARROT",
                                              "TERMINATOR",
                                              "MISC"};

typedef Tokenizer< eTokens > tTokenizer;
typedef LRParser< eTokens > tParser;
typedef tParser::Rule tRule;
typedef tParser::RuleOrTokenChain tCRule;
typedef tParser::TokenOrNode tParseNode;

/**
 * Static {@link Tokenizer} factory with all the tokens.
 */
class TokenizerFactory {
  public:
  TokenizerFactory() {
    tTokenizer::tTokenizerList tokenRegexs;
    tokenRegexs.push_back(
        tTokenizer::tTokenizer(eTokens::STRING, RegExPattern("\"[^\"]*\"")));
    tokenRegexs.push_back(
        tTokenizer::tTokenizer(eTokens::IDENTIFIER, RegExPattern("\\a\\w*")));
    tokenRegexs.push_back(
        tTokenizer::tTokenizer(eTokens::OCARROT, RegExPattern("<")));
    tokenRegexs.push_back(
        tTokenizer::tTokenizer(eTokens::CCARROT, RegExPattern(">")));
    tokenRegexs.push_back(
        tTokenizer::tTokenizer(eTokens::EQUAL, RegExPattern("=")));
    tokenRegexs.push_back(
        tTokenizer::tTokenizer(eTokens::NUMERIC, RegExPattern("\\d+")));
    tokenRegexs.push_back(
        tTokenizer::tTokenizer(eTokens::TERMINATOR, RegExPattern("/")));
    tokenRegexs.push_back(
        tTokenizer::tTokenizer(eTokens::DEFINE, RegExPattern("\\?")));
    tokenRegexs.push_back(
        tTokenizer::tTokenizer(eTokens::WS, RegExPattern("\\s")));
    tokenRegexs.push_back(
        tTokenizer::tTokenizer(eTokens::MISC, RegExPattern(".")));
    m_tokenizer = std::shared_ptr< tTokenizer >(new tTokenizer(tokenRegexs));
  }

  const tTokenizer &get() const { return *(m_tokenizer.get()); }

  private:
  std::shared_ptr< tTokenizer > m_tokenizer;
};
static const TokenizerFactory g_tokenizerFactory;

namespace core {
namespace util {
namespace files {

/**
 *
 */
XmlNode::XmlNode(
    const std::string &name,
    const std::string &content,
    std::map< std::string, std::string > &attributes,
    std::vector< XmlNode > &children)
    : m_name(name),
      m_str(content),
      m_attributes(attributes),
      m_children(children) {
}

/**
 *
 */
const std::string &XmlNode::getName() const {
  return m_name;
}

/**
 *
 */
bool XmlNode::getAttribute(const std::string &name, std::string &value) const {
  const tAttributeMap::const_iterator itr = m_attributes.find(name);
  if (itr == m_attributes.end()) {
    return false;
  }
  value = itr->second;
  return true;
}

/**
 *
 */
size_t XmlNode::getChildCount() const {
  return m_children.size();
}

/**
 *
 */
const XmlNode &XmlNode::getChild(const size_t index) const {
  CHECK(index < m_children.size());
  return m_children[index];
}

/**
 *
 */
bool XmlNode::getChild(
    const std::string &name, size_t &index, const size_t startIndex) const {
  CHECK(startIndex < m_children.size());
  std::vector< XmlNode >::const_iterator begin = m_children.begin();
  std::advance(begin, startIndex);
  for (std::vector< XmlNode >::const_iterator itr = begin;
       itr != m_children.end();
       ++itr) {
    if (itr->getName() == name) {
      index = std::distance(m_children.begin(), itr);
      return true;
    }
  }
  return false;
}

/**
 *
 */
bool genAttribute(
    tParser::iNode *&ret,
    const tParser::tTokenOrNodeList::const_iterator &begin,
    const tParser::tTokenOrNodeList::const_iterator &end) {
  tParser::tTokenOrNodeList::const_iterator itr = begin;
  tParseNode node1 = *itr;
  std::advance(itr, 2);
  tParseNode node2 = *itr;
  const tAttribute attrib(
      node1.m_token.getToken(), TrimQuotes(node2.m_token.getToken()));
  ret = new tParser::tNode< tAttribute >(attrib);
  return true;
}

/**
 *
 */
bool genAttributeList(
    tParser::iNode *&ret,
    const tParser::tTokenOrNodeList::const_iterator &begin,
    const tParser::tTokenOrNodeList::const_iterator &end) {
  tParser::tTokenOrNodeList::const_iterator itr = begin + 1;
  tParser::tNode< tAttributeMap > *rVal = nullptr;
  if (itr == end) {
    rVal = new tParser::tNode< tAttributeMap >;
    itr = begin;
  } else {
    rVal =
        reinterpret_cast< tParser::tNode< tAttributeMap > * >(begin->m_pNode);
  }
  const tParser::tNode< tAttribute > *attrib =
      reinterpret_cast< tParser::tNode< tAttribute > * >(itr->m_pNode);
  rVal->m_data.insert(std::move(attrib->m_data));
  delete attrib;
  ret = rVal;
  return true;
}

/**
 *
 */
bool genTag(
    tParser::iNode *&ret,
    const tParser::tTokenOrNodeList::const_iterator &begin,
    const tParser::tTokenOrNodeList::const_iterator &end) {
  tParser::tTokenOrNodeList::const_iterator itr = begin + 1;
  if (itr->m_type == tParseNode::TOKEN
      && itr->m_token.getId() == eTokens::TERMINATOR) {
    ++itr;
  }
  const std::string name = itr->m_token.getToken();
  ++itr;
  if (itr->m_pNode) {
    tParser::tNode< tAttributeMap > *pMap =
        reinterpret_cast< tParser::tNode< tAttributeMap > * >(itr->m_pNode);
    tParser::tNode< tXmlTag > *rVal =
        new tParser::tNode< tXmlTag >(tXmlTag(name, std::move(pMap->m_data)));
    delete pMap;
    ret = rVal;
  } else {
    ret = new tParser::tNode< tXmlTag >(tXmlTag(name, tAttributeMap()));
  }
  return true;
}

/**
 * Helper class for computing nodes from the above tags and attributes.
 */
class XmlNodeBuilder {
  public:
  XmlNodeBuilder(XmlNode &target) : m_target(target) {}

  /**
   *
   */
  bool genNode(
      tParser::iNode *&ret,
      const tParser::tTokenOrNodeList::const_iterator &begin,
      const tParser::tTokenOrNodeList::const_iterator &end) {
    tParser::tTokenOrNodeList::const_iterator itr = begin;
    tParser::tNode< tXmlTag > *pOpenTag =
        reinterpret_cast< tParser::tNode< tXmlTag > * >(itr->m_pNode);
    tParser::tNode< tXmlTag > *pCloseTag = nullptr;
    tParser::tNode< tNodeList > emptyNodeList;
    tParser::tNode< tNodeList > *pChildren = &emptyNodeList;
    ++itr;
    if (itr != end) {
      tParseNode childrenOrCloseTag = *itr;
      ++itr;
      if (itr != end) {
        tParseNode closeTag = *itr;
        pCloseTag =
            reinterpret_cast< tParser::tNode< tXmlTag > * >(closeTag.m_pNode);
        pChildren = reinterpret_cast< tParser::tNode< tNodeList > * >(
            childrenOrCloseTag.m_pNode);
      } else {
        tParseNode closeTag = childrenOrCloseTag;
        pCloseTag =
            reinterpret_cast< tParser::tNode< tXmlTag > * >(closeTag.m_pNode);
      }

      if (pCloseTag->m_data.first != pOpenTag->m_data.first) {
        Log(LL::Error) << "Mismatched closing tag <" << pCloseTag->m_data.first
                       << "> for opening <" << pOpenTag->m_data.first << ">";
        return false;
      }
    }

    const XmlNode node(
        pOpenTag->m_data.first, "", pOpenTag->m_data.second, pChildren->m_data);
    ret = new tParser::tNode< XmlNode >(node);
    delete pOpenTag;
    delete pCloseTag;
    if (pChildren != &emptyNodeList) {
      delete pChildren;
    }
    return true;
  }

  /**
   *
   */
  bool genNodeList(
      tParser::iNode *&ret,
      const tParser::tTokenOrNodeList::const_iterator &begin,
      const tParser::tTokenOrNodeList::const_iterator &end) {
    tParser::tTokenOrNodeList::const_iterator itr = begin;
    ++itr;
    tParser::tNode< tNodeList > *rVal = nullptr;
    if (itr == end) {
      rVal = new tParser::tNode< tNodeList >;
      itr = begin;
    } else {
      rVal = reinterpret_cast< tParser::tNode< tNodeList > * >(begin->m_pNode);
    }
    const tParser::tNode< XmlNode > *pNode =
        reinterpret_cast< tParser::tNode< XmlNode > * >(itr->m_pNode);
    rVal->m_data.push_back(pNode->m_data);
    delete pNode;
    ret = rVal;
    return true;
  }

  /**
   *
   */
  bool populateNode(
      tParser::iNode *&ret,
      const tParser::tTokenOrNodeList::const_iterator &begin,
      const tParser::tTokenOrNodeList::const_iterator &end) {
    (void) ret;

    tParser::tTokenOrNodeList::const_iterator itr = begin + 1;
    m_target.m_name = "root";
    tParser::tNode< tNodeList > *pNodeList =
        reinterpret_cast< tParser::tNode< tNodeList > * >(itr->m_pNode);
    m_target.m_children.insert(
        m_target.m_children.end(),
        pNodeList->m_data.begin(),
        pNodeList->m_data.end());
    delete pNodeList;
    return true;
  }

  private:
  XmlNode &m_target;
};

/**
 *
 */
Status XmlNode::parse(const std::string &fileData) {
  const tTokenizer &tokenizer = g_tokenizerFactory.get();

  tParser tParser(tokenizer);

  tRule &attribute = tParser.addRule("attribute");
  attribute |= tCRule(tCRule::tProc::from_function< genAttribute >())
               & eTokens::IDENTIFIER & eTokens::EQUAL & eTokens::STRING;

  tRule &attributeList = tParser.addRule("attributeList");
  attributeList |= tCRule(tCRule::tProc::from_function< genAttributeList >())
                   & attributeList & attribute;
  attributeList |=
      tCRule(tCRule::tProc::from_function< genAttributeList >()) & attribute;

  tRule &prolog = tParser.addRule("prolog");
  prolog |= tCRule() & eTokens::OCARROT & eTokens::DEFINE & eTokens::IDENTIFIER
            & attributeList & eTokens::DEFINE & eTokens::CCARROT;

  tRule &tag = tParser.addRule("tag");
  tag |= tCRule(tCRule::tProc::from_function< genTag >()) & eTokens::OCARROT
         & eTokens::IDENTIFIER & eTokens::CCARROT;
  tag |= tCRule(tCRule::tProc::from_function< genTag >()) & eTokens::OCARROT
         & eTokens::IDENTIFIER & attributeList & eTokens::CCARROT;

  tRule &closeTag = tParser.addRule("closeTag") |=
      tCRule(tCRule::tProc::from_function< genTag >()) & eTokens::OCARROT
      & eTokens::TERMINATOR & eTokens::IDENTIFIER & eTokens::CCARROT;

  tRule &closedTag = tParser.addRule("closedTag");
  closedTag |= tCRule(tCRule::tProc::from_function< genTag >())
               & eTokens::OCARROT & eTokens::IDENTIFIER & eTokens::TERMINATOR
               & eTokens::CCARROT;
  closedTag |= tCRule(tCRule::tProc::from_function< genTag >())
               & eTokens::OCARROT & eTokens::IDENTIFIER & attributeList
               & eTokens::TERMINATOR & eTokens::CCARROT;

  XmlNodeBuilder builder(*this);

  tRule &node = tParser.addRule("node");
  tRule &nodeList = tParser.addRule("nodelist");
  node |= tCRule(tCRule::tProc::
                     from_method< XmlNodeBuilder, &XmlNodeBuilder::genNode >(
                         &builder))
          & tag & closeTag;
  node |= tCRule(tCRule::tProc::
                     from_method< XmlNodeBuilder, &XmlNodeBuilder::genNode >(
                         &builder))
          & tag & nodeList & closeTag;
  node |= tCRule(tCRule::tProc::
                     from_method< XmlNodeBuilder, &XmlNodeBuilder::genNode >(
                         &builder))
          & closedTag;
  nodeList |=
      tCRule(tCRule::tProc::
                 from_method< XmlNodeBuilder, &XmlNodeBuilder::genNodeList >(
                     &builder))
      & nodeList & node;
  nodeList |=
      tCRule(tCRule::tProc::
                 from_method< XmlNodeBuilder, &XmlNodeBuilder::genNodeList >(
                     &builder))
      & node;

  tRule &xmlFile = tParser.addRule("xmlfile");
  xmlFile |= tCRule() & prolog;
  xmlFile |=
      tCRule(tCRule::tProc::
                 from_method< XmlNodeBuilder, &XmlNodeBuilder::populateNode >(
                     &builder))
      & xmlFile & nodeList;

  return Status(tParser.parse(eTokens::WS, fileData.begin(), fileData.end()));
}

} // namespace files
} // namespace util
} // namespace core
