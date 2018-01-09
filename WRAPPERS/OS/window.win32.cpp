#include "window.h"

#include "raw_input.h"
#include <CORE/ARCH/platform.h>
#include <CORE/BASE/checks.h>
#include <CORE/BASE/logging.h>

#if defined(PLAT_WIN32)

#  include <windows.h>

#  include <WindowsX.h>

  // #  include <WindowsX.h>
#  define GLEW_STATIC (1)
#  include <gl/glew.h>
#  include <gl/wglew.h>

#  include <gl/gl.h>

#  include <iostream>

static const char *WINDOW_CLASS_NAME = "FishyWindow";

namespace wrappers {
namespace os {

class Window::Impl {
  public:
  HINSTANCE m_hInstance;

  HWND m_hWnd;
  HDC m_hDC;
  HGLRC m_hGLRC;

  RECT m_clientRect;
  bool m_focus;

  Impl() : m_hWnd(nullptr), m_hDC(nullptr), m_hGLRC(nullptr), m_focus(false) {}
  ~Impl() {
    if (m_hGLRC) {
      wglMakeCurrent(NULL, NULL);
      wglDeleteContext(m_hGLRC);
    }
    if (m_hDC) {
      ReleaseDC(m_hWnd, m_hDC);
    }
    if (m_hWnd) {
      DestroyWindow(m_hWnd);
    }
    ShowCursor(true);
  }
};

/**
 *
 */
Window::Window(
    const char *pTitle,
    const DisplayCaps &displaySettings,
    iWindowCallback *pCallbacks)
    : m_pImpl(nullptr),
      m_settings(displaySettings),
      m_cbs(pCallbacks),
      m_title(pTitle) {
  if (pTitle == nullptr) {
    pTitle = "<unnamed app>";
  }
  if (m_cbs) {
    m_cbs->setOwner(this);
  }
}

/**
 *
 */
Window::~Window() {
  destroy();
}

/**
 *
 */
Status Window::reset() {
  destroy();
  if (!create()) {
    return Status::GENERIC_ERROR;
  }
  return Status::OK;
}

/**
 *
 */
LRESULT CALLBACK
WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
  Window *pParent = (Window *) GetWindowLongPtr(hWnd, GWLP_USERDATA);
  switch (message) {
    case WM_CREATE: {
      pParent = (Window *) (((LPCREATESTRUCT) lParam)->lpCreateParams);
      if (!pParent) {
        break;
      }
      SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR) pParent);

      if (pParent->getCallbackInterface()) {
        pParent->getCallbackInterface()->onInit();
      }

      InitRawInput(hWnd, pParent).ignoreErrors();
      break;
    }

    case WM_DESTROY: {
      if (!pParent) {
        break;
      }
      if (pParent->getCallbackInterface()) {
        pParent->getCallbackInterface()->onDest();
      }

      ShutdownRawInput(hWnd).ignoreErrors();
      break;
    }

    case WM_SETFOCUS: {
      if (!pParent) {
        break;
      }
      if (pParent->getImpl()) {
        pParent->getImpl()->m_focus = true;
      }
      if (pParent->getCallbackInterface()) {
        pParent->getCallbackInterface()->onShow();
      }
      ShowCursor(!pParent->getSettings().m_hideCursor);
      break;
    }

    case WM_KILLFOCUS: {
      if (!pParent) {
        break;
      }
      if (pParent->getImpl()) {
        pParent->getImpl()->m_focus = false;
      }
      if (pParent->getCallbackInterface()) {
        pParent->getCallbackInterface()->onHide();
        // All keys get a 'release' message on loss of focus
        for (int i = 0; i < 255; ++i) {
          for (int j = 0; j < eDeviceId::DEVICE_COUNT; ++j) {
            pParent->getCallbackInterface()->onButtonInput(
                eDeviceId::type(j), eKeyMap::type(i), false);
          }
        }
      }
      ShowCursor(true);
      break;
    }

