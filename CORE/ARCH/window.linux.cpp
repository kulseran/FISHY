/**
 * window.linux.cpp
 *
 * linux implementation of window.h
 */
#include "window.h"

#include <CORE/ARCH/platform.h>

#if defined(PLAT_LINUX)
#include <CORE/fsdefs.h>
#include <CORE/SYSTEM/logging.h>
using logging::LL;

#define GLEW_STATIC (1)
#include <GL/glew.h>
#include <GL/glx.h>
#include <GL/gl.h>

#include <X11/extensions/xf86vmode.h>
#include <X11/keysym.h>

static bool ctxErrorOccurred = false;
static int ctxErrorHandler(Display *dpy, XErrorEvent *ev) {
  (void)dpy;
  (void)ev;
  ctxErrorOccurred = true;
  return 0;
}


namespace fsengine {

struct WindowData {
  Display         *pDisplay;
  int           screen;
  ::Window        win;
  GLXContext        ctx;
  XSetWindowAttributes  attr;
  XF86VidModeModeInfo   deskMode;
  int           x, y;
  Colormap        cmap;
};

Window::Window() :
  m_opaqueWindowData(new WindowData),
  m_cbs(NULL) {
  m_settings.m_width = 1024;
  m_settings.m_height = 768;
  m_settings.m_fullScreen = false;
  m_settings.m_pixelDepth = 32;
  m_title = "<untitled app>";

  m_initalized = false;
}

Window::~Window() {
  if (m_initalized) {
    destroy();
  }
}

bool Window::create() {
  ASSERT(!m_initalized);

  // Open the X Display
  m_opaqueWindowData->pDisplay = XOpenDisplay(0);
  if (!m_opaqueWindowData->pDisplay) {
    Log(LL::Error) << "XOpenDisplay(0) failed!" << std::endl;
    return false;
  }
  m_opaqueWindowData->screen = DefaultScreen(m_opaqueWindowData->pDisplay);

  // Check the supported version (must be atleast 1.3)
  int glx_major, glx_minor;
  if (!glXQueryVersion(m_opaqueWindowData->pDisplay, &glx_major, &glx_minor) || ((glx_major == 1) && (glx_minor < 3)) || (glx_major < 1)) {
    Log(LL::Error) << "Invalid GLX version" << std::endl;
    return false;
  }

  // Get a matching FB config
  static int visual_attribs[] = {
    GLX_X_RENDERABLE    , True,
    GLX_DRAWABLE_TYPE   , GLX_WINDOW_BIT,
    GLX_RENDER_TYPE     , GLX_RGBA_BIT,
    GLX_X_VISUAL_TYPE   , GLX_TRUE_COLOR,
    GLX_RED_SIZE        , 8,
    GLX_GREEN_SIZE      , 8,
    GLX_BLUE_SIZE       , 8,
    GLX_ALPHA_SIZE      , 8,
    GLX_DEPTH_SIZE      , 24,
    GLX_STENCIL_SIZE    , 8,
    GLX_DOUBLEBUFFER    , True,
    //GLX_SAMPLE_BUFFERS  , 1,
    //GLX_SAMPLES         , 4,
    None
  };

  int fbcount;
  GLXFBConfig *fbc = glXChooseFBConfig(m_opaqueWindowData->pDisplay, m_opaqueWindowData->screen, visual_attribs, &fbcount);
  if (!fbc) {
    Log(LL::Error) << "Failed to retrieve a framebuffer config" << std::endl;
    return false;
  }

  // Pick the FB config/visual with the most samples per pixel
  int best_fbc = -1, worst_fbc = -1, best_num_samp = -1, worst_num_samp = 999;
  for (int i = 0; i < fbcount; i++) {
    XVisualInfo *vi = glXGetVisualFromFBConfig(m_opaqueWindowData->pDisplay, fbc[i]);
    if (vi) {
      int samp_buf, samples;
      glXGetFBConfigAttrib(m_opaqueWindowData->pDisplay, fbc[i], GLX_SAMPLE_BUFFERS, &samp_buf);
      glXGetFBConfigAttrib(m_opaqueWindowData->pDisplay, fbc[i], GLX_SAMPLES       , &samples);

      if ((best_fbc < 0) || (samp_buf && samples > best_num_samp)) {
        best_fbc = i, best_num_samp = samples;
      }
      if ((worst_fbc < 0) || (!samp_buf || samples < worst_num_samp)) {
        worst_fbc = i, worst_num_samp = samples;
      }
    }
    XFree(vi);
  }
  GLXFBConfig bestFbc = fbc[ best_fbc ];
  XFree(fbc);

  // Get a visual
  XVisualInfo *vi = glXGetVisualFromFBConfig(m_opaqueWindowData->pDisplay, bestFbc);
  if (!vi) {
    Log(LL::Error) << "glXChooseVisual Failed" << std::endl;
  }

  // Create a color map
  m_opaqueWindowData->cmap = XCreateColormap(m_opaqueWindowData->pDisplay, RootWindow(m_opaqueWindowData->pDisplay, vi->screen), vi->visual, AllocNone);
  m_opaqueWindowData->attr.colormap = m_opaqueWindowData->cmap;
  m_opaqueWindowData->attr.border_pixel = 0;
  m_opaqueWindowData->attr.background_pixmap = None;
  m_opaqueWindowData->attr.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | StructureNotifyMask | PointerMotionMask;

  // Set the display mode
  unsigned long window_valuemask = 0;
  if (m_settings.m_fullScreen) {
    int modeCount = 0;
    XF86VidModeModeInfo **modes;
    XF86VidModeGetAllModeLines(m_opaqueWindowData->pDisplay, m_opaqueWindowData->screen, &modeCount, &modes);
    m_opaqueWindowData->deskMode = *modes[0];

    int bestMode = 0;
    for (int i = 0; i < modeCount; i++) {
      if ((modes[i]->hdisplay == m_settings.m_width) && (modes[i]->vdisplay == m_settings.m_height)) {
        bestMode = i;
      }
    }

    XF86VidModeSwitchToMode(m_opaqueWindowData->pDisplay, m_opaqueWindowData->screen, modes[bestMode]);
    XF86VidModeSetViewPort(m_opaqueWindowData->pDisplay, m_opaqueWindowData->screen, 0, 0);
    m_settings.m_width = modes[bestMode]->hdisplay;
    m_settings.m_height = modes[bestMode]->vdisplay;
    XFree(modes);

    m_opaqueWindowData->attr.override_redirect = True;


    window_valuemask = CWBorderPixel | CWColormap | CWEventMask | CWOverrideRedirect;
  } else {
    window_valuemask = CWBorderPixel | CWColormap | CWEventMask;
  }

  // Create the window
  m_opaqueWindowData->win = XCreateWindow(m_opaqueWindowData->pDisplay, RootWindow(m_opaqueWindowData->pDisplay, vi->screen), 0, 0, m_settings.m_width, m_settings.m_height, 0, vi->depth, InputOutput, vi->visual, window_valuemask, &m_opaqueWindowData->attr);
  if (!m_opaqueWindowData->win) {
    Log(LL::Error) << "XCreateWindow() failed!" << std::endl;
    return false;
  }

  XMapWindow(m_opaqueWindowData->pDisplay, m_opaqueWindowData->win);

  // Setup Mode data for the window
  if (m_settings.m_fullScreen) {
    XWarpPointer(m_opaqueWindowData->pDisplay, None, m_opaqueWindowData->win, 0, 0, 0, 0, 0, 0);
    XMapRaised(m_opaqueWindowData->pDisplay, m_opaqueWindowData->win);
    XGrabKeyboard(m_opaqueWindowData->pDisplay, m_opaqueWindowData->win, True, GrabModeAsync, GrabModeAsync, CurrentTime);
    XGrabPointer(m_opaqueWindowData->pDisplay, m_opaqueWindowData->win, True, ButtonPressMask, GrabModeAsync, GrabModeAsync, m_opaqueWindowData->win, None, CurrentTime);
  } else {
    Atom wmDelete = XInternAtom(m_opaqueWindowData->pDisplay, "WM_DELETE_WINDOW", True);
    XSetWMProtocols(m_opaqueWindowData->pDisplay, m_opaqueWindowData->win, &wmDelete, 1);
    XSetStandardProperties(m_opaqueWindowData->pDisplay, m_opaqueWindowData->win, m_title, m_title, None, NULL, 0, NULL);
    XMapRaised(m_opaqueWindowData->pDisplay, m_opaqueWindowData->win);
  }

  ::Window winDummy;
  unsigned int borderDummy;


  // Set the event handler
  ctxErrorOccurred = false;
  int (*oldHandler)(Display *, XErrorEvent *) = XSetErrorHandler(&ctxErrorHandler);

  m_opaqueWindowData->ctx = glXCreateContext(m_opaqueWindowData->pDisplay, vi, 0, GL_TRUE);
  XSync(m_opaqueWindowData->pDisplay, False);
  XSetErrorHandler(oldHandler);

  if (ctxErrorOccurred || !m_opaqueWindowData->ctx) {
    Log(LL::Error) << "Failed to create OpenGL Context" << std::endl;
    return false;
  }
  XFree(vi);

  // Make the context current
  glXMakeCurrent(m_opaqueWindowData->pDisplay, m_opaqueWindowData->win, m_opaqueWindowData->ctx);
  GLenum error = glewInit();
  if (error != GLEW_OK) {
    Log(LL::Error) << "Failed to init GLEW" << std::endl;
    return false;
  }

  // Get all the info about our setup
  XGetGeometry(m_opaqueWindowData->pDisplay, m_opaqueWindowData->win, &winDummy, &m_opaqueWindowData->x, &m_opaqueWindowData->y, &m_settings.m_width, &m_settings.m_height, &borderDummy, &m_settings.m_pixelDepth);

  Log(LL::Info) << "OpenGL " << (glXIsDirect(m_opaqueWindowData->pDisplay, m_opaqueWindowData->ctx) ? "is" : "is not") << " Direct Rendering" << std::endl;
  Log(LL::Info) << "OpenGL mode " << (m_settings.m_fullScreen ? "fullscreen" : "windowed") << std::endl;

  int majorRev, minorRev;
  glGetIntegerv(GL_MAJOR_VERSION, &majorRev);
  glGetIntegerv(GL_MINOR_VERSION, &minorRev);
  Log(LL::Info) << "OpenGL version " << majorRev << "." << minorRev << std::endl;
  Log(LL::Info) << "OpenGL GLEW Support GL_VERSION_1_2: " << (glewIsSupported("GL_VERSION_1_2") ? "true" : "false") << std::endl;
  Log(LL::Info) << "OpenGL GLEW Support GL_VERSION_2_0: " << (glewIsSupported("GL_VERSION_2_0") ? "true" : "false") << std::endl;
  Log(LL::Info) << "OpenGL GLEW Support GL_VERSION_3_0: " << (glewIsSupported("GL_VERSION_3_0") ? "true" : "false") << std::endl;


  m_initalized = true;

  if (m_cbs) {
    m_cbs->on_init();
  }

  return true;
}

bool Window::destroy() {
  ASSERT(m_initalized);
  if (m_opaqueWindowData->ctx) {
    if (!glXMakeCurrent(m_opaqueWindowData->pDisplay, None, NULL)) {
      //printf("Error releasing drawing context : killGLWindow\n");
    }
    glXDestroyContext(m_opaqueWindowData->pDisplay, m_opaqueWindowData->ctx);
    m_opaqueWindowData->ctx = NULL;
  }

  if (m_settings.m_fullScreen) {
    XF86VidModeSwitchToMode(m_opaqueWindowData->pDisplay, m_opaqueWindowData->screen, &m_opaqueWindowData->deskMode);
    XF86VidModeSetViewPort(m_opaqueWindowData->pDisplay, m_opaqueWindowData->screen, 0, 0);
  }
  XDestroyWindow(m_opaqueWindowData->pDisplay, m_opaqueWindowData->win);
  XFreeColormap(m_opaqueWindowData->pDisplay, m_opaqueWindowData->cmap);
  XCloseDisplay(m_opaqueWindowData->pDisplay);

  m_initalized = false;
  return true;
}

bool Window::reset() {
  if (m_initalized) {
    if (!destroy()) {
      return false;
    }

    if (!create()) {
      return false;
    }
  }
  return true;
}

void Window::framestep() {
  XEvent event;
  while (XPending(m_opaqueWindowData->pDisplay) > 0) {
    XNextEvent(m_opaqueWindowData->pDisplay, &event);
    switch (event.type) {
      case Expose: {
        if (event.xexpose.count != 0) {
          break;
        }
        break;
      }

      case ConfigureNotify: {
        if ((event.xconfigure.width != (signed)m_settings.m_width)
            || (event.xconfigure.height != (signed)m_settings.m_height)) {
          m_settings.m_width = event.xconfigure.width;
          m_settings.m_height = event.xconfigure.height;
          glViewport(0, 0, m_settings.m_width, m_settings.m_height);
          if (m_cbs) { m_cbs->on_reshape(); }
        }
        break;
      }

      case ButtonPress: {
        if (m_cbs) {
          m_cbs->on_buttoninput(0xFFFF0000 + event.xbutton.button, true);
        }
        break;
      }

      case ButtonRelease: {
        if (m_cbs) {
          m_cbs->on_buttoninput(0xFFFF0000 + event.xbutton.button, false);
        }
        break;
      }

      case MotionNotify: {
        if (m_cbs) {
          m_cbs->on_axisinput(0, event.xmotion.x);
          m_cbs->on_axisinput(1, event.xmotion.y);
        }
        break;
      }

      case KeyPress: {
        if (m_cbs) {
          m_cbs->on_buttoninput(XLookupKeysym(&event.xkey, 0), true);
        }
        break;
      }

      case KeyRelease: {
        if (m_cbs) {
          m_cbs->on_buttoninput(XLookupKeysym(&event.xkey, 0), false);
        }
        break;
      }

      case ClientMessage: {
        if (*XGetAtomName(m_opaqueWindowData->pDisplay, event.xclient.message_type) == *"WM_PROTOCOLS") {
          if (m_cbs) {
            m_cbs->on_dest();
          }
        }
        break;
      }

      default: {
        break;
      }
    }
  }

  glXSwapBuffers(m_opaqueWindowData->pDisplay, m_opaqueWindowData->win);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

} // namespace fsengine

#endif
