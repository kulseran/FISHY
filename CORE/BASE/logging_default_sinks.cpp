#include "logging_default_sinks.h"

#include <CORE/UTIL/lexical_cast.h>

using std::chrono::system_clock;

namespace core {
namespace logging {

LoggingStdioSink::LoggingStdioSink(
    const core::types::BitSet< LL > &levels, const Options &options)
    : iLogSink(levels) {
  std::clog << "Logging to stdio. \n";
  m_options = options;
  std::ios::sync_with_stdio(m_options.m_syncstdio);
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

static const char *g_logLevelStr[LL::COUNT] = {"F: ", "I: ", "W: ", "E: "};

static std::string formatTime(const system_clock::time_point &timestamp) {
  std::string rVal;
  const std::chrono::seconds secs =
      std::chrono::duration_cast< std::chrono::seconds >(
          timestamp.time_since_epoch());
  if (core::util::lexical_cast(secs.count(), rVal)) {
    return rVal + "s";
  }
  return "???";
}

void LoggingStdioSink::writeToStream(
    std::ostream &channel, const LogMessage &info, const std::string &msg) {

  switch (m_options.m_format) {
    case Options::FORMAT_VERBOSE:
    case Options::FORMAT_SHORT:
      channel << formatTime(info.m_trace.m_timestamp) << " | "
              << g_logLevelStr[info.m_logLevel];
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

LoggingStdioSink::Options::Options()
    : m_format(FORMAT_SHORT), m_syncstdio(false) {
}

} // namespace logging
} // namespace core
