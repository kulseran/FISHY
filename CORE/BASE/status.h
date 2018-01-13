/**
 * Checked container for passing return codes.
 */
#ifndef FISHY_STATUS_H
#define FISHY_STATUS_H

#include <CORE/BASE/checks.h>

#include <string>

namespace core {
namespace base {

/**
 * Checked error status return value.
 */
class Status {
  public:
  /**
   * Generic error codes.
   */
  enum eError {
    // The process was a success.
    OK = 0,
    // Generic error, specific to the relevant call.
    GENERIC_ERROR = 1,
    // Bad input parameter.
    BAD_ARGUMENT = 2,
    // Requested operation could not find a requested resource.
    NOT_FOUND = 3,
    // The request timed out.
    TIMEOUT = 4,
    // The request was canceled.
    CANCELED = 5,
    // The requested input is out of bounds.
    OUT_OF_BOUNDS = 6,
    // The requested resource contained invalid data.
    BAD_INPUT = 7,
    // The requested operation could not be completed, because of an internal
    // consistency error in the state of the object.
    BAD_STATE = 8,
    // The requested operation is unsupported, or unsupported for the given
    // input.
    UNSUPPORTED = 9
  };

  /**
   * Status destrutor ensures the object is read.
   */
  inline ~Status() { ASSERT(m_consumed); }

  /**
   * Construct a Status from a pass/fail boolean. A failure is reported as
   * {@code GENERIC_ERROR}
   */
  inline explicit Status(const bool genericOk) {
    m_status = genericOk ? OK : GENERIC_ERROR;
    IF_ASSERTS(m_consumed = false);
  }

  /**
   * Construct a Status with the specified failure mode.
   */
  inline Status(const eError status) : m_status(status) {
    IF_ASSERTS(m_consumed = false);
  }

  /**
   *
   */
  inline Status(const Status &other) : m_status(other.m_status) {
    IF_ASSERTS(m_consumed = other.m_consumed);
    other.consume();
  }

  /**
   *
   */
  inline Status &operator=(Status &other) {
    m_status = other.m_status;
    IF_ASSERTS(m_consumed = other.m_consumed);
    other.consume();
    return *this;
  }

  /**
   * Return a clone of this status, which itself will now
   * require a call to {@link #getStatus}.
   */
  inline Status clone() { return Status(m_status); }

  /**
   * Static {@code OK} status.
   */
  inline static Status ok() { return Status(OK); }

  /**
   * Static {@code GENERIC_ERROR} status.
   */
  inline static Status generic_error() { return Status(GENERIC_ERROR); }

  /**
   * Retrieve the status return code.
   */
  inline eError getStatus() {
    consume();
    return m_status;
  }

  /**
   * Retrieve the status return as a boolean pass/fail.
   */
  inline operator bool() const {
    consume();
    return m_status == OK;
  }

  /**
   * Consume the status object without checking it.
   */
  inline Status &ignoreErrors() {
    consume();
    return *this;
  }

  private:
  eError m_status;
  IF_ASSERTS(mutable bool m_consumed);

  inline void consume() const { IF_ASSERTS(m_consumed = true); }
};

/**
 * Return template, allowing a rValue reference to a returned object or a status
 * object explaining the error reason.
 */
template < typename tType >
class StatusOr {
  public:
  /**
   * Construct an {@code OK} status with the relevant object.
   */
  StatusOr(const tType &&value) {
    m_returnValue = std::move(value);
    m_status = Status::ok();
  }

  /**
   * Construct a failing status, with no valid object.
   */
  StatusOr(Status &status) : m_status(status) { ASSERT(!status); }

  /**
   * Retrieve the {@code Status}.
   */
  Status &getStatus() { return m_status; }

  /**
   * Retrieve the contained object.
   * This function will *kill* the program if the status was any filure.
   */
  const tType &getOrDie() const {
    ASSERT(m_status.wasConsumed());
    CHECK(m_status);
    return m_returnValue;
  }

  private:
  Status m_status;
  tType m_returnValue;
};

} // namespace base
} // namespace core
using core::base::Status;

/**
 * RET_S(expression, status); Will 'return status' from a function if the
 * expression does no evaluate to true
 */
#  define RET_S(expr, status) \
    do {                      \
      if (!(expr)) {          \
        return status;        \
      }                       \
    } while (0)

/**
 * RET_S(expression, status); Will 'return status' from a function if the
 * expression does no evaluate to true, and log the error message
 */
#  define RET_SM(expr, status, msg)                                    \
    do {                                                               \
      if (!(expr)) {                                                   \
        Log(LL::Error) << msg << " @ " << __FILE__ << ":" << __LINE__; \
        return status;                                                 \
      }                                                                \
    } while (0)

#endif
