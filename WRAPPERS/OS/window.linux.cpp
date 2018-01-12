#include "window.h"

#include <CORE/types.h>
#include <CORE/ARCH/platform.h>
#include <CORE/BASE/logging.h>

#if defined(PLAT_LINUX)

#define GLEW_STATIC (1)
#include <GL/glew.h>
#include <GL/glx.h>

#include <GL/gl.h>

#include <X11/extensions/xf86vmode.h>
#include <X11/keysym.h>

// Remove 'Status' macro, as it's un-used below and breaks Status object from core::base
#undef Status

static bool ctxErrorOccurred = false;
static int ctxErrorHandler(Display *dpy, XErrorEvent *ev) {
  (void)dpy;
  (void)ev;
  ctxErrorOccurred = true;
  return 0;
}


namespace wrappers {
namespace os {

/**
 *
 */
class Window::Impl : public core::util::noncopyable {
  public:
  Display         *pDisplay;
  int           screen;
  ::Window        win;
  GLXContext        ctx;
  XSetWindowAttributes  attr;
  XF86VidModeModeInfo   deskMode;
  int           x, y;
  Colormap        cmap;
  bool m_fullScreen;

  Impl() :pDisplay(nullptr), screen(-1), ctx(0) { }
  ~Impl() {
		if (pDisplay) {
      if (ctx) {
				if (!glXMakeCurrent(pDisplay, None, NULL)) {
					//printf("Error releasing drawing context : killGLWindow\n");
				}
				glXDestroyContext(pDisplay, ctx);
				ctx = 0;
			}

			if (m_fullScreen) {
				XF86VidModeSwitchToMode(pDisplay, screen, &deskMode);
				XF86VidModeSetViewPort(pDisplay, screen, 0, 0);
			}
			XDestroyWindow(pDisplay, win);
			XFreeColormap(pDisplay, cmap);
			XCloseDisplay(pDisplay);
    }
  }
};

/**
 *
 */
Window::Window(
      const char *pTitle,
      const DisplayCaps &displaySettings,
      iWindowCallback *pCallbacks) :
  m_pImpl(nullptr),
  m_settings(displaySettings),
  m_cbs(pCallbacks),
  m_title(pTitle) {
  if (pTitle == nullptr) {
    m_title = "<untitled app>";
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
::core::base::Status Window::reset() {
	destroy();
	if (!create()) {
		return ::core::base::Status::GENERIC_ERROR;
	}
  return ::core::base::Status::OK;
}


/**
 *
 */
::core::base::Status Window::create() {
  Trace();
  ASSERT(!m_pImpl);
  std::unique_ptr< Impl > pImpl = std::make_unique< Impl >();

  // Open the X Display
  pImpl->pDisplay = XOpenDisplay(0);
  if (!pImpl->pDisplay) {
    Log(LL::Error) << "XOpenDisplay(0) failed!";
    return Status::GENERIC_ERROR;
  }
  pImpl->screen = DefaultScreen(pImpl->pDisplay);

  // Check the supported version (must be atleast 1.3)
  int glx_major, glx_minor;
  if (!glXQueryVersion(pImpl->pDisplay, &glx_major, &glx_minor) || ((glx_major == 1) && (glx_minor < 3)) || (glx_major < 1)) {
    Log(LL::Error) << "Invalid GLX version";
    return Status::GENERIC_ERROR;
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
  GLXFBConfig *fbc = glXChooseFBConfig(pImpl->pDisplay, pImpl->screen, visual_attribs, &fbcount);
  if (!fbc) {
    Log(LL::Error) << "Failed to retrieve a framebuffer config";
    return Status::GENERIC_ERROR;
  }

  // Pick the FB config/visual with the most samples per pixel
  int best_fbc = -1, worst_fbc = -1, best_num_samp = -1, worst_num_samp = 999;
  for (int i = 0; i < fbcount; i++) {
    XVisualInfo *vi = glXGetVisualFromFBConfig(pImpl->pDisplay, fbc[i]);
    if (vi) {
      int samp_buf, samples;
      glXGetFBConfigAttrib(pImpl->pDisplay, fbc[i], GLX_SAMPLE_BUFFERS, &samp_buf);
      glXGetFBConfigAttrib(pImpl->pDisplay, fbc[i], GLX_SAMPLES       , &samples);

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
  XVisualInfo *vi = glXGetVisualFromFBConfig(pImpl->pDisplay, bestFbc);
  if (!vi) {
    Log(LL::Error) << "glXChooseVisual Failed";
  }

  // Create a color map
  pImpl->cmap = XCreateColormap(pImpl->pDisplay, RootWindow(pImpl->pDisplay, vi->screen), vi->visual, AllocNone);
  pImpl->attr.colormap = pImpl->cmap;
  pImpl->attr.border_pixel = 0;
  pImpl->attr.background_pixmap = None;
  pImpl->attr.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | StructureNotifyMask | PointerMotionMask;

  // Set the display mode
  unsigned long window_valuemask = 0;
  pImpl->m_fullScreen = m_settings.m_fullScreen;
  if (m_settings.m_fullScreen) {
    int modeCount = 0;
    XF86VidModeModeInfo **modes;
    XF86VidModeGetAllModeLines(pImpl->pDisplay, pImpl->screen, &modeCount, &modes);
    pImpl->deskMode = *modes[0];

    int bestMode = 0;
    for (int i = 0; i < modeCount; i++) {
      if ((modes[i]->hdisplay == m_settings.m_width) && (modes[i]->vdisplay == m_settings.m_height)) {
        bestMode = i;
      }
    }

    XF86VidModeSwitchToMode(pImpl->pDisplay, pImpl->screen, modes[bestMode]);
    XF86VidModeSetViewPort(pImpl->pDisplay, pImpl->screen, 0, 0);
    m_settings.m_width = modes[bestMode]->hdisplay;
    m_settings.m_height = modes[bestMode]->vdisplay;
    XFree(modes);

    pImpl->attr.override_redirect = True;


    window_valuemask = CWBorderPixel | CWColormap | CWEventMask | CWOverrideRedirect;
  } else {
    window_valuemask = CWBorderPixel | CWColormap | CWEventMask;
  }

  // Create the window
  pImpl->win = XCreateWindow(pImpl->pDisplay, RootWindow(pImpl->pDisplay, vi->screen), 0, 0, m_settings.m_width, m_settings.m_height, 0, vi->depth, InputOutput, vi->visual, window_valuemask, &pImpl->attr);
  if (!pImpl->win) {
    Log(LL::Error) << "XCreateWindow() failed!";
    return Status::GENERIC_ERROR;
  }

  XMapWindow(pImpl->pDisplay, pImpl->win);

  // Setup Mode data for the window
  if (m_settings.m_fullScreen) {
    XWarpPointer(pImpl->pDisplay, None, pImpl->win, 0, 0, 0, 0, 0, 0);
    XMapRaised(pImpl->pDisplay, pImpl->win);
    XGrabKeyboard(pImpl->pDisplay, pImpl->win, True, GrabModeAsync, GrabModeAsync, CurrentTime);
    XGrabPointer(pImpl->pDisplay, pImpl->win, True, ButtonPressMask, GrabModeAsync, GrabModeAsync, pImpl->win, None, CurrentTime);
  } else {
    Atom wmDelete = XInternAtom(pImpl->pDisplay, "WM_DELETE_WINDOW", True);
    XSetWMProtocols(pImpl->pDisplay, pImpl->win, &wmDelete, 1);
    XSetStandardProperties(pImpl->pDisplay, pImpl->win, m_title, m_title, None, NULL, 0, NULL);
    XMapRaised(pImpl->pDisplay, pImpl->win);
  }

  ::Window winDummy;
  unsigned int borderDummy;


  // Set the event handler
  ctxErrorOccurred = false;
  int (*oldHandler)(Display *, XErrorEvent *) = XSetErrorHandler(&ctxErrorHandler);

  pImpl->ctx = glXCreateContext(pImpl->pDisplay, vi, 0, GL_TRUE);
  XSync(pImpl->pDisplay, False);
  XSetErrorHandler(oldHandler);

  if (ctxErrorOccurred || !pImpl->ctx) {
    Log(LL::Error) << "Failed to create OpenGL Context";
    return Status::GENERIC_ERROR;
  }
  XFree(vi);

  // Make the context current
  glXMakeCurrent(pImpl->pDisplay, pImpl->win, pImpl->ctx);
  GLenum error = glewInit();
  if (error != GLEW_OK) {
    Log(LL::Error) << "Failed to init GLEW";
    return Status::GENERIC_ERROR;
  }

  // Get all the info about our setup
  XGetGeometry(pImpl->pDisplay, pImpl->win, &winDummy, &pImpl->x, &pImpl->y, &m_settings.m_width, &m_settings.m_height, &borderDummy, &m_settings.m_pixelDepth);

  Log(LL::Info) << "OpenGL " << (glXIsDirect(pImpl->pDisplay, pImpl->ctx) ? "is" : "is not") << " Direct Rendering";
  Log(LL::Info) << "OpenGL mode " << (m_settings.m_fullScreen ? "fullscreen" : "windowed");

  int majorRev, minorRev;
  glGetIntegerv(GL_MAJOR_VERSION, &majorRev);
  glGetIntegerv(GL_MINOR_VERSION, &minorRev);
  Log(LL::Info) << "OpenGL version " << majorRev << "." << minorRev;
  Log(LL::Info) << "OpenGL GLEW Support GL_VERSION_1_2: " << (glewIsSupported("GL_VERSION_1_2") ? "true" : "false");
  Log(LL::Info) << "OpenGL GLEW Support GL_VERSION_2_0: " << (glewIsSupported("GL_VERSION_2_0") ? "true" : "false");
  Log(LL::Info) << "OpenGL GLEW Support GL_VERSION_3_0: " << (glewIsSupported("GL_VERSION_3_0") ? "true" : "false");

  m_pImpl = pImpl.release();

  if (m_cbs) {
    m_cbs->onInit();
  }

  return ::core::base::Status::OK;
}

void Window::destroy() {
  Trace();
  delete m_pImpl;
  m_pImpl = nullptr;
}

void Window::framestep() {
  XEvent event;
  while (XPending(m_pImpl->pDisplay) > 0) {
    XNextEvent(m_pImpl->pDisplay, &event);
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
          if (m_cbs) { m_cbs->onReshape(); }
        }
        break;
      }

      case ButtonPress: {
        if (m_cbs) {
          m_cbs->onButtonInput(eDeviceId::DEVICE_DEFAULT, Platform_RemapKeycode(0xFFFF0000 + event.xbutton.button), true);
        }
        break;
      }

      case ButtonRelease: {
        if (m_cbs) {
          m_cbs->onButtonInput(eDeviceId::DEVICE_DEFAULT, Platform_RemapKeycode(0xFFFF0000 + event.xbutton.button), false);
        }
        break;
      }

      case MotionNotify: {
        if (m_cbs) {
          m_cbs->onAxisInput(eDeviceId::DEVICE_DEFAULT, eAxisMap::AXIS_MOUSE_X, event.xmotion.x);
          m_cbs->onAxisInput(eDeviceId::DEVICE_DEFAULT, eAxisMap::AXIS_MOUSE_Y, event.xmotion.y);
        }
        break;
      }

      case KeyPress: {
        if (m_cbs) {
          m_cbs->onButtonInput(eDeviceId::DEVICE_DEFAULT, Platform_RemapKeycode(XLookupKeysym(&event.xkey, 0)), true);
        }
        break;
      }

      case KeyRelease: {
        if (m_cbs) {
          m_cbs->onButtonInput(eDeviceId::DEVICE_DEFAULT, Platform_RemapKeycode(XLookupKeysym(&event.xkey, 0)), false);
        }
        break;
      }

      case ClientMessage: {
        if (*XGetAtomName(m_pImpl->pDisplay, event.xclient.message_type) == *"WM_PROTOCOLS") {
          if (m_cbs) {
            m_cbs->onDest();
          }
        }
        break;
      }

      default: {
        break;
      }
    }
  }

  glXSwapBuffers(m_pImpl->pDisplay, m_pImpl->win);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

} // namespace os
} // namespace wrappers

#endif
