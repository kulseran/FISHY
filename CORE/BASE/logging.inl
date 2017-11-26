#ifndef FISHY_LOGGING_INL
#define FISHY_LOGGING_INL

namespace core {
namespace logging {

inline TraceInfo::TraceInfo()
    : m_file(nullptr), m_function(nullptr), m_line(-1) {
}

inline LogMessage::LogMessage() : m_logLevel(LL::Fine) {
}

inline LogMessage::LogMessage(const LL::type level, const TraceInfo &trace)
    : m_logLevel(level), m_trace(trace) {
}

inline TraceInfo::TraceInfo(const char *file, const char *function, long line)
    : m_file(file), m_function(function), m_line(line) {
  // m_threadId = ;
  // m_stackPtr = ;
}

inline LogMessageBuilder::LogMessageBuilder(
    LogManager &manager, const LL::type level, const TraceInfo &trace)
    : m_manager(manager), m_message(level, trace) {
}

inline LogMessageBuilder::~LogMessageBuilder() {
  Status ret = m_manager.write(m_message);
  if (!ret) {
    std::cerr << "Log message not logged to all sinks." << std::endl;
  }
}

/**
 *
 */
template < typename T >
inline LogMessageBuilder &LogMessageBuilder::operator<<(const T &obj) {
  m_buffer << obj;
  return *this;
}

/**
 *
 */
inline LogMessageBuilder &LogMessageBuilder::
operator<<(std::ostream &(*fn)(std::ostream &) ) {
  m_buffer << fn;
  return *this;
}

} // namespace logging
} // namespace core

#endif
