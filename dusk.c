/* See LICENSE file for copyright and license details.
 *
 * The dusk dynamic window manager is designed like any other X client as well.
 * It is driven through handling X events. In contrast to other X clients, a
 * window manager selects for SubstructureRedirectMask on the root window, to
 * receive events about window (dis-)appearance. Only one X connection at a
 * time is allowed to select for this event mask.
 *
 * The event handlers of dusk are organized in an array which is accessed
 * whenever a new event has been fetched. This allows event dispatching
 * in O(1) time.
 *
 * Each child of the root window is called a client, except windows which have
 * set the override_redirect flag. Clients are organized in a linked client
 * list on each workspace. The focus history is remembered through a stack list
 * on each workspace. A workspace client area is restricted on a per monitor
 * basis.
 *
 * Keys and workspace rules are organized as arrays and defined in config.h.
 *
 * To understand everything else, start reading main().
 */
#include <errno.h>
#include <locale.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xproto.h>
#include <X11/Xutil.h>
#ifdef XINERAMA
#include <X11/extensions/Xinerama.h>
#endif /* XINERAMA */
#include <X11/Xft/Xft.h>

#include "drw.h"
#include "util.h"

#include <assert.h>
#include <libgen.h>
#include <sys/stat.h>
#define SPAWN_CWD_DELIM " []{}()<>\"':"

/* macros */
#define Button6                 6
#define Button7                 7
#define Button8                 8
#define Button9                 9
#define BARRULES                50
#define NUM_STATUSES            10
#define STATUS_BUFFER           512
#define BUTTONMASK              (ButtonPressMask|ButtonReleaseMask)
#define CLEANMASK(mask)         (mask & ~(numlockmask|LockMask) & (ShiftMask|ControlMask|Mod1Mask|Mod2Mask|Mod3Mask|Mod4Mask|Mod5Mask))
#define INTERSECT(X,Y,W,H,Z)    (MAX(0, MIN((X)+(W),(Z)->wx+(Z)->ww) - MAX((X),(Z)->wx)) \
                               * MAX(0, MIN((Y)+(H),(Z)->wy+(Z)->wh) - MAX((Y),(Z)->wy)))
#define INTERSECTC(X,Y,W,H,Z)   (MAX(0, MIN((X)+(W),(Z)->x+(Z)->w) - MAX((X),(Z)->x)) \
                               * MAX(0, MIN((Y)+(H),(Z)->y+(Z)->h) - MAX((Y),(Z)->y)))
#define LENGTH(X)               (sizeof X / sizeof X[0])
#define MOUSEMASK               (BUTTONMASK|PointerMotionMask)
#define WIDTH(X)                ((X)->w + 2 * (X)->bw)
#define HEIGHT(X)               ((X)->h + 2 * (X)->bw)
#define WTYPE                   "_NET_WM_WINDOW_TYPE_"
#define TEXTW(X)                (drw_fontset_getwidth(drw, (X)))
#define TEXT2DW(X)              (status2dtextlength((X)))
#define CLIENT                  (arg && arg->v ? (Client*)arg->v : selws->sel)
#define NAME(X)                 ((X) ? (X)->name : "NULL")

/* enums */
enum {
	CurResizeBR,
	CurResizeBL,
	CurResizeTR,
	CurResizeTL,
	CurResizeHorzArrow,
	CurResizeVertArrow,
	CurIronCross,
	CurNormal,
	CurResize,
	CurMove,
	CurSwallow,
	CurLast
}; /* cursor */

enum {
	SchemeNorm,
	SchemeSel,
	SchemeTitleNorm,
	SchemeTitleSel,
	SchemeWsNorm,
	SchemeWsVisible,
	SchemeWsSel,
	SchemeWsOcc,
	SchemeScratchSel,
	SchemeScratchNorm,
	SchemeHidSel,
	SchemeHidNorm,
	SchemeUrg,
	SchemeMarked,
	SchemeFlexActTTB,
	SchemeFlexActLTR,
	SchemeFlexActMONO,
	SchemeFlexActGRID,
	SchemeFlexActGRIDC,
	SchemeFlexActGRD1,
	SchemeFlexActGRD2,
	SchemeFlexActGRDM,
	SchemeFlexActHGRD,
	SchemeFlexActDWDL,
	SchemeFlexActDWDLC,
	SchemeFlexActSPRL,
	SchemeFlexActSPRLC,
	SchemeFlexActTTMI,
	SchemeFlexActTTMIC,
	SchemeFlexActFloat,
	SchemeFlexInaTTB,
	SchemeFlexInaLTR,
	SchemeFlexInaMONO,
	SchemeFlexInaGRID,
	SchemeFlexInaGRIDC,
	SchemeFlexInaGRD1,
	SchemeFlexInaGRD2,
	SchemeFlexInaGRDM,
	SchemeFlexInaHGRD,
	SchemeFlexInaDWDL,
	SchemeFlexInaDWDLC,
	SchemeFlexInaSPRL,
	SchemeFlexInaSPRLC,
	SchemeFlexInaTTMI,
	SchemeFlexInaTTMIC,
	SchemeFlexInaFloat,
	SchemeFlexSelTTB,
	SchemeFlexSelLTR,
	SchemeFlexSelMONO,
	SchemeFlexSelGRID,
	SchemeFlexSelGRIDC,
	SchemeFlexSelGRD1,
	SchemeFlexSelGRD2,
	SchemeFlexSelGRDM,
	SchemeFlexSelHGRD,
	SchemeFlexSelDWDL,
	SchemeFlexSelDWDLC,
	SchemeFlexSelSPRL,
	SchemeFlexSelSPRLC,
	SchemeFlexSelTTMI,
	SchemeFlexSelTTMIC,
	SchemeFlexSelFloat,
	SchemeLast,
}; /* color schemes */

static const char *default_resource_prefixes[SchemeLast] = {
	/*                     resource prefix */
	[SchemeNorm]         = "norm",
	[SchemeSel]          = "sel",
	[SchemeTitleNorm]    = "titlenorm",
	[SchemeTitleSel]     = "titlesel",
	[SchemeWsNorm]       = "wsnorm",
	[SchemeWsVisible]    = "wsvis",
	[SchemeWsSel]        = "wssel",
	[SchemeWsOcc]        = "wsocc",
	[SchemeHidNorm]      = "hidnorm",
	[SchemeHidSel]       = "hidsel",
	[SchemeUrg]          = "urg",
	[SchemeMarked]       = "marked",
	[SchemeScratchNorm]  = "scratchnorm",
	[SchemeScratchSel]   = "scratchsel",
	[SchemeFlexActTTB]   = "act.TTB",
	[SchemeFlexActLTR]   = "act.LTR",
	[SchemeFlexActMONO]  = "act.MONO",
	[SchemeFlexActGRID]  = "act.GRID",
	[SchemeFlexActGRIDC] = "act.GRIDC",
	[SchemeFlexActGRD1]  = "act.GRD1",
	[SchemeFlexActGRD2]  = "act.GRD2",
	[SchemeFlexActGRDM]  = "act.GRDM",
	[SchemeFlexActHGRD]  = "act.HGRD",
	[SchemeFlexActDWDL]  = "act.DWDL",
	[SchemeFlexActDWDLC] = "act.DWDLC",
	[SchemeFlexActSPRL]  = "act.SPRL",
	[SchemeFlexActSPRLC] = "act.SPRLC",
	[SchemeFlexActTTMI]  = "act.TTMI",
	[SchemeFlexActTTMIC] = "act.TTMIC",
	[SchemeFlexActFloat] = "act.float",
	[SchemeFlexInaTTB]   = "norm.TTB",
	[SchemeFlexInaLTR]   = "norm.LTR",
	[SchemeFlexInaMONO]  = "norm.MONO",
	[SchemeFlexInaGRID]  = "norm.GRID",
	[SchemeFlexInaGRIDC] = "norm.GRIDC",
	[SchemeFlexInaGRD1]  = "norm.GRD1",
	[SchemeFlexInaGRD2]  = "norm.GRD2",
	[SchemeFlexInaGRDM]  = "norm.GRDM",
	[SchemeFlexInaHGRD]  = "norm.HGRD",
	[SchemeFlexInaDWDL]  = "norm.DWDL",
	[SchemeFlexInaDWDLC] = "norm.DWDLC",
	[SchemeFlexInaSPRL]  = "norm.SPRL",
	[SchemeFlexInaSPRLC] = "norm.SPRLC",
	[SchemeFlexInaTTMI]  = "norm.TTMI",
	[SchemeFlexInaTTMIC] = "norm.TTMIC",
	[SchemeFlexInaFloat] = "norm.float",
	[SchemeFlexSelTTB]   = "sel.TTB",
	[SchemeFlexSelLTR]   = "sel.LTR",
	[SchemeFlexSelMONO]  = "sel.MONO",
	[SchemeFlexSelGRID]  = "sel.GRID",
	[SchemeFlexSelGRIDC] = "sel.GRIDC",
	[SchemeFlexSelGRD1]  = "sel.GRD1",
	[SchemeFlexSelGRD2]  = "sel.GRD2",
	[SchemeFlexSelGRDM]  = "sel.GRDM",
	[SchemeFlexSelHGRD]  = "sel.HGRD",
	[SchemeFlexSelDWDL]  = "sel.DWDL",
	[SchemeFlexSelDWDLC] = "sel.DWDLC",
	[SchemeFlexSelSPRL]  = "sel.SPRL",
	[SchemeFlexSelSPRLC] = "sel.SPRLC",
	[SchemeFlexSelTTMI]  = "sel.TTMI",
	[SchemeFlexSelTTMIC] = "sel.TTMIC",
	[SchemeFlexSelFloat] = "sel.float",
};

enum {
	ClkLtSymbol,
	ClkStatusText,
	ClkWinTitle,
	ClkClientWin,
	ClkRootWin,
	ClkWorkspaceBar,
	ClkLast
}; /* clicks */

enum {
	LAYOUT,       /* controls overall layout arrangement / split */
	MASTER,       /* indicates the tile arrangement for the master area */
	STACK,        /* indicates the tile arrangement for the stack area */
	STACK2,       /* indicates the tile arrangement for the secondary stack area */
	LTAXIS_LAST,
}; /* named flextile constants */

typedef union {
	long i;
	unsigned long ui;
	float f;
	const void *v;
} Arg;

typedef struct Monitor Monitor;
typedef struct Bar Bar;

typedef struct {
	unsigned int click;
	unsigned int mask;
	unsigned int button;
	void (*func)(const Arg *arg);
	const Arg arg;
} Button;

typedef struct Workspace Workspace;
typedef struct Client Client;
struct Client {
	char name[256];
	char label[32];
	char iconpath[256];  /* maximum file path length under linux is 4096 bytes */
	float mina, maxa;
	float cfact;
	int x, y, w, h;
	int sfx, sfy, sfw, sfh;  /* stored float geometry, used on mode revert */
	int oldx, oldy, oldw, oldh;
	int basew, baseh, incw, inch, maxw, maxh, minw, minh;
	int bw, oldbw;
	int group;
	int area;  /* arrangement area (master, stack, secondary stack) */
	int arr;   /* tile arrangement (left to right, top to bottom, etc.) */
	int scheme;
	char scratchkey;
	char swallowkey;
	char swallowedby;
	unsigned int idx;
	double opacity;
	pid_t pid;
	Client *next;
	Client *snext;
	Client *swallowing;
	Client *linked;
	Workspace *ws;
	Workspace *revertws;  /* holds the original workspace info from when the client was opened */
	Window win;
	unsigned int icw, ich;
	Picture icon;
	uint64_t flags;
	uint64_t prevflags;
};

typedef struct {
	int type;
	unsigned int mod;
	#if USE_KEYCODES
	KeyCode keycode;
	#else
	KeySym keysym;
	#endif // USE_KEYCODES
	void (*func)(const Arg *);
	const Arg arg;
} Key;

typedef struct {
	int nmaster;
	int nstack;
	int layout;
	int masteraxis;  /* master stack area */
	int stack1axis;  /* primary stack area */
	int stack2axis;  /* secondary stack area, e.g. centered master */
	void (*symbolfunc)(Workspace *, unsigned int);
} LayoutPreset;

typedef struct {
	const char *symbol;
	void (*arrange)(Workspace *);
	LayoutPreset preset;
	const char *name;
} Layout;

typedef struct Preview Preview;
struct Monitor {
	int num;              /* monitor index */
	int mx, my, mw, mh;   /* screen size */
	int wx, wy, ww, wh;   /* window area  */
	int gappih;           /* horizontal gap between windows */
	int gappiv;           /* vertical gap between windows */
	int gappoh;           /* horizontal outer gaps */
	int gappov;           /* vertical outer gaps */
	int showbar;
	int orientation;      /* screen orientation: 0 = Horizontal, 1 = Vertical */
	uint64_t wsmask;
	uint64_t prevwsmask;
	unsigned int borderpx;
	Monitor *next;
	Workspace *selws;
	Bar *bar;
	Preview *preview;
};

typedef struct {
	const char *class;
	const char *role;
	const char *instance;
	const char *title;
	const char *wintype;
	const int transient;
	const double opacity;
	const uint64_t flags;
	const char *floatpos;
	const char scratchkey;
	const char *workspace;
	const char *label;
	const char swallowedby;
	const char swallowkey;
	const char *iconpath;
	int resume;
} Rule;

struct Workspace {
	int wx, wy, ww, wh;  /* workspace area */
	char ltsymbol[64];
	char name[16];
	float mfact;
	float wfact;
	int scheme[4];
	int ltaxis[4];
	int nstack;
	int nmaster;
	int enablegaps;
	int visible;
	int orientation;
	int num;
	int pinned;  /* whether workspace is pinned to assigned monitor or not */
	Client *clients;
	Client *sel;
	Client *stack;
	Client *prevzoom;
	Workspace *next;
	Monitor *mon;
	Pixmap preview;
	const Layout *layout;
	const Layout *prevlayout;
	char *icondef;  /* default icon */
	char *iconvac;  /* vacant icon (when workspace is selected, default is empty, and no clients) */
	char *iconocc;  /* when workspace has clients */
};

typedef struct {
	char name[16];
	int monitor;
	int pinned;
	int layout;
	float mfact;
	int nmaster;
	int nstack;
	int enablegaps;
	int norm_scheme;
	int vis_scheme;
	int sel_scheme;
	int occ_scheme;
	char *icondef;
	char *iconvac;
	char *iconocc;
} WorkspaceRule;

/* function declarations */
static void applyrules(Client *c);
static int reapplyrules(Client *c);
static int applysizehints(Client *c, int *x, int *y, int *w, int *h, int interact);
static void arrange(Workspace *ws);
static void arrangemon(Monitor *m);
static void arrangews(Workspace *ws);
static void attach(Client *c);
static void attachstack(Client *c);
static void buttonpress(XEvent *e);
static void checkotherwm(void);
static void cleanup(void);
static void cleanupmon(Monitor *mon);
static void clientfittomon(Client *c, Monitor *m, int *cx, int *cy, int *cw, int *ch);
static void clientfsrestore(Client *c);
static void clientsfsrestore(Client *clients);
static int clientscheme(Client *c, Client *sel);
static void clientmessage(XEvent *e);
static void clientmonresize(Client *c, Monitor *from, Monitor *to);
static void clientsmonresize(Client *clients, Monitor *from, Monitor *to);
static void clientrelposmon(Client *c, Monitor *o, Monitor *n, int *cx, int *cy, int *cw, int *ch);
static void clienttomon(const Arg *arg);
static void clientstomon(const Arg *arg);
static void configure(Client *c);
static Workspace *configurenotify(XEvent *e);
static void configurerequest(XEvent *e);
static Monitor *createmon(int num);
static Workspace *destroynotify(XEvent *e);
static void detach(Client *c);
static void detachstack(Client *c);
static Monitor *dirtomon(int dir);
static Workspace *dirtows(int dir);
static void entermon(Monitor *m, Client *next);
static void enternotify(XEvent *e);
static void expose(XEvent *e);
static void focus(Client *c);
static void focusin(XEvent *e);
static void focusmon(const Arg *arg);
static void focusstack(const Arg *arg);
static Atom getatomprop(Client *c, Atom prop, Atom req);
static Client *getpointerclient(void);
static int getrootptr(int *x, int *y);
static long getstate(Window w);
static int gettextprop(Window w, Atom atom, char *text, unsigned int size);
static void grabbuttons(Client *c, int focused);
static void grabkeys(void);
static void hide(Client *c);
static void incnmaster(const Arg *arg);
static void incnstack(const Arg *arg);
static int isatomstate(XClientMessageEvent *cme, int atom);
static void keypress(XEvent *e);
static void keyrelease(XEvent *e);
static void killclient(const Arg *arg);
static void manage(Window w, XWindowAttributes *wa);
static void mappingnotify(XEvent *e);
static void maprequest(XEvent *e);
static void maximize(Client *c, int maximize_vert, int maximize_horz);
static void motionnotify(XEvent *e);
static unsigned long long now(void);
static void propertynotify(XEvent *e);
static void restart(const Arg *arg);
static void quit(const Arg *arg);
static void raiseclient(Client *c);
static void readclientstackingorder(void);
static Monitor *recttomon(int x, int y, int w, int h);
static Workspace *recttows(int x, int y, int w, int h);
static Client *recttoclient(int x, int y, int w, int h, int include_floating);
static void resize(Client *c, int x, int y, int w, int h, int interact);
static void resizeclient(Client *c, int x, int y, int w, int h);
static void resizeclientpad(Client *c, int x, int y, int w, int h, int xpad, int ypad);
static void restack(Workspace *ws);
static void run(void);
static void scan(void);
static int sendevent(Window w, Atom proto, int m, long d0, long d1, long d2, long d3, long d4);
static void setbackground();
static void setclientstate(Client *c, long state);
static void setfocus(Client *c);
static void setfullscreen(Client *c, int fullscreen, int setfakefullscreen);
static void setlayout(const Arg *arg);
static void setmfact(const Arg *arg);
static void setup(void);
static void seturgent(Client *c, int urg);
static void show(Client *c);
static void skipfocusevents(void);
static void spawn(const Arg *arg);
static pid_t spawncmd(const Arg *arg, int buttonclick, int orphan);
static void structurenotify(XEvent *e);
static unsigned int textw_clamp(const char *str, unsigned int n);
static void togglefloating(const Arg *arg);
static void togglemaximize(Client *c, int maximize_vert, int maximize_horz);
static void unfocus(Client *c, int setfocus, Client *nextfocus);
static void unmanage(Client *c, int destroyed);
static Workspace *unmapnotify(XEvent *e);
static void updateclientlist(void);
static int updategeom(int width, int height);
static void updatenumlockmask(void);
static void updatesizehints(Client *c);
static void updatetitle(Client *c);
static void updatewmhints(Client *c);

