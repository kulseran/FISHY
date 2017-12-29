#include "state_machine.h"

#include <CORE/ARCH/intrinsics.h>
#include <CORE/BASE/logging.h>

#include <algorithm>
#include <atomic>

namespace core {
namespace util {

bool StateIdsEquals(const iState *&pState, const long id) {
  return pState->getId() == id;
}

/**
 *
 */
iState::iState() : m_id(GetUniquestateId()), m_pOwner(nullptr) {
}

/**
 *
 */
iState::~iState() {
}

/**
 *
 */
void iState::setOwner(StateMachine *pOwner) {
  ASSERT(!m_pOwner);
  m_pOwner = pOwner;
}

/**
 *
 */
StateID iState::getId() const {
  return m_id;
}

/**
 *
 */
StateID StateMachine::getCurrentState() const {
  return m_curStateID;
}

/**
 *
 */
StateMachine::StateMachine()
    : m_curStateID(STATE_INVALID), m_nextStateID(STATE_INVALID) {
}

/**
 *
 */
StateMachine::~StateMachine() {
  ASSERT(m_states.empty());
}

/**
 *
 */
Status StateMachine::addState(iState *pState) {
  ASSERT(pState);

  const int id = pState->getId();

  // Do we already have this state?
  if (std::find(m_states.begin(), m_states.end(), id) != m_states.end()) {
    return Status::BAD_ARGUMENT;
  }

  // Add the state
  const StateInfo info = {id, pState};

  m_states.push_back(info);
  pState->setOwner(this);

  // Set a state change if this is the first state. Just so we don't errror out
  // in update
  if (m_curStateID == STATE_INVALID && m_nextStateID == STATE_INVALID) {
    m_nextStateID = id;
  }

  return Status::OK;
}

/**
 *
 */
Status StateMachine::update() {
  changeStates();

  tStateVector::iterator itr =
      std::find(m_states.begin(), m_states.end(), m_curStateID);
  if (itr == m_states.end()) {
    return Status::BAD_STATE;
  }

  return itr->pState->update();
}
/**
 *
 */
void StateMachine::init() {
  Trace();
  std::for_each(
      m_states.begin(),
      m_states.end(),
      std::bind(&iState::onInit, std::placeholders::_1));
}

/**
 *
 */
void StateMachine::term() {
  Trace();

  tStateVector::iterator itr =
      std::find(m_states.begin(), m_states.end(), m_curStateID);
  if (itr != m_states.end()) {
    itr->pState->onExit();
  }

  std::for_each(
      m_states.begin(),
      m_states.end(),
      std::bind(&iState::onTerm, std::placeholders::_1));

  m_states.clear();
}

/**
 *
 */
void StateMachine::requestTransition(StateID id, StateID id_next) {
  ASSERT(id == m_curStateID || id == STATE_INVALID);
  m_nextStateID = id_next;
}

/**
 *
 */
void StateMachine::changeStates() {
  if (m_curStateID != m_nextStateID) {
    iState *pNext = nullptr;

    for (tStateVector::iterator i = m_states.begin(); i != m_states.end();
         ++i) {
      if (i->id == m_curStateID) {
        i->pState->onExit();
      } else if (i->id == m_nextStateID) {
        pNext = i->pState;
      }
    }

    ASSERT(pNext);

    if (pNext) {
      pNext->onEnter();
      m_curStateID = m_nextStateID;
    } else {
      m_curStateID = STATE_INVALID;
    }
  }
}

/**
 *
 */
StateID GetUniquestateId() {
  static std::atomic< StateID > s_id = STATE_INVALID + 1;
  return s_id++;
}

} // namespace util
} // namespace core
