/**
 * app_main.cpp
 */
#include "app_main.h"

#include <CORE/BASE/appstats.h>
#include <CORE/BASE/config.h>
#include <CORE/BASE/logging.h>
// #include <CORE/VFS/vfs.h>

#include <.generated/version.h>

#include <iostream>

#ifdef FISHY_DEBUG
#  define DEFAULT_LOG_LEVEL 3
#else
#  define DEFAULT_LOG_LEVEL 1
#endif

core::config::Flag< int >
    g_logVerbosity("log_verbosity", "Logging level [0, 4]", DEFAULT_LOG_LEVEL);
core::config::Flag< bool > g_debugHalt(
    "haltonerror", "Should this binary halt on exit on error.", true);

const core::AppInfo g_build_date_var("build_date", BUILD_TIMESTAMP);
const core::AppInfo g_build_version_var("build_version", BUILD_VERSION_HASH);
const core::AppInfo g_build_branch_var("build_branch", BUILD_BRANCH_ID);

void setupLogging();

/**
 * Main Entry Point
 *
 * Applications should implement {@link FISHY_APPLICATION}
 */
int main(const int argc, const char **argv) {
  // Parse flags
  if (!core::config::ParseFlags(argc, argv)) {
    core::config::PrintFlags();
    G_pApplication->printHelp();
    return eExitCode::INVALID_FLAGS;
  }
  setupLogging();
  Trace();

  // Setup Networking
  // core::net::Initialize();

  // Setup VFS
  // vfs::Init();
  // vfs::Mount("./", "./");
  // vfs::Mount("memfile/", "./memfile/");

  // Init Application
  G_pApplication->init(argv[0]);

  // Run Application
  Log(LL::Trace) << "Entering Application Main";
  int rVal = G_pApplication->main();
  Log(LL::Trace) << "Exiting Application Main";

  // Shutdown VFS
  // vfs::UnmountAll();
  // vfs::Dest();

  // Shutdown Networking
  // core::net::Shutdown();

  // Finish
  Log(LL::Trace) << "App Terminated: " << rVal;

#if FISHY_DEBUG
  if (g_debugHalt.get() && rVal != 0) {
    std::cout << "Press Enter to Continue...";
    std::cin.get();
  }
  ASSERT(rVal == 0);
#endif
  return rVal;
}

/**
 *
 */
void setupLogging() {
  CHECK_M(
      g_logVerbosity.get() >= 0 && g_logVerbosity.get() <= 4,
      "log_verbosity must be in the range [0, 4], was: "
          << g_logVerbosity.get());

  core::types::BitSet< LL > loggerLevels;
  switch (g_logVerbosity.get()) {
    case 4:
      loggerLevels |= LL::Trace;
    case 3:
      loggerLevels |= LL::Info;
    case 2:
      loggerLevels |= LL::Warning;
    case 1:
      loggerLevels |= LL::Error;
    default:
      break;
  }
  core::logging::RegisterSink(std::shared_ptr< core::logging::iLogSink >(
      new core::logging::LoggingStdioSink(loggerLevels)));
}
