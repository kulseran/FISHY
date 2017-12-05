/**
 * Regular expression matcher.
 * Based on the work of Russ Cox: https://swtch.com/~rsc/regexp/regexp1.html
 *
 * Implements a Thompson NFA based RegEx matcher.
 * Ken Thompson, “Regular expression search algorithm,” Communications of the
 * ACM 11(6) (June 1968), pp. 419–422. http://doi.acm.org/10.1145/363347.363387
 * (PDF)
 */
#ifndef FISHY_REGEX_H
#define FISHY_REGEX_H

#include <CORE/types.h>

#include <string>

namespace core {
namespace util {
namespace parser {

/**
 * Regular Expression pattern matcher
 */
class RegExPattern {
  public:
  /**
   * Error codes from build process.
   */
  struct eBuildError {
    enum type {
      NONE,
      MISSING_ESCAPE,
      BAD_ESCAPE,
      MISSING_PEREN,
      MISSING_BRACE,
      PEREN_INSIDE_BRACE,
      RANGE_OUTSIDE_BRACE,
      MISSING_OPERAND
    };
  };

  RegExPattern();
  ~RegExPattern();
  RegExPattern(const RegExPattern &other);
  RegExPattern &operator=(const RegExPattern &other);

  /**
   * Constructs a regex in place. Will die on error.
   *
   * @param pattern the regex pattern to build
   */
  RegExPattern(const std::string &pattern);

  /**
   * Replaces the regex in this pattern.
   *
   * @param pattern the regex pattern to build
   */
  eBuildError::type build(const std::string &pattern);

  /**
   * Scans a string for a match against this pattern.
   * The pattern matching starts at {@code begin} and continues until the first
   * of
   * {@code end} or match rejection.
   *
   * @param begin start of string to match
   * @param end end of string to match
   * @return iterator at end of match, or begin if no match.
   */
  std::string::const_iterator scan(
      const std::string::const_iterator &begin,
      const std::string::const_iterator &end) const;

  /**
   * Matches a full string against this pattern.
   *
   * @param begin start of string to match
   * @param end end of string to match
   * @return true if string is fully matched
   */
  bool match(
      const std::string::const_iterator &begin,
      const std::string::const_iterator &end) const;

  private:
  class Nfa;
  Nfa *m_machine;
};

} // namespace parser
} // namespace util
} // namespace core

#endif
