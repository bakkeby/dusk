#include <X11/extensions/Xfixes.h>
#include <X11/extensions/XInput2.h>
#include <X11/XKBlib.h>

static XKeyEvent createKeyEvent(Display *dpy, Window win, Window root, int press, int keycode, int modifiers);
static void hide_cursor(const Arg *arg);
static void show_cursor(const Arg *arg);
static void toggle_cursor(const Arg *arg);
static void genericevent(XEvent *e);

static int xi_opcode;
static unsigned long long last_button_press = 0;
static unsigned int modmask = 0;
