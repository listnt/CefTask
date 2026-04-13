#include "../backend.hpp"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include <EGL/egl.h>
#include <GLES3/gl3.h>

#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

static Display *x_display = NULL;
static Atom s_wmDeleteMessage;

EGLBoolean WinCreate(UserData *user_data, const char *title) {
  Window root;
  XSetWindowAttributes swa;
  XSetWindowAttributes xattr;
  Atom wm_state;
  XWMHints hints;
  XEvent xev;
  Window win;

  /*
   * X11 native display initialization
   */

  x_display = XOpenDisplay(NULL);
  if (x_display == NULL) {
    return EGL_FALSE;
  }

  root = DefaultRootWindow(x_display);

  swa.event_mask = ExposureMask | PointerMotionMask | KeyPressMask;
  win = XCreateWindow(x_display, root, 0, 0, user_data->WindowHeight,
                      user_data->WindowHeight, 0, CopyFromParent, InputOutput,
                      CopyFromParent, CWEventMask, &swa);
  s_wmDeleteMessage = XInternAtom(x_display, "WM_DELETE_WINDOW", False);
  XSetWMProtocols(x_display, win, &s_wmDeleteMessage, 1);

  xattr.override_redirect = False;
  XChangeWindowAttributes(x_display, win, CWOverrideRedirect, &xattr);

  hints.input = True;
  hints.flags = InputHint;
  XSetWMHints(x_display, win, &hints);

  // make the window visible on the screen
  XMapWindow(x_display, win);
  XStoreName(x_display, win, title);

  // get identifiers for the provided atom name strings
  wm_state = XInternAtom(x_display, "_NET_WM_STATE", False);

  memset(&xev, 0, sizeof(xev));
  xev.type = ClientMessage;
  xev.xclient.window = win;
  xev.xclient.message_type = wm_state;
  xev.xclient.format = 32;
  xev.xclient.data.l[0] = 1;
  xev.xclient.data.l[1] = False;
  XSendEvent(x_display, DefaultRootWindow(x_display), False,
             SubstructureNotifyMask, &xev);

  user_data->eglNativeWindow = (EGLNativeWindowType)win;
  user_data->eglNativeDisplay = (EGLNativeDisplayType)x_display;
  return EGL_TRUE;
}

GLboolean userInterrupt(UserData *userData) {
  XEvent xev;
  KeySym key;
  GLboolean userinterrupt = GL_FALSE;
  char text;

  // Pump all messages from X server. Keypresses are directed to keyfunc (if
  // defined)
  while (XPending(x_display)) {
    XNextEvent(x_display, &xev);
    if (xev.type == KeyPress) {
      if (XLookupString(&xev.xkey, &text, 1, &key, 0) == 1) {
        if (userData->keyFunc != NULL)
          userData->keyFunc(userData, text, 0, 0);
      }
    }
    if (xev.type == ClientMessage) {
      if (xev.xclient.data.l[0] == (long)s_wmDeleteMessage) {
        userinterrupt = GL_TRUE;
      }
    }
    if (xev.type == DestroyNotify)
      userinterrupt = GL_TRUE;
  }
  return userinterrupt;
}