static Client *wintoclient(Window w);
static Monitor *wintomon(Window w);
static int xerror(Display *dpy, XErrorEvent *ee);
static int xerrordummy(Display *dpy, XErrorEvent *ee);
static int xerrorstart(Display *dpy, XErrorEvent *ee);
static void zoom(const Arg *arg);

/* bar functions */
#include "lib/include.h"

/* variables */
static const char broken[] = "fubar";
static char rawstatustext[NUM_STATUSES][STATUS_BUFFER];

static int screen;
static int sw, sh;             /* X display screen geometry width, height */
static int lrpad;              /* sum of left and right padding for text */
static int force_warp = 0;     /* force warp in some situations, e.g. killclient */
static int cursor_hidden = 0;
static int mouse_x = 0;
static int mouse_y = 0;
static int prev_ptr_x = 0;
static int prev_ptr_y = 0;
static int ignore_warp = 0;    /* force skip warp in some situations, e.g. dragmfact, dragcfact */
static int num_workspaces = 0; /* the number of available workspaces */
static int combo = 0;          /* used for combo keys */
static int monitorchanged = 0; /* used for combo logic */
static int grp_idx = 0;        /* used for grouping windows together */
static int arrange_focus_on_monocle = 1; /* used in focus to arrange monocle layouts on focus */

/* Used by propertynotify to throttle repeating notifications */
static int pn_prev_state = 0;
static Window pn_prev_win = 0;
static Atom pn_prev_atom = None;
static unsigned int pn_prev_count = 0;

static int (*xerrorxlib)(Display *, XErrorEvent *);
static unsigned int numlockmask = 0;
static void (*handler[LASTEvent]) (XEvent *) = {
	[ButtonPress] = buttonpress,
	[ButtonRelease] = keyrelease,
	[ClientMessage] = clientmessage,
	[ConfigureNotify] = structurenotify,
	[ConfigureRequest] = configurerequest,
	[DestroyNotify] = structurenotify,
	[EnterNotify] = enternotify,
	[Expose] = expose,
	[FocusIn] = focusin,
	#ifdef HAVE_LIBXI
	[GenericEvent] = genericevent,
	#endif
	[KeyPress] = keypress,
	[KeyRelease] = keypress,
	[MappingNotify] = mappingnotify,
	[MapRequest] = maprequest,
	[MotionNotify] = motionnotify,
	[PropertyNotify] = propertynotify,
	[ResizeRequest] = resizerequest,
	[UnmapNotify] = structurenotify,
};
static Atom wmatom[WMLast], netatom[NetLast], allowed[NetWMActionLast], xatom[XLast], duskatom[DuskLast];
static int running = 1;
static Cur *cursor[CurLast];
static Clr **scheme;
static Display *dpy;
static Drw *drw;
static Monitor *mons, *selmon, *dummymon;
static Workspace *workspaces, *selws;
static Window root, wmcheckwin;

/* configuration, allows nested code to access above variables */
#include "config.h"

#include "lib/include.c"

/* function implementations */
void
applyrules(Client *c)
{
	const Rule *r;
	const char *class, *instance;
	Atom game_id = None, da = None, *win_types = NULL;
	char role[64] = {0};
	int di;
	unsigned long dl, nitems;
	unsigned char *p = NULL;
	unsigned int i, transient;
	Workspace *ws = NULL;
	XClassHint ch = { NULL, NULL };

	if (XGetWindowProperty(dpy, c->win, netatom[NetWMWindowType], 0L, sizeof(Atom), False, XA_ATOM,
			&da, &di, &nitems, &dl, &p) == Success && p) {
		win_types = (Atom *) p;
	}

	/* rule matching */
	XGetClassHint(dpy, c->win, &ch);
	class    = ch.res_class ? ch.res_class : broken;
	instance = ch.res_name  ? ch.res_name  : broken;
	gettextprop(c->win, wmatom[WMWindowRole], role, sizeof(role));
	game_id = getatomprop(c, duskatom[SteamGameID], AnyPropertyType);
	transient = ISTRANSIENT(c) ? 1 : 0;

	/* Steam games may come through with custom class, instance and name making it hard to create
	 * generic rules for them. Overriding the class with "steam_app_" to make this easier. */
	if (game_id && !strstr(class, "steam_app_"))
		class = "steam_app_";

	if (enabled(Debug))
		fprintf(stderr, "applyrules: new client %s (%ld), class = '%s', instance = '%s', role = '%s', wintype = '%ld'\n", c->name, c->win, class, instance, role, nitems ? win_types[0] : 0);

	for (i = 0; i < LENGTH(clientrules); i++) {
		r = &clientrules[i];
		if ((!r->title || strstr(c->name, r->title))
		&& (!r->class || strstr(class, r->class))
		&& (!r->role || strstr(role, r->role))
		&& (!r->instance || strstr(instance, r->instance))
		&& (!r->wintype || atomin(XInternAtom(dpy, r->wintype, False), win_types, nitems))
		&& (r->transient == -1 || r->transient == transient))
		{
			c->flags |= Ruled | r->flags;
			c->scratchkey = r->scratchkey;
			c->swallowedby = r->swallowedby;
			c->swallowkey = r->swallowkey;

			if (r->opacity)
				c->opacity = r->opacity;

			if (r->workspace)
				for (ws = workspaces; ws && strcmp(ws->name, r->workspace) != 0; ws = ws->next);
			c->ws = ws ? ws : selws;

			if (r->floatpos)
				setfloatpos(c, r->floatpos, 0, 1);

			if (REVERTWORKSPACE(c) && !c->ws->visible)
				c->revertws = c->ws->mon->selws;
			if (r->label)
				strlcpy(c->label, r->label, sizeof c->label);
			else
				saveclientclass(c);

			if (r->iconpath)
				load_icon_from_png_image(c, r->iconpath);

			if (enabled(Debug) || DEBUGGING(c))
				fprintf(stderr, "applyrules: client rule %d matched:\n    class: %s\n    role: %s\n    instance: %s\n    title: %s\n    wintype: %s\n    flags: %lu\n    floatpos: %s\n    workspace: %s\n    label: %s\n",
					i,
					r->class ? r->class : "NULL",
					r->role ? r->role : "NULL",
					r->instance ? r->instance : "NULL",
					r->title ? r->title : "NULL",
					r->wintype ? r->wintype : "NULL",
					r->flags,
					r->floatpos ? r->floatpos : "NULL",
					r->workspace,
					r->label ? r->label : "NULL");
			if (!r->resume)
				break; /* only allow one rule match */
		}
	}

	if (!RULED(c)) {
		if (transient)
			addflag(c, Centered);
		saveclientclass(c);
	}

	if (ch.res_class)
		XFree(ch.res_class);
	if (ch.res_name)
		XFree(ch.res_name);
	if (p)
		XFree(p);
}

/* This mimics most of what the manage function does when initially managing the window. It returns
 * 1 if rules were applied, and 0 otherwise. A window getting rules applied this way will not be
 * swallowed.
 */
int
reapplyrules(Client *c)
{
	Client *t;
	Window trans = None;
	Workspace *client_ws, *rule_ws;
	uint64_t flags;

	if (RULED(c) && !REAPPLYRULES(c))
		return 0;

	flags = c->flags;
	c->flags = 0;

	client_ws = c->ws;
	applyrules(c);

	if (!RULED(c)) {
		c->flags = flags;
		return 0;
	}

	removeflag(c, ReapplyRules);

	if (DISALLOWED(c)) {
		killclient(&((Arg) { .v = c }));
		return 1;
	}

	if (ISUNMANAGED(c)) {
		XMapWindow(dpy, c->win);
		if (LOWER(c))
			XLowerWindow(dpy, c->win);
		else if (RAISE(c))
			XRaiseWindow(dpy, c->win);
		unmanage(c, 0);
		if (client_ws) {
			arrange(client_ws);
			focus(NULL);
			drawbar(client_ws->mon);
		}
		return 1;
	}

	rule_ws = c->ws;
	if (rule_ws != client_ws || ISSTICKY(c)) {
		c->ws = client_ws;
		detach(c);
		detachstack(c);
		if (ISSTICKY(c)) {
			stickyws->mon = rule_ws->mon;
			detachws(stickyws);
			attachws(stickyws, rule_ws);
			c->ws = stickyws;
			stickyws->sel = c;
			rule_ws = stickyws;
		} else
			c->ws = rule_ws;
		attach(c);
		attachstack(c);
		if (client_ws->visible)
			arrange(client_ws);
		else if (client_ws->mon != rule_ws->mon)
			drawbar(client_ws->mon);
	}

	if (NOBORDER(c))
		c->bw = 0;
	if (c->opacity)
		opacity(c, c->opacity);

	if (ISCENTERED(c)) {
		/* Transient windows are centered within the geometry of the parent window */
		if (ISTRANSIENT(c) && XGetTransientForHint(dpy, c->win, &trans) && (t = wintoclient(trans))) {
			c->sfx = c->x = t->x + WIDTH(t) / 2 - WIDTH(c) / 2;
			c->sfy = c->y = t->y + HEIGHT(t) / 2 - HEIGHT(c) / 2;
		} else {
			c->sfx = c->x = c->ws->mon->wx + (c->ws->mon->ww - WIDTH(c)) / 2;
			c->sfy = c->y = c->ws->mon->wy + (c->ws->mon->wh - HEIGHT(c)) / 2;
		}
	}

	if (rule_ws->visible && rule_ws != stickyws)
		arrange(rule_ws);
	else
		drawbar(rule_ws->mon);

	/* If the client indicates that it is in fullscreen, or if the FullScreen flag has been
	 * explictly set via client rules, then enable fullscreen now. */
	if (getatomprop(c, netatom[NetWMState], XA_ATOM) == netatom[NetWMFullscreen] || ISFULLSCREEN(c)) {
		setflag(c, FullScreen, 0);
		setfullscreen(c, 1, 0);
	} else if (ISFLOATING(c)) {
		raiseclient(c);
		XMoveResizeWindow(dpy, c->win, c->x, c->y, c->w, c->h);
		savefloats(c);
	}

	updateclientdesktop(c);
	updatewmhints(c);
	updatemotifhints(c);
	setfloatinghint(c);

	if (SEMISCRATCHPAD(c) && c->scratchkey)
		initsemiscratchpad(c);

	if (ISVISIBLE(c))
		show(c);
	else
		hide(c);

	return 1;
}

int
applysizehints(Client *c, int *x, int *y, int *w, int *h, int interact)
{
	int baseismin;
	Monitor *m = c->ws->mon;

	/* set minimum possible */
	*w = MAX(1, *w);
	*h = MAX(1, *h);
	if (interact) {
		if (*x > sw)
			*x = sw - WIDTH(c);
		if (*y > sh)
			*y = sh - HEIGHT(c);
		if (*x + *w + 2 * c->bw < 0)
			*x = 0;
		if (*y + *h + 2 * c->bw < 0)
			*y = 0;
	} else {
		if (*x >= m->wx + m->ww)
			*x = m->wx + m->ww - WIDTH(c);
		if (*y >= m->wy + m->wh)
			*y = m->wy + m->wh - HEIGHT(c);
		if (*x + *w + 2 * c->bw <= m->wx)
			*x = m->wx;
		if (*y + *h + 2 * c->bw <= m->wy)
			*y = m->wy;
	}
	if (*h < bh)
		*h = bh;
	if (*w < bh)
		*w = bh;
	if (!IGNORESIZEHINTS(c) && (enabled(ResizeHints) || RESPECTSIZEHINTS(c) || ISFLOATING(c) || !c->ws->layout->arrange)) {
		if (REFRESHSIZEHINTS(c))
			updatesizehints(c);
		/* see last two sentences in ICCCM 4.1.2.3 */
		baseismin = c->basew == c->minw && c->baseh == c->minh;
		if (!baseismin) { /* temporarily remove base dimensions */
			*w -= c->basew;
			*h -= c->baseh;
		}
		/* adjust for aspect ratio limits */
		if (c->mina > 0 && c->maxa > 0) {
			if (c->maxa < (float)*w / *h)
				*w = *h * c->maxa + 0.5;
			else if (c->mina < (float)*h / *w)
				*h = *w * c->mina + 0.5;
		}
		if (baseismin) { /* increment calculation requires this */
			*w -= c->basew;
			*h -= c->baseh;
		}
		/* adjust for increment value */
		if (c->incw)
			*w -= *w % c->incw;
		if (c->inch)
			*h -= *h % c->inch;
		/* restore base dimensions */
		*w = MAX(*w + c->basew, c->minw);
		*h = MAX(*h + c->baseh, c->minh);
		if (c->maxw)
			*w = MIN(*w, c->maxw);
		if (c->maxh)
			*h = MIN(*h, c->maxh);
	}
	return *x != c->x || *y != c->y || *w != c->w || *h != c->h;
}

void
arrange(Workspace *ws)
{
	if (ws && !ws->visible)
		return;

	if (ws) {
		arrangews(ws);
		restack(ws);
		drawbar(ws->mon);
		return;
	}

	for (ws = workspaces; ws; ws = ws->next) {
		if (ws == stickyws)
			continue;
		arrangews(ws);
	}
	drawbars();
}

void
arrangemon(Monitor *m)
{
	Workspace *ws;
	for (ws = workspaces; ws; ws = ws->next) {
		if (ws->mon == m && ws != stickyws)
			arrangews(ws);
	}
}
void
arrangews(Workspace *ws)
{
	if (!ws->visible)
		return;

	strlcpy(ws->ltsymbol, ws->layout->symbol, sizeof ws->ltsymbol);
	if (ws->layout->arrange)
		ws->layout->arrange(ws);
	else
		restorewsfloats(ws);
}

void
attach(Client *c)
{
	c->next = c->ws->clients;
	c->ws->clients = c;
}

void
attachstack(Client *c)
{
	c->snext = c->ws->stack;
	c->ws->stack = c;
}

void
buttonpress(XEvent *e)
{
	int click, i, allow_focus;
	Arg arg = {0};
	Client *c;
	Monitor *m;
	Workspace *ws;
	XButtonPressedEvent *ev = &e->xbutton;
	click = ClkRootWin;

	allow_focus = (disabled(FocusOnClick) || (ev->button != Button4 && ev->button != Button5));

	/* focus monitor if necessary */
	if ((m = wintomon(ev->window)) && m != selmon && allow_focus) {
		ws = m->selws;
		if (ws) {
			unfocus(ws->sel, 1, NULL);
			selws = ws;
		}
		selmon = m;
		focus(NULL);
	}

	c = wintoclient(ev->window);

	#ifdef HAVE_LIBXI
	if (!c && cursor_hidden && enabled(BanishMouseCursor)) {
		c = recttoclient(mouse_x, mouse_y, 1, 1, 1);
		show_cursor(NULL);
	}
	#endif

	if (c) {
		if (allow_focus) {
			focus(c);
			if (ISSTICKY(c)) {
				restack(stickyws);
			} else {
				restack(selws);
			}
		}
		XAllowEvents(dpy, ReplayPointer, CurrentTime);
		click = ClkClientWin;
	}

	if (click == ClkRootWin) {
		barpress(ev, m, &arg, &click);
	}

	for (i = 0; i < LENGTH(buttons); i++) {
		if (click == buttons[i].click && buttons[i].func && buttons[i].button == ev->button
				&& CLEANMASK(buttons[i].mask) == CLEANMASK(ev->state)) {
			buttons[i].func((click == ClkWorkspaceBar || click == ClkWinTitle) && buttons[i].arg.i == 0 ? &arg : &buttons[i].arg);
		}
	}

	#ifdef HAVE_LIBXI
	last_button_press = now();
	#endif

	ignore_marked = 1;
}

