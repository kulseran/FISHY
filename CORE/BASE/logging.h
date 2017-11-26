/**
 * Core logging functionality
 */
#ifndef FISHY_LOGGING_H
#define FISHY_LOGGING_H

#include <CORE/BASE/status.h>
#include <CORE/TYPES/bitset.h>
#include <CORE/UTIL/noncopyable.h>

#include <sstream>
#include <string>
#include <thread>
#include <vector>

namespace core {
namespace logging {

// Logging Level
struct LL {
  enum type {
    Fine,    // Detailed logging
    Info,    // Info logging
    Warning, // Warnings
    Error,   // Errors
    COUNT
  };
};

class TraceInfo {
  public:
  TraceInfo();
  TraceInfo(const char *file, const char *function, long line);

  const char *m_file;
  const char *m_function;
  long m_line;
  std::thread::id m_thread;
  uintptr_t m_stack;
  u64 m_timestamp;
};

struct LogMessage {
  LogMessage();
  LogMessage(const LL::type, const TraceInfo &);

  LL::type m_logLevel;
  TraceInfo m_trace;
  std::string m_msg;
};

class LogManager;
class LogMessageBuilder : core::util::noncopyable {
  public:
  LogMessageBuilder(LogManager &, const LL::type, const TraceInfo &);
  ~LogMessageBuilder();

  template < typename T >
  LogMessageBuilder &operator<<(const T &obj);
  LogMessageBuilder &operator<<(std::ostream &(*fn)(std::ostream &) );

  private:
  LogManager &m_manager;
  LogMessage m_message;
  std::stringstream m_buffer;
};

class LogSink : core::util::noncopyable {
  public:
  LogSink(LogManager &, const core::types::BitSet< LL > &levels);
  ~LogSink();

  Status log(const LogMessage &);
  virtual Status flush() = 0;

  protected:
  virtual Status write(const LogMessage &) = 0;

  private:
  core::types::BitSet< LL > m_levels;
  LogManager &m_manager;
};

class LogManager : core::util::noncopyable {
  public:
  LogManager();
  ~LogManager();

  Status registerSink(LogSink *);
  Status unregisterSink(LogSink *);

  Status write(const LogMessage &);

  private:
  std::vector< LogSink * > m_sinks;
};

LogManager &GetDefaultLogger();

} // namespace logging
} // namespace core

using core::logging::LL;

#  define Log(ll)                     \
    core::logging::LogMessageBuilder( \
        core::logging::GetDefaultLogger(), ll, LOG_TRACE_INFO())

#  define LOG_TRACE_INFO() \
    core::logging::TraceInfo(__FILE__, __FUNCTION__, __LINE__)

#  include "logging.inl"
#  include "logging_default_sinks.h"

#endif
