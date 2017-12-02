#include "logging.h"

#include <algorithm>
#include <functional>

namespace core {
namespace logging {

// Set 16K messages @ ~1K/msg -> ~16Mb of log storage in memory.
static const size_t MAX_LOG_BUFFER = 1024 * 16;

/**
 * Collection of log sinks to be written to.
 */
class LogManager : core::util::noncopyable {
  public:
  LogManager();
  ~LogManager();

  Status write(const LogMessage &);
  Status registerSink(std::shared_ptr< iLogSink >);

  private:
  std::vector< std::shared_ptr< iLogSink > > m_sinks;
  core::types::ConcurrentQueue< LogMessage > m_messages;
  std::thread m_loggerThread;

  static void logFn(LogManager *);
};

/**
 * Creates a global log manager.
 */
LogManager &GetDefaultLogger() {
  static LogManager s_manager;
  return s_manager;
}

Status RegisterSink(std::shared_ptr< iLogSink > pSink) {
  return GetDefaultLogger().registerSink(pSink);
}

/**
 *
 */
LogManager::LogManager()
    : m_loggerThread(std::bind(&LogManager::logFn, this)),
      m_messages(MAX_LOG_BUFFER) {
}

static void FlushSink(std::shared_ptr< iLogSink > &pSink) {
  pSink->flush().ignoreErrors();
}

/**
 *
 */
LogManager::~LogManager() {
  m_messages.waitEmpty();
  m_messages.close();
  m_loggerThread.join();
  std::for_each(m_sinks.begin(), m_sinks.end(), FlushSink);
}

/**
 *
 */
Status LogManager::registerSink(std::shared_ptr< iLogSink > pSink) {
  if (std::find(m_sinks.begin(), m_sinks.end(), pSink) != m_sinks.end()) {
    return Status(Status::BAD_ARGUMENT);
  }
  m_sinks.push_back(pSink);
  return Status::ok();
}

/**
 *
 */
LogMessageBuilder::~LogMessageBuilder() {
  Status ret = GetDefaultLogger().write(m_message);
  if (!ret) {
    std::cerr << "Log message not logged to all sinks." << std::endl;
  }
}

/**
 *
 */
Status LogManager::write(const LogMessage &message) {
  m_messages.push(message);
  return Status::ok();
}

/**
 *
 */
void LogManager::logFn(LogManager *pManager) {
  while (true) {
    LogMessage message;
    if (!pManager->m_messages.pop(message)) {
      break;
    }

    for (std::vector< std::shared_ptr< iLogSink > >::iterator itr =
             pManager->m_sinks.begin();
         itr != pManager->m_sinks.end();
         ++itr) {
      Status ret = (*itr)->log(message);
      if (ret.getStatus() != Status::OK) {
        std::cerr << "LogMessage lost on channel "
                  << std::distance(pManager->m_sinks.begin(), itr) << "!"
                  << std::endl;
      }
    }
  }
}

/**
 *
 */
iLogSink::iLogSink(const core::types::BitSet< LL > &levels) : m_levels(levels) {
}

/**
 *
 */
iLogSink::~iLogSink() {
}

/**
 *
 */
Status iLogSink::log(const LogMessage &message) {
  if (!m_levels.isSet(message.m_logLevel)) {
    return Status::ok();
  }
  return write(message);
}

/**
 *
 */
TraceInfo::TraceInfo(const char *file, const char *function, long line)
    : m_file(file),
      m_function(function),
      m_line(line),
      m_threadId(std::this_thread::get_id()) {
  int sp;
  m_stackPtr = reinterpret_cast< uintptr_t >(&sp);
  m_timestamp = std::chrono::system_clock::now();
}

} // namespace logging
} // namespace core