void
checkotherwm(void)
{
	xerrorxlib = XSetErrorHandler(xerrorstart);
	/* this causes an error if some other window manager is running */
	XSelectInput(dpy, DefaultRootWindow(dpy), SubstructureRedirectMask);
	XSync(dpy, False);
	XSetErrorHandler(xerror);
	XSync(dpy, False);
}

void
cleanup(void)
{
	Layout foo = { "", NULL };
	Workspace *ws, *next;
	size_t i;

	/* Persist data for restart purposes. */
	for (ws = workspaces; ws; ws = ws->next) {
		if (ws == stickyws)
			continue;
		persistworkspacestate(ws);
	}
	persistworkspacestate(stickyws);

	if (restartwm) {
		persistpids();
	} else {
		autostart_killpids();
	}

	for (ws = workspaces; ws; ws = ws->next) {
		ws->layout = &foo;
		while (ws->stack)
			unmanage(ws->stack, 0);
	}
	XUngrabKey(dpy, AnyKey, AnyModifier, root);
	while (mons)
		cleanupmon(mons);
	free(dummymon);
	if (systray) {
		while (systray->icons)
			removesystrayicon(systray->icons);
		if (systray->win) {
			XUnmapWindow(dpy, systray->win);
			XDestroyWindow(dpy, systray->win);
		}
		free(systray);
		XSync(dpy, False);
	}

	for (ws = workspaces; ws; ws = next) {
		next = ws->next;
		removepreview(ws);
		free(ws);
	}

	for (i = 0; i < CurLast; i++)
		drw_cur_free(drw, cursor[i]);
	for (i = 0; i < LENGTH(colors) + 1; i++)
		free(scheme[i]);
	free(scheme);
	XDestroyWindow(dpy, wmcheckwin);
	drw_free(drw);
	XSync(dpy, False);
	XSetInputFocus(dpy, PointerRoot, RevertToPointerRoot, CurrentTime);
	XDeleteProperty(dpy, root, netatom[NetActiveWindow]);

	ipc_cleanup();

	if (close(epoll_fd) < 0)
		fprintf(stderr, "Failed to close epoll file descriptor\n");
}

void
cleanupmon(Monitor *mon)
{
	Workspace *ws;
	Monitor *m;
	Bar *bar;

	if (mon == mons)
		mons = mons->next;
	else {
		for (m = mons; m && m->next != mon; m = m->next);
		m->next = mon->next;
	}

	for (ws = workspaces; ws; ws = ws->next) {
		if (ws->mon != mon || !mons)
			continue;

		if (ws == stickyws) {
			ws->mon = mons;
			continue;
		}

		handleabandoned(ws);
	}

	for (bar = mon->bar; bar; bar = mon->bar) {
		if (!bar->external) {
			XUnmapWindow(dpy, bar->win);
			XDestroyWindow(dpy, bar->win);
		}
		mon->bar = bar->next;
		if (systray && bar == systray->bar)
			systray->bar = NULL;
		free(bar);
	}
	freepreview(mon);
	free(mon);
}

void
clientfittomon(Client *c, Monitor *m, int *cx, int *cy, int *cw, int *ch)
{
	if (*cx < m->wx)
		*cx = m->wx + m->gappov;
	if (*cy < m->wy)
		*cy = m->wy + m->gappoh;
	if (*cx + *cw > m->wx + m->ww)
		*cx = m->wx + m->ww - *cw - m->gappov;
	if (*cy + *ch > m->wy + m->wh)
		*cy = m->my + m->wh - *ch - m->gappoh;
}

void
clientmessage(XEvent *e)
{
	XClientMessageEvent *cme = &e->xclient;
	Workspace *ws;
	Client *c;
	unsigned int maximize_vert, maximize_horz;
	int setfakefullscreen = 0;

	if (systray && cme->message_type == netatom[NetSystemTrayOP]) {
		addsystrayicon(cme);
		drawbarwin(systray->bar);
		return;
	}

	if (cme->window == root) {
		if (enabled(Debug)) {
			fprintf(stderr, "clientmessage: received message type of %s (%ld) for root window\n", XGetAtomName(dpy, cme->message_type), cme->message_type);
			fprintf(stderr, "    - data 0 = %s (%ld)\n", XGetAtomName(dpy, cme->data.l[0]), cme->data.l[0]);
			fprintf(stderr, "    - data 1 = %s (%ld)\n", XGetAtomName(dpy, cme->data.l[1]), cme->data.l[1]);
			fprintf(stderr, "    - data 2 = %s (%ld)\n", XGetAtomName(dpy, cme->data.l[2]), cme->data.l[2]);
		}

		if (cme->message_type == netatom[NetCurrentDesktop]) {
			if ((ws = getwsbynum(cme->data.l[0])))
				viewwsonmon(ws, selmon, 0);
		}

		return;
	}

	c = wintoclient(cme->window);
	if (!c)
		return;

	if (enabled(Debug) || DEBUGGING(c)) {
		fprintf(stderr, "clientmessage: received message type of %s (%ld) for client %s\n", XGetAtomName(dpy, cme->message_type), cme->message_type, c->name);
		fprintf(stderr, "    - data 0 = %s (%ld)\n", (cme->data.l[0] == 0 ? "_NET_WM_STATE_REMOVE" : cme->data.l[0] == 1 ? "_NET_WM_STATE_ADD" : cme->data.l[0] == 2 ? "_NET_WM_STATE_TOGGLE" : "?"), cme->data.l[0]);
		fprintf(stderr, "    - data 1 = %s (%ld)\n", XGetAtomName(dpy, cme->data.l[1]), cme->data.l[1]);
		fprintf(stderr, "    - data 2 = %s (%ld)\n", XGetAtomName(dpy, cme->data.l[2]), cme->data.l[2]);
	}

	/* To change the state of a mapped window, a client MUST send a _NET_WM_STATE client message
	 * to the root window.
	 *
	 *   window  = the respective client window
	 *   message_type = _NET_WM_STATE
	 *   format = 32
	 *   data.l[0] = the action, as listed below
	 *   data.l[1] = first property to alter
	 *   data.l[2] = second property to alter
	 *   data.l[3] = source indication
	 *   other data.l[] elements = 0
	 *
	 * _NET_WM_STATE_REMOVE        0    // remove/unset property
	 * _NET_WM_STATE_ADD           1    // add/set property
	 * _NET_WM_STATE_TOGGLE        2    // toggle property
	 *
	 * https://specifications.freedesktop.org/wm-spec/wm-spec-1.3.html#idm45805407959456
	 */
	if (cme->message_type == netatom[NetWMState]) {
		if (isatomstate(cme, netatom[NetWMFullscreen])) {
			if (c != c->ws->sel || c->ws != selws)
				return;

			if (RESTOREFAKEFULLSCREEN(c) && ISFULLSCREEN(c))
				setfakefullscreen = 1;
			setfullscreen(c, (cme->data.l[0] == 1 /* _NET_WM_STATE_ADD */
				|| (cme->data.l[0] == 2 /* _NET_WM_STATE_TOGGLE */
				&& !ISFULLSCREEN(c)
			)), setfakefullscreen);
		} else if (isatomstate(cme, netatom[NetWMDemandsAttention])) {
			if (cme->data.l[0] == 1 || (cme->data.l[0] == 2 && !ISURGENT(c))) {
				setflag(c, Urgent, 1);
				drawbar(c->ws->mon);
			}
		} else if (isatomstate(cme, netatom[NetWMHidden])) {
			switch (cme->data.l[0]) {
			default:
			case 0: /* _NET_WM_STATE_REMOVE */
				reveal(c);
				break;
			case 1: /* _NET_WM_STATE_ADD */
				conceal(c);
				break;
			case 2: /* _NET_WM_STATE_TOGGLE */
				if (HIDDEN(c))
					reveal(c);
				else
					conceal(c);
				break;
			}
		} else if (isatomstate(cme, netatom[NetWMSkipTaskbar])) {
			toggleflagop(c, SkipTaskbar, cme->data.l[0]);
		} else if (isatomstate(cme, netatom[NetWMStaysOnTop])) {
			toggleflagop(c, AlwaysOnTop, cme->data.l[0]);
		} else if (isatomstate(cme, netatom[NetWMSticky])) {
			switch (cme->data.l[0]) {
			default:
			case 0: /* _NET_WM_STATE_REMOVE */
				unsetsticky(c);
				break;
			case 1: /* _NET_WM_STATE_ADD */
				setsticky(c);
				break;
			case 2: /* _NET_WM_STATE_TOGGLE */
				togglesticky(&((Arg) { .v = c }));
				break;
			}
		} else {
			maximize_vert = isatomstate(cme, netatom[NetWMMaximizedVert]);
			maximize_horz = isatomstate(cme, netatom[NetWMMaximizedHorz]);
			if (maximize_vert || maximize_horz) {
				switch (cme->data.l[0]) {
				default:
				case 0: /* _NET_WM_STATE_REMOVE */
					if (ISFLOATING(c) || !c->ws->layout->arrange) {
						restorefloats(c);
					}
					break;
				case 1: /* _NET_WM_STATE_ADD */
				case 2: /* _NET_WM_STATE_TOGGLE */
					togglemaximize(c, maximize_vert, maximize_horz);
					break;
				}
			}
		}
	} else if (cme->message_type == netatom[NetCloseWindow]) {
		killclient(&((Arg) { .v = c }));
	} else if (cme->message_type == netatom[NetWMDesktop]) {
		if ((ws = getwsbynum(cme->data.l[0])))
			movetows(c, ws, enabled(ViewOnWs));
	} else if (cme->message_type == netatom[NetActiveWindow]) {
		if (enabled(FocusOnNetActive) && !NOFOCUSONNETACTIVE(c)) {
			if (ISINVISIBLE(c) && c->scratchkey) {
				togglescratch(&((Arg) {.v = (const char*[]){ &c->scratchkey, NULL } }));
			}
			if (HIDDEN(c)) {
				reveal(c);
				arrange(c->ws);
				drawbar(c->ws->mon);
			}
			if (c->ws->visible) {
				focus(c);
			} else {
				viewwsonmon(c->ws, c->ws->mon, 0);
			}
		} else if (c != selws->sel && !ISURGENT(c)) {
			seturgent(c, 1);
		}
	} else if (cme->message_type == wmatom[WMChangeState]) {
		if (cme->data.l[0] == IconicState) {
			/* Some applications assume that setting the IconicState a second
			 * time will toggle the state. */
			if ((getstate(c->win) == IconicState)) {
				setclientstate(c, NormalState);
				reveal(c);
			} else if (!HIDDEN(c)) {
				setclientstate(c, IconicState);
				conceal(c);
			}
		} else if (cme->data.l[0] == NormalState ) {
			if (HIDDEN(c))
				reveal(c);
			else {
				setclientstate(c, NormalState);
				show(c);
			}
		} else if (cme->data.l[0] == WithdrawnState)
			setclientstate(c, WithdrawnState);
		arrange(c->ws);
		drawbar(c->ws->mon);
	} else if (cme->message_type == netatom[NetWMMoveResize]) {
		resizemouse(&((Arg) { .v = c }));
	}
}

void
clientmonresize(Client *c, Monitor *from, Monitor *to)
{
	if (!c || from == to || ISSTICKY(c))
		return;

	if (c->sfx == -9999)
		savefloats(c);

	savewindowfloatposition(c, from);
	if (!restorewindowfloatposition(c, to))
		clientrelposmon(c, from, to, &c->sfx, &c->sfy, &c->sfw, &c->sfh);

	if (ISFULLSCREEN(c) && !ISFAKEFULLSCREEN(c))
		clientrelposmon(c, from, to, &c->oldx, &c->oldy, &c->oldw, &c->oldh);
}

void
clientsmonresize(Client *clients, Monitor *from, Monitor *to)
{
	if (from == to || !from || !to)
		return;

	for (Client *c = clients; c; c = c->next)
		clientmonresize(c, from, to);
}

void
clientfsrestore(Client *c)
{
	if (c && ISFULLSCREEN(c) && !ISFAKEFULLSCREEN(c)) {
		resizeclient(c, c->ws->mon->mx, c->ws->mon->my, c->ws->mon->mw, c->ws->mon->mh);
		XRaiseWindow(dpy, c->win);
	} else if (c && ISFLOATING(c))
		resizeclient(c, c->sfx, c->sfy, c->sfw, c->sfh);
}

void
clientsfsrestore(Client *clients)
{
	Client *c;
	for (c = clients; c; c = c->next)
		clientfsrestore(c);
}

int
clientscheme(Client *c, Client *s)
{
	int active = 0, sel = 0, fwb = enabled(FlexWinBorders);

	if (!c)
		return SchemeTitleNorm;

	if (c->ws == selws) {
		sel = c == s;
		active = fwb && !sel && s && s->area == c->area;
	}

	if (ISMARKED(c))
		return SchemeMarked;
	if (ISURGENT(c))
		return SchemeUrg;
	if (HIDDEN(c))
		return sel ? SchemeHidSel : SchemeHidNorm;
	if (ISSCRATCHPAD(c))
		return sel ? SchemeScratchSel : SchemeScratchNorm;
	if (ISFLOATING(c) || !c->ws->layout->arrange)
		return sel ? SchemeFlexSelFloat : active ? SchemeFlexActFloat : SchemeFlexInaFloat;

	if (fwb)
		return c->arr + (sel ? SchemeFlexSelTTB : active ? SchemeFlexActTTB : SchemeFlexInaTTB);
	return sel ? SchemeTitleSel : SchemeTitleNorm;
}

/* Works out a client's (c) position on a new monitor (n) relative to that of the position on
 * another (o) monitor.
 *
 * Formula:
 *                (n->ww - c->w)
 *    x = n->wx + -------------- * (c->x - o->wx)
 *                (o->ww - c->w)
 */
void
clientrelposmon(Client *c, Monitor *o, Monitor *n, int *cx, int *cy, int *cw, int *ch)
{
	int ncw = MIN(*cw, MIN(o->ww, n->ww) - 2 * c->bw - 2 * n->gappov);
	int nch = MIN(*ch, MIN(o->wh, n->wh) - 2 * c->bw - 2 * n->gappoh);

	clientfittomon(c, o, cx, cy, cw, ch);

	if (*cw != ncw || (o->ww - *cw <= 0)) {
		*cw = ncw;
		*cx = n->wx + n->gappov;
	} else
		*cx = n->wx + (n->ww - *cw) * (*cx - o->wx) / (o->ww - *cw);

	if (*ch != nch || (o->wh - *ch <= 0)) {
		*ch = nch;
		*cy = n->wy + n->gappoh;
	} else
		*cy = n->wy + (n->wh - *ch) * (*cy - o->wy) / (o->wh - *ch);

	clientfittomon(c, n, cx, cy, cw, ch);
}

void
clienttomon(const Arg *arg)
{
	Client *c = selws->sel;
	if (!c || !mons->next)
		return;

	Monitor *m = dirtomon(arg->i);

	if (m->selws)
		movetows(c, m->selws, enabled(ViewOnWs));
}

/* Moves all clients from one monitor to another.
 * More specifically it moves all clients from the selected workspace on the current monitor
 * to the selected workspace on an adjacent monitor. */
void
clientstomon(const Arg *arg)
{
	Workspace *ws = selws;
	Monitor *m;

	if (!mons->next)
		return;

	m = dirtomon(arg->i);
	if (m->selws) {
		moveallclientstows(ws, m->selws, enabled(ViewOnWs));
		arrange(NULL);
		focus(NULL);
	}
}

void
configure(Client *c)
{
	XConfigureEvent ce;

	ce.type = ConfigureNotify;
	ce.display = dpy;
	ce.event = c->win;
	ce.window = c->win;
	ce.x = c->x;
	ce.y = c->y;
	ce.width = c->w;
	ce.height = c->h;
	ce.border_width = c->bw;

	if (noborder(c)) {
		ce.width += c->bw * 2;
		ce.height += c->bw * 2;
		ce.border_width = 0;
	}

	ce.above = None;
	ce.override_redirect = False;
	XSendEvent(dpy, c->win, False, StructureNotifyMask, (XEvent *)&ce);
}

