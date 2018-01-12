#include "virtual_input.h"

#include <algorithm>
#include <map>

#define INPUT_ENABLE_DEBUG_OUTPUT (0)

#if INPUT_ENABLE_DEBUG_OUTPUT
#  include <iostream>
#endif

namespace wrappers {
namespace os {

/**
 *
 */
InputManager::InputManager() {
  resetAll();
}

/**
 *
 */
void InputManager::resetAll(void) {
  for (u32 i = 0; i < eDeviceId::DEVICE_COUNT; ++i) {
    std::fill(m_keysdown[i], m_keysdown[i] + eKeyMap::type_COUNT, false);
    std::fill(m_keyspressed[i], m_keyspressed[i] + eKeyMap::type_COUNT, false);
    std::fill(
        m_keysreleased[i], m_keysreleased[i] + eKeyMap::type_COUNT, false);
    std::fill(m_axispos[i], m_axispos[i] + eAxisMap::type_COUNT, 0);
  }
  m_lastPressedKey = eKeyMap::type_COUNT;
  m_lastPressedDevice = eDeviceId::DEVICE_COUNT;
  m_lastReleasedKey = eKeyMap::type_COUNT;
  m_lastReleasedDevice = eDeviceId::DEVICE_COUNT;
}

/**
 *
 */
void InputManager::resetPress(void) {
  for (u32 i = 0; i < eDeviceId::DEVICE_COUNT; ++i) {
    std::fill(m_keyspressed[i], m_keyspressed[i] + eKeyMap::type_COUNT, false);
    std::fill(
        m_keysreleased[i], m_keysreleased[i] + eKeyMap::type_COUNT, false);
  }
  m_lastPressedKey = eKeyMap::type_COUNT;
  m_lastPressedDevice = eDeviceId::DEVICE_COUNT;
  m_lastReleasedKey = eKeyMap::type_COUNT;
  m_lastReleasedDevice = eDeviceId::DEVICE_COUNT;
}

/**
 *
 */
void InputManager::updateKey(
    eDeviceId::type device, eKeyMap::type ident, bool down) {
  if (down && !m_keysdown[device][ident]) {
    m_keyspressed[device][ident] = true;
    m_lastPressedDevice = device;
    m_lastPressedKey = ident;
  }
  if (!down && m_keysdown[device][ident]) {
    m_keysreleased[device][ident] = true;
    m_lastReleasedDevice = device;
    m_lastReleasedKey = ident;
  }
  m_keysdown[device][ident] = down;
}

/**
 *
 */
void InputManager::updateAxis(
    eDeviceId::type device, eAxisMap::type ident, int pos) {
  m_axispos[device][ident] = pos;
}

/**
 *
 */
static const char *g_keyStrs[] = {
    "UNKNOWN",      "KEY_MOUSE_1",   "KEY_MOUSE_2",     "KEY_MOUSE_3",
    "KEY_MOUSE_4",  "KEY_MOUSE_5",   "KEY_ESC",         "KEY_BACKSPACE",
    "KEY_TAB",      "KEY_PAUSE",     "KEY_PRINTSCREEN", "KEY_ENTER",
    "KEY_A",        "KEY_B",         "KEY_C",           "KEY_D",
    "KEY_E",        "KEY_F",         "KEY_G",           "KEY_H",
    "KEY_I",        "KEY_J",         "KEY_K",           "KEY_L",
    "KEY_M",        "KEY_N",         "KEY_O",           "KEY_P",
    "KEY_Q",        "KEY_R",         "KEY_S",           "KEY_T",
    "KEY_U",        "KEY_V",         "KEY_W",           "KEY_X",
    "KEY_Y",        "KEY_Z",         "KEY_a",           "KEY_b",
    "KEY_c",        "KEY_d",         "KEY_e",           "KEY_f",
    "KEY_g",        "KEY_h",         "KEY_i",           "KEY_j",
    "KEY_k",        "KEY_l",         "KEY_m",           "KEY_n",
    "KEY_o",        "KEY_p",         "KEY_q",           "KEY_r",
    "KEY_s",        "KEY_t",         "KEY_u",           "KEY_v",
    "KEY_w",        "KEY_x",         "KEY_y",           "KEY_z",
    "KEY_0",        "KEY_1",         "KEY_2",           "KEY_3",
    "KEY_4",        "KEY_5",         "KEY_6",           "KEY_7",
    "KEY_8",        "KEY_9",         "KEY_NUM_0",       "KEY_NUM_1",
    "KEY_NUM_2",    "KEY_NUM_3",     "KEY_NUM_4",       "KEY_NUM_5",
    "KEY_NUM_6",    "KEY_NUM_7",     "KEY_NUM_8",       "KEY_NUM_9",
    "KEY_NUM_MUL",  "KEY_NUM_ADD",   "KEY_NUM_SUB",     "KEY_NUM_PERIOD",
    "KEY_NUM_DIV",  "KEY_NUM_ENTER", "KEY_PLUS",        "KEY_MINUS",
    "KEY_LBRACKET", "KEY_RBRACKET",  "KEY_PIPE",        "KEY_COLON",
    "KEY_PERIOD",   "KEY_COMMA",     "KEY_QUOTE",       "KEY_QUESTION",
    "KEY_TILDE",    "KEY_SPACE",     "KEY_HOME",        "KEY_END",
    "KEY_LEFT",     "KEY_RIGHT",     "KEY_UP",          "KEY_DOWN",
    "KEY_INSERT",   "KEY_DELETE",    "KEY_PAGEUP",      "KEY_PAGEDOWN",
    "KEY_F1",       "KEY_F2",        "KEY_F3",          "KEY_F4",
    "KEY_F5",       "KEY_F6",        "KEY_F7",          "KEY_F8",
    "KEY_F9",       "KEY_F10",       "KEY_F11",         "KEY_F12",
    "KEY_F13",      "KEY_F14",       "KEY_F15",         "KEY_F16",
    "KEY_F17",      "KEY_F18",       "KEY_F19",         "KEY_F20",
    "KEY_F21",      "KEY_F22",       "KEY_F23",         "KEY_F24",
    "KEY_SYS_WIN",  "KEY_SYS_APP",   "KEY_LSHIFT",      "KEY_RSHIFT",
    "KEY_LCTRL",    "KEY_RCTRL",     "KEY_LALT",        "KEY_RALT",
    "KEY_JOY_0",    "KEY_JOY_1",     "KEY_JOY_2",       "KEY_JOY_3",
    "KEY_JOY_4",    "KEY_JOY_5",     "KEY_JOY_6",       "KEY_JOY_7",
    "KEY_JOY_8",    "KEY_JOY_9",     "KEY_JOY_10",      "KEY_JOY_11",
    "KEY_JOY_12",   "KEY_JOY_13",    "KEY_JOY_14",      "KEY_JOY_15",
    "KEY_JOY_16",   "KEY_JOY_17",    "KEY_JOY_18",      "KEY_JOY_19",
    "KEY_JOY_20",   "KEY_JOY_21",    "KEY_JOY_22",      "KEY_JOY_23",
    "KEY_JOY_24",   "KEY_JOY_25",    "KEY_JOY_26",      "KEY_JOY_27",
    "KEY_JOY_28",   "KEY_JOY_29",    "KEY_JOY_30",      "KEY_JOY_31",
    "KEY_JOY_32",   "KEY_JOY_33",    "KEY_JOY_34",      "KEY_JOY_35",
    "KEY_JOY_36",   "KEY_JOY_37",    "KEY_JOY_38",      "KEY_JOY_39",
    "KEY_JOY_40",   "KEY_JOY_41",    "KEY_JOY_42",      "KEY_JOY_43",
    "KEY_JOY_44",   "KEY_JOY_45",    "KEY_JOY_46",      "KEY_JOY_47",
    "KEY_JOY_48",   "KEY_JOY_49",    "KEY_JOY_50",

    "<UNKNOWN KEY>" // matches KEY_COUNT
};
COMPILE_TIME_ASSERT(ARRAY_LENGTH_M(g_keyStrs) == (eKeyMap::type_COUNT + 1));

/**
 *
 */
static const char *g_keyTextStrs[] = {
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "\t",
    "",
    "",
    "",
    "a",
    "b",
    "c",
    "d",
    "e",
    "f",
    "g",
    "h",
    "i",
    "j",
    "k",
    "l",
    "m",
    "n",
    "o",
    "p",
    "q",
    "r",
    "s",
    "t",
    "u",
    "v",
    "w",
    "x",
    "y",
    "z",
    "a",
    "b",
    "c",
    "d",
    "e",
    "f",
    "g",
    "h",
    "i",
    "j",
    "k",
    "l",
    "m",
    "n",
    "o",
    "p",
    "q",
    "r",
    "s",
    "t",
    "u",
    "v",
    "w",
    "x",
    "y",
    "z",
    "0",
    "1",
    "2",
    "3",
    "4",
    "5",
    "6",
    "7",
    "8",
    "9",
    "0",
    "1",
    "2",
    "3",
    "4",
    "5",
    "6",
    "7",
    "8",
    "9",
    "*",
    "+",
    "-",
    ".",
    "/",
    "",
    "+",
    "-",
    "[",
    "]",
    "\\",
    ";",
    ".",
    ",",
    "'",
    "/",
    "`",
    " ",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",

    "<UNKNOWN KEY>" // matches COUNT
};
COMPILE_TIME_ASSERT(ARRAY_LENGTH_M(g_keyStrs) == (eKeyMap::type_COUNT + 1));

/**
 *
 */
static const char *g_keyShiftedStrs[] = {
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "\t",
    "",
    "",
    "",
    "A",
    "B",
    "C",
    "D",
    "E",
    "F",
    "G",
    "H",
    "I",
    "J",
    "K",
    "L",
    "M",
    "N",
    "O",
    "P",
    "Q",
    "R",
    "S",
    "T",
    "U",
    "V",
    "W",
    "X",
    "Y",
    "Z",
    "A",
    "B",
    "C",
    "D",
    "E",
    "F",
    "G",
    "H",
    "I",
    "J",
    "K",
    "L",
    "M",
    "N",
    "O",
    "P",
    "Q",
    "R",
    "S",
    "T",
    "U",
    "V",
    "W",
    "X",
    "Y",
    "Z",
    "!",
    "@",
    "#",
    "$",
    "%",
    "^",
    "&",
    "*",
    "(",
    ")",
    "0",
    "1",
    "2",
    "3",
    "4",
    "5",
    "6",
    "7",
    "8",
    "9",
    "*",
    "+",
    "-",
    ".",
    "/",
    "",
    "+",
    "_",
    "{",
    "}",
    "|",
    ":",
    "<",
    ">",
    "\"",
    "?",
    "~",
    " ",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",

    "<UNKNOWN KEY>" // matches COUNT
};
COMPILE_TIME_ASSERT(
    ARRAY_LENGTH_M(g_keyShiftedStrs) == (eKeyMap::type_COUNT + 1));

/**
 *
 */
static const char *g_axisStrs[] = {
    "UNKNOWN",
    "AXIS_MOUSE_X",
    "AXIS_MOUSE_Y",
    "AXIS_MOUSE_Z",
    "AXIS_JOY_X",
    "AXIS_JOY_Y",
    "AXIS_JOY_Z",
    "AXIS_JOY_THROTTLE_1",
    "AXIS_JOY_THROTTLE_2",
    "AXIS_JOY_TRIM_1",
    "AXIS_JOY_TRIM_2",
    "AXIS_JOY_HAT_1",
    "AXIS_JOY_MOUSE_X",
    "AXIS_JOY_MOUSE_Y",

    "<UNKNOWN AXIS>" // matches AXIS_COUNT
};
COMPILE_TIME_ASSERT(ARRAY_LENGTH_M(g_axisStrs) == (eAxisMap::type_COUNT + 1));

/**
 *
 */
const char *GetKeyName(const eKeyMap::type key) {
  return g_keyStrs[key];
}

/**
 *
 */
const char *GetKeyText(const eKeyMap::type key, bool shift) {
  return shift ? g_keyShiftedStrs[key] : g_keyTextStrs[key];
}

/**
 *
 */
const char *GetAxisName(const eAxisMap::type axis) {
  return g_axisStrs[axis];
}

} // namespace os
} // namespace wrappers
