/**
 * Platform specific graphical window routines.
 */
#ifndef FISHY_WINDOW_H
#define FISHY_WINDOW_H

#include <CORE/BASE/status.h>
#include <CORE/UTIL/noncopyable.h>

#include <string>

namespace core {
namespace window {

/**
 * System possible resolutions.
 */
struct DisplayCaps {
  unsigned m_deviceId;
  unsigned m_width;
  unsigned m_height;
  unsigned m_pixelDepth;
  bool m_fullScreen;
  bool m_lockCursor;
  bool m_hideCursor;

  DisplayCaps(
      unsigned deviceId,
      unsigned width,
      unsigned height,
      unsigned pixelDepth,
      bool fullScreen,
      bool lockCursor,
      bool hideCursor)
      : m_deviceId(deviceId),
        m_width(width),
        m_height(height),
        m_pixelDepth(pixelDepth),
        m_fullScreen(fullScreen),
        m_lockCursor(lockCursor),
        m_hideCursor(hideCursor) {}
};

/**
 * Callback to handle window events.
 */
class Window;
class iWindowCallback : core::util::noncopyable {
  public:
  void setOwner(Window *pOwner) { m_pOwner = pOwner; }

  virtual void onInit(void) = 0;
  virtual void onDest(void) = 0;
  virtual void onUpdate(void) = 0;
  virtual void onShow(void) = 0;
  virtual void onHide(void) = 0;
  virtual void onReshape(void) = 0;
  virtual void onReset(void) = 0;

  protected:
  Window *m_pOwner;
};

/**
 * Window
 */
class Window : util::noncopyable {
  public:
  class Impl;

  Window(
      const char *pTitle,
      const DisplayCaps &displaySettings,
      iWindowCallback *pCallbacks);
  ~Window();

  Status create();
  void destroy();
  Status reset();

  Impl *getImpl() { return m_pImpl; };
  iWindowCallback *getCallbackInterface() { return m_cbs; }
  DisplayCaps getSettings() const { return m_settings; }

  void setSettings(const DisplayCaps &settings) { m_settings = settings; }

  void framestep();

  private:
  Impl *m_pImpl;

  DisplayCaps m_settings;
  iWindowCallback *m_cbs;
  const char *m_title;
};

} // namespace window
} // namespace core

#endif