Workspace *
configurenotify(XEvent *e)
{
	Monitor *m;
	Bar *bar;
	Workspace *ws;
	Client *c;
	XConfigureEvent *ev = &e->xconfigure;

	if (ev->window == root) {

		if (enabled(Debug)) {
			fprintf(stderr, "configurenotify: received event for root window\n");
			fprintf(stderr, "    - x = %d, y = %d, w = %d, h = %d\n", ev->x, ev->y, ev->width, ev->height);
		}

		if (updategeom(ev->width, ev->height)) {

			stickyws->ww = sw;
			stickyws->wh = sh;

			for (ws = workspaces; ws; ws = ws->next) {
				for (c = ws->clients; c; c = c->next) {
					c->sfx = (c->sfx != -9999 ? c->sfx : c->x) - c->ws->wx;
					c->sfy = (c->sfx != -9999 ? c->sfy : c->y) - c->ws->wy;
				}
			}
			drw_resize(drw, sw, sh);
			updatebars();
			setworkspaceareas();
			setviewport();
			for (m = mons; m; m = m->next) {
				for (bar = m->bar; bar; bar = bar->next)
					XMoveResizeWindow(dpy, bar->win, bar->bx, bar->by, bar->bw, bar->bh);
				freepreview(m);
			}
			for (ws = workspaces; ws; ws = ws->next) {
				for (c = ws->clients; c; c = c->next) {
					c->sfx += c->ws->wx;
					c->sfy += c->ws->wy;
					if (!ISVISIBLE(c))
						continue;
					if (ISFULLSCREEN(c) && !ISFAKEFULLSCREEN(c))
						resizeclient(c, ws->mon->mx, ws->mon->my, ws->mon->mw, ws->mon->mh);
					else if (ISFLOATING(c)) {
						c->x = c->sfx;
						c->y = c->sfy;
						show(c);
					}
				}
				removepreview(ws);
			}
			arrange(NULL);
			focus(NULL);
		}
	}

	return NULL;
}

void
configurerequest(XEvent *e)
{
	Client *c;
	Monitor *m;
	XConfigureRequestEvent *ev = &e->xconfigurerequest;
	XWindowChanges wc;
	Workspace *ws;

	if ((c = wintoclient(ev->window))) {

		if (enabled(Debug) || DEBUGGING(c)) {
			fprintf(stderr, "configurerequest: received event %ld for client %s\n", ev->value_mask, c->name);
			fprintf(stderr, "    - x = %d, y = %d, w = %d, h = %d\n", ev->x, ev->y, ev->width, ev->height);
		}

		if (IGNORECFGREQ(c))
			return;
		if (ev->value_mask & CWBorderWidth)
			c->bw = ev->border_width;
		else if (ISFLOATING(c) || !c->ws->layout->arrange) {
			if (IGNORECFGREQPOS(c) && IGNORECFGREQSIZE(c))
				return;

			m = c->ws->mon;
			if (!IGNORECFGREQPOS(c)) {
				if (ev->value_mask & CWX) {
					c->oldx = c->x;
					c->x = ev->x;
					if (CFGREQPOSRELATIVETOMONITOR(c))
						c->x += m->mx;
				}
				if (ev->value_mask & CWY) {
					c->oldy = c->y;
					c->y = ev->y;
					if (CFGREQPOSRELATIVETOMONITOR(c))
						c->y += m->my;
				}
			}

			if (!IGNORECFGREQSIZE(c)) {
				if (ev->value_mask & CWWidth) {
					c->oldw = c->w;
					c->w = ev->width;
				}
				if (ev->value_mask & CWHeight) {
					c->oldh = c->h;
					c->h = ev->height;
				}
			}

			if (CFGREQPOSRELATIVETOMONITOR(c) && !ISSTICKY(c)) {
				if ((c->x + c->w) > m->mx + m->mw && ISFLOATING(c))
					c->x = m->mx + (m->mw / 2 - WIDTH(c) / 2);  /* center in x direction */
				if ((c->y + c->h) > m->my + m->mh && ISFLOATING(c))
					c->y = m->my + (m->mh / 2 - HEIGHT(c) / 2); /* center in y direction */
			}

			ws = recttows(c->x, c->y, c->w, c->h);
			if (ws && ISSTICKY(c)) {
				stickyws->mon = ws->mon;
				drawbars();
			} else if (ISVISIBLE(c) && ws && ws != c->ws) {
				detach(c);
				detachstack(c);
				attachx(c, AttachBottom, ws);
				attachstack(c);
			}

			if ((ev->value_mask & (CWX|CWY)) && !(ev->value_mask & (CWWidth|CWHeight)))
				configure(c);
			if (ISVISIBLE(c))
				XMoveResizeWindow(dpy, c->win, c->x, c->y, c->w, c->h);
			else
				addflag(c, NeedResize);
			savefloats(c);
		} else
			configure(c);
	} else {
		wc.x = ev->x;
		wc.y = ev->y;
		wc.width = ev->width;
		wc.height = ev->height;
		wc.border_width = ev->border_width;
		wc.sibling = ev->above;
		wc.stack_mode = ev->detail;
		XConfigureWindow(dpy, ev->window, ev->value_mask, &wc);
	}
	XSync(dpy, False);
}

Monitor *
createmon(int num)
{
	Monitor *m;
	m = ecalloc(1, sizeof(Monitor));
	m->showbar = initshowbar;
	m->borderpx = borderpx;
	m->gappih = gappih;
	m->gappiv = gappiv;
	m->gappoh = gappoh;
	m->gappov = gappov;
	m->wsmask = 0;
	m->prevwsmask = 0;
	m->num = num;
	m->bar = NULL;

	createbars(m);

	return m;
}

Workspace *
destroynotify(XEvent *e)
{
	Client *c;
	Bar *bar;
	Workspace *ws = NULL;
	XDestroyWindowEvent *ev = &e->xdestroywindow;

	if ((c = wintoclient(ev->window))) {
		ws = c->ws;
		if (enabled(Debug) || DEBUGGING(c))
			fprintf(stderr, "destroynotify: received event for client %s\n", c->name);
		unmanage(c, 1);
	} else if ((c = swallowingparent(ev->window))) {
		ws = c->ws;
		if (enabled(Debug) || DEBUGGING(c))
			fprintf(stderr, "destroynotify: received event for swallowing client %s\n", c->name);
		unmanage(c->swallowing, 1);
	} else if (systray && (c = wintosystrayicon(ev->window))) {
		if (enabled(Debug) || DEBUGGING(c))
			fprintf(stderr, "destroynotify: removing systray icon for client %s\n", c->name);
		removesystrayicon(c);
		drawbarwin(systray->bar);
	} else if ((bar = wintobar(ev->window))) {
		if (enabled(Debug))
			fprintf(stderr, "destroynotify: received event for bar %s\n", bar->name);
		recreatebar(bar);
	}

	return ws;
}

void
detach(Client *c)
{
	Client **tc;
	c->idx = 0;
	for (tc = &c->ws->clients; *tc && *tc != c; tc = &(*tc)->next);
	*tc = c->next;
	c->next = NULL;
}

void
detachstack(Client *c)
{
	Client **tc, *t;

	for (tc = &c->ws->stack; *tc && *tc != c; tc = &(*tc)->snext);
	*tc = c->snext;

	if (c == c->ws->sel) {
		for (t = c->ws->stack; t && !ISVISIBLE(t); t = t->snext);
		c->ws->sel = t;
	}
	c->snext = NULL;
}

Monitor *
dirtomon(int dir)
{
	Monitor *m = NULL;

	if (dir > 0) {
		if (!(m = selmon->next))
			m = mons;
	} else if (selmon == mons)
		for (m = mons; m->next; m = m->next);
	else
		for (m = mons; m->next != selmon; m = m->next);
	return m;
}

/* Returns the workspace found in a given direction -1/+1 on the
 * current monitor. The direction is circular, i.e. it wraps around.
 * Passing -2/+2 results in only workspaces that contain clients to
 * be returned. */
Workspace *
dirtows(int dir)
{
	Workspace *ws = selws, *nws = NULL, *tws;

	if (dir > 0) { // right circular search
		for (nws = ws->next; nws; nws = nws->next)
			if (nws->mon == ws->mon && nws != stickyws && (dir != 2 || nws->clients))
				break;
		if (!nws && ws != workspaces)
			for (tws = workspaces; tws && tws != ws; tws = tws->next) {
				if (tws == stickyws)
					continue;
				if (tws->mon == ws->mon && (dir != 2 || tws->clients)) {
					nws = tws;
					break;
				}
			}
	} else { // left circular search
		for (tws = workspaces; tws && !(nws && tws == ws); tws = tws->next)
			if (tws->mon == ws->mon && tws != stickyws && (dir != -2 || tws->clients))
				nws = tws;
	}

	return nws;
}

void
entermon(Monitor *m, Client *next)
{
	Client *sel = selws->sel;
	selmon = m;
	if (m->selws) {
		if (!next)
			next = m->selws->sel;
		selws = m->selws;
		updatecurrentdesktop();
	}
	if (sel) {
		unfocus(sel, 1, next);
		if (!next || sel->ws->mon != next->ws->mon)
			drawbar(sel->ws->mon);
	}
}

void
enternotify(XEvent *e)
{
	Client *c;
	Monitor *m;
	XCrossingEvent *ev = &e->xcrossing;
	int x, y;

	if (enabled(FocusOnClick))
		return;

	if (cursor_hidden)
		return;

	getrootptr(&x, &y);
	if (x == prev_ptr_x && y == prev_ptr_y)
		return;

	if ((ev->mode != NotifyNormal || ev->detail == NotifyInferior) && ev->window != root)
		return;

	c = getpointerclient();
	if (!c)
		c = wintoclient(ev->window);

	m = c ? c->ws->mon : wintomon(ev->window);
	if (selws == m->selws && (!c || (m->selws && c == m->selws->sel)))
		return;
	if (m != selmon)
		entermon(m, c);
	focus(c);
}

void
expose(XEvent *e)
{
	Monitor *m;
	XExposeEvent *ev = &e->xexpose;
	if (ev->count == 0 && (m = wintomon(ev->window)))
		drawbar(m);
}

void
focus(Client *c)
{
	if ((c && !c->ws->visible) || !selws)
		return;

	Workspace *ws = c ? c->ws : selws;
	Window focus_return;
	int revert_to_return;
	Bar *bar;

	if (enabled(FocusFollowMouse) && !monitorchanged && (!c || ISINVISIBLE(c))) {
		c = getpointerclient();
		if (c && c->ws->mon != selmon) {
			c = NULL;
		}
	}
	if (!c || ISINVISIBLE(c))
		for (c = ws->stack; c && !ISVISIBLE(c); c = c->snext);
	if (selws->sel && selws->sel != c)
		unfocus(selws->sel, 0, c);
	if (c) {
		if (c->ws != selws) {
			if (c->ws == stickyws) {
				stickyws->mon = selmon;
				detachws(stickyws);
				attachws(stickyws, selws);
				XSetWindowBorder(dpy, c->win, scheme[SchemeFlexSelFloat][ColBorder].pixel);
			} else {
				if (c->ws->mon != selmon)
					selmon = c->ws->mon;
				c->ws->mon->selws = c->ws;
			}
			selws = c->ws;
			drawbar(ws->mon);
			updatecurrentdesktop();
		}

		if (ISURGENT(c))
			seturgent(c, 0);

		detachstack(c);
		attachstack(c);
		grabbuttons(c, 1);
		setfocus(c);

		if (enabled(FocusedOnTop)) {
			restack(c->ws);
		} else {
			XSync(dpy, False);
		}
		XSetWindowBorder(dpy, c->win, scheme[clientscheme(c, c)][ColBorder].pixel);
	} else {
		XGetInputFocus(dpy, &focus_return, &revert_to_return);

		/* Allow sticky windows to retain focus when changing to an empty workspace */
		if (!stickyws->sel || stickyws->sel != wintoclient(focus_return)) {
			for (bar = selmon->bar; bar && !bar->showbar; bar = bar->next);
			XSetInputFocus(dpy, bar ? bar->win : root, RevertToPointerRoot, CurrentTime);
			XDeleteProperty(dpy, root, netatom[NetActiveWindow]);
			ws->sel = ws->stack;
		}
	}

	if (arrange_focus_on_monocle &&
		ws->layout->arrange == flextile && (
		ws->ltaxis[MASTER] == MONOCLE ||
		ws->ltaxis[STACK]  == MONOCLE ||
		ws->ltaxis[STACK2] == MONOCLE
	)) {
		arrangews(ws);
		skipfocusevents();
	}
	drawbar(ws->mon);
}

/* There are some broken focus acquiring clients needing extra handling */
void
focusin(XEvent *e)
{
	Workspace *ws = selws;
	XFocusChangeEvent *ev = &e->xfocus;

	Client *c = wintoclient(ev->window);
	if (ws->sel && ev->window != ws->sel->win && c)
		setfocus(ws->sel);
}

void
focusmon(const Arg *arg)
{
	Monitor *m;
	Client *sel;

	if (!mons->next)
		return;
	if ((m = dirtomon(arg->i)) == selmon)
		return;
	sel = selws->sel;
	selmon = m;
	if (m->selws)
		selws = m->selws;
	unfocus(sel, 0, NULL);
	focus(NULL);
	if (canwarp(selws->sel))
		warp(selws->sel);
}

void
focusstack(const Arg *arg)
{
	Client *c = NULL, *i;
	Workspace *ws = selws, *prevws = NULL, *w = NULL;

	int n = 0;
	int firstws = 1;

	if (!ws || !getwsmask(selws->mon))
		return;

	if (arg->i > 0) {
		for (; ws && !c; ws = (ws->next ? ws->next : workspaces)) {
			if (!ws->visible || (enabled(RestrictFocusstackToMonitor) && ws->mon != selws->mon))
				continue;
			for (c = (firstws ? selws->sel : ws->clients); c && (c == selws->sel || ISINVISIBLE(c) || (arg->i == 1 && HIDDEN(c))); c = c->next);
			firstws = 0;
			if (n++ > LENGTH(wsrules))
				break;
		}
	} else {
		do {
			for (i = ws->clients; i && (n != 0 || i != ws->sel); i = i->next)
				if (!ISINVISIBLE(i) && !(arg->i == -1 && HIDDEN(i)))
					c = i;
			if (!c) {
				prevws = ws;
				ws = NULL;
				for (w = workspaces; w; w = w->next) {
					if (w->visible && (disabled(RestrictFocusstackToMonitor) || w->mon == prevws->mon))
						ws = w;
					if (ws && w->next == prevws)
						break;
				}
				if (n++ > LENGTH(wsrules))
					break;
			}
		} while (!c && ws);
	}
	if (c) {
		focus(c);
		if (enabled(FocusedOnTop)) {
			if (canwarp(c)) {
				force_warp = 1;
				warp(c);
			}
		} else {
			restack(c->ws);
		}
	}
}

Atom
getatomprop(Client *c, Atom prop, Atom req)
{
	int di;
	unsigned long dl, dm;
	unsigned char *p = NULL;
	Atom da, atom = None;

	/* FIXME getatomprop should return the number of items and a pointer to
	 * the stored data instead of this workaround */
	if (XGetWindowProperty(dpy, c->win, prop, 0L, sizeof atom, False, req,
		&da, &di, &dl, &dm, &p) == Success && p) {
		atom = *(Atom *)p;
		if (da == xatom[XembedInfo] && dl == 2)
			atom = ((Atom *)p)[1];
		XFree(p);
	}
	return atom;
}

int
getrootptr(int *x, int *y)
{
	int di;
	unsigned int dui;
	Window dummy;

	if (cursor_hidden && enabled(BanishMouseCursor)) {
		*x = mouse_x;
		*y = mouse_y;
		return 1;
	}

	return XQueryPointer(dpy, root, &dummy, &dummy, x, y, &di, &di, &dui);
}

Client *
getpointerclient(void)
{
	Window dummy, win;
	int di;
	unsigned int dui;

	if (cursor_hidden && enabled(BanishMouseCursor))
		return NULL;

	XQueryPointer(dpy, root, &dummy, &win, &di, &di, &di, &di, &dui);
	return wintoclient(win);
}

long
getstate(Window w)
{
	int format;
	long result = -1;
	unsigned char *p = NULL;
	unsigned long n, extra;
	Atom real;

	if (XGetWindowProperty(dpy, w, wmatom[WMState], 0L, 2L, False, wmatom[WMState],
		&real, &format, &n, &extra, (unsigned char **)&p) != Success)
		return -1;
	if (n != 0)
		result = *p;
	XFree(p);
	return result;
}

int
gettextprop(Window w, Atom atom, char *text, unsigned int size)
{
	char **list = NULL;
	int n;
	XTextProperty name;

	if (!text || size == 0)
		return 0;
	text[0] = '\0';
	if (!XGetTextProperty(dpy, w, &name, atom) || !name.nitems)
		return 0;
	if (name.encoding == XA_STRING) {
		strlcpy(text, (char *)name.value, size);
	} else if (XmbTextPropertyToTextList(dpy, &name, &list, &n) >= Success && n > 0 && *list) {
		strlcpy(text, *list, size);
		XFreeStringList(list);
	}
	text[size - 1] = '\0';
	XFree(name.value);
	return 1;
}

