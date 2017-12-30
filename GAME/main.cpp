#include "gameloop.h"
#include "settings.pb.h"

#include <APP_SHARED/app_main.h>
#include <APP_SHARED/fileutil.h>
#include <CORE/ARCH/raw_input.h>
#include <CORE/ARCH/timer.h>
#include <CORE/ARCH/window.h>
#include <CORE/BASE/logging.h>
#include <CORE/BASE/logging_file.h>

#include <fstream>

//----------------------------------------------------------------------------
// P R O T O T Y P E S
//----------------------------------------------------------------------------
FISHY_APPLICATION(GameClientApp);

//----------------------------------------------------------------------------
// I M P L E M E N T A T I O N
//----------------------------------------------------------------------------
class Callbacks : public core::window::iWindowCallback {
  public:
    Callbacks(
      volatile bool &active,
      volatile bool &hidden,
      core::input::InputManager &inputManager,
      core::window::Window &window)
      : m_active(active),
        m_hidden(hidden),
        m_inputManager(inputManager),
        m_window(window) {
    }

    virtual void onInit(void) {
      m_active = true;
    }

    virtual void onDest(void) {
      m_active = false;
    }

    virtual void onUpdate(void) {
    }

    virtual void onShow(void) {
      m_hidden = false;
      m_inputManager.resetPress();
    }

    virtual void onHide(void) {
      m_hidden = true;
      m_inputManager.resetPress();
    }

    virtual void onReshape(void) {
      m_inputManager.resetAll();
    }

    virtual void onReset(void) {
      m_inputManager.resetAll();
    }

    virtual void onButtonInput(core::input::eDeviceId::type device, core::input::eKeyMap::type ident, bool down) {
      m_inputManager.updateKey(device, ident, down);
    }

    virtual void onAxisInput(core::input::eDeviceId::type device, core::input::eAxisMap::type axis, int value) {
      m_inputManager.updateAxis(device, axis, value);
    }

  private:
    volatile bool &m_active;
    volatile bool &m_hidden;
    core::input::InputManager &m_inputManager;
    core::window::Window &m_window;
};

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
  TraceScope();

  std::ofstream logFile("log.txt");
  core::logging::ProtoFileFormatter fileFormatter;
  core::logging::Sink fileSink(core::logging::g_globalManager, logFile, core::containers::BitSet< LL >() | LL::Error | LL::Info | LL::Trace | LL::Warning | LL::User1 | LL::User2 | LL::User3 | LL::User4, fileFormatter);

  game::Settings settings;
  if (!appshared::parseProtoFromFile("./data/client_settings.pb", settings)) {
    Log(LL::Warning) << "Missing or corrupt game_settings.pb in runtime directory. "
                     << "Game may fail." << std::endl;
  }

  volatile bool active = true;
  volatile bool hidden = false;
  core::window::Window window;
  core::input::InputManager inputManager;

  if (!settings.has_window_settings()) {
    const game::WindowSettings &windowSettings = settings.get_window_settings();
    core::window::DisplayCaps displaySettings = window.getSettings();
    displaySettings.m_fullScreen = windowSettings.get_fullscreen();
    displaySettings.m_width = windowSettings.get_width() != 0 ? static_cast<u32>(windowSettings.get_width()) : 1024u;
    displaySettings.m_height = windowSettings.get_height() != 0 ? static_cast<u32>(windowSettings.get_height()) : 640u;
    displaySettings.m_lockCursor = windowSettings.get_lock_cursor();
    displaySettings.m_hideCursor = windowSettings.get_hide_cursor();
    window.setSettings(displaySettings);
  }

  Callbacks cbs(active, hidden, inputManager, window);
  window.setCallbackInterface(&cbs);
  window.setTitle("Silent Star");

  window.create();
  GameLoop loop(active, hidden, settings, inputManager, window);
  loop.run();
  window.destroy();

  core::logging::g_globalManager.unregisterSink(fileSink);
  return eExitCode::OK;
}