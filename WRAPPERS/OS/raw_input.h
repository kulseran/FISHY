/**
 * Get raw keyboard, mouse, and controller inputs in a platform agnostic format.
 */
#ifndef FISHY_RAW_INPUT_H
#define FISHY_RAW_INPUT_H

#include "virtual_input.h"

#include <CORE/BASE/status.h>
#include <EXTERN_LIB/srutil/delegate/delegate.hpp>

#include <map>

#if defined(PLAT_WIN32)
#  include <windows.h>
#elif defined(PLAT_LINUX)
#endif

#ifndef INPUT_ENABLE_DEBUG_OUTPUT
#  define INPUT_ENABLE_DEBUG_OUTPUT (1)
#endif

namespace wrappers {
namespace os {

eKeyMap::type Platform_RemapKeycode(int ident);
eAxisMap::type Platform_RemapAxisIdent(int ident);

/**
 * Re-maps a controller's platform id to an {@link eKeyMap} enumeration
 */
eKeyMap::type Platform_RemapControllerButton(
    long vendor, long product, long version, int ident);

/**
 * Re-maps a controller's platform id to an {@link eAxisMap} enumeration
 */
eAxisMap::type Platform_RemapControllerAxis(
    long vendor, long product, long version, int ident);

/**
 * Re-maps a controller's input to the range [-INT_MAX, INT_MAX]
 */
int Platform_RemapControllerAxisRange(
    long vendor, long product, long version, eAxisMap::type ident, int value);

typedef std::map< int, eKeyMap::type > tControllerButtonMap;
/**
 * Adds a mapping from controller button ids to {@link eKeyMap}
 */
bool Platform_AddControllerButtonRemap(
    long vendor, long product, long version, const tControllerButtonMap &);

typedef std::map< int, eAxisMap::type > tControllerAxisMap;
/**
 * Adds a mapping from controller axis ids to {@link eAxisMap}
 */
bool Platform_AddControllerAxisRemap(
    long vendor, long product, long version, const tControllerAxisMap &);

typedef std::map< eAxisMap::type, std::pair< int, int > >
    tControllerAxisRangeMap;
/**
 * Adds a re-mapping of controller axis ranges to float values.
 */
bool Platform_AddControllerAxisRangeRemap(
    long vendor, long product, long version, const tControllerAxisRangeMap &);

/**
 * Handlers for the application to hook into the controller mapping code
 * so as to report errors on bad mappings.
 */
typedef srutil::delegate< void(long, long, long, int, long) >
    tBadMappingHandler;
void SetBadAxisMapHandler(tBadMappingHandler &);
void SetBadButtonMapHandler(tBadMappingHandler &);

#if defined(PLAT_WIN32)
class Window;
Status InitRawInput(const HWND hWnd, Window *pParent);
Status ShutdownRawInput(const HWND hWnd);
void ParseRawInput(const HRAWINPUT hRawInput, Window *pParent);

/**
 * Update the keyboard/mouse button states from the default devices.
 */
void UpdateKeyStates(Window *pParent);

struct RawInputMetadata {
  HANDLE m_inputDevices[eDeviceId::DEVICE_COUNT];
};
#elif defined(PLAT_LINUX)
#else
#error "raw input not supported on this platform."
#endif


} // namespace os
} // namespace wrappers

#endif