void
grabbuttons(Client *c, int focused)
{
	updatenumlockmask();
	{
		unsigned int i, j;
		unsigned int modifiers[] = { 0, LockMask, numlockmask, numlockmask|LockMask };
		XUngrabButton(dpy, AnyButton, AnyModifier, c->win);
		if (!focused)
			XGrabButton(dpy, AnyButton, AnyModifier, c->win, False,
				BUTTONMASK, GrabModeSync, GrabModeSync, None, None);
		for (i = 0; i < LENGTH(buttons); i++)
			if (buttons[i].click == ClkClientWin
				&& ((enabled(AllowNoModifierButtons) && !ONLYMODBUTTONS(c)) || buttons[i].mask != 0)
			)
				for (j = 0; j < LENGTH(modifiers); j++)
					XGrabButton(dpy, buttons[i].button,
						buttons[i].mask | modifiers[j],
						c->win, False, BUTTONMASK,
						GrabModeAsync, GrabModeSync, None, None);
	}
}

void
grabkeys(void)
{
	updatenumlockmask();
	{
		#if USE_KEYCODES
		unsigned int i, j;
		unsigned int modifiers[] = { 0, LockMask, numlockmask, numlockmask|LockMask };

		XUngrabKey(dpy, AnyKey, AnyModifier, root);

		for (i = 0; i < LENGTH(keys); i++)
			for (j = 0; j < LENGTH(modifiers); j++)
				XGrabKey(dpy, keys[i].keycode, keys[i].mod | modifiers[j], root,
						True, GrabModeAsync, GrabModeAsync);
		#else // keysyms
		unsigned int i, j, k;
		unsigned int modifiers[] = { 0, LockMask, numlockmask, numlockmask|LockMask };
		int start, end, skip;
		KeySym *syms;

		XUngrabKey(dpy, AnyKey, AnyModifier, root);
		XDisplayKeycodes(dpy, &start, &end);
		syms = XGetKeyboardMapping(dpy, start, end - start + 1, &skip);
		if (!syms)
			return;
		for (k = start; k <= end; k++) {
			for (i = 0; i < LENGTH(keys); i++) {
				/* skip modifier codes, we do that ourselves */
				if (keys[i].keysym != syms[(k - start) * skip])
					continue;
				for (j = 0; j < LENGTH(modifiers); j++) {
					XGrabKey(dpy, k, keys[i].mod | modifiers[j], root, True,
						GrabModeAsync, GrabModeAsync);
				}
			}
		}
		XFree(syms);
		#endif // USE_KEYCODES
	}
}

void
hide(Client *c)
{
	XMoveWindow(dpy, c->win, c->x, HEIGHT(c) * -2);
}

void
incnmaster(const Arg *arg)
{
	Workspace *ws = selws;
	ws->nmaster = MAX(MIN(ws->nmaster,numtiled(ws)) + arg->i, 0);
	arrange(ws);
}

void
incnstack(const Arg *arg)
{
	Workspace *ws = selws;
	ws->nstack = MAX(ws->nstack + arg->i, 0);
	arrange(ws);
}


#ifdef XINERAMA
static int
isuniquegeom(XineramaScreenInfo *unique, size_t n, XineramaScreenInfo *info)
{
	while (n--)
		if (unique[n].x_org == info->x_org && unique[n].y_org == info->y_org
		&& unique[n].width == info->width && unique[n].height == info->height)
			return 0;
	return 1;
}
#endif /* XINERAMA */

void
keypress(XEvent *e)
{
	unsigned int i;
	#if !USE_KEYCODES
	int keysyms_return;
	KeySym* keysym;
	#endif
	XKeyEvent *ev;

	getrootptr(&prev_ptr_x, &prev_ptr_y);

	ev = &e->xkey;
	ignore_marked = 0;
	#if !USE_KEYCODES
	keysym = XGetKeyboardMapping(dpy, (KeyCode)ev->keycode, 1, &keysyms_return);
	#endif
	for (i = 0; i < LENGTH(keys); i++) {
		if (
			#if USE_KEYCODES
			ev->keycode == keys[i].keycode
			#else
			*keysym == keys[i].keysym
			#endif // USE_KEYCODES
			&& ev->type == keys[i].type
			&& CLEANMASK(keys[i].mod) == CLEANMASK(ev->state)
			&& keys[i].func
		) {
			keys[i].func(&(keys[i].arg));
		}
	}
	#if !USE_KEYCODES
	XFree(keysym);
	#endif // USE_KEYCODES
	ignore_marked = 1;

	if (ev->type == KeyRelease)
		keyrelease(e);
}

int
isatomstate(XClientMessageEvent *cme, int atom)
{
	return (cme->data.l[1] == atom || cme->data.l[2] == atom);
}

void
keyrelease(XEvent *e)
{
	if (!combo)
		return;

	togglewsmask();
	combo = 0;
}

void
killclient(const Arg *arg)
{
	Client *c = CLIENT;

	for (c = nextmarked(NULL, c); c; c = nextmarked(c->next, NULL)) {
		if (ISPERMANENT(c)) {
			if (c->scratchkey) {
				addflag(c, Invisible);
				hide(c);
			}
			continue;
		}

		if (sendevent(c->win, wmatom[WMDelete], NoEventMask, wmatom[WMDelete], CurrentTime, 0, 0, 0))
			continue;

		XGrabServer(dpy);
		XSetErrorHandler(xerrordummy);
		XSetCloseDownMode(dpy, DestroyAll);
		XKillClient(dpy, c->win);
		XSync(dpy, False);
		XSetErrorHandler(xerror);
		XUngrabServer(dpy);
		force_warp = 1;
	}
}

void
manage(Window w, XWindowAttributes *wa)
{
	Client *c = NULL, *t = NULL, *term = NULL;
	Monitor *m = NULL;
	Window trans = None;
	XWindowChanges wc = { 0 };
	int focusclient = 1;

	if (selws == stickyws)
		selws = stickyws->next;

	c = ecalloc(1, sizeof(Client));
	c->win = w;
	c->pid = winpid(w);

	/* geometry */
	c->x = c->oldx = wa->x;
	c->y = c->oldy = wa->y;
	c->sfx = -9999;
	c->sfy = -9999;
	c->sfw = c->w = c->oldw = wa->width;
	c->sfh = c->h = c->oldh = wa->height;
	c->oldbw = wa->border_width;
	c->cfact = 1.0;
	c->ws = NULL;
	c->icon = 0;

	updatetitle(c);
	updatesizehints(c);
	if (enabled(Debug))
		fprintf(stderr, "manage --> client %s\n", c->name);
	getclientflags(c);
	getclientfields(c);
	getclientopacity(c);
	getclientlabel(c);
	getclienticonpath(c);

	updateicon(c);

	if (ISSTICKY(c))
		c->ws = recttows(c->x + c->w / 2, c->y + c->h / 2, 1, 1);

	if (!c->ws) {
		if (XGetTransientForHint(dpy, w, &trans) && (t = wintoclient(trans))) {
			addflag(c, Transient);
			c->ws = t->ws;
		} else
			c->ws = selws;
	}

	restorewindowfloatposition(c, c->ws->mon);

	if (!RULED(c)) {
		applyrules(c);

		if (c->x == c->ws->mon->wx && c->y == c->ws->mon->wy)
			addflag(c, Centered);
	}

	if (DISALLOWED(c)) {
		killclient(&((Arg) { .v = c }));
		return;
	}

	if (ISUNMANAGED(c)) {
		XMapWindow(dpy, c->win);
		if (LOWER(c))
			XLowerWindow(dpy, c->win);
		else if (RAISE(c))
			XRaiseWindow(dpy, c->win);
		if (enabled(Debug))
			fprintf(stderr, "manage <-- unmanaged (%s)\n", c->name);
		free(c);
		return;
	}

	if (!ISTRANSIENT(c))
		term = termforwin(c);

	if (ISSTICKY(c)) {
		detachws(stickyws);
		attachws(stickyws, c->ws);
		stickyws->mon = c->ws->mon;
		c->ws = stickyws;
		stickyws->sel = c;
		selws = stickyws;
	}

	c->bw = (NOBORDER(c) ? 0 : c->ws->mon->borderpx);

	if (c->opacity)
		opacity(c, c->opacity);

	m = c->ws->mon;

	if (WIDTH(c) > m->mw)
		c->w = m->mw - 2 * c->bw;
	if (HEIGHT(c) > m->mh)
		c->h = m->mh - 2 * c->bw;
	if (c->x + WIDTH(c) > m->mx + m->mw)
		c->x = m->mx + m->mw - WIDTH(c);
	if (c->y + HEIGHT(c) > m->my + m->mh)
		c->y = m->my + m->mh - HEIGHT(c);
	c->x = MAX(c->x, m->mx);
	/* only fix client y-offset, if the client center might cover the bar */
	c->y = MAX(c->y, ((m->bar && m->bar->by == m->my) && (c->x + (c->w / 2) >= m->wx)
		&& (c->x + (c->w / 2) < m->wx + m->ww)) ? bh : m->my);
	wc.border_width = c->bw;
	XConfigureWindow(dpy, w, CWBorderWidth, &wc);
	configure(c); /* propagates border_width, if size doesn't change */
	updateclientdesktop(c);
	addflag(c, RefreshSizeHints);

	/* If the client indicates that it is in fullscreen, or if the FullScreen flag has been
	 * explictly set via client rules, then enable fullscreen now. */
	if (getatomprop(c, netatom[NetWMState], XA_ATOM) == netatom[NetWMFullscreen] || ISFULLSCREEN(c)) {
		setflag(c, FullScreen, 0);
		setfullscreen(c, 1, 0);
		term = NULL; /* do not allow terminals to be swallowed by windows that start in fullscreen */
	}

	updatewmhints(c);
	updatemotifhints(c);
	readswallowkey(c);

	XSelectInput(dpy, w, EnterWindowMask|FocusChangeMask|PropertyChangeMask|StructureNotifyMask);
	grabbuttons(c, 0);

	/* If this is a transient window for a window that is managed by the window manager, then it should be floating. */
	if (t)
		c->prevflags |= Floating;
	if (!ISFLOATING(c) && (ISFIXED(c) || WASFLOATING(c) || getatomprop(c, duskatom[IsFloating], AnyPropertyType)))
		SETFLOATING(c);

	if (ISFLOATING(c))
		XRaiseWindow(dpy, c->win);

	XChangeProperty(dpy, c->win, netatom[NetWMAllowedActions], XA_ATOM, 32,
		PropModeReplace, (unsigned char *) allowed, NetWMActionLast);

	/* Do not attach client if it swallows a terminal */
	if (term && swallowclient(term, c)) {
		focusclient = (c == selws->sel);
	} else {
		attachx(c, AttachDefault, NULL);

		if (focusclient || !c->ws->sel || !c->ws->stack)
			attachstack(c);
		else {
			c->snext = c->ws->sel->snext;
			c->ws->sel->snext = c;
		}
	}

	if (ISCENTERED(c) || (c->x == m->mx && c->y == m->my)) {
		/* Transient windows are centered within the geometry of the parent window */
		if (t) {
			c->sfx = c->x = t->x + WIDTH(t) / 2 - WIDTH(c) / 2;
			c->sfy = c->y = t->y + HEIGHT(t) / 2 - HEIGHT(c) / 2;
		/* Non-swallowed windows raised via terminal are centered within the geometry of terminal if it fits */
		} else if (term && !c->swallowing && term->w >= c->w && term->h >= c->h) {
			c->sfx = c->x = term->x + WIDTH(term) / 2 - WIDTH(c) / 2;
			c->sfy = c->y = term->y + HEIGHT(term) / 2 - HEIGHT(c) / 2;
		} else {
			c->sfx = c->x = m->wx + (m->ww - WIDTH(c)) / 2;
			c->sfy = c->y = m->wy + (m->wh - HEIGHT(c)) / 2;
		}
	}

	XChangeProperty(dpy, root, netatom[NetClientList], XA_WINDOW, 32, PropModeAppend,
		(unsigned char *) &(c->win), 1);
	XChangeProperty(dpy, root, netatom[NetClientListStacking], XA_WINDOW, 32, PropModePrepend,
		(unsigned char *) &(c->win), 1);

	setclientstate(c, NormalState);

	if (focusclient) {
		if (c->ws == selws)
			unfocus(selws->sel, 0, c);

		c->ws->sel = c; // needed for the XRaiseWindow that takes place in restack
	}

	if (!c->swallowing) {
		if (riopid && (riopid == 1 || RIODRAWNOMATCHPID(c) || isdescprocess(riopid, c->pid))) {
			riopid = 0;
			if (riodimensions[3] != -1)
				rioposition(c, riodimensions[0], riodimensions[1], riodimensions[2], riodimensions[3]);
			else {
				killclient(&((Arg) { .v = c }));
				return;
			}
		}
		else if (SWITCHWORKSPACE(c) && !c->ws->visible)
			viewwsonmon(c->ws, c->ws->mon, 0);
		else if (ENABLEWORKSPACE(c) && !c->ws->visible)
			viewwsonmon(c->ws, c->ws->mon, 1);
	}

	arrange(c->ws);

	if (ISFLOATING(c))
		XMoveResizeWindow(dpy, c->win, c->x, c->y, c->w, c->h);

	if (ISVISIBLE(c))
		show(c);
	else
		hide(c);
	XMapWindow(dpy, c->win);

	if (focusclient)
		focus(c);

	setfloatinghint(c);
	if (SEMISCRATCHPAD(c) && c->scratchkey)
		initsemiscratchpad(c);

	if (!c->ws->visible)
		drawbar(c->ws->mon);

	if (enabled(Debug))
		fprintf(stderr, "manage <-- (%s) on workspace %s\n", c->name, c->ws->name);
}

void
mappingnotify(XEvent *e)
{
	XMappingEvent *ev = &e->xmapping;

	XRefreshKeyboardMapping(ev);
	if (ev->request == MappingKeyboard)
		grabkeys();
}

void
maprequest(XEvent *e)
{
	static XWindowAttributes wa;
	XMapRequestEvent *ev = &e->xmaprequest;

	Client *i;
	if (systray && (i = wintosystrayicon(ev->window))) {
		sendevent(i->win, netatom[Xembed], StructureNotifyMask, CurrentTime, XEMBED_WINDOW_ACTIVATE, 0, systray->win, XEMBED_EMBEDDED_VERSION);
		drawbarwin(systray->bar);
	}

	if (!XGetWindowAttributes(dpy, ev->window, &wa))
		return;
	if (!wa.depth) {
		fprintf(stderr, "maprequest: refusing to map window %ld with depth of 0\n", ev->window);
		return;
	}
	if (wa.override_redirect) {
		XRaiseWindow(dpy, ev->window);
		return;
	}
	if (!wintoclient(ev->window))
		manage(ev->window, &wa);
}

void
maximize(Client *c, int maximize_vert, int maximize_horz)
{
	if (!maximize_vert && !maximize_horz)
		return;
	Workspace *ws = c->ws;

	SETFLOATING(c);
	XRaiseWindow(dpy, c->win);

	if (maximize_vert && maximize_horz)
		setfloatpos(c, "0% 0% 100% 100%", 1, 0);
	else if (maximize_vert)
		setfloatpos(c, "-1x 0% -1w 100%", 1, 0);
	else
		setfloatpos(c, "0% -1y 100% -1h", 1, 0);

	resizeclient(c, c->x, c->y, c->w, c->h);
	drawbar(ws->mon);
}

void
motionnotify(XEvent *e)
{
	Bar *bar;
	Monitor *m;
	Workspace *ws;
	Client *sel;
	XMotionEvent *ev = &e->xmotion;

	bar = wintobar(ev->window);

	// if (enabled(Debug)) {
	// 	if (ev->window == root) {
	// 		fprintf(stderr, "motionnotify: received event x = %d, y = %d for root window\n", ev->x_root, ev->y_root);
	// 	} else if (bar) {
	// 		fprintf(stderr, "motionnotify: received event x = %d, y = %d for bar %s\n", ev->x_root, ev->y_root, bar->name);
	// 	} else if ((sel = wintoclient(ev->window))) {
	// 		fprintf(stderr, "motionnotify: received event x = %d, y = %d for client %s\n", ev->x_root, ev->y_root, sel->name);
	// 	} else {
	// 		fprintf(stderr, "motionnotify: received event x = %d, y = %d for no window?\n", ev->x_root, ev->y_root);
	// 	}
	// }

	/* Mouse cursor moves over a bar, trigger bar hover mechanisms */
	if (bar) {
		if (bar->mon != selmon)
			entermon(bar->mon, NULL);
		barhover(e, bar);
		return;
	}

	if (selmon->preview && selmon->preview->show)
		hidepreview(selmon);

	if (ev->window != root)
		return;

	/* Mouse cursor moves from one workspace to another */
	if (!ISSTICKY(selws->sel) && (ws = recttows(ev->x_root, ev->y_root, 1, 1)) && ws != selws) {
		if (selmon != ws->mon) {
			entermon(ws->mon, NULL);
		} else {
			sel = selws->sel;
			selws = ws;
			selmon->selws = ws;
			unfocus(sel, 1, NULL);
			focus(NULL);
			drawbar(selmon);
			updatecurrentdesktop();
		}
		return;
	}

	/* Mouse cursor moves from one monitor to another */
	if (!ISSTICKY(selws->sel) && (m = recttomon(ev->x_root, ev->y_root, 1, 1)) != selmon) {
		entermon(m, NULL);
		focus(NULL);
	}
}

