#include "net_common.h"

#include <CORE/ARCH/platform.h>

#if defined(PLAT_WIN32)

#  include <CORE/BASE/logging.h>
#  include <CORE/UTIL/noncopyable.h>

namespace core {
namespace net {

#  include <WS2tcpip.h>
#  include <WinSock2.h>
#  pragma comment(lib, "Ws2_32.lib")

/**
 * Windows networking is based around the WSA lib that must be initialized.
 */
class WSA : util::noncopyable {
  public:
  WSA() : m_initialized(false) {}

  ~WSA() {
    if (m_initialized) {
      shutdown();
    }
  }

  void shutdown() {
    if (!m_initialized) {
      return;
    }
    WSACleanup();

    m_initialized = false;
  }

  void initialize() {
    if (m_initialized) {
      return;
    }
    Trace();

    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    m_initialized = true;
  }

  bool isInitialized() const { return m_initialized; }

  private:
  bool m_initialized;
};
WSA g_wsa;

/**
 *
 */
void Initialize() {
  g_wsa.initialize();
}

/**
 *
 */
void Shutdown() {
  g_wsa.shutdown();
}

/**
 *
 */
bool IsInitialized() {
  return g_wsa.isInitialized();
}

} // namespace net
} // namespace core

#endif // defined(PLAT_WIN32)
