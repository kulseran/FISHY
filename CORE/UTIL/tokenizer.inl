#ifndef FISHY_TOKENIZER_INL
#define FISHY_TOKENIZER_INL

#include <CORE/BASE/checks.h>
#include <CORE/UTIL/comparisonchain.h>

namespace core {
namespace util {
namespace parser {

/**
 *
 */
template < typename tId >
inline Tokenizer< tId >::Tokenizer(
    const typename Tokenizer< tId >::tTokenizerList &tokenizers)
    : m_tokenizers(tokenizers){};

/**
 *
 */
template < typename tId >
inline Tokenizer< tId >::Token::Token() : m_valid(false) {
}

/**
 *
 */
template < typename tId >
inline Tokenizer< tId >::Token::Token(
    const typename tId::type id,
    const std::string::const_iterator tokenBegin,
    const std::string::const_iterator tokenEnd)
    : m_id(id), m_tokenBegin(tokenBegin), m_tokenEnd(tokenEnd), m_valid(true) {
}

/**
 *
 */
template < typename tId >
inline Tokenizer< tId >::Token::operator bool() const {
  return m_valid;
}

/**
 *
 */
template < typename tId >
inline bool Tokenizer< tId >::Token::isValid() const {
  return m_valid;
}

/**
 *
 */
template < typename tId >
inline typename tId::type Tokenizer< tId >::Token::getId() const {
  CHECK(m_valid);
  return m_id;
}

/**
 *
 */
template < typename tId >
inline std::string Tokenizer< tId >::Token::getToken() const {
  CHECK(m_valid);
  return std::string(m_tokenBegin, m_tokenEnd);
}

/**
 *
 */
template < typename tId >
inline const std::string::const_iterator &
Tokenizer< tId >::Token::begin() const {
  return m_tokenBegin;
}

/**
 *
 */
template < typename tId >
inline const std::string::const_iterator &Tokenizer< tId >::Token::end() const {
  return m_tokenEnd;
}

/**
 *
 */
template < typename tId >
inline bool Tokenizer< tId >::Token::
operator==(const typename Tokenizer< tId >::Token &other) const {
  if (!m_valid && !other.m_valid) {
    return true;
  }
  return core::util::ComparisonChain()
      .andOne(m_valid, other.m_valid)
      .andOne(m_id, other.m_id)
      .andOne(m_tokenBegin, other.m_tokenBegin)
      .andOne(m_tokenEnd, other.m_tokenEnd)
      .buildEqual();
}

/**
 *
 */
template < typename tId >
inline bool Tokenizer< tId >::getNextToken(
    typename Tokenizer< tId >::Token &token,
    const std::string::const_iterator &begin,
    const std::string::const_iterator &end) const {
  for (std::vector< tTokenizer >::const_iterator itr = m_tokenizers.begin();
       itr != m_tokenizers.end();
       ++itr) {
    const std::string::const_iterator tokenEnd = itr->second.scan(begin, end);
    if (tokenEnd != begin) {
      token = Token(itr->first, begin, tokenEnd);
      return true;
    }
  }
  token = Token();
  return false;
}

/**
 *
 */
template < typename tId >
inline const typename Tokenizer< tId >::Token &
    Tokenizer< tId >::const_iterator::operator*() const {
  return m_currentToken;
}

/**
 *
 */
template < typename tId >
inline const typename Tokenizer< tId >::Token *
    Tokenizer< tId >::const_iterator::operator->() const {
  return &m_currentToken;
}

/**
 *
 */
template < typename tId >
inline typename Tokenizer< tId >::const_iterator &
Tokenizer< tId >::const_iterator::operator++() {
  if (m_currentToken) {
    m_tokenizer.getNextToken(
        m_currentToken, m_currentToken.end(), m_string.end());
  }
  return *this;
}

/**
 *
 */
template < typename tId >
inline bool Tokenizer< tId >::const_iterator::
operator==(const typename Tokenizer< tId >::const_iterator &other) const {
  return m_currentToken == other.m_currentToken;
}

/**
 *
 */
template < typename tId >
inline bool Tokenizer< tId >::const_iterator::
operator!=(const typename Tokenizer< tId >::const_iterator &other) const {
  return !(*this == other);
}

/**
 *
 */
template < typename tId >
inline Tokenizer< tId >::const_iterator::const_iterator(
    const Tokenizer< tId > &tokenizer, const std::string &str)
    : m_tokenizer(tokenizer), m_string(str) {
  m_tokenizer.getNextToken(m_currentToken, str.begin(), str.end());
}

} // namespace parser
} // namespace util
} // namespace core

#endif