    case WM_SIZE: {
      if (!pParent) {
        break;
      }
      DisplayCaps settings = pParent->getSettings();
      settings.m_height = HIWORD(lParam);
      settings.m_width = LOWORD(lParam);
      pParent->setSettings(settings);
      glViewport(0, 0, settings.m_width, settings.m_height);
      if (pParent->getCallbackInterface()) {
        pParent->getCallbackInterface()->onReshape();
      }
      break;
    }

    case WM_SYSCOMMAND: {
      if (!pParent) {
        break;
      }
      // ignore 'alt' menu
      if (wParam == SC_KEYMENU && (lParam >> 16) <= 0) {
        return 0;
      }
      break;
    }

    case WM_INPUT: {
      ParseRawInput((HRAWINPUT) lParam, pParent);
      break;
    }

    case WM_MOUSEMOVE: {
      if (pParent->getCallbackInterface()) {
        LONG xPos = GET_X_LPARAM(lParam);
        LONG yPos = GET_Y_LPARAM(lParam);
        pParent->getCallbackInterface()->onAxisInput(
            eDeviceId::DEVICE_DEFAULT, eAxisMap::AXIS_MOUSE_X, xPos);
        pParent->getCallbackInterface()->onAxisInput(
            eDeviceId::DEVICE_DEFAULT, eAxisMap::AXIS_MOUSE_Y, yPos);
      }
      break;
    }

    case WM_MOUSEWHEEL: {
      if (pParent->getCallbackInterface()) {
        LONG zPos = GET_WHEEL_DELTA_WPARAM(wParam);
        static LONG zTotalPos = 0;
        zTotalPos += zPos;
        pParent->getCallbackInterface()->onAxisInput(
            eDeviceId::DEVICE_DEFAULT, eAxisMap::AXIS_MOUSE_Z, zTotalPos);
      }
    }
  }
  return DefWindowProc(hWnd, message, wParam, lParam);
}

ATOM RegisterWindowClass(HINSTANCE hInstance) {
  WNDCLASSEX windowClass;
  memset(&windowClass, 0, sizeof(WNDCLASSEX));
  windowClass.cbSize = sizeof(WNDCLASSEX);

  windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
  windowClass.lpfnWndProc = (WNDPROC) WndProc;
  windowClass.cbClsExtra = 0;
  windowClass.cbWndExtra = 0;
  windowClass.hInstance = hInstance;
  windowClass.hIcon = LoadIcon(NULL, IDI_WINLOGO);
  windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
  windowClass.hbrBackground = NULL;
  windowClass.lpszMenuName = NULL;
  windowClass.lpszClassName = WINDOW_CLASS_NAME;
  return RegisterClassEx(&windowClass);
}

/**
 * Get a pixel format for generating an OpenGL context.
 */
Status SetDefaultPixelFormat(const HDC hDC) {
  PIXELFORMATDESCRIPTOR pfd;
  memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
  pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);

  pfd.nVersion = 1;

  // Enable double buffering, opengl support and drawing to a window
  pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;

  // 32bit RGBA color
  pfd.iPixelType = PFD_TYPE_RGBA;
  pfd.cColorBits = 32;

  // 32bit depth buffer
  pfd.cDepthBits = 24;
  pfd.cStencilBits = 8;

  pfd.iLayerType = PFD_MAIN_PLANE;

  const int pfdId = ChoosePixelFormat(hDC, &pfd);
  RET_SM(pfdId != 0, Status::GENERIC_ERROR, "ChoosePixelFormat() failed.");

  RET_SM(
      SetPixelFormat(hDC, pfdId, &pfd) == TRUE,
      Status::GENERIC_ERROR,
      "SetPixelFormat() failed.");

  return Status::OK;
}

/**
 * Get a pixel format compatible with the requested core opengl context.
 */
