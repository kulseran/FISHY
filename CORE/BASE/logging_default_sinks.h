/**
 * Default cout/cerr/clog logging sinks
 */
#ifndef FISHY_LOGGING_DEFAULT_SINKS_H
#define FISHY_LOGGING_DEFAULT_SINKS_H

#include "logging.h"

namespace core {
namespace logging {

/**
 * {@code iLogSink} that write out to stderr and stdout depending on the log
 * level.
 */
class LoggingStdioSink : public iLogSink {
  public:
  /**
   * Formatting options for the sink.
   */
  struct Options {
    Options();
    enum eFormat { FORMAT_TINY, FORMAT_SHORT, FORMAT_VERBOSE };
    eFormat m_format;
    bool m_syncstdio;
  };

  LoggingStdioSink(
      const core::types::BitSet< LL > &levels,
      const Options &options = Options());

  protected:
  virtual Status flush() override;
  virtual Status write(const LogMessage &) override;

  private:
  void writeToStream(
      std::ostream &channel, const LogMessage &info, const std::string &msg);
  Options m_options;
};

} // namespace logging
} // namespace core

#endif
