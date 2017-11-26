/**
 *
 */
#include "logging.h"

#include <algorithm>

namespace core {
namespace logging {

/**
 * Creates a global log manager.
 */
LogManager &GetDefaultLogger() {
  static LogManager s_manager;
  return s_manager;
}

/**
 *
 */
LogManager::LogManager() {
}

/**
 *
 */
LogManager::~LogManager() {
  ASSERT(m_sinks.empty());
}

/**
 *
 */
Status LogManager::registerSink(LogSink *pSink) {
  if (std::find(m_sinks.begin(), m_sinks.end(), pSink) != m_sinks.end()) {
    return Status(Status::BAD_ARGUMENT);
  }
  m_sinks.push_back(pSink);
  return Status::ok();
}

/**
 *
 */
Status LogManager::unregisterSink(LogSink *pSink) {
  std::vector< LogSink * >::const_iterator itr =
      std::find(m_sinks.begin(), m_sinks.end(), pSink);
  if (itr == m_sinks.end()) {
    return Status(Status::OUT_OF_BOUNDS);
  }
  return Status::ok();
}

/**
 *
 */
Status LogManager::write(const LogMessage &message) {
  Status rVal = Status::ok();

  for (std::vector< LogSink * >::iterator itr = m_sinks.begin();
       itr != m_sinks.end(); ++itr) {
    Status ret = (*itr)->log(message);
    if (ret.getStatus() != Status::OK) {
      rVal = ret.clone();
    }
  }

  return rVal;
}

/**
 *
 */
LogSink::LogSink(LogManager &manager, const core::types::BitSet< LL > &levels)
    : m_manager(manager), m_levels(levels) {
  CHECK(m_manager.registerSink(this));
}

/**
 *
 */
LogSink::~LogSink() {
  CHECK(m_manager.unregisterSink(this));
}

/**
 *
 */
Status LogSink::log(const LogMessage &message) {
  if (!m_levels.isSet(message.m_logLevel)) {
    return Status::ok();
  }
  return write(message);
}

} // namespace logging
} // namespace core
