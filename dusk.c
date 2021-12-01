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
#define TEXTWM(X)               (drw_fontset_getwidth(drw, (X), True))
#define TEXTW(X)                (drw_fontset_getwidth(drw, (X), False))
#define TEXT2DW(X)              (status2dtextlength((X)))
#define CLIENT                  (arg && arg->v ? (Client*)arg->v : selws->sel)

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
	SchemeFlexActFloat,
	SchemeFlexInaFloat,
	SchemeFlexSelFloat,
}; /* color schemes */

enum {
	NetActiveWindow,
	NetClientList,
	NetClientListStacking,
	NetCloseWindow,
	NetCurrentDesktop,
	NetDesktopNames,
	NetDesktopViewport,
	NetNumberOfDesktops,
	NetSupported,
	NetSystemTray,
	NetSystemTrayOP,
	NetSystemTrayOrientation,
	NetSystemTrayOrientationHorz,
	NetSystemTrayVisual,
	NetWMAllowedActions,
	NetWMCheck,
	NetWMDemandsAttention,
	NetWMDesktop,
	NetWMFullPlacement,
	NetWMFullscreen,
	NetWMHidden,
	NetWMIcon,
	NetWMName,
	NetWMState,
	NetWMStateAbove,
	NetWMMaximizedVert,
	NetWMMaximizedHorz,
	NetWMSkipTaskbar,
	NetWMStaysOnTop,
	NetWMSticky,
	NetWMWindowOpacity,
	NetWMWindowType,
	NetWMWindowTypeDock,
	NetWMMoveResize,
	NetWMUserTime,
	NetLast
}; /* EWMH atoms */

enum {
	NetWMActionMove,
	NetWMActionResize,
	NetWMActionMinimize,
	NetWMActionShade,
	NetWMActionStick,
	NetWMActionMaximizeHorz,
	NetWMActionMaximizeVert,
	NetWMActionFullscreen,
	NetWMActionChangeDesktop,
	NetWMActionClose,
	NetWMActionAbove,
	NetWMActionBelow,
	NetWMActionLast
}; /* _NET_WM_ALLOWED_ACTIONS */

enum {
	WMChangeState,
	WMClass,
	WMDelete,
	WMProtocols,
	WMState,
	WMTakeFocus,
	WMWindowRole,
	WMLast
}; /* default atoms */

enum {
	IsFloating,
	DuskClientFlags1,
	DuskClientFlags2,
	DuskClientFields,
	DuskClientLabel,
	SteamGameID,
	ClientLast
}; /* dusk client atoms */

/* https://specifications.freedesktop.org/wm-spec/latest/ar01s05.html - Application Window Properties */

enum {
	ClkLtSymbol,
	ClkStatusText,
	ClkWinTitle,
	ClkClientWin,
	ClkRootWin,
	ClkWorkspaceBar,
	ClkLast
}; /* clicks */

/* Named flextile constants */
enum {
	LAYOUT,       // controls overall layout arrangement / split
	MASTER,       // indicates the tile arrangement for the master area
	STACK,        // indicates the tile arrangement for the stack area
	STACK2,       // indicates the tile arrangement for the secondary stack area
	LTAXIS_LAST,
};

typedef struct ClientState ClientState;
struct ClientState {
	int isfixed, isfloating, isurgent, neverfocus, oldstate, isfullscreen;
};

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
	float mina, maxa;
	float cfact;
	int x, y, w, h;
	int sfx, sfy, sfw, sfh; /* stored float geometry, used on mode revert */
	int oldx, oldy, oldw, oldh;
	int basew, baseh, incw, inch, maxw, maxh, minw, minh;
	int bw, oldbw;
	int group;
	int scheme;
	char scratchkey;
	unsigned int idx;
	double opacity;
	pid_t pid;
	Client *next;
	Client *snext;
	Client *swallowing;
	Client *linked;
	Workspace *ws;
	Workspace *revertws; /* holds the original workspace info from when the client was opened */
	Window win;
	unsigned int icw, ich;
	Picture icon;
	unsigned long flags;
	unsigned long prevflags;
};

typedef struct {
	int type;
	unsigned int mod;
	KeySym keysym;
	void (*func)(const Arg *);
	const Arg arg;
} Key;

typedef struct {
	int nmaster;
	int nstack;
	int layout;
	int masteraxis; // master stack area
	int stack1axis; // primary stack area
	int stack2axis; // secondary stack area, e.g. centered master
	void (*symbolfunc)(Workspace *, unsigned int);
} LayoutPreset;

typedef struct {
	const char *symbol;
	void (*arrange)(Workspace *);
	LayoutPreset preset;
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
	unsigned long wsmask;
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
	double opacity;
	unsigned long flags;
	const char *floatpos;
	const char scratchkey;
	const char *workspace;
	const char *label;
	int resume;
} Rule;

struct Workspace {
	int wx, wy, ww, wh; /* workspace area */
	char ltsymbol[64];
	char name[16];
	float mfact;
	int ltaxis[4];
	int nstack;
	int nmaster;
	int enablegaps;
	int visible;
	int num;
	int pinned; // whether workspace is pinned to assigned monitor or not
	Client *clients;
	Client *sel;
	Client *stack;
	Client *prevzoom;
	Workspace *next;
	Monitor *mon;
	Pixmap preview;
	const Layout *layout;
	const Layout *prevlayout;
	char *icondef; // default icon
	char *iconvac; // vacant icon (when workspace is selected, default is empty, and no clients)
	char *iconocc; // when workspace has clients
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
	char *icondef;
	char *iconvac;
	char *iconocc;
} WorkspaceRule;

/* function declarations */
static void applyrules(Client *c);
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
static void clientmessage(XEvent *e);
static void clientmonresize(Client *c, Monitor *from, Monitor *to);
static void clientsmonresize(Client *clients, Monitor *from, Monitor *to);
static void clientrelposmon(Client *c, Monitor *o, Monitor *n, int *cx, int *cy, int *cw, int *ch);
static void clienttomon(const Arg *arg);
static void clientstomon(const Arg *arg);
static void configure(Client *c);
static void configurenotify(XEvent *e);
static void configurerequest(XEvent *e);
static Monitor *createmon(int num);
static void destroynotify(XEvent *e);
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
static int getrootptr(int *x, int *y);
static long getstate(Window w);
static int gettextprop(Window w, Atom atom, char *text, unsigned int size);
static void grabbuttons(Client *c, int focused);
static void grabkeys(void);
static void hide(Client *c);
static void incnmaster(const Arg *arg);
static void incnstack(const Arg *arg);
static int isatomstate(XClientMessageEvent *cme, int atom);
static int ismasterclient(Client *c);
static void keypress(XEvent *e);
static void keyrelease(XEvent *e);
static void killclient(const Arg *arg);
static void manage(Window w, XWindowAttributes *wa);
static void mappingnotify(XEvent *e);
static void maprequest(XEvent *e);
static void motionnotify(XEvent *e);
static void movemouse(const Arg *arg);
static void moveorplace(const Arg *arg);
static Client *nexttiled(Client *c);
static Client *nthtiled(Client *c, int n);
static void placemouse(const Arg *arg);
static Client *prevtiled(Client *c);
static void propertynotify(XEvent *e);
static void restart(const Arg *arg);
static void quit(const Arg *arg);
static void readclientstackingorder(void);
static Monitor *recttomon(int x, int y, int w, int h);
static Workspace *recttows(int x, int y, int w, int h);
static Client *recttoclient(int x, int y, int w, int h, int include_floating);
static void resize(Client *c, int x, int y, int w, int h, int interact);
static void resizeclient(Client *c, int x, int y, int w, int h);
static void resizeclientpad(Client *c, int x, int y, int w, int h, int xpad, int ypad);
static void resizemouse(const Arg *arg);
static void resizeorcfacts(const Arg *arg);
static void restack(Workspace *ws);
static void run(void);
static void scan(void);
static int sendevent(Window w, Atom proto, int m, long d0, long d1, long d2, long d3, long d4);
static void setclientstate(Client *c, long state);
static void setfocus(Client *c);
static void setfullscreen(Client *c, int fullscreen, int setfakefullscreen);
static void setlayout(const Arg *arg);
static void setmfact(const Arg *arg);
static void setup(void);
static void seturgent(Client *c, int urg);
static void show(Client *c);
static void sigchld(int unused);
static void skipfocusevents(void);
static void spawn(const Arg *arg);
static pid_t spawncmd(const Arg *arg, int buttonclick);
static void togglefloating(const Arg *arg);
static void togglemaximize(Client *c, int maximize_vert, int maximize_horz);
static void unfocus(Client *c, int setfocus, Client *nextfocus);
static void unmanage(Client *c, int destroyed);
static void unmapnotify(XEvent *e);
static void updateclientlist(void);
static int updategeom(void);
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
static char rawstatustext[NUM_STATUSES][512];

static int screen;
static int sw, sh;             /* X display screen geometry width, height */
static int lrpad;              /* sum of left and right padding for text */
static int force_warp = 0;     /* force warp in some situations, e.g. killclient */
static int ignore_warp = 0;    /* force skip warp in some situations, e.g. dragmfact, dragcfact */
static int num_workspaces = 0; /* the number of available workspaces */
static int combo = 0;          /* used for combo keys */
static int grp_idx = 0;        /* used for grouping windows together */
static int arrange_focus_on_monocle = 1; /* used in focus to arrange monocle layouts on focus */