unsigned long long
now(void) {
	struct timespec currentTime;
	clock_gettime(CLOCK_REALTIME, &currentTime);
	return currentTime.tv_sec * 1000LL + currentTime.tv_nsec / 1000000LL;
}

/* The structure for PropertyNotify events contains:
 *
 * typedef struct {
 *     int type;              // PropertyNotify
 *     unsigned long serial;  // # of last request processed by server
 *     Bool send_event;       // true if this came from a SendEvent request
 *     Display *display;      // Display the event was read from
 *     Window window;
 *     Atom atom;
 *     Time time;
 *     int state;             // PropertyNewValue or PropertyDelete
 * } XPropertyEvent;
 *
 * https://www.x.org/releases/current/doc/man/man3/XPropertyEvent.3.xhtml
 */
void
propertynotify(XEvent *e)
{
	Client *c;
	Window trans;
	XEvent ignored;
	XPropertyEvent *ev = &e->xproperty;

	/* Some programs may end up spamming property notifications rendering the window
	 * manager unable to do anything else than to process the backlog of these for as
	 * long as it takes. One may think of it as a DOS attack for the window manager.
	 * This section handles throttling of such events allowing only three consecutive
	 * and identical property notifications to be processed. */
	if (ev->state != pn_prev_state || ev->window != pn_prev_win || ev->atom != pn_prev_atom) {
		pn_prev_state = ev->state;
		pn_prev_win = ev->window;
		pn_prev_atom = ev->atom;
		pn_prev_count = 0;
	} else if (pn_prev_count > 3) {
		if (pn_prev_count == 4 && enabled(Debug)) {
			pn_prev_count++; /* Only print the below log line once. */
			fprintf(stderr, "propertynotify: throttling repeating %s (%ld) property notificatons for window %ld\n", XGetAtomName(dpy, ev->atom), ev->atom, ev->window);
		}
		while (XCheckMaskEvent(dpy, PropertyChangeMask, &ignored));
		return;
	}

	pn_prev_count++;

	if (systray && (c = wintosystrayicon(ev->window))) {
		if (ev->atom == XA_WM_NORMAL_HINTS) {
			updatesizehints(c);
			updatesystrayicongeom(c, c->w, c->h);
		} else {
			updatesystrayiconstate(c, ev);
		}
		drawbarwin(systray->bar);
	}

	if (ev->state == PropertyDelete) {
		if (enabled(Debug)) {
			if ((c = wintoclient(ev->window))) {
				fprintf(stderr, "propertynotify: ignored property delete event %s (%ld) for client %s\n", XGetAtomName(dpy, ev->atom), ev->atom, c->name);
			} else {
				fprintf(stderr, "propertynotify: ignored property delete event %s (%ld) for unknown client\n", XGetAtomName(dpy, ev->atom), ev->atom);
			}
		}
		return; /* ignore */
	} else if ((c = wintoclient(ev->window))) {

		if ((enabled(Debug) || DEBUGGING(c)) && ev->atom != netatom[NetWMUserTime])
			fprintf(stderr, "propertynotify: received message type of %s (%ld) for client %s\n", XGetAtomName(dpy, ev->atom), ev->atom, c->name);

		switch (ev->atom) {
		default: break;
		case XA_WM_TRANSIENT_FOR:
			if (IGNOREPROPTRANSIENTFOR(c))
				break;
			XGetTransientForHint(dpy, c->win, &trans);
			setflag(c, Floating, (wintoclient(trans)) != NULL);
			if (WASFLOATING(c) && ISFLOATING(c))
				arrange(c->ws);
			break;
		case XA_WM_NORMAL_HINTS:
			addflag(c, RefreshSizeHints);
			break;
		case XA_WM_HINTS:
			updatewmhints(c);
			if (ISURGENT(c))
				drawbars();
			break;
		}
		if (ev->atom == XA_WM_NAME || ev->atom == netatom[NetWMName]) {
			updatetitle(c);
			if (!reapplyrules(c) && c == c->ws->sel)
				drawbar(c->ws->mon);
		}
		else if (ev->atom == motifatom)
			updatemotifhints(c);
		else if (ev->atom == netatom[NetWMIcon]) {
			updateicon(c);
			if (c == selws->sel)
				drawbar(selws->mon);
		} else if (ev->atom == wmatom[WMClass]) {
			saveclientclass(c);
			if (!reapplyrules(c))
				drawbars();
		}
	}
}

void
restart(const Arg *arg)
{
	restartwm = 1;
	running = 0;
}

void
quit(const Arg *arg)
{
	running = 0;
}

void
raiseclient(Client *c)
{
	Client *s, *top = NULL;
	Workspace *ws;
	XWindowChanges wc;
	int raised = 0;

	/* If the raised client is on the sticky workspace, then refer to the previously
	 * selected workspace when for searching other clients. */
	ws = (!c ? selws : c->ws == stickyws ? stickyws->next : c->ws);
	wc.stack_mode = Above;
	wc.sibling = ws->mon->bar ? ws->mon->bar->win : wmcheckwin;

	/* If the raised client is always on top, then it should be raised first. */
	if (ALWAYSONTOP(c) || ISTRANSIENT(c) || ISSTICKY(c)) {
		top = c;
		XRaiseWindow(dpy, c->win);
		wc.stack_mode = Below;
		wc.sibling = c->win;
		raised = 1;
	}

	/* Check if there are floating always on top clients that need to be on top. */
	for (s = ws->stack; s; s = s->snext) {
		if (s == c || !ISFLOATING(s) || !(ALWAYSONTOP(s) || ISTRANSIENT(s)))
			continue;

		if (!top) {
			top = s;
			XRaiseWindow(dpy, s->win);
			wc.stack_mode = Below;
			wc.sibling = s->win;
			continue;
		}

		XConfigureWindow(dpy, s->win, CWSibling|CWStackMode, &wc);
		wc.sibling = s->win;
	}

	/* Otherwise check if there are sticky clients first that need to be on top. */
	for (s = stickyws->stack; s; s = s->snext) {
		if (s == c)
			continue;

		if (!top) {
			top = s;
			XRaiseWindow(dpy, s->win);
			wc.stack_mode = Below;
			wc.sibling = s->win;
			continue;
		}

		XConfigureWindow(dpy, s->win, CWSibling|CWStackMode, &wc);
		wc.sibling = s->win;
	}

	if (raised)
		return;

	if (top) {
		XConfigureWindow(dpy, c->win, CWSibling|CWStackMode, &wc);
		return;
	}

	XRaiseWindow(dpy, c->win);
}

/* This reads the stacking order on the X server side and updates the client
 * index (idx) value accordingly. This information can later be used to determine
 * whether one window is on top of another, for example in recttoclient.
 */
void
readclientstackingorder(void)
{
	unsigned int i, num;
	Window d1, d2, *wins = NULL;
	Client *c;

	if (XQueryTree(dpy, root, &d1, &d2, &wins, &num)) {
		for (i = 0; i < num; i++) {
			if ((c = wintoclient(wins[i])))
				c->idx = i + 1;
		}

		XFree(wins);
	}
}

Client *
recttoclient(int x, int y, int w, int h, int include_floating)
{
	Client *c, *r = NULL;
	int a, area = 1;

	for (c = selws->stack; c; c = c->snext) {
		if (!ISVISIBLE(c) || (ISFLOATING(c) && !include_floating))
			continue;
		if ((a = INTERSECTC(x, y, w, h, c)) >= area && (!r || r->idx < c->idx)) {
			area = a;
			r = c;
		}
	}
	return r;
}

Monitor *
recttomon(int x, int y, int w, int h)
{
	Monitor *m, *r = selmon;
	int a, area = 0;

	for (m = mons; m; m = m->next)
		if ((a = INTERSECT(x, y, w, h, m)) > area) {
			area = a;
			r = m;
		}
	return r;
}

Workspace *
recttows(int x, int y, int w, int h)
{
	Workspace *ws, *r = NULL;
	int a, area = 0;

	for (ws = workspaces; ws; ws = ws->next)
		if (ws->visible && ws != stickyws && (a = INTERSECT(x, y, w, h, ws)) > area) {
			area = a;
			r = ws;
		}
	return r;
}

void
resize(Client *c, int tx, int ty, int tw, int th, int interact)
{
	int wh = tw, hh = th;
	show(c);
	if (applysizehints(c, &tx, &ty, &wh, &hh, interact))
		resizeclientpad(c, tx, ty, wh, hh, tw, th);
}

void
resizeclient(Client *c, int x, int y, int w, int h)
{
	resizeclientpad(c, x, y, w, h, w, h);
}

void
resizeclientpad(Client *c, int x, int y, int w, int h, int tw, int th)
{
	XWindowChanges wc;

	if (!ISLOCKED(c)) {
		c->oldx = c->x;
		c->oldy = c->y;
		c->oldw = c->w;
		c->oldh = c->h;
	}
	wc.border_width = c->bw;
	c->x = wc.x = x;
	c->y = wc.y = y;
	c->w = wc.width = w;
	c->h = wc.height = h;

	if (enabled(CenterSizeHintsClients) && !ISFLOATING(c) && c->ws->layout->arrange) {
		if (w != tw)
			c->x = wc.x += (tw - w) / 2;
		if (h != th)
			c->y = wc.y += (th - h) / 2;
	}

	if (!c->ws->visible || MOVEPLACE(c)) {
		addflag(c, NeedResize);
		return;
	}

	if (noborder(c)) {
		wc.width += c->bw * 2;
		wc.height += c->bw * 2;
		wc.border_width = 0;
	}

	XConfigureWindow(dpy, c->win, CWX|CWY|CWWidth|CWHeight|CWBorderWidth, &wc);
	configure(c);
	XSync(dpy, False);
}

void
restack(Workspace *ws)
{
	Client *c = ws->sel;
	Client *s, *raised;
	XWindowChanges wc;

	if (!c)
		return;

	raised = (enabled(FocusedOnTopTiled) || ISFLOATING(c) ? c : NULL);

	/* Place tiled clients below the bar window */
	if (ws->layout->arrange) {
		wc.stack_mode = Below;
		wc.sibling = ws->mon->bar ? ws->mon->bar->win : wmcheckwin;
		for (s = ws->stack; s; s = s->snext) {
			if (TILED(s) && s != raised) {
				XConfigureWindow(dpy, s->win, CWSibling|CWStackMode, &wc);
				wc.sibling = s->win;
			}
		}
	}

	if (raised)
		raiseclient(raised);

	XSync(dpy, False);
	if (canwarp(c))
		warp(c);

	skipfocusevents();
}

void
run(void)
{
	int event_count = 0;
	const int MAX_EVENTS = 10;
	struct epoll_event events[MAX_EVENTS];

	XSync(dpy, False);

	/* main event loop */
	while (running) {
		event_count = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);

		for (int i = 0; i < event_count; i++) {
			int event_fd = events[i].data.fd;
			DEBUG("Got event from fd %d\n", event_fd);

			if (event_fd == dpy_fd) {
				// -1 means EPOLLHUP
				if (handlexevent(events + i) == -1)
					return;
			} else if (event_fd == ipc_get_sock_fd()) {
				ipc_handle_socket_epoll_event(events + i);
			} else if (ipc_is_client_registered(event_fd)) {
				if (ipc_handle_client_epoll_event(events + i, mons, &lastselmon, selmon,
						num_workspaces, layouts, LENGTH(layouts)) < 0) {
					fprintf(stderr, "Error handling IPC event on fd %d\n", event_fd);
				}
			} else {
				fprintf(stderr, "Got event from unknown fd %d, ptr %p, u32 %d, u64 %lu",
				event_fd, events[i].data.ptr, events[i].data.u32,
				events[i].data.u64);
				fprintf(stderr, " with events %d\n", events[i].events);
			}
		}
	}
}

void
scan(void)
{
	scanner = 1;
	char swin[256] = {0};
	unsigned int i, num;
	Window d1, d2, *wins = NULL;
	XWindowAttributes wa;

	if (XQueryTree(dpy, root, &d1, &d2, &wins, &num)) {
		for (i = 0; i < num; i++) {
			if (!XGetWindowAttributes(dpy, wins[i], &wa)
			|| wa.override_redirect || XGetTransientForHint(dpy, wins[i], &d1))
				continue;
			if (wa.map_state == IsViewable || getstate(wins[i]) == IconicState)
				manage(wins[i], &wa);
			else if (gettextprop(wins[i], netatom[NetClientList], swin, sizeof swin))
				manage(wins[i], &wa);
		}
		for (i = 0; i < num; i++) { /* now the transients */
			if (!XGetWindowAttributes(dpy, wins[i], &wa) || wa.override_redirect)
				continue;
			if (XGetTransientForHint(dpy, wins[i], &d1)
			&& (wa.map_state == IsViewable || getstate(wins[i]) == IconicState))
				manage(wins[i], &wa);
		}
		XFree(wins);
	}
	scanner = 0;
}

void
setbackground()
{
	int di;
	unsigned long dl;
	unsigned char *p = NULL;
	Atom da, atom = None;
	/* Set a solid background, but only if a wallpaper has not been set. */
	if (!(XGetWindowProperty(dpy, root, XInternAtom(dpy, "_XROOTPMAP_ID", False), 0L, sizeof atom,
			False, AnyPropertyType, &da, &di, &dl, &dl, &p) == Success && p)) {
		XSetWindowBackground(dpy, root, scheme[SchemeNorm][ColBg].pixel);
		XClearWindow(dpy, root);
	}
}

void
setclientstate(Client *c, long state)
{
	long data[] = { state, None };

	XChangeProperty(dpy, c->win, wmatom[WMState], wmatom[WMState], 32,
		PropModeReplace, (unsigned char *)data, 2);
}

int
sendevent(Window w, Atom proto, int mask, long d0, long d1, long d2, long d3, long d4)
{
	int n;
	Atom *protocols;
	Atom mt;
	int exists = 0;
	XEvent ev;

	if (proto == wmatom[WMTakeFocus] || proto == wmatom[WMDelete]) {
		mt = wmatom[WMProtocols];
		if (XGetWMProtocols(dpy, w, &protocols, &n)) {
			while (!exists && n--)
				exists = protocols[n] == proto;
			XFree(protocols);
		}
	} else {
		exists = True;
		mt = proto;
	}

	if (exists) {
		ev.type = ClientMessage;
		ev.xclient.window = w;
		ev.xclient.message_type = mt;
		ev.xclient.format = 32;
		ev.xclient.data.l[0] = d0;
		ev.xclient.data.l[1] = d1;
		ev.xclient.data.l[2] = d2;
		ev.xclient.data.l[3] = d3;
		ev.xclient.data.l[4] = d4;
		XSendEvent(dpy, w, False, mask, &ev);
	}
	return exists;
}

void
setfocus(Client *c)
{
	if (!NEVERFOCUS(c)) {
		XSetInputFocus(dpy, c->win, RevertToPointerRoot, CurrentTime);
		XChangeProperty(dpy, root, netatom[NetActiveWindow],
			XA_WINDOW, 32, PropModeReplace,
			(unsigned char *) &(c->win), 1);
	}
	selws->sel = c;
	if (selws != c->ws)
		c->ws->sel = c;

	if (STEAMGAME(c))
		setclientstate(c, NormalState);
	sendevent(c->win, wmatom[WMTakeFocus], NoEventMask, wmatom[WMTakeFocus], CurrentTime, 0, 0, 0);
}

