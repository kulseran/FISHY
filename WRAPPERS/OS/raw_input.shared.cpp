#include "raw_input.h"

#include <CORE/types.h>

#include <algorithm>
#include <map>

#if INPUT_ENABLE_DEBUG_OUTPUT
#  include <iostream>
#endif

namespace wrappers {
namespace os {

static std::map< std::pair< long, long >, tControllerButtonMap > g_buttonMaps;
static std::map< std::pair< long, long >, tControllerAxisMap > g_axisMaps;
static std::map< std::pair< long, long >, tControllerAxisRangeMap >
    g_axisRangeMaps;

static tBadMappingHandler g_badAxisHandler;
static tBadMappingHandler g_badButtonHandler;

/**
 *
 */
void SetBadAxisMapHandler(tBadMappingHandler &handler) {
  g_badAxisHandler = handler;
}

/**
 *
 */
void SetBadButtonMapHandler(tBadMappingHandler &handler) {
  g_badButtonHandler = handler;
}

/**
 *
 */
bool Platform_AddControllerButtonRemap(
    long vendor,
    long product,
    long version,
    const tControllerButtonMap &mapping) {
  (void) version;

  auto mapItr = g_buttonMaps.find(std::make_pair(vendor, product));
  if (mapItr != g_buttonMaps.end()) {
    return false;
  }
  g_buttonMaps[std::make_pair(vendor, product)] = mapping;
  return true;
}

/**
 *
 */
bool Platform_AddControllerAxisRemap(
    long vendor,
    long product,
    long version,
    const tControllerAxisMap &mapping) {
  (void) version;

  auto mapItr = g_axisMaps.find(std::make_pair(vendor, product));
  if (mapItr != g_axisMaps.end()) {
    return false;
  }
  g_axisMaps[std::make_pair(vendor, product)] = mapping;
  return true;
}

/**
 *
 */
bool Platform_AddControllerAxisRangeRemap(
    long vendor,
    long product,
    long version,
    const tControllerAxisRangeMap &mapping) {
  (void) version;

  auto mapItr = g_axisRangeMaps.find(std::make_pair(vendor, product));
  if (mapItr != g_axisRangeMaps.end()) {
    return false;
  }
  g_axisRangeMaps[std::make_pair(vendor, product)] = mapping;
  return true;
}

/**
 *
 */
eKeyMap::type Platform_RemapControllerButton(
    long vendor, long product, long version, int ident) {
  eKeyMap::type defaultKey =
      (ident <= (eKeyMap::KEY_JOY_50 - eKeyMap::KEY_JOY_0))
          ? eKeyMap::type(eKeyMap::KEY_JOY_0 + ident)
          : eKeyMap::type_COUNT;

  auto mapItr = g_buttonMaps.find(std::make_pair(vendor, product));
  if (mapItr == g_buttonMaps.end()) {
    if (g_badButtonHandler) {
      g_badButtonHandler(vendor, product, version, ident, 1);
    }
    return defaultKey;
  }
  auto keyItr = mapItr->second.find(ident);
  if (keyItr == mapItr->second.end()) {
    if (g_badButtonHandler) {
      g_badButtonHandler(vendor, product, version, ident, 1);
    }
    return defaultKey;
  }
  return keyItr->second;
}

/**
 *
 */
eAxisMap::type Platform_RemapControllerAxis(
    long vendor, long product, long version, int ident) {
  auto mapItr = g_axisMaps.find(std::make_pair(vendor, product));
  if (mapItr == g_axisMaps.end()) {
#if INPUT_ENABLE_DEBUG_OUTPUT
    std::cerr << "Unknown device " << vendor << " : " << product << std::endl;
#endif
    if (g_badAxisHandler) {
      g_badAxisHandler(
          vendor, product, version, ident, std::numeric_limits< long >::min());
    }
    return eAxisMap::type_COUNT;
  }
  auto axisItr = mapItr->second.find(ident);
  if (axisItr == mapItr->second.end()) {
    if (g_badAxisHandler) {
      g_badAxisHandler(
          vendor, product, version, ident, std::numeric_limits< long >::min());
    }
    return eAxisMap::type_COUNT;
  }
  return axisItr->second;
}

/**
 *
 */
int Platform_RemapControllerAxisRange(
    long vendor, long product, long version, eAxisMap::type ident, int value) {
  auto mapItr = g_axisRangeMaps.find(std::make_pair(vendor, product));
  if (mapItr == g_axisRangeMaps.end()) {
#if INPUT_ENABLE_DEBUG_OUTPUT
    std::cerr << "Unknown device " << vendor << " : " << product << std::endl;
#endif
    if (g_badAxisHandler) {
      g_badAxisHandler(vendor, product, version, ident, value);
    }
    return value;
  }
  auto axisItr = mapItr->second.find(ident);
  if (axisItr == mapItr->second.end()) {
    if (g_badAxisHandler) {
      g_badAxisHandler(vendor, product, version, ident, value);
    }
    return value;
  }

  const long range = axisItr->second.second - axisItr->second.first;
  const long stride = (std::numeric_limits< int >::max() / range) * 2;
  const long valueAbs = value - axisItr->second.first;
  const long valueFinal =
      (valueAbs * stride) - std::numeric_limits< int >::max();
  return valueFinal;
}

} // namespace os
} // namespace wrappers
