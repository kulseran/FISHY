/**
 * stringutil.cpp
 */
#include "stringutil.h"

#include <CORE/BASE/checks.h>

#include <algorithm>

namespace core {
namespace util {

/**
 *
 */
std::vector< std::string >
Splitter::split(const std::string &item, const unsigned maxLen) {
  std::vector< std::string > rVal;
  std::string::size_type offset = 0;
  std::string::size_type itr;

  do {
    if (rVal.size() + 1 < maxLen) {
      itr = item.find_first_of(m_ch, offset);
    } else {
      itr = item.npos;
    }

    if (m_trim) {
      rVal.push_back(
          core::util::trimWhitespace(item.substr(offset, itr - offset)));
    } else {
      rVal.push_back(item.substr(offset, itr - offset));
    }
    offset = itr + 1;
  } while (itr != item.npos);

  return rVal;
}

/**
 *
 */
std::string trimWhitespace(const std::string &v) {
  if (v.empty()) {
    return v;
  }

  std::string::size_type beg = 0;
  std::string::size_type end = v.length();
  while ((v.at(beg) == ' ' || v.at(beg) == '\t') && beg < end) {
    beg++;
  }
  while (v.at(end - 1) == ' ' && end >= beg) {
    end--;
  }
  if (end > beg) {
    return v.substr(beg, end - beg);
  }
  return "";
}

/**
 *
 */
std::string trimQuotes(const std::string &v) {
  if (v.empty()) {
    return v;
  }

  std::string::size_type beg = 0;
  std::string::size_type end = v.length();
  if (v.at(0) == '\"') {
    beg++;
  }
  if (v.at(end - 1) == '\"') {
    end--;
  }
  if (end > beg) {
    return v.substr(beg, end - beg);
  }
  return "";
}

/**
 *
 */
std::string unescape(const std::string &str) {
  std::string rVal;
  rVal.reserve(str.size());
  std::string::const_iterator itr = str.begin();
  while (itr != str.end()) {
    if (*itr == '\\') {
      ++itr;
      if (itr == str.end()) {
        break;
      }
      switch (*itr) {
        case 'n':
          rVal.push_back('\n');
          break;
        case 'r':
          rVal.push_back('\r');
          break;
        case 't':
          rVal.push_back('\t');
          break;
        case '"':
          rVal.push_back('\"');
          break;
        case '\\':
          rVal.push_back('\\');
          break;
        default:
          if (isdigit(*itr) && isdigit(*(itr + 1)) && isdigit(*(itr + 2))) {
            const int d1 = *(itr++) - '0';
            if (itr == str.end()) {
              break;
            }
            const int d2 = *(itr++) - '0';
            if (itr == str.end()) {
              break;
            }
            const int d3 = *itr - '0';
            const char ch = d1 * 100 + d2 * 10 + d3;
            rVal.push_back(ch);
          } else {
            rVal.push_back('\\');
            rVal.push_back(*itr);
          }
          break;
      }
    } else {
      rVal.push_back(*itr);
    }
    ++itr;
  }
  return rVal;
}

/**
 *
 */
std::string escape(const std::string &str) {
  std::string rVal;
  rVal.reserve(str.size());
  std::string::const_iterator itr = str.begin();
  while (itr != str.end()) {
    switch (*itr) {
      case '\n':
        rVal.push_back('\\');
        rVal.push_back('n');
        break;
      case '\r':
        rVal.push_back('\\');
        rVal.push_back('r');
        break;
      case '\t':
        rVal.push_back('\\');
        rVal.push_back('t');
        break;
      case '\"':
        rVal.push_back('\\');
        rVal.push_back('\"');
        break;
      case '\\':
        rVal.push_back('\\');
        rVal.push_back('\\');
        break;
      default:
        if (isprint(*itr)) {
          rVal.push_back(*itr);
        } else {
          rVal.push_back('\\');
          char buf[4] = {};
          const unsigned int v = static_cast< unsigned char >(*itr);
          sprintf(buf, "%03d", v);
          rVal.append(buf);
        }
        break;
    }
    ++itr;
  }
  return rVal;
}

/**
 *
 */
std::string replaceStr(
    const std::string &input, const std::string &match,
    const std::string &replacement) {
  std::string rVal;

  // reserve the correct space
  {
    int count = 0;
    std::string::size_type itr = 0;
    while ((itr = input.find(match, itr)) != input.npos) {
      ++itr;
      ++count;
    }
    int delta = replacement.size() - match.size();
    rVal.reserve(input.size() + delta * count + 1);
  }

  // Replace all instances
  std::string::size_type begin = 0;
  while (begin < input.length()) {
    std::string::size_type end = input.find(match, begin);
    if (end == input.npos) {
      rVal.append(input.substr(begin, end - begin));
      begin = end;
    } else {
      rVal.append(input.substr(begin, end - begin));
      rVal.append(replacement);
      begin = end + match.length();
    }
  }

  return rVal;
}

/**
 *
 */
std::string identifierSafe(const std::string &input) {
  if (input.length() == 0) {
    return input;
  }

  std::string rVal;
  rVal.reserve(input.size());
  std::string::const_iterator itr = input.begin();
  if (!(*itr >= 'a' && *itr <= 'z') && !(*itr >= 'A' && *itr <= 'Z')) {
    rVal.push_back('_');
    ++itr;
  }

  for (; itr != input.end(); ++itr) {
    if (!(*itr >= 'a' && *itr <= 'z') && !(*itr >= 'A' && *itr <= 'Z')
        && !(*itr >= '0' && *itr <= '9')) {
      rVal.push_back('_');
    } else {
      rVal.push_back(*itr);
    }
  }
  return rVal;
}

static const char *g_sizePostfix[] = {" B",   " KiB", " MiB", " GiB", " TiB",
                                      " PiB", " EiB", " ZiB", " YiB", " ERROR"};

/**
 *
 */
std::string prettySize(const u64 v) {
  u64 nv = v;
  unsigned idx = 0;
  while (nv >= 1024ull && ((idx + 1) < ARRAY_LENGTH(g_sizePostfix))) {
    nv /= 1024;
    idx++;
  }
  std::stringstream formatter;
  formatter << nv;
  formatter << g_sizePostfix[idx];
  return formatter.str();
}

/**
 *
 */
u32 countLines(
    const std::string::const_iterator &begin,
    const std::string::const_iterator &end) {
  return std::count(begin, end, '\n');
}

} // namespace util
} // namespace core
