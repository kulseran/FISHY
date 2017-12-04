#include "regex.h"

#include <CORE/BASE/checks.h>

#include <algorithm>
#include <stack>
#include <vector>

/**
 * Fast 'set' operations for regex states.
 */
class vectorset {
  public:
  vectorset() : m_pSet(m_set), m_sz(0) {}

  typedef const u8 *const_iterator;
  const_iterator begin() const { return m_pSet; }
  const_iterator end() const { return m_pSet + m_sz; }

  void clear() { m_sz = 0; }
  bool empty() const { return m_sz == 0; }
  void insert(const u32 &elem) {
    const_iterator lb = std::lower_bound(begin(), end(), elem);
    if (lb == end() || *lb != elem) {
      const size_t findpos = std::distance(begin(), lb);
      const size_t afterlen = m_sz - findpos;
      u8 *const pStart = m_pSet + findpos;
      memmove(pStart + 1, lb, afterlen);
      *pStart = elem;
      m_sz++;
    }
  }

  static void swap(vectorset &a, vectorset &b) {
    std::swap(a.m_pSet, b.m_pSet);
    std::swap(a.m_sz, b.m_sz);
  }

  private:
  u8 m_set[256];
  u8 *m_pSet;
  u8 m_sz;
};

namespace core {
namespace util {
namespace parser {

/**
 * Token container, to translate input characters to a type-safe action.
 */
class ReToken {
  public:
  enum eType {
    LITERAL,
    INV_LITERAL,
    RANGE,
    INV_RANGE,
    OPERATOR,
  };

  ReToken(const eType type, const char ch) : m_type(type), m_ch(ch) {}

  eType getType() const { return m_type; }
  char getCh() const { return m_ch; }

  private:
  eType m_type;
  char m_ch;
};
typedef std::vector< ReToken > tTokenList;

/**
 * NFA
 */
class RegExPattern::Nfa {
  public:
  class State {
    public:
    enum eType { MATCH, LITERAL, RANGE, INV_LITERAL, INV_RANGE, SPLIT };
    explicit State(eType type, u32 out = -1, u32 out1 = -1)
        : m_type(type), m_literal('?'), m_out(out), m_out1(out1) {}
    explicit State(eType type, char ch)
        : m_type(type), m_literal(ch), m_out(-1), m_out1(-1) {}
    u32 m_out;
    u32 m_out1;
    eType m_type;
    char m_literal;
  };
  Nfa() {}
  void build(const tTokenList &pattern);
  std::string::const_iterator scan(
      const std::string::const_iterator &begin,
      const std::string::const_iterator &end) const;

  private:
  std::vector< State > m_states;
  u32 m_start;
  u32 m_accept;

  class Fragment {
    public:
    Fragment(u32 start) : m_start(start) {}
    u32 m_start;
    std::vector< u32 > m_terminals;

