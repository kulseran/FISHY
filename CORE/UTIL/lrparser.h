/**
 * LR parser grammer
 */
#ifndef FISHY_LRPARSER_H
#define FISHY_LRPARSER_H

#include <CORE/BASE/logging.h>
#include <EXTERN_LIB/srutil/delegate/delegate.hpp>

#include "tokenizer.h"

namespace core {
namespace util {
namespace parser {

/**
 * Shift Reduce parser for LR grammers
 * tId must contain enum type and const char *name[].
 */
template < typename tId >
class LRParser {
  public:
  class Rule;

  /**
   * Base class for rule node information in {@link TokenOrNode}
   */
  class iNode {
    public:
    virtual ~iNode() { }
  };
  template <typename tType>
  class tNode : public iNode {
    public:
    tType m_data;
  };

  /**
   * Container for a parse token or a rule node.
   */
  class TokenOrNode {
    public:
    enum type { TOKEN, RULE_NODE, INVALID };

    TokenOrNode();
    TokenOrNode(typename Tokenizer< tId >::Token token, const size_t line);
    TokenOrNode(const size_t rule, const size_t line, iNode *pNode);
    bool matches(const TokenOrNode &other) const;

    type m_type;
    typename Tokenizer< tId >::Token m_token;
    size_t m_ruleId;
    iNode *m_pNode;
    size_t m_line;
  };
  typedef std::vector< TokenOrNode > tTokenOrNodeList;

  /**
   * Container for defining a chain of Rules/Tokens that ends in a callback
   * when the chain is matched.
   */
  class RuleOrTokenChain {
    public:
    /**
     * A callback for processing a rule reduction.  This callback is responsible
     * for cleaning up any node pointers returned from the child rules that are
     * passed into it.
     *
     * @param node the output node information from your parser implementation
     * for this rule
     * @param begin the start of the token chain that matched this rule
     * @param end the end of the token chain that matched this rule
     */
    typedef srutil::delegate< bool(
        iNode *&node,
        const typename tTokenOrNodeList::const_iterator &begin,
        const typename tTokenOrNodeList::const_iterator &end) >
        tProc;

    RuleOrTokenChain();
    /**
     * Generates a new RuleOrTokenChain with the given callback.
     * The callback will be processed whenever this chain is used as a reduction
     * rule in the parser.
     *
     * @param callback the callback
     */
    RuleOrTokenChain(const tProc &callback);

    /**
     * Add a {@link Rule} as the next token in this reduction.
     *
     * @param rule the rule to add
     * @return a chain with this rule appended
     */
    RuleOrTokenChain operator&(const Rule &rule) const;

    /**
     * Add a Token as the next token in this reduction.
     *
     * @param token the token to add
     * @return a chain with this token appended
     */
    RuleOrTokenChain operator&(const typename tId::type &token) const;

    tTokenOrNodeList m_nodes;
    tProc m_callback;
  };

  /**
   * Container for a single reduction rule in the parser.
   */
  class Rule {
    public:
    Rule();
    Rule(const char *name, size_t id);

    /**
     * Add a reduction definition to this rule.
     *
     * @param chain the chain of tokens and rules to consider as a reduction
     * @return a self reference to the modified rule
     */
    Rule &operator|=(const RuleOrTokenChain &chain);

    bool reduce(
        TokenOrNode &node,
        const size_t chainId,
        const typename tTokenOrNodeList::const_iterator &begin,
        const typename tTokenOrNodeList::const_iterator &end) const;
    size_t matchLen(
        const size_t chainId,
        const typename tTokenOrNodeList::const_iterator &begin,
        const typename tTokenOrNodeList::const_iterator &end) const;

    size_t chains() const;
    size_t tokenLen(size_t chain) const;
    size_t getId() const;
    const char *getName() const;

    private:
    const char *m_name;
    size_t m_id;

    typedef std::vector< RuleOrTokenChain > RuleOrTokenChainList;
    RuleOrTokenChainList m_chains;
  };

  public:
  typedef std::vector< Rule > RuleList;
  typedef srutil::delegate< void(const std::string &) > tErrorCB;

  /**
   * Construct a Parser with the given rules and tokenizer
   */
  LRParser(const Tokenizer< tId > &tokenizer, const size_t stackDepth = 1024);

  /**
   * Generate a new rule in the parser.
   * The last generated rule is considered the final target token.
   *
   * @param name the debugging name of the rule
   * @return a {@link Rule} reference that can be edited
   */
  Rule &addRule(const char *name);

  /**
   * Parse the input string.
   *
   * @param ignore the 'whitespace' type symbol in the token stream to ignore
   * @param begin the start of the text to tokenize and parse
   * @param end the end of the text to tokenize and parse
   * @return success status
   */
  bool parse(
      const typename tId::type ignore,
      const std::string::const_iterator &begin,
      const std::string::const_iterator &end) const;

  /**
   * Parse the input string.
   *
   * @param ignore the 'whitespace' type symbol in the token stream to ignore
   * @param begin the start of the text to tokenize and parse
   * @param end the end of the text to tokenize and parse
   * @param cb the error callback that will be called with any parse errors
   * @return success status
   */
  bool parse(
      const typename tId::type ignore,
      const std::string::const_iterator &begin,
      const std::string::const_iterator &end,
      const tErrorCB &cb) const;

  private:
  const Tokenizer< tId > &m_tokenizer;
  const size_t m_stackDepth;
  RuleList m_rules;

  bool reduce(std::vector< TokenOrNode > &stack) const;

  void storeBestError(
      const std::vector< TokenOrNode > &stack, const tErrorCB &cb) const;

  static void nullErrorCallback(const std::string &logged) {
    Log(LL::Error) << "Parse error: " << logged;
  }
};

} // namespace parser
} // namespace util
} // namespace core

#  include "lrparser.inl"

#endif
