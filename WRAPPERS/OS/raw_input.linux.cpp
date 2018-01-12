#include "raw_input.h"

#include <CORE/types.h>
#include <CORE/ARCH/platform.h>

#include <algorithm>

#ifdef linux
#include <X11/keysym.h>

namespace wrappers {
namespace os {
// Returns the axis code for the os 'ident'
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

// Returns the key code for the os 'ident'
eKeyMap::type Platform_RemapKeycode(int ident) {
  switch (ident) {
    case 0xFFFF0001:
      return eKeyMap::KEY_MOUSE_1;
    case 0xFFFF0002:
      return eKeyMap::KEY_MOUSE_3;
    case 0xFFFF0003:
      return eKeyMap::KEY_MOUSE_2;

    case XK_Escape:
      return eKeyMap::KEY_ESC;
    case XK_BackSpace:
      return eKeyMap::KEY_BACKSPACE;
    case XK_Tab:
      return eKeyMap::KEY_TAB;
    case XK_Pause:
      return eKeyMap::KEY_PAUSE;
    case XK_Sys_Req:
      return eKeyMap::KEY_PRINTSCREEN;
    case XK_space:
      return eKeyMap::KEY_SPACE;

    case XK_A:
      return eKeyMap::KEY_A;
    case XK_B:
      return eKeyMap::KEY_B;
    case XK_C:
      return eKeyMap::KEY_C;
    case XK_D:
      return eKeyMap::KEY_D;
    case XK_E:
      return eKeyMap::KEY_E;
    case XK_F:
      return eKeyMap::KEY_F;
    case XK_G:
      return eKeyMap::KEY_G;
    case XK_H:
      return eKeyMap::KEY_H;
    case XK_I:
      return eKeyMap::KEY_I;
    case XK_J:
      return eKeyMap::KEY_J;
    case XK_K:
      return eKeyMap::KEY_K;
    case XK_L:
      return eKeyMap::KEY_L;
    case XK_M:
      return eKeyMap::KEY_M;
    case XK_N:
      return eKeyMap::KEY_N;
    case XK_O:
      return eKeyMap::KEY_O;
    case XK_P:
      return eKeyMap::KEY_P;
    case XK_Q:
      return eKeyMap::KEY_Q;
    case XK_R:
      return eKeyMap::KEY_R;
    case XK_S:
      return eKeyMap::KEY_S;
    case XK_T:
      return eKeyMap::KEY_T;
    case XK_U:
      return eKeyMap::KEY_U;
    case XK_V:
      return eKeyMap::KEY_V;
    case XK_W:
      return eKeyMap::KEY_W;
    case XK_X:
      return eKeyMap::KEY_X;
    case XK_Y:
      return eKeyMap::KEY_Y;
    case XK_Z:
      return eKeyMap::KEY_Z;

    case XK_a:
      return eKeyMap::KEY_a;
    case XK_b:
      return eKeyMap::KEY_b;
    case XK_c:
      return eKeyMap::KEY_c;
    case XK_d:
      return eKeyMap::KEY_d;
    case XK_e:
      return eKeyMap::KEY_e;
    case XK_f:
      return eKeyMap::KEY_f;
    case XK_g:
      return eKeyMap::KEY_g;
    case XK_h:
      return eKeyMap::KEY_h;
    case XK_i:
      return eKeyMap::KEY_i;
    case XK_j:
      return eKeyMap::KEY_j;
    case XK_k:
      return eKeyMap::KEY_k;
    case XK_l:
      return eKeyMap::KEY_l;
    case XK_m:
      return eKeyMap::KEY_m;
    case XK_n:
      return eKeyMap::KEY_n;
    case XK_o:
      return eKeyMap::KEY_o;
    case XK_p:
      return eKeyMap::KEY_p;
    case XK_q:
      return eKeyMap::KEY_q;
    case XK_r:
      return eKeyMap::KEY_r;
    case XK_s:
      return eKeyMap::KEY_s;
    case XK_t:
      return eKeyMap::KEY_t;
    case XK_u:
      return eKeyMap::KEY_u;
    case XK_v:
      return eKeyMap::KEY_v;
    case XK_w:
      return eKeyMap::KEY_w;
    case XK_x:
      return eKeyMap::KEY_x;
    case XK_y:
      return eKeyMap::KEY_y;
    case XK_z:
      return eKeyMap::KEY_z;

    //TODO(kulseran): Finish key mappings
    /*
    eKeyMap::KEY_0,
    eKeyMap::KEY_1,
    eKeyMap::KEY_2,
    eKeyMap::KEY_3,
    eKeyMap::KEY_4,
    eKeyMap::KEY_5,
    eKeyMap::KEY_6,
    eKeyMap::KEY_7,
    eKeyMap::KEY_8,
    eKeyMap::KEY_9,

    eKeyMap::KEY_NUM_0,
    eKeyMap::KEY_NUM_1,
    eKeyMap::KEY_NUM_2,
    eKeyMap::KEY_NUM_3,
    eKeyMap::KEY_NUM_4,
    eKeyMap::KEY_NUM_5,
    eKeyMap::KEY_NUM_6,
    eKeyMap::KEY_NUM_7,
    eKeyMap::KEY_NUM_8,
    eKeyMap::KEY_NUM_9,
    eKeyMap::KEY_NUM_MUL,
    eKeyMap::KEY_NUM_ADD,
    eKeyMap::KEY_NUM_SUB,
    eKeyMap::KEY_NUM_PERIOD,
    eKeyMap::KEY_NUM_DIV,

    eKeyMap::KEY_PLUS,
    eKeyMap::KEY_MINUS,
    eKeyMap::KEY_LBRACKET,
    eKeyMap::KEY_RBRACKET,
    eKeyMap::KEY_PIPE,
    eKeyMap::KEY_COLON,
    eKeyMap::KEY_PERIOD,
    eKeyMap::KEY_COMMA,
    eKeyMap::KEY_QUOTE,
    eKeyMap::KEY_QUESTION,

    eKeyMap::KEY_TILDE,

    eKeyMap::KEY_SPACE,

    eKeyMap::KEY_HOME,
    eKeyMap::KEY_END,
    eKeyMap::KEY_LEFT,
    eKeyMap::KEY_RIGHT,
    eKeyMap::KEY_UP,
    eKeyMap::KEY_DOWN,
    eKeyMap::KEY_INSERT,
    eKeyMap::KEY_DELETE,
    eKeyMap::KEY_PAGEUP,
    eKeyMap::KEY_PAGEDOWN,

    eKeyMap::KEY_F1,
    eKeyMap::KEY_F2,
    eKeyMap::KEY_F3,
    eKeyMap::KEY_F4,
    eKeyMap::KEY_F5,
    eKeyMap::KEY_F6,
    eKeyMap::KEY_F7,
    eKeyMap::KEY_F8,
    eKeyMap::KEY_F9,
    eKeyMap::KEY_F10,
    eKeyMap::KEY_F11,
    eKeyMap::KEY_F12,
    eKeyMap::KEY_F13,
    eKeyMap::KEY_F14,
    eKeyMap::KEY_F15,
    eKeyMap::KEY_F16,
    eKeyMap::KEY_F17,
    eKeyMap::KEY_F18,
    eKeyMap::KEY_F19,
    eKeyMap::KEY_F20,
    eKeyMap::KEY_F21,
    eKeyMap::KEY_F22,
    eKeyMap::KEY_F23,
    eKeyMap::KEY_F24,

    eKeyMap::KEY_SYS_WIN,
    eKeyMap::KEY_SYS_APP,

    eKeyMap::KEY_LSHIFT,
    eKeyMap::KEY_RSHIFT,
    eKeyMap::KEY_LCTRL,
    eKeyMap::KEY_RCTRL,
    eKeyMap::KEY_LALT,
    eKeyMap::KEY_RALT,
    */

    default: {
      return eKeyMap::type_COUNT;
    }
  }
}
} // namespace os
} // namespace wrappers

#endif
