#ifndef FISHY_LOGGING_INL
#define FISHY_LOGGING_INL

#include <CORE/UTIL/lexical_cast.h>

#include <inttypes.h>

namespace core {
namespace logging {

inline TraceInfo::TraceInfo()
    : m_file(nullptr), m_function(nullptr), m_line(-1) {
}

inline LogMessage::LogMessage() : m_logLevel(LL::Trace), m_msgLen(0) {
  m_msg[0] = 0;
}

inline LogMessage::LogMessage(const LL::type level, const TraceInfo &trace)
    : m_logLevel(level), m_trace(trace), m_msgLen(0) {
  m_msg[0] = 0;
}

inline LogMessageBuilder::LogMessageBuilder(
    const LL::type level, const TraceInfo &trace)
    : m_message(level, trace) {
}

inline LogMessageBuilder::~LogMessageBuilder() {
  extern Status Write(const LogMessage &);
  Status ret = Write(m_message);
  if (!ret) {
    std::cerr << "Log message not logged to all sinks." << std::endl;
  }
}

/**
 *
 */
template < typename T >
inline LogMessageBuilder &LogMessageBuilder::operator<<(const T &obj) {
  std::string str;
  const size_t remainingLen = MAX_LOG_LEN - 4 - (m_message.m_msgLen + 1);
  if (core::util::lexical_cast(obj, str)) {
    strncat(m_message.m_msg, str.c_str(), remainingLen);
    if (remainingLen < str.length()) {
      strcat(m_message.m_msg, "...");
    }
  } else {
    char buffer[32] = {0};
    snprintf(
        buffer,
        ARRAY_LENGTH(buffer),
        "(unknown) 0x%" PRIXPTR,
        (uintptr_t) &obj);
    strncat(m_message.m_msg, buffer, remainingLen);
    if (remainingLen < strlen(buffer)) {
      strcat(m_message.m_msg, "...");
    }
  }
  return *this;
}

/**
 *
 */
template < typename T, unsigned N >
inline LogMessageBuilder &LogMessageBuilder::operator<<(const T (&obj)[N]) {
  std::string str;
  const size_t remainingLen = MAX_LOG_LEN - 4 - (m_message.m_msgLen + 1);
  if (std::is_same< char, T >::value) {
    strncat(m_message.m_msg, obj, remainingLen);
    if (remainingLen < str.length()) {
      strcat(m_message.m_msg, "...");
    }
  } else {
    char buffer[32] = {0};
    snprintf(
        buffer,
        ARRAY_LENGTH(buffer),
        "(unknown array) 0x%" PRIXPTR,
        (uintptr_t) &obj);
    strncat(m_message.m_msg, buffer, remainingLen);
    if (remainingLen < strlen(buffer)) {
      strcat(m_message.m_msg, "...");
    }
  }
  return *this;
}

/**
 *
 */
inline ScopedLogger::ScopedLogger(const TraceInfo &trace) : m_trace(trace) {
  core::logging::LogMessageBuilder(LL::Trace, m_trace)
      << "Enter " << m_trace.m_function;
}

/**
 *
 */
inline ScopedLogger::~ScopedLogger() {
  core::logging::LogMessageBuilder(LL::Trace, m_trace)
      << "Exit " << m_trace.m_function;
}

} // namespace logging
} // namespace core

#endif
