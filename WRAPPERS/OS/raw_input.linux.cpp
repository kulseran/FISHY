/**
 * raw_input.linux.cpp
 *
 * linux implementation of raw_input.h
 */
#include "raw_input.h"

#include <CORE/types.h>
#include <CORE/ARCH/platform.h>

#include <algorithm>

#ifdef linux
#include <X11/keysym.h>

namespace core {
namespace input {
// Returns the axis code for the os 'ident'
eAxisMap::type Platform_RemapAxisIdent(int ident) {
  switch (ident) {
    case 0:
      return AXIS_MOUSE_X;
    case 1:
      return AXIS_MOUSE_Y;
    case 2:
      return AXIS_MOUSE_Z;
  }

  return AXIS_COUNT;
}

// Returns the key code for the os 'ident'
eKeyMap::type Platform_RemapKeycode(int ident) {
  switch (ident) {
    case 0xFFFF0001:
      return KEY_MOUSE_1;
    case 0xFFFF0002:
      return KEY_MOUSE_3;
    case 0xFFFF0003:
      return KEY_MOUSE_2;

    case XK_Escape:
      return KEY_ESC;
    case XK_BackSpace:
      return KEY_BACKSPACE;
    case XK_Tab:
      return KEY_TAB;
    case XK_Pause:
      return KEY_PAUSE;
    case XK_Sys_Req:
      return KEY_PRINTSCREEN;
    case XK_space:
      return KEY_SPACE;

    case XK_A:
      return KEY_A;
    case XK_B:
      return KEY_B;
    case XK_C:
      return KEY_C;
    case XK_D:
      return KEY_D;
    case XK_E:
      return KEY_E;
    case XK_F:
      return KEY_F;
    case XK_G:
      return KEY_G;
    case XK_H:
      return KEY_H;
    case XK_I:
      return KEY_I;
    case XK_J:
      return KEY_J;
    case XK_K:
      return KEY_K;
    case XK_L:
      return KEY_L;
    case XK_M:
      return KEY_M;
    case XK_N:
      return KEY_N;
    case XK_O:
      return KEY_O;
    case XK_P:
      return KEY_P;
    case XK_Q:
      return KEY_Q;
    case XK_R:
      return KEY_R;
    case XK_S:
      return KEY_S;
    case XK_T:
      return KEY_T;
    case XK_U:
      return KEY_U;
    case XK_V:
      return KEY_V;
    case XK_W:
      return KEY_W;
    case XK_X:
      return KEY_X;
    case XK_Y:
      return KEY_Y;
    case XK_Z:
      return KEY_Z;

    case XK_a:
      return KEY_a;
    case XK_b:
      return KEY_b;
    case XK_c:
      return KEY_c;
    case XK_d:
      return KEY_d;
    case XK_e:
      return KEY_e;
    case XK_f:
      return KEY_f;
    case XK_g:
      return KEY_g;
    case XK_h:
      return KEY_h;
    case XK_i:
      return KEY_i;
    case XK_j:
      return KEY_j;
    case XK_k:
      return KEY_k;
    case XK_l:
      return KEY_l;
    case XK_m:
      return KEY_m;
    case XK_n:
      return KEY_n;
    case XK_o:
      return KEY_o;
    case XK_p:
      return KEY_p;
    case XK_q:
      return KEY_q;
    case XK_r:
      return KEY_r;
    case XK_s:
      return KEY_s;
    case XK_t:
      return KEY_t;
    case XK_u:
      return KEY_u;
    case XK_v:
      return KEY_v;
    case XK_w:
      return KEY_w;
    case XK_x:
      return KEY_x;
    case XK_y:
      return KEY_y;
    case XK_z:
      return KEY_z;

    //TODO(kulseran): Finish key mappings
    /*
    KEY_0,
    KEY_1,
    KEY_2,
    KEY_3,
    KEY_4,
    KEY_5,
    KEY_6,
    KEY_7,
    KEY_8,
    KEY_9,

    KEY_NUM_0,
    KEY_NUM_1,
    KEY_NUM_2,
    KEY_NUM_3,
    KEY_NUM_4,
    KEY_NUM_5,
    KEY_NUM_6,
    KEY_NUM_7,
    KEY_NUM_8,
    KEY_NUM_9,
    KEY_NUM_MUL,
    KEY_NUM_ADD,
    KEY_NUM_SUB,
    KEY_NUM_PERIOD,
    KEY_NUM_DIV,

    KEY_PLUS,
    KEY_MINUS,
    KEY_LBRACKET,
    KEY_RBRACKET,
    KEY_PIPE,
    KEY_COLON,
    KEY_PERIOD,
    KEY_COMMA,
    KEY_QUOTE,
    KEY_QUESTION,

    KEY_TILDE,

    KEY_SPACE,

    KEY_HOME,
    KEY_END,
    KEY_LEFT,
    KEY_RIGHT,
    KEY_UP,
    KEY_DOWN,
    KEY_INSERT,
    KEY_DELETE,
    KEY_PAGEUP,
    KEY_PAGEDOWN,

    KEY_F1,
    KEY_F2,
    KEY_F3,
    KEY_F4,
    KEY_F5,
    KEY_F6,
    KEY_F7,
    KEY_F8,
    KEY_F9,
    KEY_F10,
    KEY_F11,
    KEY_F12,
    KEY_F13,
    KEY_F14,
    KEY_F15,
    KEY_F16,
    KEY_F17,
    KEY_F18,
    KEY_F19,
    KEY_F20,
    KEY_F21,
    KEY_F22,
    KEY_F23,
    KEY_F24,

    KEY_SYS_WIN,
    KEY_SYS_APP,

    KEY_LSHIFT,
    KEY_RSHIFT,
    KEY_LCTRL,
    KEY_RCTRL,
    KEY_LALT,
    KEY_RALT,
    */

    default: {
      return KEY_COUNT;
    }
  }
}
} // namespace input
} // namespace core

#endif