Status SetCorePixelFormat(const HDC hDC) {
  PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = nullptr;
  wglChoosePixelFormatARB = reinterpret_cast< PFNWGLCHOOSEPIXELFORMATARBPROC >(
      wglGetProcAddress("wglChoosePixelFormatARB"));
  RET_SM(
      wglChoosePixelFormatARB != nullptr,
      Status::GENERIC_ERROR,
      "wglGetProcAddress() failed to find wglChoosePixelFormatARB");

  const int pixelAttribs[] = {WGL_DRAW_TO_WINDOW_ARB,
                              GL_TRUE,
                              WGL_SUPPORT_OPENGL_ARB,
                              GL_TRUE,
                              WGL_DOUBLE_BUFFER_ARB,
                              GL_TRUE,
                              WGL_PIXEL_TYPE_ARB,
                              WGL_TYPE_RGBA_ARB,
                              WGL_ACCELERATION_ARB,
                              WGL_FULL_ACCELERATION_ARB,
                              WGL_COLOR_BITS_ARB,
                              32,
                              WGL_ALPHA_BITS_ARB,
                              8,
                              WGL_DEPTH_BITS_ARB,
                              24,
                              WGL_STENCIL_BITS_ARB,
                              8,
                              0};
  UINT numFormats;
  int pfdId;
  bool status =
      wglChoosePixelFormatARB(hDC, pixelAttribs, NULL, 1, &pfdId, &numFormats);

  RET_SM(
      status && numFormats > 0,
      Status::GENERIC_ERROR,
      "wglChoosePixelFormatARB() failed.");

  PIXELFORMATDESCRIPTOR pfd;
  RET_SM(
      DescribePixelFormat(hDC, pfdId, sizeof(pfd), &pfd) != 0,
      Status::GENERIC_ERROR,
      "DescribePixelFormat() failed.");

  RET_SM(
      SetPixelFormat(hDC, pfdId, &pfd),
      Status::GENERIC_ERROR,
      "SetPixelFormat() failed.");

  return Status::OK;
}

