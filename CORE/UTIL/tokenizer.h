/**
 * String tokenization
 */
#ifndef FISHY_TOKENIZER_H
#define FISHY_TOKENIZER_H

#include <CORE/UTIL/regex.h>

#include <iterator>
#include <vector>

namespace core {
namespace util {
namespace parser {

/**
 * String tokenization class.
 * Splits the input string into {@code Token}s based on a set of {@link
 * RegExPattern}s which define the types of tokens.
 *
 * @template tId should be a struct of the form
 *     struct myTokenIds {
 *       enum type {
 *         TOKEN_NAME_1,
 *         ...
 *       }
 *     }
 */
template < typename tId >
class Tokenizer {
  public:
  typedef std::pair< typename tId::type, RegExPattern > tTokenizer;
  typedef std::vector< typename tTokenizer > tTokenizerList;

  /**
   * Construct a Tokenizer with the given
   */
  Tokenizer(const tTokenizerList &tokenizers);

  /**
   * Container class that is used to indicate a matching token in the
   * input stream.
   */
  class Token {
    public:
    Token();
    Token(
        const typename tId::type id,
        const std::string::const_iterator tokenBegin,
        const std::string::const_iterator tokenEnd);

    /**
     * @return true if the token is not an empty/error token.
     */
    operator bool() const;

    /**
     * @return true if the token is not an empty/error token.
     */
    bool isValid() const;

    /**
     * @return the {@code tId} identifier name of this token.
     */
    typename tId::type getId() const;

    /**
     * @return lazily construct the substring matched by this token.
     */
    std::string getToken() const;

    /**
     * @return iterator to the start of the token match.
     */
    const std::string::const_iterator &begin() const;

    /**
     * @return iterator to the end of the token match.
     */
    const std::string::const_iterator &end() const;

    bool operator==(const Token &other) const;

    private:
    typename tId::type m_id;
    std::string::const_iterator m_tokenBegin;
    std::string::const_iterator m_tokenEnd;
    bool m_valid;
  };

  /**
   * Iterator wrapper for {@link #getNextToken}.
   */
  class const_iterator
      : public std::iterator< std::forward_iterator_tag, Token > {
    public:
    const Token &operator*() const;
    const Token *operator->() const;

    const_iterator &operator++();
    bool operator==(const const_iterator &other) const;
    bool operator!=(const const_iterator &other) const;

    private:
    friend class Tokenizer;

    const_iterator(const Tokenizer &tokenizer, const std::string &str);

    Token m_currentToken;
    const Tokenizer &m_tokenizer;
    const std::string &m_string;
  };

  /**
   * Scans forward in a string, attempting to match a token starting at {@code
   * begin}.
   *
   * @param token output container for the found token.
   * @param begin the start of the string to search
   * @param end the end of the string to search
   * @return true if a valid token is located
   */
  bool getNextToken(
      Token &token,
      const std::string::const_iterator &begin,
      const std::string::const_iterator &end) const;

  /**
   * @param str string to iterate
   * @return an iterator over the given string.
   */
  const_iterator begin(const std::string &str) const {
    return const_iterator(*this, str);
  }

  /**
   * @return generic end token iterator.
   */
  const_iterator end() const { return const_iterator(*this, ""); }

  private:
  std::vector< tTokenizer > m_tokenizers;
};

} // namespace parser
} // namespace util
} // namespace core

#  include "tokenizer.inl"

#endif
