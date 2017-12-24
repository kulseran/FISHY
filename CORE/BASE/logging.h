/**
 * Core logging functionality
 *
 * Logging is provided via a macro {@code Log(level)} which can log at 4 levels
 * of detail. The macro links in a {@link core::logging::TraceInfo} which
 * contains detailed information about the call site.
 */
#ifndef FISHY_LOGGING_H
#define FISHY_LOGGING_H

#include <CORE/BASE/status.h>
#include <CORE/TYPES/bitset.h>
#include <CORE/TYPES/concurrent_queue.h>
#include <CORE/UTIL/noncopyable.h>

#include <atomic>
#include <chrono>
#include <memory>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

namespace core {
namespace logging {

/**
 * Maximum length of a single log line.
 * If this limit is reached, the log will be written out as 1019 characters of
 * requested log, followed by ...\0
 */
static const unsigned MAX_LOG_LEN = 1024;

/**
 * Logging level
 */
struct LL {
  enum type {
    Trace,   // Detailed logging
    Info,    // Info logging
    Warning, // Warnings
    Error,   // Errors
    COUNT
  };
};

/**
 * Information about the call site of the logger.
 */
class TraceInfo {
  public:
  TraceInfo();
  TraceInfo(const char *file, const char *function, long line);

  const char *m_file;
  const char *m_function;
  long m_line;
  std::thread::id m_threadId;
  uintptr_t m_stackPtr;
  std::chrono::system_clock::time_point m_timestamp;
};

/**
 * A log message containing the level, trace and message.
 */
struct LogMessage {
  LogMessage();
  LogMessage(const LL::type, const TraceInfo &);

  LL::type m_logLevel;
  TraceInfo m_trace;
  size_t m_msgLen;
  char m_msg[MAX_LOG_LEN];
};

/**
 * Helper class to build a log message and submit it to the {@link LogManager}.
 * Uses stream style {@code operator <<} to write out a message.
 */
class LogMessageBuilder : core::util::noncopyable {
  public:
  LogMessageBuilder(const LL::type, const TraceInfo &);
  ~LogMessageBuilder();

  template < typename T >
  LogMessageBuilder &operator<<(const T &obj);
  template < typename T, unsigned N >
  LogMessageBuilder &operator<<(const T (&obj)[N]);

  private:
  LogMessage m_message;
};

/**
 * A class that triggers an "Entering"/"Exiting" message for a given scope.
 */
class ScopedLogger : core::util::noncopyable {
  public:
  ScopedLogger(const TraceInfo &trace);
  ~ScopedLogger();

  private:
  TraceInfo m_trace;
};

/**
 * Interface for sinking log messages to storage.
 * This implementation does not have to be thread safe, as the {@link
 * LogManager} is guarenteed to log only from one thread.
 */
class iLogSink : core::util::noncopyable {
  public:
  /**
   * @param levels The log levels to allow writing of.
   */
  iLogSink(const core::types::BitSet< LL > &levels);

  /**
   *
   */
  virtual ~iLogSink();

  /**
   * Commit a single {@link LogMessage} to storage, if the message matches the
   * configured logger levels for this instance of the sink.
   *
   * @return Status of {@link write} implementation.
   */
  Status log(const LogMessage &);

  /**
   * Implementers should use this function to flush any pending writes.
   */
  virtual Status flush() = 0;

  protected:
  /**
   * Implementers should use thsi function to either write or queue for write
   * the given log message.
   *
   * @return Status of the write process, or async enqueue.
   */
  virtual Status write(const LogMessage &) = 0;

  private:
  core::types::BitSet< LL > m_levels;
};

/**
 * Register a sink.
 */
Status RegisterSink(std::shared_ptr< iLogSink >);

} // namespace logging
} // namespace core

using core::logging::LL;

/**
 * Log a message at the given log level.
 * Usage:
 *     Log(LL::Info) << "my Message"
 */
#  define Log(ll) core::logging::LogMessageBuilder(ll, LOG_TRACE_INFO())

  /**
   * Log a message now, and a message at the end of the scope containing the
   * trace.
   */
#  define Trace() core::logging::ScopedLogger scope_logger(LOG_TRACE_INFO())

/**
 * Create a logger {@link core::logging::TraceInfo} for the current calling
 * context.
 */
#  define LOG_TRACE_INFO() \
    core::logging::TraceInfo(__FILE__, __FUNCTION__, __LINE__)

/**
 * RET_M(expression, message); Will 'return false' from a function if the
 * expression does no evaluate to true
 */
#  define RET_M(expr, msg)                                             \
    do {                                                               \
      if (!(expr)) {                                                   \
        Log(LL::Error) << msg << " @ " << __FILE__ << ":" << __LINE__; \
        return false;                                                  \
      }                                                                \
    } while (0)

/**
 * RET_ERRORCODE_M(expression, message; Will 'return expression' from a
 * function if the expression does not evaluate to 0.
 */
#  define RET_ERRORCODE_M(expr, msg)                                   \
    do {                                                               \
      const int ret = (expr);                                          \
      if (ret != 0) {                                                  \
        Log(LL::Error) << msg << " @ " << __FILE__ << ":" << __LINE__; \
        return ret;                                                    \
      }                                                                \
    } while (0)

#  include "logging.inl"
#  include "logging_default_sinks.h"

#endif
