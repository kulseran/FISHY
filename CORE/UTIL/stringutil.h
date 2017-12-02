/**
 * String manipulation utilities.
 *   Splitter - for string splitting
 *   Joiner - for string joining
 *   Various sanitizer functions.
 */
#ifndef FISHY_STRINGUTIL_H
#define FISHY_STRINGUTIL_H

#include <CORE/UTIL/lexical_cast.h>
#include <CORE/types.h>

#include <limits>
#include <string>
#include <vector>

namespace core {
namespace util {

/**
 * Preforms string splitting.
 *   Splitter().on(',').trimWhitespace().split("some, string, with, commas")
 * returns
 *   { "some", "string", "with", "commas" }
 */
class Splitter {
  public:
  Splitter() : m_ch(' '), m_trim(false) {}

  /**
   * Set the split delimiter, defaults to ' '.
   */
  Splitter &on(const char ch) {
    m_ch = ch;
    return *this;
  }

  /**
   * Set whitespace trimming, defaults to false.
   */
  Splitter &trimWhitespace() {
    m_trim = true;
    return *this;
  }

  /**
   * @return a list of split elements
   */
  std::vector< std::string > split(const std::string &in) {
    return split(in, std::numeric_limits< unsigned >::max());
  }

  /**
   * @return a list of split elements of maxLen length
   */
  std::vector< std::string >
  split(const std::string &in, const unsigned maxLen);

  private:
  char m_ch;
  bool m_trim;
};

/**
 * Performs string joining.
 *   std::vector<std::string> strs;
 *   strs.push_back("cat"); strs.push_back("in"); strs.push_back("hat");
 *   Joiner().on(":").join(strs.begin(), strs.end());
 * returns
 *   "cat:in:hat"
 */
class Joiner {
  public:
  Joiner() : m_sep(", ") {}

  /**
   * Set the join characters, defaults to ', '.
   */
  Joiner &on(const std::string sep) {
    m_sep = sep;
    return *this;
  }

  /**
   * @return a string consisting of the join of all elements by the supplied sep
   */
  template < typename tIterator >
  std::string join(const tIterator &begin, const tIterator &end) {
    if (begin == end) {
      return "";
    }

    std::string rVal;
    tIterator itr = begin;
    core::util::lexical_cast(*itr, rVal);
    ++itr;
    while (itr != end) {
      std::string str;
      core::util::lexical_cast(*itr, str);

      rVal.append(m_sep);
      rVal.append(str);
      ++itr;
    }
    return rVal;
  }

  private:
  std::string m_sep;
};

/**
 * Trims the front and back of a string for whitespace.
 */
std::string TrimWhitespace(const std::string &);

/**
 * Trims quotes off a string.
 */
std::string TrimQuotes(const std::string &);

/**
 * Convert standard \? escape sequences to their appropriate symbol.
 */
std::string Unescape(const std::string &);

/**
 * Convert standard control characters to their appropriate \? symbols.
 */
std::string Escape(const std::string &);

/**
 * Replaces all occurances of a substring
 */
std::string ReplaceStr(
    const std::string &input,
    const std::string &match,
    const std::string &replacement);

/**
 * Replaces non-word characters with '_' to make input viable as an identifier.
 */
std::string IdentifierSafe(const std::string &input);

/**
 * Prints a number as a human readable size
 */
std::string PrettySize(const u64 input);

/**
 * Counts the number of newlines in the string segment.
 */
size_t CountLines(
    const std::string::const_iterator &begin,
    const std::string::const_iterator &end);

} // namespace util
} // namespace core

#endif