Status Window::create() {
  Trace();
  std::unique_ptr< Impl > pImpl = std::make_unique< Impl >();

  pImpl->m_hInstance = GetModuleHandle(NULL);
  ATOM windowClass = RegisterWindowClass(pImpl->m_hInstance);
  RET_SM(
      MAKEINTATOM(windowClass) != 0,
      Status::GENERIC_ERROR,
      "Could not RegisterCLassEx()");

  const DWORD style = m_settings.m_fullScreen ? WS_POPUP : WS_OVERLAPPEDWINDOW;
  {
    std::unique_ptr< Impl > pFakeImpl = std::make_unique< Impl >();
    pFakeImpl->m_hWnd = CreateWindow(
        MAKEINTATOM(windowClass),
        "Fake Window",
        style,
        0,    // x
        0,    // y
        1,    // width
        1,    // height
        NULL, // parent window
        NULL, // menu
        pImpl->m_hInstance,
        NULL // user data
    );
    RET_SM(pFakeImpl->m_hWnd, Status::GENERIC_ERROR, "CreateWindow() failed.");

    pFakeImpl->m_hDC = GetDC(pFakeImpl->m_hWnd); // Device Context

    if (!SetDefaultPixelFormat(pFakeImpl->m_hDC)) {
      return Status::GENERIC_ERROR;
    }

    pFakeImpl->m_hGLRC = wglCreateContext(pFakeImpl->m_hDC);

    RET_SM(
        pFakeImpl->m_hGLRC, Status::GENERIC_ERROR, "wglCreateContex() failed.");

    RET_SM(
        wglMakeCurrent(pFakeImpl->m_hDC, pFakeImpl->m_hGLRC),
        Status::GENERIC_ERROR,
        "wglMakeCurrent() failed.");

    const DWORD dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
    u32 height = m_settings.m_height;
    u32 width = m_settings.m_width;
    if (!m_settings.m_fullScreen) {
      RECT r;
      r.top = 0;
      r.bottom = height;
      r.left = 0;
      r.right = width;
      AdjustWindowRect(&r, style, FALSE);
      height = r.bottom - r.top;
      width = r.right - r.left;
    }

    pImpl->m_hWnd = CreateWindowEx(
        dwExStyle,
        MAKEINTATOM(windowClass),
        m_title,
        style,
        CW_USEDEFAULT,
        0,
        width,
        height,
        NULL,
        NULL,
        pImpl->m_hInstance,
        this);
    RET_SM(pImpl->m_hWnd, Status::GENERIC_ERROR, "CreateWindowEx() failed.");

    GetClientRect(pImpl->m_hWnd, &pImpl->m_clientRect);

    pImpl->m_hDC = GetDC(pImpl->m_hWnd);

    if (!SetCorePixelFormat(pImpl->m_hDC)) {
      return Status::GENERIC_ERROR;
    }

    // Create final context
    PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = nullptr;
    wglCreateContextAttribsARB =
        reinterpret_cast< PFNWGLCREATECONTEXTATTRIBSARBPROC >(
            wglGetProcAddress("wglCreateContextAttribsARB"));
    if (wglCreateContextAttribsARB == nullptr) {
      Log(LL::Error)
          << "wglGetProcAddress() failed for wglCreateContextAttribsARB.";
      return Status::GENERIC_ERROR;
    }

    const int contextAttribs[] = {
      WGL_CONTEXT_MAJOR_VERSION_ARB,
      4,
      WGL_CONTEXT_MINOR_VERSION_ARB,
      5,
      WGL_CONTEXT_PROFILE_MASK_ARB,
      WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
#  if defined(FISHY_DEBUG)
      WGL_CONTEXT_FLAGS_ARB,
      WGL_CONTEXT_DEBUG_BIT_ARB,
#  endif
      0
    };

    pImpl->m_hGLRC =
        wglCreateContextAttribsARB(pImpl->m_hDC, 0, contextAttribs);
    RET_SM(
        pImpl->m_hGLRC,
        Status::GENERIC_ERROR,
        "wglCreateContextAttribsARB() failed.");
  }

  RET_SM(
      wglMakeCurrent(pImpl->m_hDC, pImpl->m_hGLRC),
      Status::GENERIC_ERROR,
      "wglMakeCurrent() failed.");

  // Init GLEW
  GLenum error = glewInit();
  RET_SM(error == GLEW_OK, Status::GENERIC_ERROR, "Failed to init GLEW");

  Log(LL::Info) << "OpenGL mode "
                << (m_settings.m_fullScreen ? "fullscreen" : "windowed");

  int majorRev, minorRev;
  glGetIntegerv(GL_MAJOR_VERSION, &majorRev);
  glGetIntegerv(GL_MINOR_VERSION, &minorRev);
  Log(LL::Info) << "OpenGL version " << majorRev << "." << minorRev;
  Log(LL::Info) << "OpenGL GLEW Support GL_VERSION_1_2: "
                << (glewIsSupported("GL_VERSION_1_2") ? "true" : "false");
  Log(LL::Info) << "OpenGL GLEW Support GL_VERSION_2_0: "
                << (glewIsSupported("GL_VERSION_2_0") ? "true" : "false");
  Log(LL::Info) << "OpenGL GLEW Support GL_VERSION_3_0: "
                << (glewIsSupported("GL_VERSION_3_0") ? "true" : "false");

  // Finalize
  m_pImpl = pImpl.release();
  ShowWindow(m_pImpl->m_hWnd, SW_SHOW);
  SetFocus(m_pImpl->m_hWnd);
  UpdateWindow(m_pImpl->m_hWnd);
  ShowCursor(!m_settings.m_hideCursor);

  return Status::OK;
}

void Window::destroy() {
  Trace();
  delete m_pImpl;
  m_pImpl = nullptr;
}

void Window::framestep() {
  ASSERT(m_pImpl);

  MSG msg;
  while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  UpdateKeyStates(this);

  SwapBuffers(m_pImpl->m_hDC);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  if (m_cbs) {
    m_cbs->onUpdate();
  }
  if (m_settings.m_lockCursor && m_pImpl->m_focus) {
    RECT r;
    GetWindowRect(m_pImpl->m_hWnd, &r);
    ClipCursor(&r);
  }
}

} // namespace os
} // namespace wrappers

#endif
