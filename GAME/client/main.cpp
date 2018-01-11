#include <GAME/client/settings.pb.h>

#include <APP_SHARED/app_main.h>
#include <APP_SHARED/fileutil.h>
#include <CORE/ARCH/timer.h>
#include <CORE/BASE/logging.h>

#include <fstream>

FISHY_APPLICATION(GameClientApp);

/**
 *
 */
void GameClientApp::printHelp() {
}

/**
 *
 */
void GameClientApp::init(const char *arg0) {
  (void) arg0;
}

int GameClientApp::main() {
  Trace();

  game::Settings settings;
  if (!appshared::parseProtoFromFile(
          "./data/client_settings.pb.txt", settings)) {
    Log(LL::Warning)
        << "Missing or corrupt game_settings.pb in runtime directory. "
        << "Game may fail.";
  }

  return eExitCode::OK;
}
