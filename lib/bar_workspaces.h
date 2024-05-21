#include <ctype.h>

static int size_workspaces(Bar *bar, BarArg *a);
static int draw_workspaces(Bar *bar, BarArg *a);
static int click_workspaces(Bar *bar, Arg *arg, BarArg *a);
static int hover_workspaces(Bar *bar, BarArg *a, XMotionEvent *ev);
static void nextwsicon(Bar *bar, Workspace *ws, Workspace **next, char **nexticon, int *nextw);
static Client *getworkspacelabelclient(Workspace *ws);
