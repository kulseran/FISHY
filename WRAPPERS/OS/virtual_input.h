/**
 * Container for handling a virtual input device.
 */
#ifndef FISHY_VIRTUAL_INPUT_H
#define FISHY_VIRTUAL_INPUT_H

#include <WRAPPERS/OS/raw_input_cfg.pb.h>

namespace wrappers {
namespace os {

/**
 * Listing of attached devices.
 * Normal inputs go into "DEFAULT", while specific controllers
 * get placed into CONTROLLER_X
 */
struct eDeviceId {
  enum type {
    DEVICE_DEFAULT,
    DEVICE_CONTROLLER_1,
    DEVICE_CONTROLLER_2,
    DEVICE_CONTROLLER_3,
    DEVICE_CONTROLLER_4,
    DEVICE_CONTROLLER_5,
    DEVICE_CONTROLLER_6,
    DEVICE_CONTROLLER_7,
    DEVICE_CONTROLLER_8,
    DEVICE_COUNT
  };
};

/**
 * Storage for input states
 */
class InputManager {
  public:
  InputManager();

  /**
   * @return true if {@code ident} key transitioned from up to down this frame.
   */
  bool keyWasPressed(eDeviceId::type device, eKeyMap::type ident) const {
    return m_keyspressed[device][ident];
  }

  /**
   * @return true if {@code ident} key transitioned from down to up this frame
   */
  bool keyWasReleased(eDeviceId::type device, eKeyMap::type ident) const {
    return m_keysreleased[device][ident];
  }

  /**
   * @return the current {@code ident} key is pressed
   */
  bool keyIsDown(eDeviceId::type device, eKeyMap::type ident) const {
    return m_keysdown[device][ident];
  }

  /**
   * @return the current {@code ident} axis' position
   */
  int axisGetPos(eDeviceId::type device, eAxisMap::type ident) const {
    return m_axispos[device][ident];
  }

  /**
   * @return the last manipulated device
   */
  eDeviceId::type getLastPressDevice() const { return m_lastPressedDevice; }

  /**
   * @return the last pressed key
   */
  eKeyMap::type getLastPressedKey() const { return m_lastPressedKey; }

  /**
   * @return the last released key
   */
  eKeyMap::type getLastReleasedKey() const { return m_lastReleasedKey; }

  /**
   * The window manager should call this to update a moving axis.
   *
   * @param ident the platform specific identifier for this axis
   * @param pos the position of the axis
   */
  void updateAxis(eDeviceId::type device, eAxisMap::type ident, int pos);

  /**
   * The window manager should call this to update a key.
   *
   * @param ident the platform specific identifier for this axis
   * @param down true if the key is currently down
   */
  void updateKey(eDeviceId::type, eKeyMap::type ident, bool down);

  /**
   * The window manager should call this to reset the key states at the end of
   * the frame
   */
  void resetAll(void);

  /**
   * The window manager should call this to reset the key states at the end of
   * the frame
   */
  void resetPress(void);

  private:
  bool m_keysdown[eDeviceId::DEVICE_COUNT][eKeyMap::type_COUNT];
  bool m_keyspressed[eDeviceId::DEVICE_COUNT][eKeyMap::type_COUNT];
  bool m_keysreleased[eDeviceId::DEVICE_COUNT][eKeyMap::type_COUNT];
  int m_axispos[eDeviceId::DEVICE_COUNT][eAxisMap::type_COUNT];

  eDeviceId::type m_lastPressedDevice;
  eDeviceId::type m_lastReleasedDevice;
  eKeyMap::type m_lastPressedKey;
  eKeyMap::type m_lastReleasedKey;
};

/**
 * Convert {@link eKeyMap} to a string name.
 */
const char *GetKeyName(const eKeyMap::type);

/**
 * Convert {@link eKeyMap} to the textual representation of that key.
 */
const char *GetKeyText(const eKeyMap::type key, bool shifted);

/**
 * Convert {@link eAxisMap} to a string name.
 */
const char *GetAxisName(const eAxisMap::type);

} // namespace os
} // namespace wrappers

#endif
