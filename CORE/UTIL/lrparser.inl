/**
 * lrparser.inl
 *
 * LR parser grammer
 */
#ifndef FISHY_LRPARSER_INL
#define FISHY_LRPARSER_INL

#include <CORE/UTIL/stringutil.h>

namespace core {
namespace util {
namespace parser {

/**
 *
 */
template< typename tId >
inline LRParser< tId >::TokenOrNode::TokenOrNode()
  : m_pNode(nullptr),
    m_type(INVALID),
    m_line(0) {
}

/**
 *
 */
template< typename tId >
inline LRParser< tId >::TokenOrNode::TokenOrNode(typename Tokenizer< tId >::Token token, const u32 line)
  : m_token(token),
    m_pNode(nullptr),
    m_type(TOKEN),
    m_line(line) {
};

/**
 *
 */
template< typename tId >
inline LRParser< tId >::TokenOrNode::TokenOrNode(const u32 rule, const u32 line, void *pNode)
  : m_ruleId(rule),
    m_pNode(pNode),
    m_type(RULE_NODE),
    m_line(line) {
}

/**
 *
 */
template< typename tId >
inline bool LRParser< tId >::TokenOrNode::matches(const TokenOrNode &other) const {
  if (m_type != other.m_type) {
    return false;
  }
  switch (m_type) {
    case TOKEN:
      return m_token.getId() == other.m_token.getId();
    case RULE_NODE:
      return m_ruleId == other.m_ruleId;
    default:
      CHECK_UNREACHABLE();
  }
  CHECK_UNREACHABLE();
  return false;
}

/**
 *
 */
template< typename tId >
inline LRParser< tId >::RuleOrTokenChain::RuleOrTokenChain() {
}

/**
 *
 */
template< typename tId >
inline LRParser< tId >::RuleOrTokenChain::RuleOrTokenChain(const typename LRParser< tId >::RuleOrTokenChain::tProc &callback)
  : m_callback(callback) {
}

/**
 *
 */
template< typename tId >
inline typename LRParser< tId >::RuleOrTokenChain LRParser< tId >::RuleOrTokenChain::operator & (const typename LRParser< tId >::Rule &rule) const {
  RuleOrTokenChain rVal = *this;
  rVal.m_nodes.push_back(TokenOrNode(rule.getId(), 0, nullptr));
  return rVal;
}

/**
 *
 */
template< typename tId >
inline typename LRParser< tId >::RuleOrTokenChain LRParser< tId >::RuleOrTokenChain::operator & (const typename tId::type &token) const {
  RuleOrTokenChain rVal = *this;
  rVal.m_nodes.push_back(TokenOrNode(Tokenizer< tId >::Token(token, std::string().begin(), std::string().end()), 0));
  return rVal;
}

/**
 *
 */
template< typename tId >
inline LRParser< tId >::Rule::Rule()
  : m_name(nullptr),
    m_id(-1) {
}

/**
 *
 */
template< typename tId >
inline LRParser< tId >::Rule::Rule(const char *name, u32 id)
  : m_name(name),
    m_id(id) {
}

/**
 *
 */
template< typename tId >
inline const char *LRParser< tId >::Rule::getName() const {
  return m_name;
}

/**
 *
 */
template< typename tId >
inline typename LRParser< tId >::Rule &LRParser< tId >::Rule::operator |= (const typename LRParser< tId >::RuleOrTokenChain &chain) {
  m_chains.push_back(chain);
  return *this;
}

/**
 *
 */
template< typename tId >
inline bool LRParser< tId >::Rule::reduce(
  TokenOrNode &node,
  const u32 chainId,
  const typename tTokenOrNodeList::const_iterator &begin,
  const typename tTokenOrNodeList::const_iterator &end) const {
  const u32 line = begin->m_line;
  const RuleOrTokenChain &chain = m_chains[chainId];

  typename tTokenOrNodeList::const_iterator cur = begin;
  for (typename tTokenOrNodeList::const_iterator itr = chain.m_nodes.begin(); itr != chain.m_nodes.end() && cur != end; ++itr, ++cur) {
    if (!itr->matches(*cur)) {
      return false;
    }
  }
  if (cur != end) {
    return false;
  }

  void *pNode = nullptr;
  if (chain.m_callback) {
    if (!chain.m_callback(pNode, begin, end)) {
      return false;
    }
  } else {
    for (cur = begin; cur != end; ++cur) {
      delete cur->m_pNode;
    }
  }
  node = TokenOrNode(m_id, line, pNode);
  return true;
}

/**
 *
 */
template< typename tId >
inline u32 LRParser< tId >::Rule::matchLen(
  const u32 chainId,
  const typename tTokenOrNodeList::const_iterator &begin,
  const typename tTokenOrNodeList::const_iterator &end) const {
  const RuleOrTokenChain &chain = m_chains[chainId];

  u32 matchLen = 0;
  typename tTokenOrNodeList::const_iterator cur = begin;
  for (typename tTokenOrNodeList::const_iterator itr = chain.m_nodes.begin(); itr != chain.m_nodes.end() && cur != end; ++itr, ++cur) {
    if (itr->matches(*cur)) {
      matchLen++;
    } else {
      break;
    }
  }
  return matchLen;
}

/**
 *
 */
template< typename tId >
inline u32 LRParser< tId >::Rule::chains() const {
  return m_chains.size();
}

/**
 *
 */
template< typename tId >
inline u32 LRParser< tId >::Rule::tokenLen(u32 chain) const {
  return m_chains[chain].m_nodes.size();
}

/**
 *
 */
template< typename tId >
inline u32 LRParser< tId >::Rule::getId() const {
  return m_id;
}
/**
 *
 */
template< typename tId >
inline LRParser< tId >::LRParser(const Tokenizer< tId > &tokenizer, const u32 stackDepth)
  : m_tokenizer(tokenizer),
    m_stackDepth(stackDepth) {
  m_rules.reserve(128);
}

/**
 *
 */
template< typename tId >
inline typename LRParser< tId >::Rule &LRParser< tId >::addRule(const char *name) {
  CHECK(m_rules.size() < m_rules.capacity());

  Rule rule(name, m_rules.size());
  m_rules.push_back(rule);
  return m_rules.back();
}

/**
 *
 */
template< typename tId >
inline bool LRParser< tId >::parse(const typename tId::type ignore, const std::string::const_iterator &begin, const std::string::const_iterator &end) const {
  return parse(ignore, begin, end, tErrorCB::from_function<nullErrorCallback>());
}

/**
 *
 */
template< typename tId >
inline bool LRParser< tId >::parse(const typename tId::type ignore, const std::string::const_iterator &begin, const std::string::const_iterator &end, const tErrorCB &cb) const {
  std::vector< TokenOrNode > stack;
  stack.reserve(m_stackDepth);

  std::string::const_iterator cur = begin;
  Tokenizer< tId >::Token token;
  u32 line = 0;
  while (m_tokenizer.getNextToken(token, cur, end)) {
    cur = token.end();
    line += core::util::countLines(token.begin(), token.end());
    if (token.getId() == ignore) {
      continue;
    }
    stack.push_back(TokenOrNode(token, line));
    while (reduce(stack)) {
    }
  }

  if (stack.size() != 1 || stack.back().m_ruleId != m_rules.back().getId()) {
    storeBestError(stack, cb);
    return false;
  }
  return true;
}

/**
 *
 */
template< typename tId >
inline bool LRParser< tId >::reduce(std::vector< typename LRParser< tId >::TokenOrNode > &stack) const {
  for (RuleList::const_iterator itr = m_rules.begin(); itr != m_rules.end(); ++itr) {
    for (u32 chain = 0; chain < itr->chains(); ++chain) {
      const u32 len = itr->tokenLen(chain);
      if (len > stack.size()) {
        continue;
      }
      tTokenOrNodeList::const_iterator begin = stack.begin() + (stack.size() - len);
      TokenOrNode token;
      if (itr->reduce(token, chain, begin, stack.end())) {
        stack.erase(begin, stack.end());
        stack.push_back(token);
        return true;
      }
    }
  }
  return false;
}

/**
 *
 */
template< typename tId >
inline void LRParser< tId >::storeBestError(const std::vector< typename LRParser< tId >::TokenOrNode > &stack, const typename LRParser< tId >::tErrorCB &cb) const {
  std::vector< TokenOrNode >::const_iterator itr = stack.begin();

  u32 bestRule = m_rules.size();
  u32 lastFollow = m_rules.size();
  while (itr != stack.end()) {
    u32 bestMatch = 0;
    for (RuleList::const_iterator rule = m_rules.begin(); rule != m_rules.end(); ++rule) {
      if (itr->m_type == TokenOrNode::RULE_NODE && itr->m_ruleId == std::distance(m_rules.begin(), rule)) {
        bestMatch = 1;
        lastFollow = std::distance(m_rules.begin(), rule);
        bestRule = m_rules.size();
      }
      for (u32 chain = 0; chain < rule->chains(); ++chain) {
        const u32 matchLen = rule->matchLen(chain, itr, stack.end());
        if (matchLen > bestMatch) {
          bestMatch = matchLen;
          bestRule = std::distance(m_rules.begin(), rule);
          lastFollow = m_rules.size();
        }
      }
    }
    if (bestMatch > 0) {
      std::advance(itr, bestMatch);
    } else {
      cb("Unable to match symbols");
      return;
    }
    std::string tag = "<EOF>";
    int lastLine = -1;
    if (itr != stack.end()) {
      if (itr->m_type == TokenOrNode::TOKEN) {
        tag = tId::names[itr->m_token.getId()];
      } else if (itr->m_type == TokenOrNode::RULE_NODE) {
        tag = m_rules[itr->m_ruleId].getName();
      } else {
        CHECK_UNREACHABLE();
      }
      lastLine = itr->m_line;
    }
    std::stringstream msg;
    if (bestRule != m_rules.size()) {
      msg << "Unexpected " << tag << " while parsing " << m_rules[bestRule].getName() << " line " << lastLine;
    } else if (lastFollow != m_rules.size()) {
      msg << "Unexpected " << tag << " following " << m_rules[lastFollow].getName() << " line " << lastLine;
    } else {
      msg << "Unexpected " << tag << " with no matching rules. Line " << lastLine;
    }
    cb(msg.str());
  }
  return;
}


} // namespace parser
} // namespace util
} // namespace core

#endif
