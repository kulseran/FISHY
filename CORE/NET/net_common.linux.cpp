#include "net_common.h"

#include <CORE/ARCH/platform.h>

#if defined(PLAT_LINUX)

namespace core {
namespace net {

/**
 *
 */
void Initialize() {
}

/**
 *
 */
void Shutdown() {
}

/**
 *
 */
bool IsInitialized() {
  return true;
}

} // namespace net
} // namespace core

#endif // defined(PLAT_WIN32)
