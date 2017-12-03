/**
 * app_main.cpp
 */
#include "app_main.h"

#include <CORE/BASE/appstats.h>
#include <CORE/BASE/config.h>
#include <CORE/BASE/logging.h>
// #include <CORE/NET/net_common.h>
// #include <CORE/VFS/vfs.h>

#include <.generated/version.h>

using core::AppInfo;
using core::config::Flag;

Flag< int >
    g_statusPort("status_port", "Port for hosting the HTTP status server", 80);
Flag< bool > g_debugHalt(
    "haltonerror", "Should this binary halt on exit on error.", true);

const AppInfo g_build_date_var("build_date", BUILD_TIMESTAMP);
const AppInfo g_build_branch_var("build_branch", BUILD_BRANCH_ID);
const AppInfo g_build_version_var("build_version", BUILD_VERSION_HASH);

/**
 * Main Entry Point
 *
 * Applications should implement {@link FISHY_APPLICATION}
 */
int main(const int argc, const char **argv) {
  Trace();

  // Parse flags
  if (!core::config::ParseFlags(argc, argv)) {
    core::config::PrintFlags();
    G_pApplication->printHelp();
    return eExitCode::INVALID_FLAGS;
  }

  // Setup Networking
  // core::net::Initialize();

  // Setup VFS
  // vfs::Init();
  // vfs::Mount("./", "./");
  // vfs::Mount("memfile/", "./memfile/");

  // Init Application
  G_pApplication->init(argv[0]);

  // Init status server
  // G_pApplication->getStatusServer().start(core::net::ServerDef::Builder()
  // .set_dns_name("localhost")
  // .set_port(g_statusPort.get())
  // .set_connection_timeout_sec(30)
  // .set_max_connections(10)
  // .build());

  // Run Application
  Log(LL::Trace) << "Entering Application Main";
  int rVal = G_pApplication->main();
  Log(LL::Trace) << "Exiting Application Main";

  // Terminate status server
  // G_pApplication->getStatusServer().stop();

  // Shutdown VFS
  // vfs::UnmountAll();
  // vfs::Dest();

  // Shutdown Networking
  // core::net::Shutdown();

  // Finish
  Log(LL::Trace) << "App terminated with status: " << rVal;

#if FISHY_DEBUG
  if (g_debugHalt.get() && rVal != 0) {
    std::cout << "Press Enter to Continue...";
    std::cin.get();
  }
  ASSERT(rVal == 0);
#endif
  return rVal;
}
