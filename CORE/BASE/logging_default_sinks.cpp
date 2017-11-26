#include "logging_default_sinks.h"

namespace core {
namespace logging {

LoggingStdioSink::LoggingStdioSink(
    LogManager &manager,
    const core::types::BitSet< LL > &levels,
    const Options &options)
    : LogSink(manager, levels) {
  std::clog << "Logging to stdio. \n";
  m_options = options;
}

/**
 *
 */
Status LoggingStdioSink::flush() {
  std::cout.flush();
  std::cout.clear();
  std::cerr.flush();
  std::cerr.clear();
  return Status::ok();
}

static std::string CleanupMessage(const std::string &msgIn) {
  if (msgIn.empty() || msgIn.back() == '\n') {
    return msgIn;
  }
  return (msgIn + '\n');
}

static const char *LogLevelToString(const LL::type level) {
  switch (level) {
    case LL::Fine:
      return "F ";
    case LL::Info:
      return "I ";
    case LL::Warning:
      return "W ";
    case LL::Error:
      return "E ";
  }
  CHECK_UNREACHABLE();
}

void LoggingStdioSink::writeToStream(
    std::ostream &channel, const LogMessage &info, const std::string &msg) {
  switch (m_options.m_format) {
    case Options::FORMAT_VERBOSE:
    case Options::FORMAT_SHORT:
      channel << LogLevelToString(info.m_logLevel);
    case Options::FORMAT_TINY:
      channel << msg;
  }
}

Status LoggingStdioSink::write(const LogMessage &message) {
  std::string msg = CleanupMessage(message.m_msg);
  if (message.m_logLevel == LL::Warning || message.m_logLevel == LL::Error) {
    writeToStream(std::cerr, message, msg);
  } else {
    writeToStream(std::cout, message, msg);
  }
  return Status::ok();
}

LoggingStdioSink::Options::Options() : m_format(FORMAT_SHORT) {
}
} // namespace logging
} // namespace core
