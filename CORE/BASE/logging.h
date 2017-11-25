
/**
 * logging.h
 *
 * Logging utility functions.
 * Basic logging is performed with std::iostream style messages:
 *   Log(LL::Info) << "message to display: " << someVar << std::endl;
 *
 * Special "trace" messages that will compile to nothing in release:
 *   Trace()
 *   TraceMsg("message to display: " << someVar)
 *   TraceScope()
 *
 * Special logging sinks may be added.
 */
#ifndef FISHY_LOGGING_H
#define FISHY_LOGGING_H

namespace core {
namespace logging {

/**
 * Logging levels
 */
struct LL {
  enum type {
    Trace,
    Info,
    Warning,
    Error,

    COUNT,
  };
};

/**
 * Logging Channels
 */
struct LC {
  enum type {
    Channel_Default = 0,
    Channel_1 = 1,
    Channel_2 = 1 << 1,
    Channel_3 = 1 << 2,
    Channel_4 = 1 << 3,
    Channel_5 = 1 << 4,
    Channel_6 = 1 << 5,
    Channel_7 = 1 << 6,
    Channel_8 = 1 << 7,
    COUNT
  };
};

struct TraceInfo {

};

} // namespace logging
} // namespace core

#if defined(Log) || defined(Trace)
#  error something defined Log or Trace! unable to use logging macros!
#endif

using core::logging::LL;
#define LOG_TRACE_INFO() core::logging::Message::TraceInfo( \
    __FILE__, \
    __FUNCTION__, \
    __LINE__, \
    std::this_thread::get_id())

/**
 * Core logging function.
 *   Log(LL:: ... ) << "message" << std::endl;
 */
#define Log(l) core::logging::Logger() \
    << LOG_TRACE_INFO() \
    << static_cast<core::logging::LL::type>(l)

/**
 * Trace logging.
 * These messages will compile to nothing in release. Don't have side-effects within the message.
 *   Trace("some message");
 */
#if defined(FISHY_OPT)
#  define Trace()
#  define TraceMsg(msg)
#  define TraceScope(msg)
#else
#  define Trace() core::logging::Logger() \
    << LOG_TRACE_INFO() \
    << core::logging::LL::Trace \
    << std::endl

#  define TraceMsg(msg) core::logging::Logger() \
    << LOG_TRACE_INFO() \
    << core::logging::LL::Trace \
    << msg \
    << std::endl
#  define TraceScope() core::logging::ScopedLogger scopedLogger(LOG_TRACE_INFO());
#endif

#endif