    void addTerminal(u32 s) { m_terminals.push_back(s); }
    void addAllTerminal(const std::vector< u32 > &other) {
      for (std::vector< u32 >::const_iterator itr = other.begin();
           itr != other.end();
           ++itr) {
        m_terminals.push_back(*itr);
      }
    }
  };
  void connect(Fragment &f, const u32 next);
  u32 newState(const State &s);
  void addSplit(const u32 state, vectorset &nextStates) const;
  bool isMatch(const vectorset &states) const;
};

/**
 * Prototypes
 */
static RegExPattern::eBuildError::type expandPattern(tTokenList &tokens);
static RegExPattern::eBuildError::type
escapePattern(tTokenList &out, const std::string &pattern);
static void print(const tTokenList &pattern);
static RegExPattern::eBuildError::type infixToPostfix(tTokenList &pattern);

/**
 *
 */
RegExPattern::RegExPattern() : m_machine(nullptr) {
}

/**
 *
 */
RegExPattern::RegExPattern(const std::string &pattern) : m_machine(nullptr) {
  CHECK_M(
      build(pattern) == eBuildError::NONE, "Unable to statically build RegEx.");
}

/**
 *
 */
RegExPattern::~RegExPattern() {
  delete m_machine;
}

/**
 *
 */
RegExPattern::RegExPattern(const RegExPattern &other) : m_machine(nullptr) {
  if (other.m_machine) {
    m_machine = new Nfa();
    *m_machine = *other.m_machine;
  }
}

/**
 *
 */
RegExPattern &RegExPattern::operator=(const RegExPattern &other) {
  if (this->m_machine == other.m_machine) {
    return *this;
  }

  delete m_machine;
  if (other.m_machine) {
    m_machine = new Nfa();
    *m_machine = *other.m_machine;
  } else {
    m_machine = nullptr;
  }
  return *this;
}

/**
 *
 */
RegExPattern::eBuildError::type
RegExPattern::build(const std::string &pattern) {
  delete m_machine;
  m_machine = new Nfa();

  tTokenList tokens;
  eBuildError::type ret;
  ret = escapePattern(tokens, pattern);
  if (ret != RegExPattern::eBuildError::NONE) {
    return ret;
  }
  ret = expandPattern(tokens);
  if (ret != RegExPattern::eBuildError::NONE) {
    return ret;
  }
  ret = infixToPostfix(tokens);
  if (ret != RegExPattern::eBuildError::NONE) {
    return ret;
  }
  m_machine->build(tokens);
  return eBuildError::NONE;
}

/**
 *
 */
std::string::const_iterator RegExPattern::scan(
    const std::string::const_iterator &begin,
    const std::string::const_iterator &end) const {
  if (!m_machine) {
    return begin;
  }
  return m_machine->scan(begin, end);
}

/**
 * Runs the NFA state machine.
 * The state buffers may grow to at most contain every state.
 * Worst case complexity of a match is O(n*m) in the number of states from the
 * RegEx and length of the input.
 */
std::string::const_iterator RegExPattern::Nfa::scan(
    const std::string::const_iterator &begin,
    const std::string::const_iterator &end) const {

  // These two buffers hold the machine state.
  vectorset currentStates;
  vectorset nextStates;

  // Adds initial state transitions.
  addSplit(m_start, currentStates);

  // We return partial matches, so these two variables keep track of the end of
  // the best match.
  std::string::const_iterator retVal = begin;
  bool wasMatched = false;

  // Scan the buffer
  for (std::string::const_iterator itr = begin;
       itr != end && !currentStates.empty();
       ++itr) {
    for (vectorset::const_iterator stateId = currentStates.begin();
         stateId != currentStates.end();
         ++stateId) {
      const State &state = m_states[*stateId];
      bool match = false;
      switch (state.m_type) {
        case State::LITERAL:
          match = state.m_literal == *itr;
          break;
        case State::INV_LITERAL:
          match = state.m_literal != *itr;
          break;
        case State::RANGE:
          switch (state.m_literal) {
            case '.':
              match = true;
              break;
            case 's':
              match =
                  *itr == ' ' || *itr == '\n' || *itr == '\t' || *itr == '\r';
              break;
            case 'd':
              match = *itr >= '0' && *itr <= '9';
              break;
            case 'w':
              match = (*itr >= 'a' && *itr <= 'z')
                      || (*itr >= 'A' && *itr <= 'Z') || (*itr == '_')
                      || (*itr >= '0' && *itr <= '9');
              break;
            case 'a':
              match =
                  (*itr >= 'a' && *itr <= 'z') || (*itr >= 'A' && *itr <= 'Z');
              break;

            default:
              CHECK_M(false, "unknown range escape");
              break;
          }
      }

      // If this state matches, add all the transitions to the next pass.
      if (match) {
        addSplit(state.m_out, nextStates);
      }
    }
    // std::swap(currentStates, nextStates);
    vectorset::swap(currentStates, nextStates);
    nextStates.clear();

    // If we're at a terminal state of the match, record the best matching
    // iterator location.
    if (isMatch(currentStates)) {
      retVal = itr;
      wasMatched = true;
    } else if (wasMatched) {
      break;
    }
  }

  if (wasMatched) {
    return ++retVal;
  }
  return begin;
}

/**
 * Adds the NFA's next possible states to the state set.
 */
void RegExPattern::Nfa::addSplit(
    const u32 stateId, vectorset &nextStates) const {
  if (stateId == -1) {
    return;
  }

  const State &state = m_states[stateId];
  if (state.m_type == State::SPLIT) {
    addSplit(state.m_out, nextStates);
    addSplit(state.m_out1, nextStates);
  } else {
    nextStates.insert(stateId);
  }
}

/**
 * Checks if any of the states in the state set are the terminal 'match' state.
 */
bool RegExPattern::Nfa::isMatch(const vectorset &states) const {
  for (vectorset::const_iterator stateId = states.begin();
       stateId != states.end();
       ++stateId) {
    const State &state = m_states[*stateId];
    if (state.m_type == State::MATCH) {
      return true;
    }
  }
  return false;
}

/**
 *
 */
bool RegExPattern::match(
    const std::string::const_iterator &begin,
    const std::string::const_iterator &end) const {
  return m_machine && scan(begin, end) == end;
}

/**
 * Scans through the string representation of the RegEx Pattern.
 * It converts both the \ escape characters and the ^ inversion characters.
 * Attempts to then convert the resulting symbols into one of the 5:
 *   Operator
 *   (inverted) Literal
 *   (inverted) Range
 * Checks for mis-matched perens or braces so that further processing steps
 * can avoid those checks.
 *
 * @param out the output tokens
 * @param pattern the input pattern string
 * @return error code on bad regex pattern
 */
RegExPattern::eBuildError::type
escapePattern(tTokenList &out, const std::string &pattern) {
  out.reserve(pattern.size());
  int perens = 0;
  int braces = 0;
  bool isInverted = false;
  for (std::string::const_iterator itr = pattern.begin(); itr != pattern.end();
       ++itr) {
    bool isLiteral = false;
    char ch = *itr;
    if (ch == '\\') {
      isLiteral = true;
      itr++;
      if (itr == pattern.end()) {
        return RegExPattern::eBuildError::MISSING_ESCAPE;
      }
      switch (*itr) {
        case 'n':
          ch = '\n';
          break;
        case 't':
          ch = '\t';
          break;
        case 'r':
          ch = '\r';
          break;
        case '(':
        case ')':
        case '\\':
        case '[':
        case ']':
        case '.':
        case '+':
        case '*':
        case '?':
        case '|':
        case '^':
          ch = *itr;
          break;
        case 's': // white space
        case 'd': // digit
        case 'w': // word
        case 'a': // alpha
          if (isInverted) {
            out.push_back(ReToken(ReToken::INV_RANGE, *itr));
          } else {
            out.push_back(ReToken(ReToken::RANGE, *itr));
          }
          isInverted = false;
          continue;
        default:
          return RegExPattern::eBuildError::BAD_ESCAPE;
      }
    } else {
      switch (*itr) {
        case '(':
          if (braces) {
            return RegExPattern::eBuildError::PEREN_INSIDE_BRACE;
          }
          perens++;
          break;
        case '[':
          if (braces) {
            return RegExPattern::eBuildError::MISSING_BRACE;
          }
          braces++;
          break;
        case ')':
          if (!perens) {
            return RegExPattern::eBuildError::MISSING_PEREN;
          }
          perens--;
          break;
        case ']':
          if (!braces) {
            return RegExPattern::eBuildError::MISSING_BRACE;
          }
          braces--;
          break;
        case '^':
          if (!braces) {
            return RegExPattern::eBuildError::RANGE_OUTSIDE_BRACE;
          }
          isInverted = true;
          continue;
        case '+':
        case '|':
        case '*':
        case '?':
          break;
        case '.':
          out.push_back(ReToken(ReToken::RANGE, *itr));
          break;
        default:
          isLiteral = true;
          break;
      }
    }
    if (isLiteral) {
      if (isInverted) {
        out.push_back(ReToken(ReToken::INV_LITERAL, *itr));
      } else {
        out.push_back(ReToken(ReToken::LITERAL, *itr));
      }
      isInverted = false;
    } else {
      if (isInverted) {
        return RegExPattern::eBuildError::RANGE_OUTSIDE_BRACE;
      }
      out.push_back(ReToken(ReToken::OPERATOR, *itr));
    }
  }
  if (perens != 0) {
    return RegExPattern::eBuildError::MISSING_PEREN;
  }
  if (braces != 0) {
    return RegExPattern::eBuildError::MISSING_BRACE;
  }
  return RegExPattern::eBuildError::NONE;
}

/**
 * Expands braces within the pattern.
 * Converts a [asdf] range into a (a|s|d|f) group.
 */
RegExPattern::eBuildError::type expandPattern(tTokenList &tokens) {
  const ReToken OP_OR(ReToken::OPERATOR, '|');
  tTokenList retVal;

  for (tTokenList::const_iterator itr = tokens.begin(); itr != tokens.end();
       ++itr) {
    if (itr->getType() == ReToken::OPERATOR && itr->getCh() == '[') {
      retVal.push_back(ReToken(ReToken::OPERATOR, '('));
      itr++;
      if (!(itr->getType() == ReToken::OPERATOR && itr->getCh() == ']')) {
        retVal.push_back(*itr);
        itr++;
        while (!(itr->getType() == ReToken::OPERATOR && itr->getCh() == ']')) {
          retVal.push_back(OP_OR);
          retVal.push_back(*itr);
          itr++;
        }
      }
      retVal.push_back(ReToken(ReToken::OPERATOR, ')'));
    } else {
      retVal.push_back(*itr);
    }
  }

  tokens = retVal;
  return RegExPattern::eBuildError::NONE;
}

/**
 * Converts the infix notation pattern sequence into a Postfix notation.
 */
RegExPattern::eBuildError::type infixToPostfix(tTokenList &pattern) {
  const ReToken OP_CONCAT(ReToken::OPERATOR, '&');
  const ReToken OP_OR(ReToken::OPERATOR, '|');
  tTokenList ret;
  ret.reserve(pattern.size());
  std::stack< int > alts;
  std::stack< int > lits;
  alts.push(0);
  lits.push(0);
  for (tTokenList::const_iterator itr = pattern.begin(); itr != pattern.end();
       ++itr) {
    if (itr->getType() == ReToken::OPERATOR) {
      switch (itr->getCh()) {
        case '(':
          if (lits.top() > 1) {
            lits.top()--;
            ret.push_back(OP_CONCAT);
          }
          alts.push(0);
          lits.push(0);
          continue;
        case ')':
          while (--lits.top()) {
            ret.push_back(OP_CONCAT);
          }
          for (; alts.top(); alts.top()--) {
            ret.push_back(OP_OR);
          }
          alts.pop();
          lits.pop();
          lits.top()++;
          continue;
        case '|':
          if (lits.top() == 0) {
            return RegExPattern::eBuildError::MISSING_OPERAND;
          }
          while (--lits.top()) {
            ret.push_back(OP_CONCAT);
          }
          alts.top()++;
          continue;
        case '+':
        case '?':
        case '*':
          if (!lits.top()) {
            return RegExPattern::eBuildError::MISSING_OPERAND;
          }
          ret.push_back(*itr);
          continue;
      }
    }

    if (itr->getType() != ReToken::OPERATOR) {
      if (lits.top() > 1) {
        lits.top()--;
        ret.push_back(OP_CONCAT);
      }
      ret.push_back(*itr);
      lits.top()++;
    }
  }

  while (--lits.top()) {
    ret.push_back(OP_CONCAT);
  }
  for (; alts.top(); alts.top()--) {
    ret.push_back(OP_OR);
  }
  pattern = ret;
  return RegExPattern::eBuildError::NONE;
}

/**
 * Buids an NFA from the pattern tokens
 */
void RegExPattern::Nfa::build(const tTokenList &pattern) {
  m_states.clear();
  m_states.reserve(pattern.size() + 2);
  std::stack< Fragment > fragments;
  for (tTokenList::const_iterator itr = pattern.begin(); itr != pattern.end();
       ++itr) {
    if (itr->getType() == ReToken::OPERATOR) {
      switch (itr->getCh()) {
        case '+': {
          Fragment frag = fragments.top();
          fragments.pop();
          u32 next = newState(State(State::SPLIT, frag.m_start));
          connect(frag, next);
          fragments.push(frag);
          break;
        }
        case '*': {
          Fragment &frag = fragments.top();
          u32 next = newState(State(State::SPLIT, frag.m_start));
          connect(frag, next);
          frag.m_start = next;
          break;
        }
        case '?': {
          Fragment &frag = fragments.top();
          u32 next = newState(State(State::SPLIT, frag.m_start));
          frag.m_start = next;
          frag.addTerminal(next);
          break;
        }
        case '|': {
          Fragment fragNext = fragments.top();
          fragments.pop();
          Fragment &frag = fragments.top();
          u32 newStart =
              newState(State(State::SPLIT, frag.m_start, fragNext.m_start));
          frag.addAllTerminal(fragNext.m_terminals);
          frag.m_start = newStart;
          break;
        }
        case '&': {
          Fragment fragNext = fragments.top();
          fragments.pop();
          Fragment &frag = fragments.top();
          connect(frag, fragNext.m_start);
          frag.m_terminals = fragNext.m_terminals;
          break;
        }
      }
    } else {
      State::eType type = State::LITERAL;
      switch (itr->getType()) {
        case ReToken::LITERAL:
          type = State::LITERAL;
          break;
        case ReToken::INV_LITERAL:
          type = State::INV_LITERAL;
          break;
        case ReToken::RANGE:
          type = State::RANGE;
          break;
        case ReToken::INV_RANGE:
          type = State::INV_RANGE;
          break;
      }
      u32 s = newState(State(type, itr->getCh()));
      Fragment frag(s);
      frag.addTerminal(s);
      fragments.push(frag);
    }
  }
  m_accept = newState(State(State::MATCH));
  connect(fragments.top(), m_accept);
  m_start = fragments.top().m_start;
  CHECK_M(m_states.size() < 255, "Regex too complicated.");
}

/**
 *
 */
void RegExPattern::Nfa::connect(
    RegExPattern::Nfa::Fragment &f, const u32 next) {
  for (std::vector< u32 >::const_iterator itr = f.m_terminals.begin();
       itr != f.m_terminals.end();
       ++itr) {
    State *s = &m_states[*itr];
    if (s->m_out == -1) {
      s->m_out = next;
    } else {
      s->m_out1 = next;
    }
  }
  f.m_terminals.clear();
  f.addTerminal(next);
}

/**
 * Reserves a new state.
 * States are indexed by their allocation order to allow this object to be
 * copyable.
 */
u32 RegExPattern::Nfa::newState(const RegExPattern::Nfa::State &s) {
  m_states.push_back(s);
  return static_cast< u32 >(m_states.size()) - 1;
}

} // namespace parser
} // namespace util
} // namespace core
