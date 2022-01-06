#include <X11/Xresource.h>

static void loadxrdb(void);
static int loadxrdbcolor(XrmDatabase xrdb, char **dest, char *resource);
static void xrdb(const Arg *arg);
