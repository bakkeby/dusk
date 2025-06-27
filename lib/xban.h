#include <X11/extensions/Xfixes.h>
#include <X11/extensions/XInput2.h>

static void hide_cursor(const Arg *arg);
static void show_cursor(const Arg *arg);
static void toggle_cursor(const Arg *arg);
static void ban_to_corner(Client *c);
static void genericevent(XEvent *e);

static int xi_opcode;
static unsigned long long last_button_press = 0;