static int (*xerrorxlib)(Display *, XErrorEvent *);
static unsigned int numlockmask = 0;
static void (*handler[LASTEvent]) (XEvent *) = {
	[ButtonPress] = buttonpress,
	[ButtonRelease] = keyrelease,
	[ClientMessage] = clientmessage,
	[ConfigureNotify] = configurenotify,
	[ConfigureRequest] = configurerequest,
	[DestroyNotify] = destroynotify,
	[EnterNotify] = enternotify,
	[Expose] = expose,
	[FocusIn] = focusin,
	[KeyPress] = keypress,
	[KeyRelease] = keypress,
	[MappingNotify] = mappingnotify,
	[MapRequest] = maprequest,
	[MotionNotify] = motionnotify,
	[PropertyNotify] = propertynotify,
	[ResizeRequest] = resizerequest,
	[UnmapNotify] = unmapnotify
};
static Atom wmatom[WMLast], netatom[NetLast], allowed[NetWMActionLast], xatom[XLast], clientatom[ClientLast];
static int running = 1;
static Cur *cursor[CurLast];
static Clr **scheme;
static Display *dpy;
static Drw *drw;
static Monitor *mons, *selmon;
static Workspace *workspaces, *selws;
static Window root, wmcheckwin;

/* configuration, allows nested code to access above variables */
#include "config.h"

#include "lib/include.c"

/* function implementations */
void
applyrules(Client *c)
{
	const char *class, *instance;
	Atom wintype, game_id;
	char role[64] = {0};
	unsigned int i;
	const Rule *r;
	Workspace *ws = NULL;
	XClassHint ch = { NULL, NULL };

	/* rule matching */
	XGetClassHint(dpy, c->win, &ch);
	class    = ch.res_class ? ch.res_class : broken;
	instance = ch.res_name  ? ch.res_name  : broken;
	wintype  = getatomprop(c, netatom[NetWMWindowType], XA_ATOM);
	gettextprop(c->win, wmatom[WMWindowRole], role, sizeof(role));
	game_id = getatomprop(c, clientatom[SteamGameID], AnyPropertyType);

	/* Steam games may come through with custom class, instance and name making it hard to create
	 * generic rules for them. Overriding the class with "steam_app_" to make this easier. */
	if (game_id && !strstr(class, "steam_app_"))
		class = "steam_app_";

	if (enabled(Debug))
		fprintf(stderr, "applyrules: new client %s, class = '%s', instance = '%s', role = '%s', wintype = '%ld'\n", c->name, class, instance, role, wintype);

	for (i = 0; i < LENGTH(clientrules); i++) {
		r = &clientrules[i];
		if ((!r->title || strstr(c->name, r->title))
		&& (!r->class || strstr(class, r->class))
		&& (!r->role || strstr(role, r->role))
		&& (!r->instance || strstr(instance, r->instance))
		&& (!r->wintype || wintype == XInternAtom(dpy, r->wintype, False)))
		{
			c->flags |= Ruled | r->flags;
			c->scratchkey = r->scratchkey;

			if (r->opacity)
				c->opacity = r->opacity;

			if (r->workspace)
				for (ws = workspaces; ws && strcmp(ws->name, r->workspace) != 0; ws = ws->next);
			c->ws = ws ? ws : selws;

			if (r->floatpos)
				setfloatpos(c, r->floatpos, 0);

			if (REVERTWORKSPACE(c) && !c->ws->visible)
				c->revertws = c->ws->mon->selws;

			if (r->label)
				strcpy(c->label, r->label);
			else
				saveclientclass(c);

			if (enabled(Debug))
				fprintf(stderr, "applyrules: client rule %d matched:\n    class: %s\n    role: %s\n    instance: %s\n    title: %s\n    wintype: %s\n    flags: %ld\n    floatpos: %s\n    workspace: %s\n    label: %s\n",
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
				break; // only allow one rule match
		}
	}

	if (ch.res_class)
		XFree(ch.res_class);
	if (ch.res_name)
		XFree(ch.res_name);
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
		/* see last two sentences in ICCCM 4.1.2.3 */
		baseismin = c->basew == c->minw && c->baseh == c->minh;
		if (!baseismin) { /* temporarily remove base dimensions */
			*w -= c->basew;
			*h -= c->baseh;
		}
		/* adjust for aspect limits */
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
	} else {
		for (ws = workspaces; ws; ws = ws->next)
			arrangews(ws);
		drawbars();
	}
}