void
setfullscreen(Client *c, int fullscreen, int restorefakefullscreen)
{
	Monitor *m = c->ws->mon;
	int savestate = 0, restorestate = 0;

	if ((!ISFAKEFULLSCREEN(c) && fullscreen && !ISFULLSCREEN(c)) // normal fullscreen
			|| (RESTOREFAKEFULLSCREEN(c) && fullscreen)) // fake fullscreen --> actual fullscreen
		savestate = 1; // go actual fullscreen
	else if ((!ISFAKEFULLSCREEN(c) && !fullscreen && ISFULLSCREEN(c)) // normal fullscreen exit
			|| ((RESTOREFAKEFULLSCREEN(c) || restorefakefullscreen) && !fullscreen)) // fullscreen exit --> fake fullscreen
		restorestate = 1; // go back into tiled

	/* If leaving fullscreen and the window was previously fake fullscreen, then restore that while
	 * staying in fullscreen. The exception to this is if we are in said state, but the client
	 * itself disables fullscreen (restorefakefullscreen) then we let the client go out of while
	 * keeping fake fullscreen enabled (as otherwise there will be a mismatch between the client
	 * and the window manager's perception of the client's fullscreen state). */
	if (RESTOREFAKEFULLSCREEN(c) && !fullscreen && ISFULLSCREEN(c)) {
		restorefakefullscreen = 1;
		fullscreen = 1;
	}

	if (fullscreen != ISFULLSCREEN(c)) { // only send property change if necessary
		if (fullscreen)
			XChangeProperty(dpy, c->win, netatom[NetWMState], XA_ATOM, 32,
				PropModeReplace, (unsigned char*)&netatom[NetWMFullscreen], 1);
		else
			XChangeProperty(dpy, c->win, netatom[NetWMState], XA_ATOM, 32,
				PropModeReplace, (unsigned char*)0, 0);
	}

	setflag(c, FullScreen, fullscreen);

	/* Some clients, e.g. firefox, will send a client message informing the window manager
	 * that it is going into fullscreen after receiving the above signal. This has the side
	 * effect of this function (setfullscreen) sometimes being called twice when toggling
	 * fullscreen on and off via the window manager as opposed to the application itself.
	 * To protect against obscure issues where the client settings are stored or restored
	 * when they are not supposed to we add an additional bit-lock so that settings can
	 * only be stored and restored in that precise order. */
	if (savestate && !ISLOCKED(c)) {
		c->oldbw = c->bw;
		c->bw = 0;
		SETFLOATING(c);
		resizeclient(c, m->mx, m->my, m->mw, m->mh);
		XRaiseWindow(dpy, c->win);
		LOCK(c);
	} else if (restorestate && ISLOCKED(c)) {
		UNLOCK(c);
		c->bw = c->oldbw;
		setflag(c, Floating, WASFLOATING(c));
		if (restorefakefullscreen) {
			addflag(c, FakeFullScreen);
			removeflag(c, RestoreFakeFullScreen);
		}
		/* The client may have been moved to another monitor whilst in fullscreen which if tiled
		 * we address by doing a full arrange of tiled clients. If the client is floating then the
		 * height and width may be larger than the monitor's window area, so we cap that by
		 * ensuring max / min values. */
		if (ISFLOATING(c)) {
			c->x = MAX(m->wx, c->oldx);
			c->y = MAX(m->wy, c->oldy);
			c->w = MIN(m->ww - c->x + m->wx - 2*c->bw, c->oldw);
			c->h = MIN(m->wh - c->y + m->wy - 2*c->bw, c->oldh);
			resizeclient(c, c->x, c->y, c->w, c->h);
			restack(c->ws);
		} else
			arrange(c->ws);
	} else
		resizeclient(c, c->x, c->y, c->w, c->h);

	/* Exception: if the client was in actual fullscreen and we exit out to fake fullscreen
	 * mode, then the focus would sometimes drift to whichever window is under the mouse cursor
	 * at the time. To avoid this we ask X for all EnterNotify events and just ignore them.
	 */
	if (!ISFULLSCREEN(c))
		skipfocusevents();
}

void
setlayout(const Arg *arg)
{
	Workspace *ws = selws;
	const Layout *tmplayout;

	if (!ws || !arg || (arg->i > 0 && arg->i > LENGTH(layouts))) {
		return;
	}

	if (arg->i < 0) {
		tmplayout = ws->layout;
		ws->layout = ws->prevlayout;
		ws->prevlayout = tmplayout;
	} else if (&layouts[arg->i] != ws->layout) {
		ws->prevlayout = ws->layout;
		ws->layout = &layouts[arg->i];
	}

	if (ws->layout->preset.nmaster != -1)
		ws->nmaster = ws->layout->preset.nmaster;
	if (ws->layout->preset.nstack != -1)
		ws->nstack = ws->layout->preset.nstack;

	ws->ltaxis[LAYOUT] = ws->layout->preset.layout;
	ws->ltaxis[MASTER] = ws->layout->preset.masteraxis;
	ws->ltaxis[STACK]  = ws->layout->preset.stack1axis;
	ws->ltaxis[STACK2] = ws->layout->preset.stack2axis;

	strlcpy(ws->ltsymbol, ws->layout->symbol, sizeof ws->ltsymbol);

	arrange(ws);
	setfloatinghints(ws);
}

/* arg > 1.0 will set mfact absolutely */
void
setmfact(const Arg *arg)
{
	float f;
	Workspace *ws = selws;

	if (!arg || !ws->layout->arrange)
		return;
	f = arg->f < 1.0 ? arg->f + ws->mfact : arg->f - 1.0;
	if (f < 0.05 || f > 0.95)
		return;

	ws->mfact = f;
	arrangews(ws);
}

void
setup(void)
{
	Monitor *m;
	int i, colorscheme;
	XSetWindowAttributes wa;
	Atom utf8string;
	struct sigaction chld, hup, term;

	/* Handle children when they terminate. */
	sigemptyset(&chld.sa_mask);
	chld.sa_flags = SA_NOCLDSTOP | SA_RESTART;
	chld.sa_handler = sigchld;
	sigaction(SIGCHLD, &chld, NULL);

	/* Handle hang up signal */
	sigemptyset(&hup.sa_mask);
	hup.sa_flags = SA_RESTART;
	hup.sa_handler = sighup;
	sigaction(SIGHUP, &hup, NULL);

	/* Handle terminate signal */
	sigemptyset(&term.sa_mask);
	term.sa_flags = SA_RESTART;
	term.sa_handler = sigterm;
	sigaction(SIGTERM, &term, NULL);

	/* Clean up any zombies (inherited from .xinitrc etc) immediately. */
	while (waitpid(-1, NULL, WNOHANG) > 0);

	putenv("_JAVA_AWT_WM_NONREPARENTING=1");

	enablefunc(functionality);

	/* init screen */
	screen = DefaultScreen(dpy);
	sw = DisplayWidth(dpy, screen);
	sh = DisplayHeight(dpy, screen);
	root = RootWindow(dpy, screen);
	xinitvisual();
	drw = drw_create(dpy, screen, root, sw, sh, visual, depth, cmap);

	/* init appearance */
	scheme = ecalloc(LENGTH(colors) + 1, sizeof(Clr *));
	scheme[LENGTH(colors)] = drw_scm_create(drw, colors[0], default_alphas, 3); // ad-hoc color scheme used by status2d

	for (i = 0; i < LENGTH(colors); i++) {
		colorscheme = i;
		/* Fall back to SchemeTitleNorm / Sel for SchemeFlex colors if not defined. */
		if (!colors[i][0]) {
			colorscheme = (i >= SchemeFlexSelTTB ? SchemeTitleSel : SchemeTitleNorm);
		}
		scheme[i] = drw_scm_create(drw, colors[colorscheme], default_alphas, 3);
	}

	if (enabled(Xresources))
		loadxrdb();

	if (!drw_fontset_create(drw, fonts, LENGTH(fonts)))
		die("no fonts could be loaded.");

	lrpad = drw->fonts->h + horizpadbar;
	bh = bar_height ? bar_height : drw->fonts->h + vertpadbar;

	/* One off calculating workspace label widths, used by WorkspaceLabels functionality */
	occupied_workspace_label_format_length = TEXT2DW(occupied_workspace_label_format) - TEXTW(workspace_label_placeholder) * 2;
	vacant_workspace_label_format_length = TEXT2DW(vacant_workspace_label_format) - TEXTW(workspace_label_placeholder);

	updategeom(sw, sh);

	/* init atoms */
	utf8string = XInternAtom(dpy, "UTF8_STRING", False);
	motifatom = XInternAtom(dpy, "_MOTIF_WM_HINTS", False);
	XInternAtoms(dpy, wmatom_names, WMLast, False, wmatom);
	XInternAtoms(dpy, dusk_names, DuskLast, False, duskatom);
	XInternAtoms(dpy, netatom_names, NetLast, False, netatom);
	XInternAtoms(dpy, allowed_names, NetWMActionLast, False, allowed);
	XInternAtoms(dpy, xembed_names, XLast, False, xatom);

	/* init cursors */
	cursor[CurNormal] = drw_cur_create(drw, XC_left_ptr);
	cursor[CurResize] = drw_cur_create(drw, XC_rightbutton);
	cursor[CurResizeBR] = drw_cur_create(drw, XC_bottom_right_corner);
	cursor[CurResizeBL] = drw_cur_create(drw, XC_bottom_left_corner);
	cursor[CurResizeTR] = drw_cur_create(drw, XC_top_right_corner);
	cursor[CurResizeTL] = drw_cur_create(drw, XC_top_left_corner);
	cursor[CurResizeHorzArrow] = drw_cur_create(drw, XC_sb_h_double_arrow);
	cursor[CurResizeVertArrow] = drw_cur_create(drw, XC_sb_v_double_arrow);
	cursor[CurIronCross] = drw_cur_create(drw, XC_iron_cross);
	cursor[CurMove] = drw_cur_create(drw, XC_fleur);
	cursor[CurSwallow] = drw_cur_create(drw, XC_target);

	createworkspaces();
	updatebars();
	initsystray();

	/* supporting window for NetWMCheck */
	wmcheckwin = XCreateSimpleWindow(dpy, root, 0, 0, 1, 1, 0, 0, 0);
	XChangeProperty(dpy, wmcheckwin, netatom[NetWMCheck], XA_WINDOW, 32,
		PropModeReplace, (unsigned char *) &wmcheckwin, 1);
	XChangeProperty(dpy, wmcheckwin, netatom[NetWMName], utf8string, 8,
		PropModeReplace, (unsigned char *) "dusk", 4);
	XChangeProperty(dpy, root, netatom[NetWMCheck], XA_WINDOW, 32,
		PropModeReplace, (unsigned char *) &wmcheckwin, 1);
	/* EWMH support per view */
	XChangeProperty(dpy, root, netatom[NetSupported], XA_ATOM, 32,
		PropModeReplace, (unsigned char *) netatom, NetLast);
	setbackground();
	setnumdesktops();
	updatecurrentdesktop();
	setdesktopnames();
	setviewport();
	restorepids();
	XDeleteProperty(dpy, root, netatom[NetClientList]);
	XDeleteProperty(dpy, root, netatom[NetClientListStacking]);
	/* select events */
	wa.cursor = cursor[CurNormal]->cursor;
	wa.event_mask = SubstructureRedirectMask|SubstructureNotifyMask
		|ButtonPressMask|PointerMotionMask|EnterWindowMask
		|LeaveWindowMask|StructureNotifyMask|PropertyChangeMask;
	XChangeWindowAttributes(dpy, root, CWEventMask|CWCursor, &wa);
	XSelectInput(dpy, root, wa.event_mask);

	#ifdef HAVE_LIBXI
	if (!XQueryExtension(dpy, "XInputExtension", &xi_opcode, &i, &i)) {
		fprintf(stderr, "Warning: XInput is not available.");
	}
	/* Tell XInput to send us all RawMotion events. */
	unsigned char mask_bytes[XIMaskLen(XI_LASTEVENT)];
	memset(mask_bytes, 0, sizeof(mask_bytes));
	XISetMask(mask_bytes, XI_RawMotion);
	XISetMask(mask_bytes, XI_RawKeyRelease);
	XISetMask(mask_bytes, XI_RawTouchBegin);
	XISetMask(mask_bytes, XI_RawTouchEnd);
	XISetMask(mask_bytes, XI_RawTouchUpdate);

	XIEventMask mask;
	mask.deviceid = XIAllMasterDevices;
	mask.mask_len = sizeof(mask_bytes);
	mask.mask = mask_bytes;
	XISelectEvents(dpy, root, &mask, 1);
	#endif
	grabkeys();
	focus(NULL);
	setupepoll();

	for (m = mons; m; m = m->next)
		showws(m->selws);
	selws = selmon->selws;
}

void
seturgent(Client *c, int urg)
{
	XWMHints *wmh;

	setflag(c, Urgent, urg);
	if (!(wmh = XGetWMHints(dpy, c->win)))
		return;
	wmh->flags = urg ? (wmh->flags | XUrgencyHint) : (wmh->flags & ~XUrgencyHint);
	XSetWMHints(dpy, c->win, wmh);
	XFree(wmh);
}

void
show(Client *c)
{
	XMoveWindow(dpy, c->win, c->x, c->y);
}

void
skipfocusevents(void)
{
	XEvent ev;
	while (XCheckMaskEvent(dpy, EnterWindowMask, &ev)); // skip any new EnterNotify events
}

void
spawn(const Arg *arg)
{
	spawncmd(arg, 0, 1);
}

pid_t
spawncmd(const Arg *arg, int buttonclick, int orphan)
{
	struct sigaction sa;
	pid_t pid = fork();

	if (pid == 0) {

		if (orphan && fork() != 0)
			exit(EXIT_SUCCESS);

		if (dpy)
			close(ConnectionNumber(dpy));
		if (enabled(SpawnCwd) && selws->sel) {
			const char* const home = getenv("HOME");
			assert(home && strchr(home, '/'));
			const size_t homelen = strlen(home);
			char *cwd, *pathbuf = NULL;
			struct stat statbuf;

			cwd = strtok(selws->sel->name, SPAWN_CWD_DELIM);
			/* NOTE: strtok() alters selws->sel->name in-place,
			 * but that does not matter because we are going to
			 * exec() below anyway; nothing else will use it */
			while (cwd) {
				if (*cwd == '~') { /* replace ~ with $HOME */
					if (!(pathbuf = malloc(homelen + strlen(cwd)))) /* ~ counts for NULL term */
						die("fatal: could not malloc() %u bytes\n", homelen + strlen(cwd));
					strcpy(strcpy(pathbuf, home) + homelen, cwd + 1);
					cwd = pathbuf;
				}

				if (strchr(cwd, '/') && !stat(cwd, &statbuf)) {
					if (!S_ISDIR(statbuf.st_mode))
						cwd = dirname(cwd);

					if (strlen(cwd) > 1 && !chdir(cwd))
						break;
				}

				cwd = strtok(NULL, SPAWN_CWD_DELIM);
			}

			free(pathbuf);
		}

		if (buttonclick) {
			char button[11];
			sprintf(button, "%d", buttonclick);
			setenv("BLOCK_BUTTON", button, 1);
		}

		if (statusclicked > -1) {
			char status[11];
			sprintf(status, "%d", statusclicked);
			setenv("BLOCK_STATUS", status, 1);
			statusclicked = -1;
		}

		setsid();

		/* Restore SIGCHLD sighandler to default before spawning a program */
		sigemptyset(&sa.sa_mask);
		sa.sa_flags = 0;
		sa.sa_handler = SIG_DFL;
		sigaction(SIGCHLD, &sa, NULL);

		execvp(((char **)arg->v)[1], ((char **)arg->v)+1);
		fprintf(stderr, "dusk: execvp %s", ((char **)arg->v)[1]);
		perror(" failed");
		exit(EXIT_SUCCESS);
	}
	return pid;
}

void
structurenotify(XEvent *e)
{
	Workspace *ws = NULL, *prevws = NULL;
	int multiws = 0;

	do {
		switch (e->type) {
		case UnmapNotify:
			ws = unmapnotify(e);
			break;
		case DestroyNotify:
			ws = destroynotify(e);
			break;
		case ConfigureNotify:
			ws = configurenotify(e);
			break;
		}
		if (prevws && prevws != ws)
			multiws = 1;
		prevws = ws;
	} while (XCheckMaskEvent(dpy, StructureNotifyMask|SubstructureNotifyMask, e));

	if (multiws) {
		arrange(NULL);
		focus(NULL);
		drawbars();
	} else if (ws) {
		arrange(ws);
		focus(NULL);
		drawbar(ws->mon);
	}
}

static unsigned int
textw_clamp(const char *str, unsigned int n)
{
	unsigned int w = drw_fontset_getwidth_clamp(drw, str, n);
	return MIN(w, n);
}

void
togglefloating(const Arg *arg)
{
	Client *c = CLIENT;
	Workspace *ws = NULL;
	XWindowChanges wc;
	wc.stack_mode = Above;

	for (c = nextmarked(NULL, c); c; c = nextmarked(c->next, NULL)) {
		if (ISFULLSCREEN(c) && !ISFAKEFULLSCREEN(c)) /* no support for fullscreen windows */
			continue;
		if (ISFIXED(c))
			continue;
		if (ws && c->ws != ws) {
			drawbar(ws->mon);
			arrange(ws);
		}
		toggleflag(c, Floating);
		if (!MOVERESIZE(c) && ISFLOATING(c)) {
			if (c->sfx == -9999)
				floatpos(&((Arg) { .v = toggle_float_pos }));
			else
				restorefloats(c);
			wc.sibling = c->ws->mon->bar ? c->ws->mon->bar->win : wmcheckwin;
			XConfigureWindow(dpy, c->win, CWSibling|CWStackMode, &wc);
		}

		setfloatinghint(c);
		ws = c->ws;
	}

	if (ws) {
		XSync(dpy, False);
		drawbar(ws->mon);
		arrange(ws);
	}
}

