#include "raw_input.h"
#include "window.h"

#include <CORE/ARCH/platform.h>
#include <CORE/BASE/logging.h>
#include <CORE/types.h>

#include <algorithm>

#if defined(PLAT_WIN32)
#  include <windows.h>

#  include <WindowsX.h>
extern "C" {
#  include <hidsdi.h>

#  include <hidpi.h>
}

namespace wrappers {
namespace os {

/**
 *
 */
eAxisMap::type Platform_RemapAxisIdent(int ident) {
  switch (ident) {
    case 0:
      return eAxisMap::AXIS_MOUSE_X;
    case 1:
      return eAxisMap::AXIS_MOUSE_Y;
    case 2:
      return eAxisMap::AXIS_MOUSE_Z;
  }

  return eAxisMap::type_COUNT;
}

/**
 *
 */
eKeyMap::type Platform_RemapKeycode(int ident) {
  switch (ident) {
    case VK_LBUTTON:
      return eKeyMap::KEY_MOUSE_1;
    case VK_RBUTTON:
      return eKeyMap::KEY_MOUSE_3;
    case VK_MBUTTON:
      return eKeyMap::KEY_MOUSE_2;
    case VK_XBUTTON1:
      return eKeyMap::KEY_MOUSE_4;
    case VK_XBUTTON2:
      return eKeyMap::KEY_MOUSE_5;

    case VK_ESCAPE:
      return eKeyMap::KEY_ESC;
    case VK_BACK:
      return eKeyMap::KEY_BACKSPACE;
    case VK_TAB:
      return eKeyMap::KEY_TAB;
    case VK_PAUSE:
      return eKeyMap::KEY_PAUSE;
    case VK_SNAPSHOT:
      return eKeyMap::KEY_PRINTSCREEN;
    case VK_SPACE:
      return eKeyMap::KEY_SPACE;
    case VK_RETURN:
      return eKeyMap::KEY_ENTER;

    case VK_HOME:
      return eKeyMap::KEY_HOME;
    case VK_END:
      return eKeyMap::KEY_END;
    case VK_LEFT:
      return eKeyMap::KEY_LEFT;
    case VK_RIGHT:
      return eKeyMap::KEY_RIGHT;
    case VK_UP:
      return eKeyMap::KEY_UP;
    case VK_DOWN:
      return eKeyMap::KEY_DOWN;
    case VK_INSERT:
      return eKeyMap::KEY_INSERT;
    case VK_DELETE:
      return eKeyMap::KEY_DELETE;
    case VK_PRIOR:
      return eKeyMap::KEY_PAGEDOWN;
    case VK_NEXT:
      return eKeyMap::KEY_PAGEUP;

    case VK_LWIN:
      return eKeyMap::KEY_SYS_WIN;
    case VK_RWIN:
      return eKeyMap::KEY_SYS_WIN;
    case VK_APPS:
      return eKeyMap::KEY_SYS_APP;

    case 0x30:
      return eKeyMap::KEY_0;
    case 0x31:
      return eKeyMap::KEY_1;
    case 0x32:
      return eKeyMap::KEY_2;
    case 0x33:
      return eKeyMap::KEY_3;
    case 0x34:
      return eKeyMap::KEY_4;
    case 0x35:
      return eKeyMap::KEY_5;
    case 0x36:
      return eKeyMap::KEY_6;
    case 0x37:
      return eKeyMap::KEY_7;
    case 0x38:
      return eKeyMap::KEY_8;
    case 0x39:
      return eKeyMap::KEY_9;

    case VK_NUMPAD0:
      return eKeyMap::KEY_NUM_0;
    case VK_NUMPAD1:
      return eKeyMap::KEY_NUM_1;
    case VK_NUMPAD2:
      return eKeyMap::KEY_NUM_2;
    case VK_NUMPAD3:
      return eKeyMap::KEY_NUM_3;
    case VK_NUMPAD4:
      return eKeyMap::KEY_NUM_4;

    case VK_CLEAR: // appears to be numpad 5 when numlock is off?
    case VK_NUMPAD5:
      return eKeyMap::KEY_NUM_5;
    case VK_NUMPAD6:
      return eKeyMap::KEY_NUM_6;
    case VK_NUMPAD7:
      return eKeyMap::KEY_NUM_7;
    case VK_NUMPAD8:
      return eKeyMap::KEY_NUM_8;
    case VK_NUMPAD9:
      return eKeyMap::KEY_NUM_9;
    case VK_MULTIPLY:
      return eKeyMap::KEY_NUM_MUL;
    case VK_ADD:
      return eKeyMap::KEY_NUM_ADD;
    case VK_SUBTRACT:
      return eKeyMap::KEY_NUM_SUB;
    case VK_DECIMAL:
      return eKeyMap::KEY_NUM_PERIOD;
    case VK_DIVIDE:
      return eKeyMap::KEY_NUM_DIV;

    case 0x41:
      return eKeyMap::KEY_A;
    case 0x42:
      return eKeyMap::KEY_B;
    case 0x43:
      return eKeyMap::KEY_C;
    case 0x44:
      return eKeyMap::KEY_D;
    case 0x45:
      return eKeyMap::KEY_E;
    case 0x46:
      return eKeyMap::KEY_F;
    case 0x47:
      return eKeyMap::KEY_G;
    case 0x48:
      return eKeyMap::KEY_H;
    case 0x49:
      return eKeyMap::KEY_I;
    case 0x4A:
      return eKeyMap::KEY_J;
    case 0x4B:
      return eKeyMap::KEY_K;
    case 0x4C:
      return eKeyMap::KEY_L;
    case 0x4D:
      return eKeyMap::KEY_M;
    case 0x4E:
      return eKeyMap::KEY_N;
    case 0x4F:
      return eKeyMap::KEY_O;
    case 0x50:
      return eKeyMap::KEY_P;
    case 0x51:
      return eKeyMap::KEY_Q;
    case 0x52:
      return eKeyMap::KEY_R;
    case 0x53:
      return eKeyMap::KEY_S;
    case 0x54:
      return eKeyMap::KEY_T;
    case 0x55:
      return eKeyMap::KEY_U;
    case 0x56:
      return eKeyMap::KEY_V;
    case 0x57:
      return eKeyMap::KEY_W;
    case 0x58:
      return eKeyMap::KEY_X;
    case 0x59:
      return eKeyMap::KEY_Y;
    case 0x5a:
      return eKeyMap::KEY_Z;

    case VK_F1:
      return eKeyMap::KEY_F1;
    case VK_F2:
      return eKeyMap::KEY_F2;
    case VK_F3:
      return eKeyMap::KEY_F3;
    case VK_F4:
      return eKeyMap::KEY_F4;
    case VK_F5:
      return eKeyMap::KEY_F5;
    case VK_F6:
      return eKeyMap::KEY_F6;
    case VK_F7:
      return eKeyMap::KEY_F7;
    case VK_F8:
      return eKeyMap::KEY_F8;
    case VK_F9:
      return eKeyMap::KEY_F9;
    case VK_F10:
      return eKeyMap::KEY_F10;
    case VK_F11:
      return eKeyMap::KEY_F11;
    case VK_F12:
      return eKeyMap::KEY_F12;
    case VK_F13:
      return eKeyMap::KEY_F13;
    case VK_F14:
      return eKeyMap::KEY_F14;
    case VK_F15:
      return eKeyMap::KEY_F15;
    case VK_F16:
      return eKeyMap::KEY_F16;
    case VK_F17:
      return eKeyMap::KEY_F17;
    case VK_F18:
      return eKeyMap::KEY_F18;
    case VK_F19:
      return eKeyMap::KEY_F19;
    case VK_F20:
      return eKeyMap::KEY_F20;
    case VK_F21:
      return eKeyMap::KEY_F21;
    case VK_F22:
      return eKeyMap::KEY_F22;
    case VK_F23:
      return eKeyMap::KEY_F23;
    case VK_F24:
      return eKeyMap::KEY_F24;

    case VK_LSHIFT:
      return eKeyMap::KEY_LSHIFT;
    case VK_RSHIFT:
      return eKeyMap::KEY_RSHIFT;
      //    case VK_SHIFT:
      //      return eKeyMap::KEY_LSHIFT;
    case VK_LCONTROL:
      return eKeyMap::KEY_LCTRL;
    case VK_RCONTROL:
      return eKeyMap::KEY_RCTRL;
      //    case VK_CONTROL:
      //      return eKeyMap::KEY_LCTRL;
    case VK_LMENU:
      return eKeyMap::KEY_LALT;
    case VK_RMENU:
      return eKeyMap::KEY_RALT;
      //    case VK_MENU:
      //      return eKeyMap::KEY_LALT;

    case VK_OEM_PLUS:
      return eKeyMap::KEY_PLUS;
    case VK_OEM_MINUS:
      return eKeyMap::KEY_MINUS;
    case VK_OEM_COMMA:
      return eKeyMap::KEY_COMMA;
    case VK_OEM_PERIOD:
      return eKeyMap::KEY_PERIOD;
    case VK_OEM_1:
      return eKeyMap::KEY_COLON;
    case VK_OEM_2:
      return eKeyMap::KEY_QUESTION;
    case VK_OEM_3:
      return eKeyMap::KEY_TILDE;
    case VK_OEM_4:
      return eKeyMap::KEY_LBRACKET;
    case VK_OEM_5:
      return eKeyMap::KEY_PIPE;
    case VK_OEM_6:
      return eKeyMap::KEY_RBRACKET;
    case VK_OEM_7:
      return eKeyMap::KEY_QUOTE;

    default:
      return eKeyMap::type_COUNT;
  }
}

Status InitRawInput(const HWND hWnd, Window *pParent) {
  RAWINPUTDEVICE rid[2] = {0};
  rid[0].usUsagePage = 1;
  rid[0].usUsage = 0x04; // Joysticks
  rid[0].dwFlags = 0;
  rid[0].hwndTarget = hWnd;

  rid[1].usUsagePage = 1;
  rid[1].usUsage = 0x05; // Game Pads
  rid[1].dwFlags = 0;
  rid[1].hwndTarget = hWnd;

  RET_SM(
      RegisterRawInputDevices(rid, ARRAY_LENGTH(rid), sizeof(RAWINPUTDEVICE)),
      Status::GENERIC_ERROR,
      "RegisterRawInputDevices() failed.");

  if (!pParent) {
    return Status::OK;
  }
  RawInputMetadata &metadata = pParent->getRawInput();
  memset(metadata.m_inputDevices, 0, sizeof(HANDLE) * eDeviceId::DEVICE_COUNT);

  return Status::OK;
}

Status ShutdownRawInput(const HWND hWnd) {
  RAWINPUTDEVICE rid[2] = {0};
  rid[0].usUsagePage = 1;
  rid[0].usUsage = 0x04; // Joysticks
  rid[0].dwFlags = RIDEV_REMOVE;
  rid[0].hwndTarget = hWnd;

  rid[1].usUsagePage = 1;
  rid[1].usUsage = 0x05; // Game Pads
  rid[1].dwFlags = RIDEV_REMOVE;
  rid[1].hwndTarget = hWnd;

  RET_SM(
      RegisterRawInputDevices(rid, ARRAY_LENGTH(rid), sizeof(RAWINPUTDEVICE)),
      Status::GENERIC_ERROR,
      "RegisterRawInputDevices() failed.");

  return Status::OK;
}

/**
 * If possible, assign each device a unique ID.
 */
eDeviceId::type AssignDevice(const RAWINPUTHEADER header, Window *pParent) {
  const HANDLE hDevice = header.hDevice;
  RawInputMetadata &metadata = pParent->getRawInput();

  if (header.dwType == RIM_TYPEKEYBOARD || header.dwType == RIM_TYPEMOUSE) {
    return eDeviceId::DEVICE_DEFAULT;
  }

  for (s32 i = eDeviceId::DEVICE_CONTROLLER_1; i < eDeviceId::DEVICE_COUNT;
       ++i) {
    if (metadata.m_inputDevices[i] == 0) {
      metadata.m_inputDevices[i] = hDevice;
      return eDeviceId::type(i);
    }
    if (metadata.m_inputDevices[i] == hDevice) {
      return eDeviceId::type(i);
    }
  }
  return eDeviceId::DEVICE_COUNT;
}

static void ParseRawInputKeyboard(
    const PRAWINPUT pRawInput,
    const eDeviceId::type deviceId,
    Window *pParent) {
  const RAWKEYBOARD &device = pRawInput->data.keyboard;
#  if INPUT_ENABLE_DEBUG_OUTPUT
  Log(LL::Info) << " VKKey " << device.VKey << " flags " << device.Flags
                << " message " << device.Message << " code " << device.MakeCode
                << " extra " << device.ExtraInformation << " reserved "
                << device.Reserved;
#  endif
  const eKeyMap::type keyCode = Platform_RemapKeycode(device.VKey);
  const bool isDown = device.Message == WM_KEYDOWN;
  iWindowCallback *pCb = pParent->getCallbackInterface();
  pCb->onButtonInput(deviceId, keyCode, isDown);
}

static void ParseRawInputMouse(
    const PRAWINPUT pRawInput,
    const eDeviceId::type deviceId,
    Window *pParent) {
  const RAWMOUSE &device = pRawInput->data.mouse;
#  if INPUT_ENABLE_DEBUG_OUTPUT
  Log(LL::Info) << " x " << device.lLastX << " y " << device.lLastY
                << " buttons " << device.ulButtons << " rawbuttons "
                << device.ulRawButtons << " buttondata " << device.usButtonData
                << " buttonflag " << device.usButtonFlags << " buttondata "
                << device.usFlags << " extra " << device.ulExtraInformation;
#  endif
}

static void ParseRawInputHID(
    const PRAWINPUT pRawInput,
    const eDeviceId::type deviceId,
    Window *pParent) {
  UINT bufferSize;
  HIDP_CAPS caps;
  PHIDP_PREPARSED_DATA pPreparsedData;
  PHIDP_BUTTON_CAPS pButtonCaps;
  PHIDP_VALUE_CAPS pValueCaps;
  HANDLE hHeap = GetProcessHeap();

  if (GetRawInputDeviceInfo(
          pRawInput->header.hDevice, RIDI_DEVICEINFO, NULL, &bufferSize)
      != 0) {
    return;
  }
  PRID_DEVICE_INFO pDeviceInfo =
      (PRID_DEVICE_INFO) HeapAlloc(hHeap, 0, bufferSize);
  GetRawInputDeviceInfo(
      pRawInput->header.hDevice, RIDI_DEVICEINFO, pDeviceInfo, &bufferSize);

  if (GetRawInputDeviceInfo(
          pRawInput->header.hDevice, RIDI_PREPARSEDDATA, NULL, &bufferSize)
      != 0) {
    return;
  }
  pPreparsedData = (PHIDP_PREPARSED_DATA) HeapAlloc(hHeap, 0, bufferSize);

  // CHECK >= 0
  GetRawInputDeviceInfo(
      pRawInput->header.hDevice,
      RIDI_PREPARSEDDATA,
      pPreparsedData,
      &bufferSize);

  // CHECK == HIDP_STATUS_SUCCESS
  HidP_GetCaps(pPreparsedData, &caps);
  pButtonCaps = (PHIDP_BUTTON_CAPS) HeapAlloc(
      hHeap, 0, sizeof(HIDP_BUTTON_CAPS) * caps.NumberInputButtonCaps);

  USHORT capsLength = caps.NumberInputButtonCaps;
  // CHECK == HIDP_STATUS_SUCCESS
  HidP_GetButtonCaps(HidP_Input, pButtonCaps, &capsLength, pPreparsedData);
  UINT numButtons =
      pButtonCaps->Range.UsageMax - pButtonCaps->Range.UsageMin + 1;

  ULONG usageLength = numButtons;
  USAGE usage[256];
  // CHECK == == HIDP_STATUS_SUCCESS
  HidP_GetUsages(
      HidP_Input,
      pButtonCaps->UsagePage,
      0,
      usage,
      &usageLength,
      pPreparsedData,
      (PCHAR) pRawInput->data.hid.bRawData,
      pRawInput->data.hid.dwSizeHid);

  // Queue up buttons
  for (u32 i = 0; i < usageLength; i++) {
    int buttonId = usage[i] - pButtonCaps->Range.UsageMin;
    wrappers::os::eKeyMap::type key =
        wrappers::os::Platform_RemapControllerButton(
            pDeviceInfo->hid.dwVendorId,
            pDeviceInfo->hid.dwProductId,
            pDeviceInfo->hid.dwVersionNumber,
            buttonId);
    if (key != wrappers::os::eKeyMap::type_COUNT) {
      pParent->getCallbackInterface()->onButtonInput(deviceId, key, true);
    }
  }

  pValueCaps = (PHIDP_VALUE_CAPS) HeapAlloc(
      hHeap, 0, sizeof(HIDP_VALUE_CAPS) * caps.NumberInputValueCaps);
  capsLength = caps.NumberInputValueCaps;
  // CHECK == HIDP_STATUS_SUCCESS
  HidP_GetValueCaps(HidP_Input, pValueCaps, &capsLength, pPreparsedData);

  // Queue in Axis
  for (u32 i = 0; i < caps.NumberInputValueCaps; i++) {
    // CHECK == HIDP_STATUS_SUCCESS
    ULONG value;
    HidP_GetUsageValue(
        HidP_Input,
        pValueCaps[i].UsagePage,
        0,
        pValueCaps[i].Range.UsageMin,
        &value,
        pPreparsedData,
        (PCHAR) pRawInput->data.hid.bRawData,
        pRawInput->data.hid.dwSizeHid);
    int axisId = pValueCaps[i].Range.UsageMin;
    wrappers::os::eAxisMap::type axis =
        wrappers::os::Platform_RemapControllerAxis(
            pDeviceInfo->hid.dwVendorId,
            pDeviceInfo->hid.dwProductId,
            pDeviceInfo->hid.dwVersionNumber,
            axisId);
    if (axis != wrappers::os::eAxisMap::type_COUNT) {
      value = wrappers::os::Platform_RemapControllerAxisRange(
          pDeviceInfo->hid.dwVendorId,
          pDeviceInfo->hid.dwProductId,
          pDeviceInfo->hid.dwVersionNumber,
          axis,
          (int) value);
      pParent->getCallbackInterface()->onAxisInput(deviceId, axis, value);
    } else {
      value = wrappers::os::Platform_RemapControllerAxisRange(
          pDeviceInfo->hid.dwVendorId,
          pDeviceInfo->hid.dwProductId,
          pDeviceInfo->hid.dwVersionNumber,
          (wrappers::os::eAxisMap::type) axisId,
          (int) value);
    }
  }

  HeapFree(hHeap, 0, pPreparsedData);
  HeapFree(hHeap, 0, pButtonCaps);
  HeapFree(hHeap, 0, pValueCaps);
}

void ParseRawInput(const HRAWINPUT hRawInput, Window *pParent) {
  UINT bufferSize;
  GetRawInputData(
      hRawInput, RID_INPUT, NULL, &bufferSize, sizeof(RAWINPUTHEADER));

  HANDLE hHeap = GetProcessHeap();
  PRAWINPUT pRawInput = (PRAWINPUT) HeapAlloc(hHeap, 0, bufferSize);
  CHECK(pRawInput);

  GetRawInputData(
      hRawInput, RID_INPUT, pRawInput, &bufferSize, sizeof(RAWINPUTHEADER));
  const eDeviceId::type deviceId = AssignDevice(pRawInput->header, pParent);
  if (deviceId != eDeviceId::DEVICE_COUNT) {
    if (pRawInput->header.dwType == RIM_TYPEKEYBOARD) {
      ParseRawInputKeyboard(pRawInput, deviceId, pParent);
    } else if (pRawInput->header.dwType == RIM_TYPEMOUSE) {
      ParseRawInputMouse(pRawInput, deviceId, pParent);
    } else if (pRawInput->header.dwType == RIM_TYPEHID) {
      ParseRawInputHID(pRawInput, deviceId, pParent);
    } else {
      CHECK_UNREACHABLE();
    }
  }
  HeapFree(hHeap, 0, pRawInput);
}

/**
 *
 */
void UpdateKeyStates(Window *pParent) {
  ASSERT(pParent);
  iWindowCallback *pCbs = pParent->getCallbackInterface();
  if (!pCbs) {
    return;
  }

  BYTE keyStates[256];
  GetKeyboardState(keyStates);
  for (int i = 0; i < 256; ++i) {
    const eKeyMap::type key = Platform_RemapKeycode(i);
    if (key == eKeyMap::type_COUNT) {
      continue;
    }
    const bool isDown = (keyStates[i] & (1 << 7)) != 0;
    pCbs->onButtonInput(eDeviceId::DEVICE_DEFAULT, key, isDown);
  }
}

} // namespace os
} // namespace wrappers

#endif
