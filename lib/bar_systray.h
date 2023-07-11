#define SYSTEM_TRAY_REQUEST_DOCK    0
#define _NET_SYSTEM_TRAY_ORIENTATION_HORZ 0

/* XEMBED messages */
#define XEMBED_EMBEDDED_NOTIFY      0
#define XEMBED_WINDOW_ACTIVATE      1
#define XEMBED_FOCUS_IN             4
#define XEMBED_MODALITY_ON         10

#define XEMBED_MAPPED              (1 << 0)
#define XEMBED_WINDOW_ACTIVATE      1
#define XEMBED_WINDOW_DEACTIVATE    2

#define VERSION_MAJOR               0
#define VERSION_MINOR               0
#define XEMBED_EMBEDDED_VERSION (VERSION_MAJOR << 16) | VERSION_MINOR

typedef struct SystrayWin SystrayWin;
struct SystrayWin {
	Window win;
	Client *icons;
	Bar *bar;
	int h;
};

static SystrayWin *systray = NULL;

/* bar integration */
static int size_systray(Bar *bar, BarArg *a);
static int draw_systray(Bar *bar, BarArg *a);
static int click_systray(Bar *bar, Arg *arg, BarArg *a);

/* function declarations */
static int initsystray(void);
static void addsystrayicon(XClientMessageEvent *cme);
static void removesystrayicon(Client *i);
static void resizerequest(XEvent *e);
static void updatesystrayicongeom(Client *i, int w, int h);
static void updatesystrayiconstate(Client *i, XPropertyEvent *ev);
static Client *wintosystrayicon(Window w);