void
arrangemon(Monitor *m)
{
	Workspace *ws;
	for (ws = workspaces; ws; ws = ws->next)
		if (ws->mon == m)
			arrangews(ws);
}
void
arrangews(Workspace *ws)
{
	if (!ws->visible)
		return;

	strncpy(ws->ltsymbol, ws->layout->symbol, sizeof ws->ltsymbol);
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
	int click, i;
	Arg arg = {0};
	Client *c;
	Monitor *m;
	Workspace *ws;
	XButtonPressedEvent *ev = &e->xbutton;
	click = ClkRootWin;
	/* focus monitor if necessary */
	if ((m = wintomon(ev->window)) && m != selmon) {
		ws = m->selws;
		if (ws) {
			unfocus(ws->sel, 1, NULL);
			selws = ws;
		}
		selmon = m;
		focus(NULL);
	}

	barpress(ev, m, &arg, &click);

	if (click == ClkRootWin && (c = wintoclient(ev->window))) {
		focus(c);
		restack(selws);
		XAllowEvents(dpy, ReplayPointer, CurrentTime);
		click = ClkClientWin;
	}

	for (i = 0; i < LENGTH(buttons); i++) {
		if (click == buttons[i].click && buttons[i].func && buttons[i].button == ev->button
				&& CLEANMASK(buttons[i].mask) == CLEANMASK(ev->state)) {
			buttons[i].func((click == ClkWorkspaceBar || click == ClkWinTitle) && buttons[i].arg.i == 0 ? &arg : &buttons[i].arg);
		}
	}
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
	selws->layout = &foo;
	for (ws = workspaces; ws; ws = ws->next)
		while (ws->stack)
			unmanage(ws->stack, 0);
	XUngrabKey(dpy, AnyKey, AnyModifier, root);
	while (mons)
		cleanupmon(mons);
	if (systray) {
		while (systray->icons)
			removesystrayicon(systray->icons);
		if (systray->win) {
			XUnmapWindow(dpy, systray->win);
			XDestroyWindow(dpy, systray->win);
		}
		free(systray);
	}

	for (ws = workspaces; ws; ws = next) {
		next = ws->next;
		if (ws->preview)
			XFreePixmap(dpy, ws->preview);
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
	for (ws = workspaces; ws; ws = ws->next)
		if (ws->mon == mon) {
			adjustwsformonitor(ws, mons);
			ws->mon = mons;
			ws->visible = 0;
			ws->pinned = 0;
			hidewsclients(ws->stack);
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
	if (mon->preview) {
		XUnmapWindow(dpy, mon->preview->win);
		XDestroyWindow(dpy, mon->preview->win);
		free(mon->preview);
	}

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
	XWindowAttributes wa;
	XSetWindowAttributes swa;
	XClientMessageEvent *cme = &e->xclient;
	Workspace *ws;
	Client *c;
	unsigned int maximize_vert, maximize_horz;
	int setfakefullscreen = 0;

	if (enabled(Systray) && systray && cme->window == systray->win && cme->message_type == netatom[NetSystemTrayOP]) {
		/* add systray icons */
		if (cme->data.l[1] == SYSTEM_TRAY_REQUEST_DOCK) {
			if (!(c = (Client *)calloc(1, sizeof(Client))))
				die("fatal: could not malloc() %u bytes\n", sizeof(Client));
			if (!(c->win = cme->data.l[2])) {
				free(c);
				return;
			}

			c->ws = selws;
			c->next = systray->icons;
			systray->icons = c;
			XGetWindowAttributes(dpy, c->win, &wa);
			c->x = c->oldx = c->y = c->oldy = 0;
			c->w = c->oldw = wa.width;
			c->h = c->oldh = wa.height;
			c->oldbw = wa.border_width;
			c->bw = 0;
			SETFLOATING(c);
			updatesizehints(c);
			updatesystrayicongeom(c, wa.width, wa.height);
			XAddToSaveSet(dpy, c->win);
			XSelectInput(dpy, c->win, StructureNotifyMask | PropertyChangeMask | ResizeRedirectMask);
			XClassHint ch = {"systray", "systray"};
			XSetClassHint(dpy, c->win, &ch);
			XReparentWindow(dpy, c->win, systray->win, 0, 0);
			/* use parents background color */
			swa.background_pixel = scheme[SchemeNorm][ColBg].pixel;
			XChangeWindowAttributes(dpy, c->win, CWBackPixel, &swa);
			sendevent(c->win, netatom[Xembed], StructureNotifyMask, CurrentTime, XEMBED_EMBEDDED_NOTIFY, 0 , systray->win, XEMBED_EMBEDDED_VERSION);
			XSync(dpy, False);
			setclientstate(c, NormalState);
		}
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
			if ((ws = getwsbyindex(cme->data.l[0])))
				viewwsonmon(ws, selmon, 0);
		}

		return;
	}

	c = wintoclient(cme->window);
	if (!c)
		return;

	if (enabled(Debug)) {
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
			if (maximize_vert || maximize_horz)
				togglemaximize(c, maximize_vert, maximize_horz);
		}
	} else if (cme->message_type == netatom[NetCloseWindow]) {
		killclient(&((Arg) { .v = c }));
	} else if (cme->message_type == netatom[NetWMDesktop]) {
		if ((ws = getwsbyindex(cme->data.l[0])))
			movetows(c, ws);
	} else if (cme->message_type == netatom[NetActiveWindow]) {
		if (HIDDEN(c)) {
			reveal(c);
			arrange(c->ws);
			drawbar(c->ws->mon);
		}
		if (enabled(FocusOnNetActive) && !NOFOCUSONNETACTIVE(c)) {
			if (c->ws->visible)
				focus(c);
			else
				viewwsonmon(c->ws, c->ws->mon, 0);
		} else if (c != selws->sel && !ISURGENT(c))
			seturgent(c, 1);
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
	if (from == to)
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
		resizeclient(c, c->x, c->y, c->w, c->h);
}

void
clientsfsrestore(Client *clients)
{
	Client *c;
	for (c = clients; c; c = c->next)
		clientfsrestore(c);
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
		movetows(c, m->selws);
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
		moveallclientstows(ws, m->selws);
		focus(NULL);
		arrange(NULL);
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
	ce.above = None;
	ce.override_redirect = False;
	XSendEvent(dpy, c->win, False, StructureNotifyMask, (XEvent *)&ce);
}

void
configurenotify(XEvent *e)
{
	Monitor *m;
	Bar *bar;
	Workspace *ws;
	Client *c;
	XConfigureEvent *ev = &e->xconfigure;
	int dirty;

	if (ev->window == root) {

		if (enabled(Debug)) {
			fprintf(stderr, "configurenotify: received event for root window\n");
			fprintf(stderr, "    - x = %d, y = %d, w = %d, h = %d\n", ev->x, ev->y, ev->width, ev->height);
		}

		dirty = (sw != ev->width || sh != ev->height);
		stickyws->ww = sw = ev->width;
		stickyws->wh = sh = ev->height;

		if (updategeom() || dirty) {
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
			for (m = mons; m; m = m->next)
				for (bar = m->bar; bar; bar = bar->next)
					XMoveResizeWindow(dpy, bar->win, bar->bx, bar->by, bar->bw, bar->bh);
			for (ws = workspaces; ws; ws = ws->next) {
				for (c = ws->clients; c; c = c->next) {
					c->sfx += c->ws->wx;
					c->sfy += c->ws->wy;
					if (ISFULLSCREEN(c) && !ISFAKEFULLSCREEN(c))
						resizeclient(c, ws->mon->mx, ws->mon->my, ws->mon->mw, ws->mon->mh);
					else if (ISFLOATING(c)) {
						c->x = c->sfx;
						c->y = c->sfy;
						show(c);
					}
				}
			}
			focus(NULL);
			arrange(NULL);
		}
	}
}

void
configurerequest(XEvent *e)
{
	Client *c;
	Monitor *m;
	XConfigureRequestEvent *ev = &e->xconfigurerequest;
	XWindowChanges wc;
	Workspace *ws = selws;

	if ((c = wintoclient(ev->window))) {

		if (enabled(Debug)) {
			fprintf(stderr, "configurerequest: received event %ld for client %s\n", ev->value_mask, c->name);
			fprintf(stderr, "    - x = %d, y = %d, w = %d, h = %d\n", ev->x, ev->y, ev->width, ev->height);
		}

		if (IGNORECFGREQ(c))
			return;
		if (ev->value_mask & CWBorderWidth)
			c->bw = ev->border_width;
		else if (ISFLOATING(c) || !ws->layout->arrange) {
			if (IGNORECFGREQPOS(c) && IGNORECFGREQSIZE(c))
				return;

			m = c->ws->mon;
			if (!IGNORECFGREQPOS(c)) {
				if (ev->value_mask & CWX) {
					c->oldx = c->x;
					c->x = m->mx + ev->x;
				}
				if (ev->value_mask & CWY) {
					c->oldy = c->y;
					c->y = m->my + ev->y;
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
			if ((c->x + c->w) > m->mx + m->mw && ISFLOATING(c))
				c->x = m->mx + (m->mw / 2 - WIDTH(c) / 2);  /* center in x direction */
			if ((c->y + c->h) > m->my + m->mh && ISFLOATING(c))
				c->y = m->my + (m->mh / 2 - HEIGHT(c) / 2); /* center in y direction */
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
	m->num = num;
	m->bar = NULL;

	createbars(m);

	return m;
}

void
destroynotify(XEvent *e)
{
	Client *c;
	Bar *bar;
	XDestroyWindowEvent *ev = &e->xdestroywindow;

	if ((c = wintoclient(ev->window))) {
		if (enabled(Debug))
			fprintf(stderr, "destroynotify: received event for client %s\n", c->name);
		unmanage(c, 1);
	}
	else if ((c = swallowingclient(ev->window))) {
		if (enabled(Debug))
			fprintf(stderr, "destroynotify: received event for swallowing client %s\n", c->name);
		unmanage(c->swallowing, 1);
	}
	else if (enabled(Systray) && (c = wintosystrayicon(ev->window))) {
		if (enabled(Debug))
			fprintf(stderr, "destroynotify: removing systray icon for client %s\n", c->name);
		removesystrayicon(c);
		drawbarwin(systray->bar);
	}
	else if ((bar = wintobar(ev->window))) {
		if (enabled(Debug))
			fprintf(stderr, "destroynotify: received event for bar %s\n", bar->name);
		recreatebar(bar);
	}
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
		for (nws = ws->next; nws && !(nws->mon == ws->mon && (dir != 2 || nws->clients)); nws = nws->next);
		if (!nws && ws != workspaces)
			for (tws = workspaces; tws && tws != ws; tws = tws->next)
				if (tws->mon == ws->mon && (dir != 2 || tws->clients)) {
					nws = tws;
					break;
				}
	} else { // left circular search
		for (tws = workspaces; tws && !(nws && tws == ws); tws = tws->next)
			if (tws->mon == ws->mon && (dir != -2 || tws->clients))
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
	if ((ev->mode != NotifyNormal || ev->detail == NotifyInferior) && ev->window != root)
		return;
	c = wintoclient(ev->window);
	m = c ? c->ws->mon : wintomon(ev->window);
	if (m != selmon)
		entermon(m, c);
	else if (selws == m->selws && (!c || (m->selws && c == m->selws->sel)))
		return;

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
	if (c && !c->ws->visible)
		return;

	Workspace *ws = c ? c->ws : selws;
	Client *f;
	XWindowChanges wc;

	if (!c || ISINVISIBLE(c))
		for (c = ws->stack; c && !ISVISIBLE(c); c = c->snext);
	if (selws->sel && selws->sel != c)
		unfocus(selws->sel, 0, c);
	if (c) {
		if (c->ws != selws) {
			if (ISSTICKY(c)) {
				stickyws->mon = selmon;
				stickyws->next = selws;
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
			/* Move all visible tiled clients that are not marked as on top below the bar window */
			wc.stack_mode = Below;
			wc.sibling = c->ws->mon->bar->win;
			for (f = c->ws->stack; f; f = f->snext)
				if (f != c && !ISFLOATING(f) && ISVISIBLE(f) && !(ALWAYSONTOP(f) || ISTRANSIENT(f))) {
					XConfigureWindow(dpy, f->win, CWSibling|CWStackMode, &wc);
					wc.sibling = f->win;
				}

			/* Move the currently focused client above the bar window */
			wc.stack_mode = Above;
			wc.sibling = c->ws->mon->bar->win;
			XConfigureWindow(dpy, c->win, CWSibling|CWStackMode, &wc);

			/* Move all visible floating windows that are not marked as on top below the current window */
			wc.stack_mode = Below;
			wc.sibling = c->win;
			for (f = c->ws->stack; f; f = f->snext)
				if (f != c && ISFLOATING(f) && ISVISIBLE(f) && !(ALWAYSONTOP(f) || ISTRANSIENT(f))) {
					XConfigureWindow(dpy, f->win, CWSibling|CWStackMode, &wc);
					wc.sibling = f->win;
				}
			XSync(dpy, False);
			skipfocusevents();
		}
	} else {
		XSetInputFocus(dpy, root, RevertToPointerRoot, CurrentTime);
		XDeleteProperty(dpy, root, netatom[NetActiveWindow]);
		ws->sel = ws->stack;
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

/* there are some broken focus acquiring clients needing extra handling */
void
focusin(XEvent *e)
{
	Workspace *ws = selws;
	XFocusChangeEvent *ev = &e->xfocus;
	if (ws->sel && ev->window != ws->sel->win)
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
	if (enabled(Warp))
		warp(selws->sel);
}

void
focusstack(const Arg *arg)
{
	Client *c = NULL, *i;
	Workspace *ws = selws, *prevws = NULL, *w = NULL;
	int n = (selws->sel == NULL);

	if (!ws)
		return;
	if (arg->i > 0) {
		for (; ws && !c; ws = (ws->next ? ws->next : workspaces)) {
			if (!ws->visible || (enabled(RestrictFocusstackToMonitor) && ws->mon != selws->mon))
				continue;
			for (c = (n == 0 && ws->sel ? ws->sel->next : ws->clients); c && (ISINVISIBLE(c) || (arg->i == 1 && HIDDEN(c))); c = c->next);
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
			if (enabled(Warp)) {
				force_warp = 1;
				for (n = 0, i = nexttiled(c->ws->clients); i; i = nexttiled(i->next), n++);
				if (ISFLOATING(c) || !(c->ws->ltaxis[MASTER] == MONOCLE && (abs(c->ws->ltaxis[LAYOUT] == NO_SPLIT || !c->ws->nmaster || n <= c->ws->nmaster))))
					warp(c);
			}
		} else
			restack(c->ws);
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

	return XQueryPointer(dpy, root, &dummy, &dummy, x, y, &di, &di, &dui);
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
	if (name.encoding == XA_STRING)
		strncpy(text, (char *)name.value, size - 1);
	else {
		if (XmbTextPropertyToTextList(dpy, &name, &list, &n) >= Success && n > 0 && *list) {
			strncpy(text, *list, size - 1);
			XFreeStringList(list);
		}
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
		unsigned int i, j;
		unsigned int modifiers[] = { 0, LockMask, numlockmask, numlockmask|LockMask };
		KeyCode code;

		XUngrabKey(dpy, AnyKey, AnyModifier, root);
		for (i = 0; i < LENGTH(keys); i++)
			if ((code = XKeysymToKeycode(dpy, keys[i].keysym)))
				for (j = 0; j < LENGTH(modifiers); j++)
					XGrabKey(dpy, code, keys[i].mod | modifiers[j], root,
						True, GrabModeAsync, GrabModeAsync);
	}
}

void
hide(Client *c)
{
	XMoveWindow(dpy, c->win, WIDTH(c) * -2, c->y);
}

void
incnmaster(const Arg *arg)
{
	Workspace *ws = selws;
	ws->nmaster = MAX(ws->nmaster + arg->i, 0);
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
	int keysyms_return;
	KeySym* keysym;
	XKeyEvent *ev;

	ev = &e->xkey;
	ignore_marked = 0;
	keysym = XGetKeyboardMapping(dpy, (KeyCode)ev->keycode, 1, &keysyms_return);
	for (i = 0; i < LENGTH(keys); i++) {
		if (*keysym == keys[i].keysym
				&& ev->type == keys[i].type
				&& CLEANMASK(keys[i].mod) == CLEANMASK(ev->state)
				&& keys[i].func)
			keys[i].func(&(keys[i].arg));
	}
	XFree(keysym);
	ignore_marked = 1;

	if (ev->type == KeyRelease)
		keyrelease(e);
}

int
isatomstate(XClientMessageEvent *cme, int atom)
{
	return (cme->data.l[1] == atom || cme->data.l[2] == atom);
}

int
ismasterclient(Client *c)
{
	Client *i;
	int n;
	for (n = 0, i = nexttiled(c->ws->clients); i && n < c->ws->nmaster; i = nexttiled(i->next), ++n)
		if (i == c)
			return 1;

	return 0;
}

void
keyrelease(XEvent *e)
{
	if (!combo)
		return;

	unsigned long wsmask;
	Monitor *m = selmon;

	wsmask = getwsmask(m);
	if (prevwsmask == wsmask)
		viewwsmask(m, m->wsmask);
	else
		m->wsmask = prevwsmask;
	combo = 0;
}

void
killclient(const Arg *arg)
{
	Client *c = CLIENT;

	for (c = nextmarked(NULL, c); c; c = nextmarked(c->next, NULL)) {
		if (ISPERMANENT(c))
			continue;

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

	updateicon(c);
	updatetitle(c);
	fprintf(stderr, "manage --> client %s\n", c->name);
	getclientflags(c);
	getclientfields(c);
	getclientopacity(c);
	getclientlabel(c);

	if (ISSTICKY(c))
		c->ws = recttows(c->x + c->w / 2, c->y + c->h / 2, 1, 1);

	if (!c->ws) {
		if (XGetTransientForHint(dpy, w, &trans) && (t = wintoclient(trans))) {
			addflag(c, Transient);
			addflag(c, Centered);
			c->ws = t->ws;
		} else
			c->ws = selws;
	}

	restorewindowfloatposition(c, c->ws->mon);

	if (!RULED(c)) {
		if (c->x == c->ws->mon->wx && c->y == c->ws->mon->wy)
			addflag(c, Centered);

		if (!ISTRANSIENT(c))
			applyrules(c);
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
		free(c);
		fprintf(stderr, "manage <-- unmanaged (%s)\n", c->name);
		return;
	}

	if (!ISTRANSIENT(c))
		term = termforwin(c);

	if (ISSTICKY(c)) {
		stickyws->next = c->ws;
		stickyws->mon = c->ws->mon;
		c->ws = stickyws;
		stickyws->sel = c;
		selws = stickyws;
	}

	c->bw = (NOBORDER(c) ? 0 : c->ws->mon->borderpx);

	if (c->opacity)
		opacity(c, c->opacity);

	m = c->ws->mon;

	if (c->x + WIDTH(c) > m->mx + m->mw)
		c->x = m->mx + m->mw - WIDTH(c);
	if (c->y + HEIGHT(c) > m->my + m->mh)
		c->y = m->my + m->mh - HEIGHT(c);
	c->x = MAX(c->x, m->mx);
	/* only fix client y-offset, if the client center might cover the bar */
	c->y = MAX(c->y, ((m->bar->by == m->my) && (c->x + (c->w / 2) >= m->wx)
		&& (c->x + (c->w / 2) < m->wx + m->ww)) ? bh : m->my);
	wc.border_width = c->bw;
	XConfigureWindow(dpy, w, CWBorderWidth, &wc);
	configure(c); /* propagates border_width, if size doesn't change */
	updatesizehints(c);
	updateclientdesktop(c);

	/* If the client indicates that it is in fullscreen, or if the FullScreen flag has been
	 * explictly set via client rules, then enable fullscreen now. */
	if (getatomprop(c, netatom[NetWMState], XA_ATOM) == netatom[NetWMFullscreen] || ISFULLSCREEN(c)) {
		setflag(c, FullScreen, 0);
		setfullscreen(c, 1, 0);
	}

	updatewmhints(c);
	updatemotifhints(c);

	XSelectInput(dpy, w, EnterWindowMask|FocusChangeMask|PropertyChangeMask|StructureNotifyMask);
	grabbuttons(c, 0);

	if (trans != None)
		c->prevflags |= Floating;
	if (!ISFLOATING(c) && (ISFIXED(c) || WASFLOATING(c) || getatomprop(c, clientatom[IsFloating], AnyPropertyType)))
		SETFLOATING(c);

	if (ISFLOATING(c))
		XRaiseWindow(dpy, c->win);

	XChangeProperty(dpy, c->win, netatom[NetWMAllowedActions], XA_ATOM, 32,
		PropModeReplace, (unsigned char *) allowed, NetWMActionLast);

	/* Do not attach client if it is being swallowed */
	if (term && swallow(term, c)) {
		/* Do not let swallowed client steal focus unless the terminal has focus */
		focusclient = (term == selws->sel);
	} else {
		attachx(c, 0, NULL);

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

	if (LOWER(c))
		XLowerWindow(dpy, c->win);
	else if (RAISE(c))
		XRaiseWindow(dpy, c->win);

	setfloatinghint(c);
	if (SEMISCRATCHPAD(c) && c->scratchkey)
		initsemiscratchpad(c);

	if (!c->ws->visible)
		drawbar(c->ws->mon);

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
	if (enabled(Systray) && systray && (i = wintosystrayicon(ev->window))) {
		sendevent(i->win, netatom[Xembed], StructureNotifyMask, CurrentTime, XEMBED_WINDOW_ACTIVATE, 0, systray->win, XEMBED_EMBEDDED_VERSION);
		drawbarwin(systray->bar);
	}

	if (!XGetWindowAttributes(dpy, ev->window, &wa))
		return;
	if (!wa.depth) {
		fprintf(stderr, "maprequest: refusing to map window %ld with depth of 0\n", ev->window);
		return;
	}
	if (wa.override_redirect)
		return;
	if (!wintoclient(ev->window))
		manage(ev->window, &wa);
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

	if (selmon->preview->show) {
		selmon->preview->show = 0;
		hidewspreview(selmon);
	}

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

void
moveorplace(const Arg *arg)
{
	if (!selws || !selws->sel)
		return;

	if (!selws->layout->arrange || ISFLOATING(selws->sel))
		movemouse(arg);
	else
		placemouse(arg);
}

void
movemouse(const Arg *arg)
{
	int i, g, x, y, w, h, nx, ny, sx, sy, vsnap, hsnap, xoff, yoff, group_after;
	Client *c, *s;
	Workspace *ws;
	Monitor *m;
	XEvent ev;
	Time lasttime = 0;
	double prevopacity;
	ignore_marked = 0; // movemouse supports marked clients

	if (!(c = selws->sel))
		return;
	if (ISFULLSCREEN(c) && !ISFAKEFULLSCREEN(c)) /* no support moving fullscreen windows by mouse */
		return;
	restack(selws);

	group_after = c->group;
	if (ISMARKED(c)) {
		group(NULL);
	}

	/* Snap girders */
	int gap = (gappih + gappiv) / 2;
	int ngirders = 0;
	int lgirder[100] = {0};
	int rgirder[100] = {0};
	int tgirder[100] = {0};
	int bgirder[100] = {0};

	/* Grouped floating windows */
	Client *group[10] = {c};
	int ngroup = 1;
	int ocx[10] = {c->x};
	int ocy[10] = {c->y};
	int ocw[10] = {WIDTH(c)};
	int och[10] = {HEIGHT(c)};

	for (m = mons; m; m = m->next) {
		lgirder[ngirders] = m->mx + (enabled(BarPadding) ? sidepad : 0);
		rgirder[ngirders] = m->mx + m->mw - (enabled(BarPadding) ? sidepad : 0);
		tgirder[ngirders] = m->my + (enabled(BarPadding) ? vertpad : 0);
		bgirder[ngirders] = m->my + m->mh - (enabled(BarPadding) ? vertpad : 0);
		ngirders++;
	}

	for (ws = workspaces; ws; ws = ws->next) {
		if (!ws->visible)
			continue;
		lgirder[ngirders] = ws->wx + gappov;
		rgirder[ngirders] = ws->wx + ws->ww - gappov;
		tgirder[ngirders] = ws->wy + gappoh;
		bgirder[ngirders] = ws->wy + ws->wh - gappoh;
		ngirders++;
		if (disabled(SnapToWindows) || arg->i == 11)
			continue;
		for (s = ws->stack; s; s = s->snext) {
			if ((!ISFLOATING(s) && ws->layout->arrange) || !ISVISIBLE(s) || s == c)
				continue;
			if (c->group && s->group == c->group) {
				group[ngroup] = s;
				ocx[ngroup] = s->x;
				ocy[ngroup] = s->y;
				och[ngroup] = HEIGHT(s);
				ocw[ngroup] = WIDTH(s);
				ngroup++;
				if (moveopacity)
					opacity(s, moveopacity);
				continue;
			}

			h = HEIGHT(s);
			w = WIDTH(s);
			lgirder[ngirders] = s->x;
			rgirder[ngirders] = s->x + w;
			tgirder[ngirders] = s->y;
			bgirder[ngirders] = s->y + h;
			ngirders++;
			lgirder[ngirders] = s->x + w + gap;
			rgirder[ngirders] = s->x - gap;
			tgirder[ngirders] = s->y + h + gap;
			bgirder[ngirders] = s->y - gap;
			ngirders++;
		}
	}

	if (XGrabPointer(dpy, root, False, MOUSEMASK, GrabModeAsync, GrabModeAsync,
		None, cursor[CurMove]->cursor, CurrentTime) != GrabSuccess)
		return;
	if (!getrootptr(&x, &y))
		return;

	for (g = 0; g < ngroup; g++) {
		if (moveopacity) {
			prevopacity = group[0]->opacity;
			opacity(group[g], moveopacity);
		}
		addflag(group[g], MoveResize);
	}

	do {
		XMaskEvent(dpy, MOUSEMASK|ExposureMask|SubstructureRedirectMask, &ev);
		switch (ev.type) {
		case ConfigureRequest:
		case Expose:
		case MapRequest:
			handler[ev.type](&ev);
			break;
		case MotionNotify:
			if ((ev.xmotion.time - lasttime) <= (1000 / 60))
				continue;
			lasttime = ev.xmotion.time;

			sx = nx = ocx[0] + (ev.xmotion.x - x);
			sy = ny = ocy[0] + (ev.xmotion.y - y);
			vsnap = hsnap = snap;

			if (!ISFLOATING(c) && selws->layout->arrange
					&& (abs(nx - c->x) > snap || abs(ny - c->y) > snap))
				togglefloating(NULL);

			for (i = 0; i < ngirders; i++) {
				for (g = 0; g < ngroup; g++) {
					xoff = (g == 0 ? 0 : ocx[g] - ocx[0]);
					yoff = (g == 0 ? 0 : ocy[g] - ocy[0]);

					if (abs(lgirder[i] - nx - xoff) < vsnap) {
						sx = lgirder[i] - xoff;
						vsnap = abs(sx - nx);
					}
					if (abs(rgirder[i] - nx - ocw[g] - xoff) < vsnap) {
						sx = rgirder[i] - ocw[g] - xoff;
						vsnap = abs(sx - nx);
					}
					if (abs(tgirder[i] - ny - yoff) < hsnap) {
						sy = tgirder[i] - yoff;
						hsnap = abs(sy - ny);
					}
					if (abs(bgirder[i] - ny - och[g] - yoff) < hsnap) {
						sy = bgirder[i] - och[g] - yoff;
						hsnap = abs(sy - ny);
					}
				}
			}

			nx = sx;
			ny = sy;

			for (g = 0; g < ngroup; g++) {
				xoff = (g == 0 ? 0 : ocx[g] - ocx[0]);
				yoff = (g == 0 ? 0 : ocy[g] - ocy[0]);
				resize(group[g], nx + xoff, ny + yoff, group[g]->w, group[g]->h, 1);
				savefloats(group[g]);
			}
			break;
		}
	} while (ev.type != ButtonRelease);
	XUngrabPointer(dpy, CurrentTime);

	for (g = ngroup - 1; g > -1; g--) {
		c = group[g];

		ws = recttows(c->x, c->y, c->w, c->h);
		if (ws && ISSTICKY(c)) {
			stickyws->mon = ws->mon;
			drawbars();
		} else if (ws && ws != c->ws) {
			detach(c);
			detachstack(c);
			attachx(c, AttachBottom, ws);
			attachstack(c);
			if (g == 0) {
				selws = ws;
				selmon = ws->mon;
			}
		}

		removeflag(c, MoveResize);
		if (moveopacity)
			opacity(c, prevopacity);
		c->group = group_after;
	}

	focus(group[0]);
	drawbars();
}

void
placemouse(const Arg *arg)
{
	int x, y, px, py, ocx, ocy, nx = -9999, ny = -9999, freemove = 0;
	Client *c, *r = NULL, *prevr;
	Workspace *w, *ws = selws;
	XEvent ev;
	XWindowAttributes wa;
	double prevopacity;
	Time lasttime = 0;
	unsigned long attachmode, prevattachmode;
	attachmode = prevattachmode = AttachMaster;

	if (!(c = ws->sel) || !ws->layout->arrange) /* no support for placemouse when floating layout is used */
		return;
	if (ISFULLSCREEN(c) && !ISFAKEFULLSCREEN(c)) /* no support placing fullscreen windows by mouse */
		return;
	if (ISSTICKY(c))
		return;
	restack(ws);
	prevr = c;
	if (XGrabPointer(dpy, root, False, MOUSEMASK, GrabModeAsync, GrabModeAsync,
		None, cursor[CurMove]->cursor, CurrentTime) != GrabSuccess)
		return;

	addflag(c, MovePlace);
	removeflag(c, Floating);
	if (placeopacity) {
		prevopacity = c->opacity;
		opacity(c, placeopacity);
	}

	XGetWindowAttributes(dpy, c->win, &wa);
	ocx = wa.x;
	ocy = wa.y;

	if (arg->i == 2) // warp cursor to client center
		XWarpPointer(dpy, None, c->win, 0, 0, 0, 0, WIDTH(c) / 2, HEIGHT(c) / 2);

	if (!getrootptr(&x, &y))
		return;

	do {
		XMaskEvent(dpy, MOUSEMASK|ExposureMask|SubstructureRedirectMask, &ev);
		switch (ev.type) {
		case ConfigureRequest:
		case Expose:
		case MapRequest:
			handler[ev.type](&ev);
			break;
		case MotionNotify:
			if ((ev.xmotion.time - lasttime) <= (1000 / 60))
				continue;
			lasttime = ev.xmotion.time;

			nx = ocx + (ev.xmotion.x - x);
			ny = ocy + (ev.xmotion.y - y);

			if (!freemove && (abs(nx - ocx) > snap || abs(ny - ocy) > snap))
				freemove = 1;

			if (freemove)
				XMoveWindow(dpy, c->win, nx, ny);

			if ((w = recttows(ev.xmotion.x, ev.xmotion.y, 1, 1)) && w != selws) {
				selws = w;
				selmon = w->mon;
			}

			if (arg->i == 1) { // tiled position is relative to the client window center point
				px = nx + wa.width / 2;
				py = ny + wa.height / 2;
			} else { // tiled position is relative to the mouse cursor
				px = ev.xmotion.x;
				py = ev.xmotion.y;
			}

			r = recttoclient(px, py, 1, 1, 0);

			if (!r || r == c)
				break;

			if ((((float)(r->y + r->h - py) / r->h) > ((float)(r->x + r->w - px) / r->w)
			    	&& (abs(r->y - py) < r->h / 2)) || (abs(r->x - px) < r->w / 2))
				attachmode = AttachAbove;
			else
				attachmode = AttachBelow;

			if ((r && r != prevr) || (attachmode != prevattachmode)) {
				detachstack(c);
				detach(c);
				if (c->ws != r->ws)
					arrangews(c->ws);

				r->ws->sel = r;
				attachx(c, attachmode, r->ws);
				attachstack(c);
				arrangews(r->ws);
				prevr = r;
				prevattachmode = attachmode;
			}
			break;
		}
	} while (ev.type != ButtonRelease);
	XUngrabPointer(dpy, CurrentTime);

	if ((w = recttows(ev.xmotion.x, ev.xmotion.y, 1, 1)) && w != c->ws) {
		detach(c);
		detachstack(c);
		arrangews(c->ws);
		attachx(c, AttachBottom, w);
		attachstack(c);
		selws = w;
		selmon = w->mon;
	}

	focus(c);
	removeflag(c, MovePlace);
	if (nx != -9999)
		resize(c, nx, ny, c->w, c->h, 0);
	arrangews(c->ws);
	if (placeopacity)
		opacity(c, prevopacity);
}

Client *
nexttiled(Client *c)
{
	for (; c && (ISFLOATING(c) || !ISVISIBLE(c)); c = c->next);
	return c;
}

Client *
nthtiled(Client *c, int n)
{
	int i;
	for (i = 0; c && i < n; c = c->next)
		if (!ISFLOATING(c) && ISVISIBLE(c))
			i++;
	return c;
}

Client *
prevtiled(Client *c)
{
	Client *p, *r;
	for (p = nexttiled(c->ws->clients), r = NULL; p && p != c && (r = p); p = nexttiled(p->next));
	return r;
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
	XPropertyEvent *ev = &e->xproperty;

	if (enabled(Systray) && (c = wintosystrayicon(ev->window))) {
		if (ev->atom == XA_WM_NORMAL_HINTS) {
			updatesizehints(c);
			updatesystrayicongeom(c, c->w, c->h);
		}
		else
			updatesystrayiconstate(c, ev);
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

		if (enabled(Debug) && ev->atom != netatom[NetWMUserTime])
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
			updatesizehints(c);
			arrange(c->ws);
			break;
		case XA_WM_HINTS:
			updatewmhints(c);
			if (ISURGENT(c))
				drawbars();
			break;
		}
		if (ev->atom == XA_WM_NAME || ev->atom == netatom[NetWMName]) {
			updatetitle(c);
			if (c == c->ws->sel)
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
			drawbars();
		}
	}
}

void
restart(const Arg *arg)
{
	Workspace *ws;
	restartsig = 1;
	quit(arg);

	for (ws = workspaces; ws; ws = ws->next)
		persistworkspacestate(ws);
	persistworkspacestate(stickyws);
}

void
quit(const Arg *arg)
{
	size_t i;
	running = 0;

	/* kill child processes */
	for (i = 0; i < autostart_len; i++) {
		if (0 < autostart_pids[i]) {
			kill(autostart_pids[i], SIGTERM);
			waitpid(autostart_pids[i], NULL, 0);
		}
	}
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
		if (ws->visible && (a = INTERSECT(x, y, w, h, ws)) > area) {
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

	if (enabled(CenterSizeHintsClients) && !ISFLOATING(c)) {
		if (w != tw)
			c->x = wc.x += (tw - w) / 2;
		if (h != th)
			c->y = wc.y += (th - h) / 2;
	}

	if (!c->ws->visible || MOVEPLACE(c)) {
		addflag(c, NeedResize);
		return;
	}

	if (enabled(NoBorders) && ((nexttiled(c->ws->clients) == c && !nexttiled(c->next)))
		&& (ISFAKEFULLSCREEN(c) || !ISFULLSCREEN(c))
		&& !ISFLOATING(c)
		&& c->ws->layout->arrange)
	{
		wc.width += c->bw * 2;
		wc.height += c->bw * 2;
		wc.border_width = 0;
	}
	XConfigureWindow(dpy, c->win, CWX|CWY|CWWidth|CWHeight|CWBorderWidth, &wc);
	configure(c);
	XSync(dpy, False);
}

void
resizemouse(const Arg *arg)
{
	int ocx, ocy, nw, nh;
	int opx, opy, och, ocw, nx, ny;
	int horizcorner, vertcorner;
	unsigned int dui;
	Window dummy;
	Client *c;
	XEvent ev;
	Workspace *w, *ws = selws;
	Time lasttime = 0;
	double prevopacity;

	if (!(c = ws->sel))
		return;
	if (ISFULLSCREEN(c) && !ISFAKEFULLSCREEN(c)) /* no support resizing fullscreen windows by mouse */
		return;

	if (resizeopacity) {
		prevopacity = c->opacity;
		opacity(c, resizeopacity);
	}
	restack(selws);
	ocx = c->x;
	ocy = c->y;
	och = c->h;
	ocw = c->w;
	if (!XQueryPointer(dpy, c->win, &dummy, &dummy, &opx, &opy, &nx, &ny, &dui))
		return;
	horizcorner = nx < c->w / 2;
	vertcorner  = ny < c->h / 2;
	if (XGrabPointer(dpy, root, False, MOUSEMASK, GrabModeAsync, GrabModeAsync,
		None, cursor[horizcorner | (vertcorner << 1)]->cursor, CurrentTime) != GrabSuccess)
		return;
	addflag(c, MoveResize);
	do {
		XMaskEvent(dpy, MOUSEMASK|ExposureMask|SubstructureRedirectMask, &ev);
		switch (ev.type) {
		case ConfigureRequest:
		case Expose:
		case MapRequest:
			handler[ev.type](&ev);
			break;
		case MotionNotify:
			if ((ev.xmotion.time - lasttime) <= (1000 / 60))
				continue;
			lasttime = ev.xmotion.time;

			nx = horizcorner ? (ocx + ev.xmotion.x - opx) : c->x;
			ny = vertcorner ? (ocy + ev.xmotion.y - opy) : c->y;
			nw = MAX(horizcorner ? (ocx + ocw - nx) : (ocw + (ev.xmotion.x - opx)), 1);
			nh = MAX(vertcorner ? (ocy + och - ny) : (och + (ev.xmotion.y - opy)), 1);

			if (c->ws->mon->wx + nw >= selmon->wx && c->ws->mon->wx + nw <= selmon->wx + selmon->ww
			&& c->ws->mon->wy + nh >= selmon->wy && c->ws->mon->wy + nh <= selmon->wy + selmon->wh)
			{
				if (!ISFLOATING(c) && ws->layout->arrange
				&& (abs(nw - c->w) > snap || abs(nh - c->h) > snap))
					togglefloating(NULL);
			}
			if (!ws->layout->arrange || ISFLOATING(c)) {
				resize(c, nx, ny, nw, nh, 1);
				savefloats(c);
			}
			break;
		}
	} while (ev.type != ButtonRelease);

	XUngrabPointer(dpy, CurrentTime);
	skipfocusevents();

	if (!ISSTICKY(c) && (w = recttows(c->x, c->y, c->w, c->h)) && w != selws) {
		detach(c);
		detachstack(c);
		attachx(c, AttachBottom, w);
		attachstack(c);
		selws = w;
		selmon = w->mon;
		focus(c);
	}
	removeflag(c, MoveResize);
	if (resizeopacity)
		opacity(c, prevopacity);
}

void
resizeorcfacts(const Arg *arg)
{
	if (!selws || !selws->sel)
		return;

	if (!selws->layout->arrange || ISFLOATING(selws->sel))
		resizemouse(arg);
	else
		dragcfact(arg);
}

void
restack(Workspace *ws)
{
	Client *c;
	XWindowChanges wc;
	int n = 0;

	if (!ws->sel)
		return;
	if (ISFLOATING(ws->sel) || !ws->layout->arrange)
		XRaiseWindow(dpy, ws->sel->win);
	if (ws->layout->arrange) {
		wc.stack_mode = Below;
		wc.sibling = ws->mon->bar->win;
		for (c = ws->stack; c; c = c->snext)
			if (!ISFLOATING(c) && ISVISIBLE(c)) {
				XConfigureWindow(dpy, c->win, CWSibling|CWStackMode, &wc);
				wc.sibling = c->win;
			}
	}
	XSync(dpy, False);
	skipfocusevents();

	if (enabled(Warp)) {
		if (ws->nmaster)
			for (c = nexttiled(ws->clients); c && n <= ws->nmaster; c = nexttiled(c->next), n++);
		if (ws == selws && (
			!(ws->ltaxis[MASTER] == MONOCLE && (abs(ws->ltaxis[LAYOUT] == NO_SPLIT || !ws->nmaster || n <= ws->nmaster)))
			|| ISFLOATING(ws->sel))
		)
			warp(ws->sel);
	}
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
			if (!XGetWindowAttributes(dpy, wins[i], &wa))
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
	const Layout *tmp = ws->layout;

	ws->layout = ws->prevlayout;
	ws->prevlayout = tmp;

	if (arg && arg->v)
		ws->layout = (Layout *)arg->v;

	if (ws->layout->preset.nmaster != -1)
		ws->nmaster = ws->layout->preset.nmaster;
	if (ws->layout->preset.nstack != -1)
		ws->nstack = ws->layout->preset.nstack;

	ws->ltaxis[LAYOUT] = ws->layout->preset.layout;
	ws->ltaxis[MASTER] = ws->layout->preset.masteraxis;
	ws->ltaxis[STACK]  = ws->layout->preset.stack1axis;
	ws->ltaxis[STACK2] = ws->layout->preset.stack2axis;

	strncpy(ws->ltsymbol, ws->layout->symbol, sizeof ws->ltsymbol);

	arrange(ws);
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
	int i;
	XSetWindowAttributes wa;
	Atom utf8string;

	/* clean up any zombies immediately */
	sigchld(0);
	signal(SIGHUP, sighup);
	signal(SIGTERM, sigterm);

	putenv("_JAVA_AWT_WM_NONREPARENTING=1");

	enablefunc(functionality);

	if (enabled(Xresources))
		loadxrdb();

	/* init screen */
	screen = DefaultScreen(dpy);
	sw = DisplayWidth(dpy, screen);
	sh = DisplayHeight(dpy, screen);
	root = RootWindow(dpy, screen);
	xinitvisual();
	drw = drw_create(dpy, screen, root, sw, sh, visual, depth, cmap);
	if (!drw_fontset_create(drw, fonts, LENGTH(fonts)))
		die("no fonts could be loaded.");

	lrpad = drw->fonts->h + horizpadbar;
	bh = bar_height ? bar_height : drw->fonts->h + vertpadbar;

	/* One off calculating workspace label widths, used by WorkspaceLabels functionality */
	occupied_workspace_label_format_length = TEXT2DW(occupied_workspace_label_format) - TEXTW(workspace_label_placeholder) * 2;
	vacant_workspace_label_format_length = TEXT2DW(vacant_workspace_label_format) - TEXTW(workspace_label_placeholder);
	textw_single_char = TEXTW("A");


	updategeom();
	createworkspaces();

	/* init atoms */
	utf8string = XInternAtom(dpy, "UTF8_STRING", False);
	wmatom[WMClass] = XInternAtom(dpy, "WM_CLASS", False);
	wmatom[WMProtocols] = XInternAtom(dpy, "WM_PROTOCOLS", False);
	wmatom[WMDelete] = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
	wmatom[WMState] = XInternAtom(dpy, "WM_STATE", False);
	wmatom[WMTakeFocus] = XInternAtom(dpy, "WM_TAKE_FOCUS", False);
	wmatom[WMWindowRole] = XInternAtom(dpy, "WM_WINDOW_ROLE", False);
	wmatom[WMChangeState] = XInternAtom(dpy, "WM_CHANGE_STATE", False);
	clientatom[IsFloating] = XInternAtom(dpy, "_IS_FLOATING", False);
	clientatom[DuskClientFlags1] = XInternAtom(dpy, "_DUSK_CLIENT_FLAGS1", False);
	clientatom[DuskClientFlags2] = XInternAtom(dpy, "_DUSK_CLIENT_FLAGS2", False);
	clientatom[DuskClientFields] = XInternAtom(dpy, "_DUSK_CLIENT_FIELDS", False);
	clientatom[DuskClientLabel] = XInternAtom(dpy, "_DUSK_CLIENT_LABEL", False);
	clientatom[SteamGameID] = XInternAtom(dpy, "STEAM_GAME", False);
	netatom[NetActiveWindow] = XInternAtom(dpy, "_NET_ACTIVE_WINDOW", False);
	netatom[NetClientList] = XInternAtom(dpy, "_NET_CLIENT_LIST", False);
	netatom[NetClientListStacking] = XInternAtom(dpy, "_NET_CLIENT_LIST_STACKING", False);
	netatom[NetCloseWindow] = XInternAtom(dpy, "_NET_CLOSE_WINDOW", False);
	netatom[NetCurrentDesktop] = XInternAtom(dpy, "_NET_CURRENT_DESKTOP", False);
	netatom[NetDesktopNames] = XInternAtom(dpy, "_NET_DESKTOP_NAMES", False);
	netatom[NetDesktopViewport] = XInternAtom(dpy, "_NET_DESKTOP_VIEWPORT", False);
	netatom[NetNumberOfDesktops] = XInternAtom(dpy, "_NET_NUMBER_OF_DESKTOPS", False);
	netatom[NetSupported] = XInternAtom(dpy, "_NET_SUPPORTED", False);
	netatom[NetSystemTray] = XInternAtom(dpy, "_NET_SYSTEM_TRAY_S0", False);
	netatom[NetSystemTrayOP] = XInternAtom(dpy, "_NET_SYSTEM_TRAY_OPCODE", False);
	netatom[NetSystemTrayOrientation] = XInternAtom(dpy, "_NET_SYSTEM_TRAY_ORIENTATION", False);
	netatom[NetSystemTrayOrientationHorz] = XInternAtom(dpy, "_NET_SYSTEM_TRAY_ORIENTATION_HORZ", False);
	netatom[NetSystemTrayVisual] = XInternAtom(dpy, "_NET_SYSTEM_TRAY_VISUAL", False);
	netatom[NetWMAllowedActions] = XInternAtom(dpy, "_NET_WM_ALLOWED_ACTIONS", False);
	netatom[NetWMCheck] = XInternAtom(dpy, "_NET_SUPPORTING_WM_CHECK", False);
	netatom[NetWMDemandsAttention] = XInternAtom(dpy, "_NET_WM_STATE_DEMANDS_ATTENTION", False);
	netatom[NetWMDesktop] = XInternAtom(dpy, "_NET_WM_DESKTOP", False);
	netatom[NetWMFullPlacement] = XInternAtom(dpy, "_NET_WM_FULL_PLACEMENT", False); /* https://specifications.freedesktop.org/wm-spec/latest/ar01s07.html */
	netatom[NetWMFullscreen] = XInternAtom(dpy, "_NET_WM_STATE_FULLSCREEN", False);
	netatom[NetWMHidden] = XInternAtom(dpy, "_NET_WM_STATE_HIDDEN", False);
	netatom[NetWMIcon] = XInternAtom(dpy, "_NET_WM_ICON", False);
	netatom[NetWMMaximizedVert] = XInternAtom(dpy, "_NET_WM_STATE_MAXIMIZED_VERT", False);
	netatom[NetWMMaximizedHorz] = XInternAtom(dpy, "_NET_WM_STATE_MAXIMIZED_HORZ", False);
	netatom[NetWMSkipTaskbar] = XInternAtom(dpy, "_NET_WM_STATE_SKIP_TASKBAR", False);
	netatom[NetWMStaysOnTop] = XInternAtom(dpy, "_NET_WM_STATE_STAYS_ON_TOP", False);
	netatom[NetWMSticky] = XInternAtom(dpy, "_NET_WM_STATE_STICKY", False);
	netatom[NetWMMoveResize] = XInternAtom(dpy, "_NET_WM_MOVERESIZE", False);
	netatom[NetWMUserTime] = XInternAtom(dpy, "_NET_WM_USER_TIME", False);
	netatom[NetWMName] = XInternAtom(dpy, "_NET_WM_NAME", False);
	netatom[NetWMState] = XInternAtom(dpy, "_NET_WM_STATE", False);
	netatom[NetWMWindowOpacity] = XInternAtom(dpy, "_NET_WM_WINDOW_OPACITY", False);
	netatom[NetWMWindowType] = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE", False);
	netatom[NetWMWindowTypeDock] = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_DOCK", False);
	allowed[NetWMActionMove] = XInternAtom(dpy, "_NET_WM_ACTION_MOVE", False);
	allowed[NetWMActionResize] = XInternAtom(dpy, "_NET_WM_ACTION_RESIZE", False);
	allowed[NetWMActionMinimize] = XInternAtom(dpy, "_NET_WM_ACTION_MINIMIZE", False);
	allowed[NetWMActionShade] = XInternAtom(dpy, "_NET_WM_ACTION_SHADE", False);
	allowed[NetWMActionStick] = XInternAtom(dpy, "_NET_WM_ACTION_STICK", False);
	allowed[NetWMActionMaximizeHorz] = XInternAtom(dpy, "_NET_WM_ACTION_MAXIMIZE_HORZ", False);
	allowed[NetWMActionMaximizeVert] = XInternAtom(dpy, "_NET_WM_ACTION_MAXIMIZE_VERT", False);
	allowed[NetWMActionFullscreen] = XInternAtom(dpy, "_NET_WM_ACTION_FULLSCREEN", False);
	allowed[NetWMActionChangeDesktop] = XInternAtom(dpy, "_NET_WM_ACTION_CHANGE_DESKTOP", False);
	allowed[NetWMActionClose] = XInternAtom(dpy, "_NET_WM_ACTION_CLOSE", False);
	allowed[NetWMActionAbove] = XInternAtom(dpy, "_NET_WM_ACTION_ABOVE", False);
	allowed[NetWMActionBelow] = XInternAtom(dpy, "_NET_WM_ACTION_BELOW", False);
	motifatom = XInternAtom(dpy, "_MOTIF_WM_HINTS", False);
	xatom[Manager] = XInternAtom(dpy, "MANAGER", False);
	xatom[Xembed] = XInternAtom(dpy, "_XEMBED", False);
	xatom[XembedInfo] = XInternAtom(dpy, "_XEMBED_INFO", False);
	/* init cursors */
	cursor[CurNormal] = drw_cur_create(drw, XC_left_ptr);
	cursor[CurResize] = drw_cur_create(drw, XC_sizing);
	cursor[CurResizeBR] = drw_cur_create(drw, XC_bottom_right_corner);
	cursor[CurResizeBL] = drw_cur_create(drw, XC_bottom_left_corner);
	cursor[CurResizeTR] = drw_cur_create(drw, XC_top_right_corner);
	cursor[CurResizeTL] = drw_cur_create(drw, XC_top_left_corner);
	cursor[CurResizeHorzArrow] = drw_cur_create(drw, XC_sb_h_double_arrow);
	cursor[CurResizeVertArrow] = drw_cur_create(drw, XC_sb_v_double_arrow);
	cursor[CurIronCross] = drw_cur_create(drw, XC_iron_cross);
	cursor[CurMove] = drw_cur_create(drw, XC_fleur);
	/* init appearance */

	scheme = ecalloc(LENGTH(colors) + 1, sizeof(Clr *));
	scheme[LENGTH(colors)] = drw_scm_create(drw, colors[0], alphas[0], ColCount); // ad-hoc color scheme used by status2d

	for (i = 0; i < LENGTH(colors); i++)
		scheme[i] = drw_scm_create(drw, colors[i], alphas[i], ColCount);

	updatebars();

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
	setnumdesktops();
	updatecurrentdesktop();
	setdesktopnames();
	setviewport();
	XDeleteProperty(dpy, root, netatom[NetClientList]);
	XDeleteProperty(dpy, root, netatom[NetClientListStacking]);
	/* select events */
	wa.cursor = cursor[CurNormal]->cursor;
	wa.event_mask = SubstructureRedirectMask|SubstructureNotifyMask
		|ButtonPressMask|PointerMotionMask|EnterWindowMask
		|LeaveWindowMask|StructureNotifyMask|PropertyChangeMask;
	XChangeWindowAttributes(dpy, root, CWEventMask|CWCursor, &wa);
	XSelectInput(dpy, root, wa.event_mask);
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
sigchld(int unused)
{
	pid_t pid;
	if (signal(SIGCHLD, sigchld) == SIG_ERR)
		die("can't install SIGCHLD handler:");
	while (0 < (pid = waitpid(-1, NULL, WNOHANG))) {
		pid_t *p, *lim;

		if (!(p = autostart_pids))
			continue;
		lim = &p[autostart_len];

		for (; p < lim; p++) {
			if (*p == pid) {
				*p = -1;
				break;
			}
		}
	}
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
	spawncmd(arg, 0);
}

pid_t
spawncmd(const Arg *arg, int buttonclick)
{
	pid_t pid;
	if ((pid = fork()) == 0) {
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

					if (!chdir(cwd))
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
		execvp(((char **)arg->v)[1], ((char **)arg->v)+1);
		fprintf(stderr, "dusk: execvp %s", ((char **)arg->v)[1]);
		perror(" failed");
		exit(EXIT_SUCCESS);
	}
	return pid;
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

			wc.sibling = c->ws->mon->bar->win;
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

 	SETFLOATING(c);
 	XRaiseWindow(dpy, c->win);

	if (maximize_vert && maximize_horz)
		setfloatpos(c, "0% 0% 100% 100%", 1);
	else if (maximize_vert)
		setfloatpos(c, "-1x 0% -1w 100%", 1);
	else
		setfloatpos(c, "0% -1y 100% -1h", 1);

	resizeclient(c, c->x, c->y, c->w, c->h);
}

void
unfocus(Client *c, int setfocus, Client *nextfocus)
{
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
	if (!ISMARKED(c))
		XSetWindowBorder(dpy, c->win, scheme[c->scheme][ColBorder].pixel);
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

	ws = c->ws;
	revertws = c->revertws;

	if (c->swallowing)
		unswallow(c);

	s = swallowingclient(c->win);
	if (s) {
		s->swallowing = NULL;
		revertws = NULL;
	}

	if (ISMARKED(c))
		unmarkclient(c);

	if (enabled(AutoReduceNmaster) && ws->nmaster > 1 && ismasterclient(c))
		ws->nmaster--;

	detach(c);
	detachstack(c);
	freeicon(c);

	if (!destroyed) {
		wc.border_width = c->oldbw;
		XGrabServer(dpy); /* avoid race conditions */
		XSetErrorHandler(xerrordummy);
		XConfigureWindow(dpy, c->win, CWBorderWidth, &wc); /* restore border */
		XUngrabButton(dpy, AnyButton, AnyModifier, c->win);
		setclientstate(c, WithdrawnState);
		XSync(dpy, False);
		XSetErrorHandler(xerror);
		XUngrabServer(dpy);
	}
	free(c);

	focus(NULL);
	arrange(ws);
	drawbar(ws->mon);
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
void
unmapnotify(XEvent *e)
{
	Client *c;
	XUnmapEvent *ev = &e->xunmap;
	if (enabled(Debug))
		fprintf(stderr, "unmapnotify: received event type %s (%d), serial %ld, window %ld, event %ld, ev->send_event = %d, ev->from_configure = %d\n", XGetAtomName(dpy, ev->type), ev->type, ev->serial, ev->window, ev->event, ev->send_event, ev->from_configure);

	if ((c = wintoclient(ev->window))) {
		if (enabled(Debug))
			fprintf(stderr, "unmapnotify: window %ld --> client %s (%s)\n", ev->window, c->name, ev->send_event ? "WithdrawnState" : "unmanage");
		if (ev->send_event)
			setclientstate(c, WithdrawnState);
		else
			unmanage(c, 0);
	} else if (enabled(Systray) && (c = wintosystrayicon(ev->window))) {
		/* KLUDGE! sometimes icons occasionally unmap their windows, but do
		 * _not_ destroy them. We map those windows back */
		XMapRaised(dpy, c->win);
		drawbarwin(systray->bar);
	}
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
updategeom(void)
{
	int dirty = 0;
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
		if (n <= nn) { /* new monitors available */
			for (i = n; i < nn; i++) {
				for (m = mons; m && m->next; m = m->next);
				if (m)
					m->next = createmon(i);
				else
					mons = createmon(i);
			}
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
					updatebarpos(m);
					setworkspaceareasformon(mons);
					createpreview(m);
				}
			}
		} else { /* less monitors available nn < n */
			for (i = nn; i < n; i++) {
				for (m = mons; m && m->next; m = m->next);
				if (m == selmon)
					selmon = mons;
				cleanupmon(m);
			}
		}
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
			updatebarpos(mons);
			setworkspaceareasformon(mons);
			createpreview(mons);
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
	for (c = stickyws->clients; c; c = c->next)
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
	if (ee->error_code == BadWindow
	|| (ee->request_code == X_GetAtomName && ee->error_code == BadAtom)
	|| (ee->request_code == X_SetInputFocus && ee->error_code == BadMatch)
	|| (ee->request_code == X_PolyText8 && ee->error_code == BadDrawable)
	|| (ee->request_code == X_PolyFillRectangle && ee->error_code == BadDrawable)
	|| (ee->request_code == X_PolySegment && ee->error_code == BadDrawable)
	|| (ee->request_code == X_ConfigureWindow && ee->error_code == BadMatch)
	|| (ee->request_code == X_GrabButton && ee->error_code == BadAccess)
	|| (ee->request_code == X_GrabKey && ee->error_code == BadAccess)
	|| (ee->request_code == X_CopyArea && ee->error_code == BadDrawable))
		return 0;
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
	Client *c = CLIENT, *at = NULL, *cold, *cprevious = NULL, *p;
	if (!c)
		return;

	if (c && ISFLOATING(c))
		togglefloating(&((Arg) { .v = c }));

	if (!c->ws->layout->arrange || (c && ISFLOATING(c)) || !c)
		return;

	if (c == nexttiled(c->ws->clients)) {
		p = c->ws->prevzoom;
		at = findbefore(p);
		if (at)
			cprevious = nexttiled(at->next);
		if (!cprevious || cprevious != p) {
			c->ws->prevzoom = NULL;
			if (!c || !(c = nexttiled(c->next)))
				return;
		} else
			c = cprevious;
	}

	cold = nexttiled(c->ws->clients);
	if (c != cold && !at)
		at = findbefore(c);

	detach(c);
	attach(c);

	/* swap windows instead of pushing the previous one down */
	if (c != cold && at) {
		c->ws->prevzoom = cold;
		if (cold && at != cold) {
			detach(cold);
			cold->next = at->next;
			at->next = cold;
		}
	}

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
	autostart_exec();
	setup();
#ifdef __OpenBSD__
	if (pledge("stdio rpath proc exec ps", NULL) == -1)
		die("pledge");
#endif /* __OpenBSD__ */
	scan();
	run();
	if (restartsig)
		execvp(argv[0], argv);
	cleanup();
	XCloseDisplay(dpy);
	return EXIT_SUCCESS;
}
