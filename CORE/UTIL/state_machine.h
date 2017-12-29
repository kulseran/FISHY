/**
 * State machine logic.
 */
#ifndef FISHY_STATE_MACHINE_H
#define FISHY_STATE_MACHINE_H

#include <CORE/BASE/status.h>
#include <CORE/types.h>

#include <vector>

namespace core {
namespace util {

typedef long StateID;
const StateID STATE_INVALID = (StateID) 0;

class StateMachine;

/**
 * State Machine's State Interface
 */
class iState {
  public:
  iState();
  virtual ~iState();

  /**
   * Called when the state is first added to a machine
   */
  virtual void onInit(){};

  /**
   * Called when the state machine is shut down
   */
  virtual void onTerm(){};

  /**
   * Called once per update, if this state is active
   */
  virtual Status update() { return Status::OK; };

  /**
   * Called when this state is transitioned into
   */
  virtual void onEnter(){};

  /**
   * Called when this state is transitioned out of
   */
  virtual void onExit(){};

  /**
   * Get the unique ID for this state. Used to tell the machine to transition to
   * this state.
   */
  StateID getId() const;

  /**
   * Should return a user-friendly name for debug output.
   */
  virtual const char *getDebugName() const = 0;

  protected:
  friend class StateMachine;
  void setOwner(StateMachine *pOwner);

  StateID m_id;
  StateMachine *m_pOwner;
};

/**
 * State Machine Class
 */
class StateMachine {
  public:
  StateMachine();
  ~StateMachine();

  /**
   * Call once at startup
   */
  void init();

  /**
   * Call once per frame.
   *
   * @return the {@link Status} code from {@link iState#update}.
   */
  Status update();

  /**
   * Call to add a state to the machine
   *
   * @param pState the state to add to the machine. The mahine does not take
   *     ownership of this pointer, and it must live longer than the machine.
   */
  Status addState(iState *pState);

  /**
   * Call to clean up all the states in the machine
   */
  void term();

  /**
   * Call with the current state ID, and the next state id inorder to invoke a
   * transition
   */
  void requestTransition(StateID curid, StateID id_next);

  /**
   * Call to get the current state ID
   */
  StateID getCurrentState() const;

  private:
  struct StateInfo {
    StateID id;
    iState *pState;
  };

  void changeStates();

  typedef std::vector< StateInfo > tStateVector;

  tStateVector m_states;
  StateID m_curStateID;
  StateID m_nextStateID;
};

/**
 * Create a new, unique StateID for use with a state or state machine.
 */
StateID GetUniquestateId();

} // namespace util
} // namespace core

#endif