void
togglemaximize(Client *c, int maximize_vert, int maximize_horz)
{
	if (!maximize_vert && !maximize_horz)
		return;
	Workspace *ws = c->ws;
	Monitor *m = ws->mon;

	if (ISFLOATING(c)) {
		if (maximize_vert && maximize_horz) {
			if (abs(c->x - m->wx) <= m->gappov && abs(c->y - m->wy) <= m->gappoh) {
				if (!WASFLOATING(c))
					togglefloating(&((Arg) { .v = c }));
				else
					restorefloats(c);
				return;
			}
		} else if (maximize_vert && abs(c->y - m->wy) <= m->gappoh) {
			resizeclient(c,
				c->x,
				ws->wy + (c->sfy - m->wy) * ws->wh / m->wh,
				c->w,
				c->sfh * ws->wh / m->wh
			);
			return;
		} else if (maximize_horz && abs(c->x - m->wx) <= m->gappov) {
			resizeclient(
				c,
				ws->wx + (c->sfx - m->wx) * ws->ww / m->ww,
				c->y,
				c->sfw * ws->ww / m->ww,
				c->h
			);
			return;
		}
		savefloats(c);
	}

	maximize(c, maximize_vert, maximize_horz);
}

void
unfocus(Client *c, int setfocus, Client *nextfocus)
{
	XWindowChanges wc;

	if (!c)
		return;
	if (ISFULLSCREEN(c) && ISVISIBLE(c) && c->ws == selws && nextfocus && !ISFLOATING(nextfocus) && !STEAMGAME(c))
		if (!ISFAKEFULLSCREEN(c))
			setfullscreen(c, 0, 0);
	grabbuttons(c, 0);
	if (setfocus) {
		XSetInputFocus(dpy, root, RevertToPointerRoot, CurrentTime);
		XDeleteProperty(dpy, root, netatom[NetActiveWindow]);
	}

	if (enabled(FlexWinBorders))
		setwindowborders(c->ws, nextfocus);
	else
		XSetWindowBorder(dpy, c->win, scheme[clientscheme(c, nextfocus)][ColBorder].pixel);

	if (enabled(FocusedOnTopTiled) && !ISFLOATING(c)) {
		wc.stack_mode = Below;
		wc.sibling = c->ws->mon->bar ? c->ws->mon->bar->win : wmcheckwin;
		XConfigureWindow(dpy, c->win, CWSibling|CWStackMode, &wc);
	}

	c->ws->sel = NULL;
}

void
unmanage(Client *c, int destroyed)
{
	Client *s;
	Workspace *ws, *revertws;
	XWindowChanges wc;

	if (SEMISCRATCHPAD(c))
		c = unmanagesemiscratchpad(c);

	for (ws = workspaces; ws; ws = ws->next)
		if (ws->prevzoom == c)
			ws->prevzoom = NULL;

	ws = c->ws;
	revertws = c->revertws;

	s = swallowingparent(c->win);
	if (s) {
		s->swallowing = c->swallowing;
		c->swallowing = NULL;
		revertws = NULL;
	}

	if (c->swallowing) {
		unswallow(&((Arg) { .v = c }));
	} else if (enabled(AutoReduceNmaster) && ws->nmaster > 1 && ismasterclient(c)) {
		ws->nmaster--;
	}

	if (ISMARKED(c))
		unmarkclient(c);

	detach(c);
	detachstack(c);
	freeicon(c);

	if (!destroyed) {
		wc.border_width = c->oldbw;
		XGrabServer(dpy); /* avoid race conditions */
		XSetErrorHandler(xerrordummy);
		XSelectInput(dpy, c->win, NoEventMask);
		XConfigureWindow(dpy, c->win, CWBorderWidth, &wc); /* restore border */
		XUngrabButton(dpy, AnyButton, AnyModifier, c->win);
		setclientstate(c, WithdrawnState);
		XSync(dpy, False);
		XSetErrorHandler(xerror);
		XUngrabServer(dpy);
	}

	free(c);

	updateclientlist();

	if (revertws) {
		if (!revertws->visible)
			viewwsonmon(revertws, revertws->mon, 0);
		else if (ws->visible)
			viewwsonmon(ws, ws->mon, 1);
	}
}

/* The structure for XUnmapEvent events contains:
 *
 * typedef struct {
 *     int type;             // UnmapNotify
 *     unsigned long serial; // # of last request processed by server
 *     Bool send_event;      // true if this came from a SendEvent request
 *     Display *display;     // the display the event was read from
 *     Window event;         // the window the notification originates from
 *     Window window;        // the window the notification is for
 *     Bool from_configure;  // true if the event was generated as a result of a resizing of the
 *                           // window's parent when the window itself had a win_gravity of
 *                           // UnmapGravity
 * } XUnmapEvent;
 *
 * https://tronche.com/gui/x/xlib/events/window-state-change/unmap.html
 */
Workspace *
unmapnotify(XEvent *e)
{
	Client *c;
	XUnmapEvent *ev;
	Workspace *ws = NULL;

	ev = &e->xunmap;

	if (enabled(Debug))
		fprintf(stderr, "unmapnotify: received event type %s (%d), serial %ld, window %ld, event %ld, ev->send_event = %d, ev->from_configure = %d\n", XGetAtomName(dpy, ev->type), ev->type, ev->serial, ev->window, ev->event, ev->send_event, ev->from_configure);

	if ((c = wintoclient(ev->window))) {
		ws = c->ws;
		if (enabled(Debug) || DEBUGGING(c))
			fprintf(stderr, "unmapnotify: window %ld --> client %s (%s)\n", ev->window, c->name, ev->send_event ? "WithdrawnState" : "unmanage");
		if (ev->send_event)
			setclientstate(c, WithdrawnState);
		else
			unmanage(c, 0);
	} else if ((c = swallowingparent(ev->window))) {
		ws = c->ws;
		if (enabled(Debug) || DEBUGGING(c))
			fprintf(stderr, "unmapnotify: received event for swallowing client %s\n", c->name);
		if (ev->send_event)
			setclientstate(c, WithdrawnState);
		else
			unmanage(c->swallowing, 0);
	} else if (systray && (c = wintosystrayicon(ev->window))) {
		removesystrayicon(c);
		drawbarwin(systray->bar);
	}

	return ws;
}

void
updateclientlist()
{
	Client *c;
	Workspace *ws;

	XDeleteProperty(dpy, root, netatom[NetClientList]);
	for (ws = workspaces; ws; ws = ws->next)
		for (c = ws->clients; c; c = c->next)
			XChangeProperty(dpy, root, netatom[NetClientList],
				XA_WINDOW, 32, PropModeAppend,
				(unsigned char *) &(c->win), 1);

	XDeleteProperty(dpy, root, netatom[NetClientListStacking]);
	for (ws = workspaces; ws; ws = ws->next)
		for (c = ws->stack; c; c = c->snext)
			XChangeProperty(dpy, root, netatom[NetClientListStacking],
				XA_WINDOW, 32, PropModeAppend,
				(unsigned char *) &(c->win), 1);
}

int
updategeom(int width, int height)
{
	int dirty = 0;
	if (sw != width || sh != height) {
		sw = width;
		sh = height;
		dirty = 1;
	}

#ifdef XINERAMA
	if (XineramaIsActive(dpy)) {
		int i, j, n, nn;
		Monitor *m;
		XineramaScreenInfo *info = XineramaQueryScreens(dpy, &nn);
		XineramaScreenInfo *unique = NULL;

		for (n = 0, m = mons; m; m = m->next, n++);
		/* only consider unique geometries as separate screens */
		unique = ecalloc(nn, sizeof(XineramaScreenInfo));
		for (i = 0, j = 0; i < nn; i++)
			if (isuniquegeom(unique, j, &info[i]))
				memcpy(&unique[j++], &info[i], sizeof(XineramaScreenInfo));
		XFree(info);
		nn = j;
		if (enabled(SortScreens))
			sortscreens(unique, nn);

		for (i = n; i < nn; i++) {
			for (m = mons; m && m->next; m = m->next);
			if (m)
				m->next = createmon(i);
			else
				mons = createmon(i);
		}
		dummymon = createmon(7);

		for (m = mons; m && m->num < nn; m = m->next) {
			if (m->num >= n
			|| unique[m->num].x_org != m->mx || unique[m->num].y_org != m->my
			|| unique[m->num].width != m->mw || unique[m->num].height != m->mh)
			{
				dirty = 1;
				m->mx = m->wx = unique[m->num].x_org;
				m->my = m->wy = unique[m->num].y_org;
				m->mw = m->ww = unique[m->num].width;
				m->mh = m->wh = unique[m->num].height;
				m->orientation = (m->mw < m->mh);
				updatebarpos(m);
			}
		}

		reorientworkspaces();

		if (n < nn)
			redistributeworkspaces();

		for (i = nn; i < n; i++) {
			for (m = mons; m && m->next; m = m->next);
			if (m == selmon)
				selmon = mons;
			cleanupmon(m);
		}

		reviewworkspaces();

		free(unique);
	} else
#endif /* XINERAMA */
	{ /* default monitor setup */
		if (!mons)
			mons = createmon(0);
		if (mons->mw != sw || mons->mh != sh) {
			dirty = 1;
			mons->mw = mons->ww = sw;
			mons->mh = mons->wh = sh;
			mons->orientation = (mons->mw < mons->mh);
			updatebarpos(mons);
		}
	}
	if (dirty) {
		selmon = mons;
		selmon = wintomon(root);
	}
	return dirty;
}

void
updatenumlockmask(void)
{
	unsigned int i, j;
	XModifierKeymap *modmap;

	numlockmask = 0;
	modmap = XGetModifierMapping(dpy);
	for (i = 0; i < 8; i++)
		for (j = 0; j < modmap->max_keypermod; j++)
			if (modmap->modifiermap[i * modmap->max_keypermod + j]
				== XKeysymToKeycode(dpy, XK_Num_Lock))
				numlockmask = (1 << i);
	XFreeModifiermap(modmap);
}

void
updatesizehints(Client *c)
{
	long msize;
	XSizeHints size;

	if (!XGetWMNormalHints(dpy, c->win, &size, &msize))
		/* size is uninitialized, ensure that size.flags aren't used */
		size.flags = PSize;
	if (size.flags & PBaseSize) {
		c->basew = size.base_width;
		c->baseh = size.base_height;
	} else if (size.flags & PMinSize) {
		c->basew = size.min_width;
		c->baseh = size.min_height;
	} else
		c->basew = c->baseh = 0;
	if (size.flags & PResizeInc) {
		c->incw = size.width_inc;
		c->inch = size.height_inc;
	} else
		c->incw = c->inch = 0;
	if (size.flags & PMaxSize) {
		c->maxw = size.max_width;
		c->maxh = size.max_height;
	} else
		c->maxw = c->maxh = 0;
	if (!IGNOREMINIMUMSIZEHINTS(c) && size.flags & PMinSize) {
		c->minw = size.min_width;
		c->minh = size.min_height;
	} else if (!IGNOREMINIMUMSIZEHINTS(c) && size.flags & PBaseSize) {
		c->minw = size.base_width;
		c->minh = size.base_height;
	} else
		c->minw = c->minh = 0;
	if (size.flags & PAspect) {
		c->mina = (float)size.min_aspect.y / size.min_aspect.x;
		c->maxa = (float)size.max_aspect.x / size.max_aspect.y;
	} else
		c->maxa = c->mina = 0.0;
	setflag(c, Fixed, (c->maxw && c->maxh && c->maxw == c->minw && c->maxh == c->minh));
	removeflag(c, RefreshSizeHints);
}

void
updatetitle(Client *c)
{
	if (!gettextprop(c->win, netatom[NetWMName], c->name, sizeof c->name))
		gettextprop(c->win, XA_WM_NAME, c->name, sizeof c->name);
	if (c->name[0] == '\0') /* hack to mark broken clients */
		strcpy(c->name, broken);
}

void
updatewmhints(Client *c)
{
	XWMHints *wmh;

	if ((wmh = XGetWMHints(dpy, c->win))) {
		if (c == selws->sel && wmh->flags & XUrgencyHint) {
			wmh->flags &= ~XUrgencyHint;
			XSetWMHints(dpy, c->win, wmh);
		} else
			setflag(c, Urgent, wmh->flags & XUrgencyHint);

		if (ISURGENT(c))
			XSetWindowBorder(dpy, c->win, scheme[SchemeUrg][ColBorder].pixel);

		setflag(c, NeverFocus, wmh->flags & InputHint ? !wmh->input : 0);
		XFree(wmh);
	}
}

Client *
wintoclient(Window w)
{
	Client *c;
	Workspace *ws;

	for (ws = workspaces; ws; ws = ws->next)
		for (c = ws->clients; c; c = c->next)
			if (c->win == w)
				return c;
	return NULL;
}

Monitor *
wintomon(Window w)
{
	int x, y;
	Client *c;
	Monitor *m;
	Bar *bar;

	if (w == root && getrootptr(&x, &y))
		return recttomon(x, y, 1, 1);
	for (m = mons; m; m = m->next)
		for (bar = m->bar; bar; bar = bar->next)
			if (w == bar->win)
				return m;
	if ((c = wintoclient(w)))
		return c->ws->mon;

	return selmon;
}

/* There's no way to check accesses to destroyed windows, thus those cases are
 * ignored (especially on UnmapNotify's). Other types of errors call Xlibs
 * default error handler, which may call exit. */
int
xerror(Display *dpy, XErrorEvent *ee)
{
	Client *i;
	if (ee->error_code == BadWindow) {
		if ((i = wintosystrayicon(ee->resourceid)))
			addflag(i, Unmanaged);
		return 0;
	} else if ((ee->request_code == X_GetAtomName && ee->error_code == BadAtom)
	|| (ee->request_code == X_SetInputFocus && ee->error_code == BadMatch)
	|| (ee->request_code == X_PolyText8 && ee->error_code == BadDrawable)
	|| (ee->request_code == X_PolyFillRectangle && ee->error_code == BadDrawable)
	|| (ee->request_code == X_PolySegment && ee->error_code == BadDrawable)
	|| (ee->request_code == X_ConfigureWindow && ee->error_code == BadMatch)
	|| (ee->request_code == X_GrabButton && ee->error_code == BadAccess)
	|| (ee->request_code == X_GrabKey && ee->error_code == BadAccess)
	|| (ee->request_code == X_CopyArea && ee->error_code == BadDrawable)) {
		if (enabled(Debug))
			fprintf(stderr, "xerror: got XErrorEvent type %d serial %ld, error code %d request code %d minor code %d resource ID %ld\n", ee->type, ee->serial, ee->error_code, ee->request_code, ee->minor_code, ee->resourceid);
		return 0;
	}
	fprintf(stderr, "dusk: fatal error: request code=%d, error code=%d\n",
		ee->request_code, ee->error_code);
	return xerrorxlib(dpy, ee); /* may call exit */
}

int
xerrordummy(Display *dpy, XErrorEvent *ee)
{
	return 0;
}

/* Startup Error handler to check if another window manager
 * is already running. */
int
xerrorstart(Display *dpy, XErrorEvent *ee)
{
	die("dusk: another window manager is already running");
	return -1;
}

void
zoom(const Arg *arg)
{
	Client *c = CLIENT, *master = NULL;
	Workspace *ws;

	if (!c)
		return;

	ws = c->ws;
	if (c && ISFLOATING(c))
		togglefloating(&((Arg) { .v = c }));

	if (!ws->layout->arrange || (c && ISFLOATING(c)) || !c)
		return;

	if (ismasterclient(c)) {
		master = c;
		if (ws->prevzoom && ws->prevzoom->ws == ws) {
			c = ws->prevzoom;
		} else {
			c = nthstack(ws->clients, 1, 1);
		}
	} else {
		master = nexttiled(ws->clients);
	}

	swap(master, c);
	ws->prevzoom = master;
	focus(c);

	arrange(c->ws);
}

int
main(int argc, char *argv[])
{
	if (argc == 2 && !strcmp("-v", argv[1]))
		die("dusk-"VERSION);
	else if (argc != 1)
		die("usage: dusk [-v]");
	if (!setlocale(LC_CTYPE, "") || !XSupportsLocale())
		fputs("warning: no locale support\n", stderr);
	if (!(dpy = XOpenDisplay(NULL)))
		die("dusk: cannot open display");

	if (!(xcon = XGetXCBConnection(dpy)))
		die("dusk: cannot get xcb connection\n");

	checkotherwm();
	XrmInitialize(); // needed for xrdb / Xresources
	setup();
	autostart_exec();
#ifdef __OpenBSD__
	if (pledge("stdio rpath proc exec ps", NULL) == -1)
		die("pledge");
#endif /* __OpenBSD__ */
	scan();
	run();
	cleanup();
	XCloseDisplay(dpy);
	if (restartwm)
		execvp(argv[0], argv);
	return EXIT_SUCCESS;
}
