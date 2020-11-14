/* See LICENSE file for copyright and license details.
 *
 * The dawn dynamic window manager is designed like any other X client as well.
 * It is driven through handling X events. In contrast to other X clients, a
 * window manager selects for SubstructureRedirectMask on the root window, to
 * receive events about window (dis-)appearance. Only one X connection at a
 * time is allowed to select for this event mask.
 *
 * The event handlers of dawn are organized in an array which is accessed
 * whenever a new event has been fetched. This allows event dispatching
 * in O(1) time.
 *
 * Each child of the root window is called a client, except windows which have
 * set the override_redirect flag. Clients are organized in a linked client
 * list on each monitor, the focus history is remembered through a stack list
 * on each monitor. Each client contains a bit array to indicate the tags of a
 * client.
 *
 * Keys and tagging rules are organized as arrays and defined in config.h.
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
#define NUMTAGS                 9
#define BARRULES                20
#define BUTTONMASK              (ButtonPressMask|ButtonReleaseMask)
#define CLEANMASK(mask)         (mask & ~(numlockmask|LockMask) & (ShiftMask|ControlMask|Mod1Mask|Mod2Mask|Mod3Mask|Mod4Mask|Mod5Mask))
#define INTERSECT(x,y,w,h,m)    (MAX(0, MIN((x)+(w),(m)->wx+(m)->ww) - MAX((x),(m)->wx)) \
                               * MAX(0, MIN((y)+(h),(m)->wy+(m)->wh) - MAX((y),(m)->wy)))
#define ISVISIBLEONTAG(C, T)    ((C->tags & T) || (C->flags & Sticky))
#define ISVISIBLE(C)            ISVISIBLEONTAG(C, C->mon->tagset[C->mon->seltags])
#define LENGTH(X)               (sizeof X / sizeof X[0])
#define MOUSEMASK               (BUTTONMASK|PointerMotionMask)
#define WIDTH(X)                ((X)->w + 2 * (X)->bw)
#define HEIGHT(X)               ((X)->h + 2 * (X)->bw)
#define WTYPE                   "_NET_WM_WINDOW_TYPE_"
#define TOTALTAGS               (NUMTAGS + LENGTH(scratchpads))
#define TAGMASK                 ((1 << TOTALTAGS) - 1)
#define SPTAG(i)                ((1 << NUMTAGS) << (i))
#define SPTAGMASK               (((1 << LENGTH(scratchpads))-1) << NUMTAGS)
#define TEXTWM(X)               (drw_fontset_getwidth(drw, (X), True) + lrpad)
#define TEXTW(X)                (drw_fontset_getwidth(drw, (X), False) + lrpad)
#define HIDDEN(C)               ((getstate(C->win) == IconicState))

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
	SchemeTagsNorm,
	SchemeTagsSel,
	SchemeHid,
	SchemeUrg,
	SchemeFlexActTTB,
	SchemeFlexActLTR,
	SchemeFlexActMONO,
	SchemeFlexActGRID,
	SchemeFlexActGRD1,
	SchemeFlexActGRD2,
	SchemeFlexActGRDM,
	SchemeFlexActHGRD,
	SchemeFlexActDWDL,
	SchemeFlexActSPRL,
	SchemeFlexInaTTB,
	SchemeFlexInaLTR,
	SchemeFlexInaMONO,
	SchemeFlexInaGRID,
	SchemeFlexInaGRD1,
	SchemeFlexInaGRD2,
	SchemeFlexInaGRDM,
	SchemeFlexInaHGRD,
	SchemeFlexInaDWDL,
	SchemeFlexInaSPRL,
	SchemeFlexSelTTB,
	SchemeFlexSelLTR,
	SchemeFlexSelMONO,
	SchemeFlexSelGRID,
	SchemeFlexSelGRD1,
	SchemeFlexSelGRD2,
	SchemeFlexSelGRDM,
	SchemeFlexSelHGRD,
	SchemeFlexSelDWDL,
	SchemeFlexSelSPRL,
	SchemeFlexActFloat,
	SchemeFlexInaFloat,
	SchemeFlexSelFloat,
}; /* color schemes */

enum {
	NetActiveWindow,
	NetClientList,
	NetClientListStacking,
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
	NetWMActionClose,
	NetWMCheck,
	NetWMFullPlacement,
	NetWMFullscreen,
	NetWMName,
	NetWMState,
	NetWMStateAbove,
	NetWMWindowOpacity,
	NetWMWindowType,
	NetWMWindowTypeDock,
	NetWMMaximizedVert,
	NetWMMaximizedHorz,
	NetWMMoveResize,
	NetLast
}; /* EWMH atoms */

enum {
	WMChangeState,
	WMDelete,
	WMProtocols,
	WMState,
	WMTakeFocus,
	WMWindowRole,
	WMLast
}; /* default atoms */

enum {
	IsFloating,
	DawnClientFlags,
	DawnMonitorTags,
	ClientLast
}; /* dawn client atoms */

/* https://specifications.freedesktop.org/wm-spec/latest/ar01s05.html - Application Window Properties */

enum {
	ClkButton,
	ClkTagBar,
	ClkLtSymbol,
	ClkStatusText,
	ClkWinTitle,
	ClkClientWin,
	ClkRootWin,
	ClkLast
}; /* clicks */

enum {
	BAR_ALIGN_LEFT,
	BAR_ALIGN_CENTER,
	BAR_ALIGN_RIGHT,
	BAR_ALIGN_LEFT_LEFT,
	BAR_ALIGN_LEFT_RIGHT,
	BAR_ALIGN_LEFT_CENTER,
	BAR_ALIGN_NONE,
	BAR_ALIGN_RIGHT_LEFT,
	BAR_ALIGN_RIGHT_RIGHT,
	BAR_ALIGN_RIGHT_CENTER,
	BAR_ALIGN_LAST
}; /* bar alignment */

/* Named flextile constants */
enum {
	LAYOUT,       // controls overall layout arrangement / split
	MASTER,       // indicates the tile arrangement for the master area
	STACK,        // indicates the tile arrangement for the stack area
	STACK2,       // indicates the tile arrangement for the secondary stack area
	LTAXIS_LAST,
};

typedef struct TagState TagState;
struct TagState {
       int selected;
       int occupied;
       int urgent;
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
struct Bar {
	Window win;
	Monitor *mon;
	Bar *next;
	int idx;
	int showbar;
	int topbar;
	int external;
	int borderpx;
	int groupactive;
	int bx, by, bw, bh; /* bar geometry */
	int w[BARRULES]; // width, array length == barrules, then use r index for lookup purposes
	int x[BARRULES]; // x position, array length == ^
};

typedef struct {
	int x;
	int y;
	int h;
	int w;
} BarArg;

typedef struct {
	int monitor;
	int bar;
	int alignment; // see bar alignment enum
	int (*widthfunc)(Bar *bar, BarArg *a);
	int (*drawfunc)(Bar *bar, BarArg *a);
	int (*clickfunc)(Bar *bar, Arg *arg, BarArg *a);
	char *name; // for debugging
	int x, w; // position, width for internal use
} BarRule;

typedef struct {
	unsigned int click;
	unsigned int mask;
	unsigned int button;
	void (*func)(const Arg *arg);
	const Arg arg;
} Button;

typedef struct Client Client;
struct Client {
	char name[256];
	float mina, maxa;
	float cfact;
	int x, y, w, h;
	int sfx, sfy, sfw, sfh; /* stored float geometry, used on mode revert */
	int oldx, oldy, oldw, oldh;
	int basew, baseh, incw, inch, maxw, maxh, minw, minh;
	int bw, oldbw;
	unsigned int id;
	unsigned int tags;
	unsigned int reverttags; /* holds the original tag info from when the client was opened */
	double opacity;
	pid_t pid;
	Client *next;
	Client *snext;
	Client *swallowing;
	Monitor *mon;
	Window win;
	ClientState prevstate;
	unsigned long flags;
	unsigned long prevflags;
};

typedef struct {
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
	void (*symbolfunc)(Monitor *, unsigned int);
} LayoutPreset;

typedef struct {
	const char *symbol;
	void (*arrange)(Monitor *);
	LayoutPreset preset;
} Layout;

typedef struct Pertag Pertag;
struct Monitor {
	int index;
	char ltsymbol[16];
	float mfact;
	int ltaxis[4];
	int nstack;
	int nmaster;
	int num;
	int mx, my, mw, mh;   /* screen size */
	int wx, wy, ww, wh;   /* window area  */
	int gappih;           /* horizontal gap between windows */
	int gappiv;           /* vertical gap between windows */
	int gappoh;           /* horizontal outer gaps */
	int gappov;           /* vertical outer gaps */
	unsigned int borderpx;
	unsigned int seltags;
	unsigned int sellt;
	unsigned int tagset[2];
	int showbar;
	Client *clients;
	Client *sel;
	Client *stack;
	Monitor *next;
	Bar *bar;
	const Layout *lt[2];
	unsigned int alttag;
	Pertag *pertag;
	char lastltsymbol[16];
	TagState tagstate;
	Client *lastsel;
	const Layout *lastlt;
};

typedef struct {
	const char *class;
	const char *role;
	const char *instance;
	const char *title;
	const char *wintype;
	double opacity;
	unsigned int tags;
	unsigned long flags;
	const char *floatpos;
	int monitor;
} Rule;

#define RULE(...) { .monitor = -1, ##__VA_ARGS__ },

typedef struct {
	int monitor;
	int tag;
	int layout;
	float mfact;
	int nmaster;
	int showbar;
	int topbar;
} MonitorRule;

/* function declarations */
static void applyrules(Client *c);
static int applysizehints(Client *c, int *x, int *y, int *w, int *h, int interact);
static void arrange(Monitor *m);
static void arrangemon(Monitor *m);
static void attach(Client *c);
static void attachstack(Client *c);
static void buttonpress(XEvent *e);
static void checkotherwm(void);
static void cleanup(void);
static void cleanupmon(Monitor *mon);
static void clientmessage(XEvent *e);
static void configure(Client *c);
static void configurenotify(XEvent *e);
static void configurerequest(XEvent *e);
static Monitor *createmon(void);
static void destroynotify(XEvent *e);
static void detach(Client *c);
static void detachstack(Client *c);
static Monitor *dirtomon(int dir);
static void drawbar(Monitor *m);
static void drawbars(void);
static void drawbarwin(Bar *bar);
static void enternotify(XEvent *e);
static void expose(XEvent *e);
static void focus(Client *c);
static void focusin(XEvent *e);
static void focusmon(const Arg *arg);
static void focusstack(const Arg *arg);
static Atom getatomprop(Client *c, Atom prop);
static int getrootptr(int *x, int *y);
static long getstate(Window w);
static int gettextprop(Window w, Atom atom, char *text, unsigned int size);
static void grabbuttons(Client *c, int focused);
static void grabkeys(const Arg *arg);
static void incnmaster(const Arg *arg);
static void keypress(XEvent *e);
static void killclient(const Arg *arg);
static void manage(Window w, XWindowAttributes *wa);
static void mappingnotify(XEvent *e);
static void maprequest(XEvent *e);
static void motionnotify(XEvent *e);
static void movemouse(const Arg *arg);
static Client *nexttiled(Client *c);
static Client *prevtiled(Client *c);
static void pop(Client *);
static void propertynotify(XEvent *e);
static void quit(const Arg *arg);
static Monitor *recttomon(int x, int y, int w, int h);
static void resize(Client *c, int x, int y, int w, int h, int interact);
static void resizeclient(Client *c, int x, int y, int w, int h);
static void resizemouse(const Arg *arg);
static void restack(Monitor *m);
static void run(void);
static void scan(void);
static int sendevent(Window w, Atom proto, int m, long d0, long d1, long d2, long d3, long d4);
static void sendmon(Client *c, Monitor *m);
static void setclientstate(Client *c, long state);
static void setfocus(Client *c);
static void setfullscreen(Client *c, int fullscreen, int setfakefullscreen);
static void setlayout(const Arg *arg);
static void setmfact(const Arg *arg);
static void setup(void);
static void seturgent(Client *c, int urg);
static void showhide(Client *c);
static void sigchld(int unused);
static void spawn(const Arg *arg);
static void tag(const Arg *arg);
static void tagfittomon(Client *c, Monitor *m, int *cx, int *cy, int *cw, int *ch);
static void tagmon(const Arg *arg);
static void tagmonresize(Client *c, Monitor *old, Monitor *m);
static void tagrelposmon(Client *c, Monitor *o, Monitor *n, int *cx, int *cy, int *cw, int *ch);
static void togglebar(const Arg *arg);
static void togglefloating(const Arg *arg);
static void togglemaximize(Client *c, int maximize_vert, int maximize_horz);
static void toggletag(const Arg *arg);
static void toggleview(const Arg *arg);
static void unfocus(Client *c, int setfocus, Client *nextfocus);
static void unmanage(Client *c, int destroyed);
static void unmapnotify(XEvent *e);
static void updatebarpos(Monitor *m);
static void updatebars(void);
static void updateclientlist(void);
static int updategeom(void);
static void updatenumlockmask(void);
static void updatesizehints(Client *c);
static void updatestatus(void);
static void updatetitle(Client *c);
static void updatewmhints(Client *c);
static void view(const Arg *arg);
static Client *wintoclient(Window w);
static Monitor *wintomon(Window w);
static int xerror(Display *dpy, XErrorEvent *ee);
static int xerrordummy(Display *dpy, XErrorEvent *ee);
static int xerrorstart(Display *dpy, XErrorEvent *ee);
static void zoom(const Arg *arg);

/* bar functions */

#include "patch/include.h"

/* variables */
static const char broken[] = "broken";
static char stext[1024];
static char rawstext[1024];
static char estext[1024];
static char rawestext[1024];

static int screen;
static int sw, sh;           /* X display screen geometry width, height */
static int bh;               /* bar geometry */
static int lrpad;            /* sum of left and right padding for text */
static int force_warp = 0;   /* force warp in some situations, e.g. killclient */
static int ignore_warp = 0;  /* force skip warp in some situations, e.g. dragmfact, dragcfact */

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
	[KeyRelease] = keyrelease,
	[MappingNotify] = mappingnotify,
	[MapRequest] = maprequest,
	[MotionNotify] = motionnotify,
	[PropertyNotify] = propertynotify,
	[ResizeRequest] = resizerequest,
	[UnmapNotify] = unmapnotify
};
static Atom wmatom[WMLast], netatom[NetLast], xatom[XLast], clientatom[ClientLast];
static int running = 1;
static Cur *cursor[CurLast];
static Clr **scheme;
static Display *dpy;
static Drw *drw;
static Monitor *mons, *selmon;
static Window root, wmcheckwin;

/* configuration, allows nested code to access above variables */
#include "config.h"

#include "patch/include.c"

/* compile-time check if all tags fit into an unsigned int bit array. */
struct NumTags { char limitexceeded[NUMTAGS > 31 ? -1 : 1]; };

/* function implementations */
void
applyrules(Client *c)
{
	const char *class, *instance;
	Atom wintype;
	char role[64];
	unsigned int i;
	unsigned int newtagset;
	const Rule *r;
	Monitor *m;
	XClassHint ch = { NULL, NULL };

	/* rule matching */
	c->tags = 0;
	if (!c->opacity)
		c->opacity = defaultopacity;
	XGetClassHint(dpy, c->win, &ch);
	class    = ch.res_class ? ch.res_class : broken;
	instance = ch.res_name  ? ch.res_name  : broken;
	wintype  = getatomprop(c, netatom[NetWMWindowType]);
	gettextprop(c->win, wmatom[WMWindowRole], role, sizeof(role));

	if (enabled(Debug))
		fprintf(stderr, "applyrules: new client %s, class = '%s', instance = '%s', role = '%s', wintype = '%ld'\n", c->name, class, instance, role, wintype);

	for (i = 0; i < LENGTH(rules); i++) {
		r = &rules[i];
		if ((!r->title || strstr(c->name, r->title))
		&& (!r->class || strstr(class, r->class))
		&& (!r->role || strstr(role, r->role))
		&& (!r->instance || strstr(instance, r->instance))
		&& (!r->wintype || wintype == XInternAtom(dpy, r->wintype, False)))
		{
			c->flags |= Ruled | r->flags;
			c->tags |= r->tags;

			if ((r->tags & SPTAGMASK) && ISFLOATING(c)) {
				c->x = c->mon->wx + (c->mon->ww / 2 - WIDTH(c) / 2);
				c->y = c->mon->wy + (c->mon->wh / 2 - HEIGHT(c) / 2);
			}

			if (r->opacity)
				c->opacity = r->opacity;

			for (m = mons; m && m->num != r->monitor; m = m->next);
			if (m)
				c->mon = m;

			if (ISFLOATING(c) && r->floatpos)
				setfloatpos(c, r->floatpos);

			if ((SWITCHTAG(c) || ENABLETAG(c)) && (NOSWALLOW(c) || !termforwin(c))) {
				selmon = c->mon;
				newtagset = SWITCHTAG(c) ? c->tags : c->mon->tagset[c->mon->seltags] | c->tags;

				/* Switch to the client's tag, but only if that tag is not already shown */
				if (newtagset && !(c->tags & c->mon->tagset[c->mon->seltags])) {
					if (REVERTTAG(c))
						c->reverttags = c->mon->tagset[c->mon->seltags];
					if (SWITCHTAG(c)) {
						pertagview(&((Arg) { .ui = newtagset }));
					} else {
						c->mon->tagset[c->mon->seltags] = newtagset;
					}
				}
			}

			if (enabled(Debug))
				fprintf(stderr, "applyrules: client rule %d matched:\n    class: %s\n    role: %s\n    instance: %s\n    title: %s\n    wintype: %s\n    tags: %d\n    flags: %ld\n    floatpos: %s\n    monitor: %d\n",
					i,
					r->class ? r->class : "NULL",
					r->role ? r->role : "NULL",
					r->instance ? r->instance : "NULL",
					r->title ? r->title : "NULL",
					r->wintype ? r->wintype : "NULL",
					r->tags,
					r->flags,
					r->floatpos ? r->floatpos : "NULL",
					r->monitor);
			break; // only allow one rule match
		}
	}

	if (ch.res_class)
		XFree(ch.res_class);
	if (ch.res_name)
		XFree(ch.res_name);

	c->tags = c->tags & TAGMASK ? c->tags & TAGMASK : (c->mon->tagset[c->mon->seltags] & ~SPTAGMASK);
}

int
applysizehints(Client *c, int *x, int *y, int *w, int *h, int interact)
{
	int baseismin;
	Monitor *m = c->mon;

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
	if (!IGNORESIZEHINTS(c) && (resizehints || ISFLOATING(c) || !c->mon->lt[c->mon->sellt]->arrange)) {
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
arrange(Monitor *m)
{
	if (m)
		showhide(m->stack);
	else for (m = mons; m; m = m->next)
		showhide(m->stack);
	if (m) {
		arrangemon(m);
		restack(m);
	} else for (m = mons; m; m = m->next)
		arrangemon(m);
}

void
arrangemon(Monitor *m)
{
	strncpy(m->ltsymbol, m->lt[m->sellt]->symbol, sizeof m->ltsymbol);
	if (m->lt[m->sellt]->arrange)
		m->lt[m->sellt]->arrange(m);
}

void
attach(Client *c)
{
	c->next = c->mon->clients;
	c->mon->clients = c;
}

void
attachstack(Client *c)
{
	c->snext = c->mon->stack;
	c->mon->stack = c;
}

void
buttonpress(XEvent *e)
{
	int click, i, r;
	Arg arg = {0};
	Client *c;
	Monitor *m;
	Bar *bar;
	XButtonPressedEvent *ev = &e->xbutton;
	const BarRule *br;
	BarArg carg = { 0, 0, 0, 0 };
	click = ClkRootWin;
	/* focus monitor if necessary */
	if ((m = wintomon(ev->window)) && m != selmon) {
		unfocus(selmon->sel, 1, NULL);
		selmon = m;
		focus(NULL);
	}

	for (bar = selmon->bar; bar; bar = bar->next) {
		if (ev->window == bar->win) {
			for (r = 0; r < LENGTH(barrules); r++) {
				br = &barrules[r];
				if (br->bar != bar->idx || (br->monitor == 'A' && m != selmon) || br->clickfunc == NULL)
					continue;
				if (br->monitor != 'A' && br->monitor != -1 && br->monitor != bar->mon->index)
					continue;
				if (bar->x[r] <= ev->x && ev->x <= bar->x[r] + bar->w[r]) {
					carg.x = ev->x - bar->x[r];
					carg.y = ev->y - bar->borderpx;
					carg.w = bar->w[r];
					carg.h = bar->bh - 2 * bar->borderpx;
					click = br->clickfunc(bar, &arg, &carg);
					if (click < 0)
						return;
					break;
				}
			}
			break;
		}
	}

	if (click == ClkRootWin && (c = wintoclient(ev->window))) {
		focus(c);
		restack(selmon);
		XAllowEvents(dpy, ReplayPointer, CurrentTime);
		click = ClkClientWin;
	}

	for (i = 0; i < LENGTH(buttons); i++) {
		if (click == buttons[i].click && buttons[i].func && buttons[i].button == ev->button
				&& CLEANMASK(buttons[i].mask) == CLEANMASK(ev->state)) {
			buttons[i].func((click == ClkTagBar || click == ClkWinTitle) && buttons[i].arg.i == 0 ? &arg : &buttons[i].arg);
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
	Arg a = {.ui = ~0};
	Layout foo = { "", NULL };
	Monitor *m;
	size_t i;
	view(&a);
	selmon->lt[selmon->sellt] = &foo;
	for (m = mons; m; m = m->next)
		while (m->stack)
			unmanage(m->stack, 0);
	XUngrabKey(dpy, AnyKey, AnyModifier, root);
	while (mons)
		cleanupmon(mons);
	if (enabled(Systray) && systray) {
		if (systray->win) {
			XUnmapWindow(dpy, systray->win);
			XDestroyWindow(dpy, systray->win);
		}
		free(systray);
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
	Monitor *m;
	Bar *bar;

	if (mon == mons)
		mons = mons->next;
	else {
		for (m = mons; m && m->next != mon; m = m->next);
		m->next = mon->next;
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
	free(mon);
}

void
clientmessage(XEvent *e)
{
	XWindowAttributes wa;
	XSetWindowAttributes swa;
	XClientMessageEvent *cme = &e->xclient;
	Client *c = wintoclient(cme->window);
	unsigned int i, maximize_vert, maximize_horz;
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

			c->mon = selmon;
			c->next = systray->icons;
			systray->icons = c;
			XGetWindowAttributes(dpy, c->win, &wa);
			c->x = c->oldx = c->y = c->oldy = 0;
			c->w = c->oldw = wa.width;
			c->h = c->oldh = wa.height;
			c->oldbw = wa.border_width;
			c->bw = 0;
			SETFLOATING(c);
			/* reuse tags field as mapped status */
			c->tags = 1;
			updatesizehints(c);
			updatesystrayicongeom(c, wa.width, wa.height);
			XAddToSaveSet(dpy, c->win);
			XSelectInput(dpy, c->win, StructureNotifyMask | PropertyChangeMask | ResizeRedirectMask);
			XClassHint ch = {"dawnsystray", "dawnsystray"};
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
		if (cme->data.l[1] == netatom[NetWMFullscreen]
		 || cme->data.l[2] == netatom[NetWMFullscreen]) {
			if (RESTOREFAKEFULLSCREEN(c) && ISFULLSCREEN(c))
				setfakefullscreen = 1;
			setfullscreen(c, (cme->data.l[0] == 1 /* _NET_WM_STATE_ADD    */
				|| (cme->data.l[0] == 2 /* _NET_WM_STATE_TOGGLE */
				&& !ISFULLSCREEN(c)
			)), setfakefullscreen);
		}

		maximize_vert = (cme->data.l[1] == netatom[NetWMMaximizedVert] || cme->data.l[2] == netatom[NetWMMaximizedVert]);
		maximize_horz = (cme->data.l[1] == netatom[NetWMMaximizedHorz] || cme->data.l[2] == netatom[NetWMMaximizedHorz]);
		if (maximize_vert || maximize_horz)
			togglemaximize(c, maximize_vert, maximize_horz);
	} else if (cme->message_type == netatom[NetActiveWindow]) {
		if (enabled(FocusOnNetActive)) {
			if (c->tags & c->mon->tagset[c->mon->seltags]) {
				selmon = c->mon;
				focus(c);
			} else {
				for (i = 0; i < NUMTAGS && !((1 << i) & c->tags); i++);
				if (i < NUMTAGS) {
					selmon = c->mon;
					if (((1 << i) & TAGMASK) != selmon->tagset[selmon->seltags])
						view(&((Arg) { .ui = 1 << i }));
					focus(c);
					restack(selmon);
				}
			}
		} else if (c != selmon->sel && !ISURGENT(c))
			seturgent(c, 1);
	} else if (cme->message_type == wmatom[WMChangeState]) {
		if (cme->data.l[0] == IconicState && !HIDDEN(c))
			hide(c);
		else if (cme->data.l[0] == NormalState && HIDDEN(c))
			show(c);
	} else if (cme->message_type == netatom[NetWMActionClose]) {
		selmon->sel = c;
		killclient(NULL);
	} else if (cme->message_type == netatom[NetWMMoveResize]) {
		resizemouse(&((Arg) { .v = c }));
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
	Client *c;
	XConfigureEvent *ev = &e->xconfigure;
	int dirty;
	/* TODO: updategeom handling sucks, needs to be simplified */
	if (ev->window == root) {

		if (enabled(Debug)) {
			fprintf(stderr, "configurenotify: received event for root window\n");
			fprintf(stderr, "    - x = %d, y = %d, w = %d, h = %d\n", ev->x, ev->y, ev->width, ev->height);
		}

		dirty = (sw != ev->width || sh != ev->height);
		sw = ev->width;
		sh = ev->height;
		if (updategeom() || dirty) {
			drw_resize(drw, sw, sh);
			updatebars();
			for (m = mons; m; m = m->next) {
				for (c = m->clients; c; c = c->next)
					if (ISFULLSCREEN(c) && !ISFAKEFULLSCREEN(c))
						resizeclient(c, m->mx, m->my, m->mw, m->mh);
				for (bar = m->bar; bar; bar = bar->next)
					XMoveResizeWindow(dpy, bar->win, bar->bx, bar->by, bar->bw, bar->bh);
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

	if ((c = wintoclient(ev->window))) {

		if (enabled(Debug)) {
			fprintf(stderr, "configurerequest: received event %ld for client %s\n", ev->value_mask, c->name);
			fprintf(stderr, "    - x = %d, y = %d, w = %d, h = %d\n", ev->x, ev->y, ev->width, ev->height);
		}

		if (IGNORECFGREQ(c) || MOVERESIZE(c))
			return;
		if (ev->value_mask & CWBorderWidth)
			c->bw = ev->border_width;
		else if (ISFLOATING(c) || !selmon->lt[selmon->sellt]->arrange) {
			if (IGNORECFGREQPOS(c) && IGNORECFGREQSIZE(c))
				return;

			m = c->mon;
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
createmon(void)
{
	Monitor *m, *mon;
	int i, n, mi, max_bars = 2, istopbar = topbar;
	int layout;

	const BarRule *br;
	Bar *bar;
	int j;
	const MonitorRule *mr;

	m = ecalloc(1, sizeof(Monitor));
	m->tagset[0] = m->tagset[1] = 1;
	m->mfact = mfact;
	m->nmaster = nmaster;
	m->nstack = nstack;
	m->showbar = showbar;
	m->borderpx = borderpx;
	m->gappih = gappih;
	m->gappiv = gappiv;
	m->gappoh = gappoh;
	m->gappov = gappov;
	for (mi = 0, mon = mons; mon; mon = mon->next, mi++); // monitor index
	m->index = mi;
	for (j = 0; j < LENGTH(monrules); j++) {
		mr = &monrules[j];
		if ((mr->monitor == -1 || mr->monitor == mi)
				&& (mr->tag <= 0 || (m->tagset[0] & (1 << (mr->tag - 1))))) {
			layout = MAX(mr->layout, 0);
			layout = MIN(layout, LENGTH(layouts) - 1);
			m->lt[0] = &layouts[layout];
			m->lt[1] = &layouts[1 % LENGTH(layouts)];
			strncpy(m->ltsymbol, layouts[layout].symbol, sizeof m->ltsymbol);

			if (mr->mfact > -1)
				m->mfact = mr->mfact;
			if (mr->nmaster > -1)
				m->nmaster = mr->nmaster;
			if (mr->showbar > -1)
				m->showbar = mr->showbar;
			if (mr->topbar > -1)
				istopbar = mr->topbar;
			break;
		}
	}

	/* Derive the number of bars for this monitor based on bar rules */
	for (n = -1, i = 0; i < LENGTH(barrules); i++) {
		br = &barrules[i];
		if (br->monitor == 'A' || br->monitor == -1 || br->monitor == mi)
			n = MAX(br->bar, n);
	}

	for (i = 0; i <= n && i < max_bars; i++) {
		bar = ecalloc(1, sizeof(Bar));
		bar->mon = m;
		bar->idx = i;
		bar->next = m->bar;
		bar->topbar = istopbar;
		m->bar = bar;
		istopbar = !istopbar;
		bar->showbar = 1;
		bar->external = 0;
		bar->borderpx = enabled(BarBorder) ? borderpx : 0;
		bar->bh = bh + bar->borderpx * 2;
	}

	m->ltaxis[LAYOUT] = m->lt[0]->preset.layout;
	m->ltaxis[MASTER] = m->lt[0]->preset.masteraxis;
	m->ltaxis[STACK]  = m->lt[0]->preset.stack1axis;
	m->ltaxis[STACK2] = m->lt[0]->preset.stack2axis;

	if (!(m->pertag = (Pertag *)calloc(1, sizeof(Pertag))))
		die("fatal: could not malloc() %u bytes\n", sizeof(Pertag));
	m->pertag->curtag = m->pertag->prevtag = 1;
	for (i = 0; i <= NUMTAGS; i++) {
		m->pertag->nstacks[i] = m->nstack;
		m->pertag->prevzooms[i] = NULL;

		/* init layouts */
		for (j = 0; j < LENGTH(monrules); j++) {
			mr = &monrules[j];
			if ((mr->monitor == -1 || mr->monitor == mi) && (mr->tag == -1 || mr->tag == i)) {
				layout = MAX(mr->layout, 0);
				layout = MIN(layout, LENGTH(layouts) - 1);
				m->pertag->ltidxs[i][0] = &layouts[layout];
				m->pertag->ltidxs[i][1] = m->lt[0];
				m->pertag->nmasters[i] = (mr->nmaster > -1 ? mr->nmaster : m->nmaster);
				m->pertag->mfacts[i] = (mr->mfact > -1 ? mr->mfact : m->mfact);
				m->pertag->showbars[i] = (mr->showbar > -1 ? mr->showbar : m->showbar);
				m->pertag->ltaxis[i][LAYOUT] = m->pertag->ltidxs[i][0]->preset.layout;
				m->pertag->ltaxis[i][MASTER] = m->pertag->ltidxs[i][0]->preset.masteraxis;
				m->pertag->ltaxis[i][STACK]  = m->pertag->ltidxs[i][0]->preset.stack1axis;
				m->pertag->ltaxis[i][STACK2] = m->pertag->ltidxs[i][0]->preset.stack2axis;
				break;
			}
		}
		m->pertag->sellts[i] = m->sellt;

		m->pertag->enablegaps[i] = 1;
	}
	return m;
}

void
destroynotify(XEvent *e)
{
	Client *c;
	XDestroyWindowEvent *ev = &e->xdestroywindow;

	if ((c = wintoclient(ev->window))) {
		if (enabled(Debug))
			fprintf(stderr, "destroynotify: received event for client %s\n", c->name);
		unmanage(c, 1);
	}
	else if ((c = swallowingclient(ev->window)))
		unmanage(c->swallowing, 1);
	else if (enabled(Systray) && (c = wintosystrayicon(ev->window))) {
		if (enabled(Debug))
			fprintf(stderr, "destroynotify: removing systray icon for client %s\n", c->name);
		removesystrayicon(c);
		drawbarwin(systray->bar);
	}
}

void
detach(Client *c)
{
	Client **tc;

	for (tc = &c->mon->clients; *tc && *tc != c; tc = &(*tc)->next);
	*tc = c->next;
}

void
detachstack(Client *c)
{
	Client **tc, *t;

	for (tc = &c->mon->stack; *tc && *tc != c; tc = &(*tc)->snext);
	*tc = c->snext;

	if (c == c->mon->sel) {
		for (t = c->mon->stack; t && !ISVISIBLE(t); t = t->snext);
		c->mon->sel = t;
	}
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

void
drawbar(Monitor *m)
{
	Bar *bar;
	for (bar = m->bar; bar; bar = bar->next)
		drawbarwin(bar);
}

void
drawbars(void)
{
	Monitor *m;
	for (m = mons; m; m = m->next)
		drawbar(m);
}

void
drawbarwin(Bar *bar)
{
	if (!bar || !bar->win || bar->external)
		return;

	int r, w, total_drawn = 0, groupactive, ignored;
	int rx, lx, rw, lw; // bar size, split between left and right if a center module is added
	const BarRule *br;
	Monitor *lastmon;

	if (bar->borderpx) {
		if (enabled(BarActiveGroupBorderColor))
			getclientcounts(bar->mon, &groupactive, &ignored, &ignored, &ignored, &ignored, &ignored, &ignored);
		else
			groupactive = GRP_MASTER;
		XSetForeground(drw->dpy, drw->gc, scheme[getschemefor(bar->mon, groupactive, bar->mon == selmon)][ColBorder].pixel);
		XFillRectangle(drw->dpy, drw->drawable, drw->gc, 0, 0, bar->bw, bar->bh);
	}

	BarArg warg = { 0 };
	BarArg darg  = { 0 };
	warg.h = bar->bh - 2 * bar->borderpx;

	rw = lw = bar->bw - 2 * bar->borderpx;
	rx = lx = bar->borderpx;

	for (lastmon = mons; lastmon && lastmon->next; lastmon = lastmon->next);

	drw_setscheme(drw, scheme[SchemeNorm]);
	drw_rect(drw, lx, bar->borderpx, lw, bar->bh - 2 * bar->borderpx, 1, 1);
	for (r = 0; r < LENGTH(barrules); r++) {
		br = &barrules[r];
		if (br->bar != bar->idx || !br->widthfunc || (br->monitor == 'A' && bar->mon != selmon))
			continue;
		if (br->monitor != 'A' && br->monitor != -1 && br->monitor != bar->mon->index &&
				!(br->drawfunc == draw_systray && br->monitor > lastmon->index && bar->mon->index == 0)) // hack: draw systray on first monitor if the designated one is not available
			continue;
		drw_setscheme(drw, scheme[SchemeNorm]);
		warg.w = (br->alignment < BAR_ALIGN_RIGHT_LEFT ? lw : rw);

		w = br->widthfunc(bar, &warg);
		w = MIN(warg.w, w);

		if (lw <= 0) { // if left is exhausted then switch to right side, and vice versa
			lw = rw;
			lx = rx;
		} else if (rw <= 0) {
			rw = lw;
			rx = lx;
		}

		switch(br->alignment) {
		default:
		case BAR_ALIGN_NONE:
		case BAR_ALIGN_LEFT_LEFT:
		case BAR_ALIGN_LEFT:
			bar->x[r] = lx;
			if (lx == rx) {
				rx += w;
				rw -= w;
			}
			lx += w;
			lw -= w;
			break;
		case BAR_ALIGN_LEFT_RIGHT:
		case BAR_ALIGN_RIGHT:
			bar->x[r] = lx + lw - w;
			if (lx == rx)
				rw -= w;
			lw -= w;
			break;
		case BAR_ALIGN_LEFT_CENTER:
		case BAR_ALIGN_CENTER:
			bar->x[r] = lx + lw / 2 - w / 2;
			if (lx == rx) {
				rw = rx + rw - bar->x[r] - w;
				rx = bar->x[r] + w;
			}
			lw = bar->x[r] - lx;
			break;
		case BAR_ALIGN_RIGHT_LEFT:
			bar->x[r] = rx;
			if (lx == rx) {
				lx += w;
				lw -= w;
			}
			rx += w;
			rw -= w;
			break;
		case BAR_ALIGN_RIGHT_RIGHT:
			bar->x[r] = rx + rw - w;
			if (lx == rx)
				lw -= w;
			rw -= w;
			break;
		case BAR_ALIGN_RIGHT_CENTER:
			bar->x[r] = rx + rw / 2 - w / 2;
			if (lx == rx) {
				lw = lx + lw - bar->x[r] + w;
				lx = bar->x[r] + w;
			}
			rw = bar->x[r] - rx;
			break;
		}
		bar->w[r] = w;
		darg.x = bar->x[r];
		darg.y = bar->borderpx;
		darg.h = bar->bh - 2 * bar->borderpx;
		darg.w = bar->w[r];
		if (br->drawfunc)
			total_drawn += br->drawfunc(bar, &darg);
	}

	if (total_drawn == 0 && bar->showbar) {
		bar->showbar = 0;
		updatebarpos(bar->mon);
		XMoveResizeWindow(dpy, bar->win, bar->bx, bar->by, bar->bw, bar->bh);
		arrangemon(bar->mon);
	}
	else if (total_drawn > 0 && !bar->showbar) {
		bar->showbar = 1;
		updatebarpos(bar->mon);
		XMoveResizeWindow(dpy, bar->win, bar->bx, bar->by, bar->bw, bar->bh);
		drw_map(drw, bar->win, 0, 0, bar->bw, bar->bh);
		arrangemon(bar->mon);
	} else
		drw_map(drw, bar->win, 0, 0, bar->bw, bar->bh);
}

void
enternotify(XEvent *e)
{
	Client *c, *sel;
	Monitor *m;
	XCrossingEvent *ev = &e->xcrossing;

	if ((ev->mode != NotifyNormal || ev->detail == NotifyInferior) && ev->window != root)
		return;
	c = wintoclient(ev->window);
	m = c ? c->mon : wintomon(ev->window);
	if (m != selmon) {
		sel = selmon->sel;
		selmon = m;
		unfocus(sel, 1, c);
	} else if (!c || c == selmon->sel)
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
	Client *f;
	XWindowChanges wc;
	if (!c || !ISVISIBLE(c))
		for (c = selmon->stack; c && !ISVISIBLE(c); c = c->snext);
	if (selmon->sel && selmon->sel != c)
		unfocus(selmon->sel, 0, c);
	if (c) {
		if (c->mon != selmon)
			selmon = c->mon;
		if (ISURGENT(c))
			seturgent(c, 0);
		detachstack(c);
		attachstack(c);
		grabbuttons(c, 1);
		setfocus(c);
		if (enabled(FocusedOnTop)) {
			/* Move all visible tiled clients that are not marked as on top below the bar window */
			wc.stack_mode = Below;
			wc.sibling = c->mon->bar->win;
			for (f = c->mon->stack; f; f = f->snext)
				if (f != c && !ISFLOATING(f) && ISVISIBLE(f) && !(ALWAYSONTOP(f) || ISTRANSIENT(f))) {
					XConfigureWindow(dpy, f->win, CWSibling|CWStackMode, &wc);
					wc.sibling = f->win;
				}

			/* Move the currently focused client above the bar window */
			wc.stack_mode = Above;
			wc.sibling = c->mon->bar->win;
			XConfigureWindow(dpy, c->win, CWSibling|CWStackMode, &wc);

			/* Move all visible floating windows that are not marked as on top below the current window */
			wc.stack_mode = Below;
			wc.sibling = c->win;
			for (f = c->mon->stack; f; f = f->snext)
				if (f != c && ISFLOATING(f) && ISVISIBLE(f) && !(ALWAYSONTOP(f) || ISTRANSIENT(f))) {
					XConfigureWindow(dpy, f->win, CWSibling|CWStackMode, &wc);
					wc.sibling = f->win;
				}
		}
	} else {
		XSetInputFocus(dpy, root, RevertToPointerRoot, CurrentTime);
		XDeleteProperty(dpy, root, netatom[NetActiveWindow]);
	}
	selmon->sel = c;
	if (selmon->lt[selmon->sellt]->arrange == flextile && (
			selmon->ltaxis[MASTER] == MONOCLE ||
			selmon->ltaxis[STACK] == MONOCLE ||
			selmon->ltaxis[STACK2] == MONOCLE))
		arrangemon(selmon);
	drawbars();
}

/* there are some broken focus acquiring clients needing extra handling */
void
focusin(XEvent *e)
{
	XFocusChangeEvent *ev = &e->xfocus;
	if (selmon->sel && ev->window != selmon->sel->win)
		setfocus(selmon->sel);
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
	sel = selmon->sel;
	selmon = m;
	unfocus(sel, 0, NULL);
	focus(NULL);
	if (enabled(Warp))
		warp(selmon->sel);
}

void
focusstack(const Arg *arg)
{
	Client *c = NULL, *i;
	int n;

	if (!selmon->sel)
		return;
	if (arg->i > 0) {
		for (c = selmon->sel->next; c && (!ISVISIBLE(c) || (arg->i == 1 && HIDDEN(c))); c = c->next);
		if (!c)
			for (c = selmon->clients; c && (!ISVISIBLE(c) || (arg->i == 1 && HIDDEN(c))); c = c->next);
	} else {
		for (i = selmon->clients; i != selmon->sel; i = i->next)
			if (ISVISIBLE(i) && !(arg->i == -1 && HIDDEN(i)))
				c = i;
		if (!c)
			for (; i; i = i->next)
				if (ISVISIBLE(i) && !(arg->i == -1 && HIDDEN(i)))
					c = i;
	}
	if (c) {
		focus(c);
		if (enabled(FocusedOnTop)) {
			if (enabled(Warp)) {
				force_warp = 1;
				for (n = 0, i = nexttiled(c->mon->clients); i; i = nexttiled(i->next), n++);
				if (ISFLOATING(c) || !(c->mon->ltaxis[MASTER] == MONOCLE && (abs(c->mon->ltaxis[LAYOUT] == NO_SPLIT || !c->mon->nmaster || n <= c->mon->nmaster))))
					warp(c);
			}
		} else
			restack(selmon);
	}
}

Atom
getatomprop(Client *c, Atom prop)
{
	int di;
	unsigned long dl;
	unsigned char *p = NULL;
	Atom da, atom = None;

	/* FIXME getatomprop should return the number of items and a pointer to
	 * the stored data instead of this workaround */
	Atom req = XA_ATOM;
	if (prop == xatom[XembedInfo])
		req = xatom[XembedInfo];
	if (prop == clientatom[IsFloating] || prop == clientatom[DawnClientFlags] || prop == clientatom[DawnMonitorTags])
		req = AnyPropertyType;
	if (prop == netatom[NetWMWindowOpacity])
		req = AnyPropertyType;

	if (XGetWindowProperty(dpy, c->win, prop, 0L, sizeof atom, False, req,
		&da, &di, &dl, &dl, &p) == Success && p) {
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
grabkeys(const Arg *arg)
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
incnmaster(const Arg *arg)
{
	selmon->nmaster = selmon->pertag->nmasters[selmon->pertag->curtag] = MAX(selmon->nmaster + arg->i, 0);
	arrangemon(selmon);
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
	keysym = XGetKeyboardMapping(dpy, (KeyCode)ev->keycode, 1, &keysyms_return);
	for (i = 0; i < LENGTH(keys); i++)
		if (*keysym == keys[i].keysym
				&& CLEANMASK(keys[i].mod) == CLEANMASK(ev->state)
				&& keys[i].func)
			keys[i].func(&(keys[i].arg));
	XFree(keysym);
}

void
killclient(const Arg *arg)
{
	Client *c = selmon->sel;
	if (!c || ISPERMANENT(c))
		return;
	if (!sendevent(c->win, wmatom[WMDelete], NoEventMask, wmatom[WMDelete], CurrentTime, 0, 0, 0)) {
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
	Client *c, *t = NULL;
	Client *term = NULL;
	Window trans = None;
	XWindowChanges wc;

	c = ecalloc(1, sizeof(Client));
	c->win = w;
	c->pid = winpid(w);

	/* geometry */
	c->x = c->oldx = wa->x;
	c->y = c->oldy = wa->y;
	c->w = c->oldw = wa->width;
	c->h = c->oldh = wa->height;
	c->oldbw = wa->border_width;
	c->cfact = 1.0;
	c->mon = NULL;

	updatetitle(c);
	getdawnclientflags(c);
	getdawnmonitortags(c);
	getclientopacity(c);

	if (!c->mon) {
		if (XGetTransientForHint(dpy, w, &trans) && (t = wintoclient(trans))) {
			addflag(c, Transient);
			addflag(c, Centered);
			c->mon = t->mon;
			c->tags = t->tags;
		} else {
			c->mon = selmon;
		}
	}

	c->bw = c->mon->borderpx;

	if (!RULED(c)) {
		if (c->x == c->mon->wx && c->y == c->mon->wy)
			addflag(c, Centered);

		if (!ISTRANSIENT(c)) {
			applyrules(c);
			term = termforwin(c);
			if (term)
				c->mon = term->mon;
		}
	}

	if (c->opacity)
		opacity(c, c->opacity);

	if (c->x + WIDTH(c) > c->mon->mx + c->mon->mw)
		c->x = c->mon->mx + c->mon->mw - WIDTH(c);
	if (c->y + HEIGHT(c) > c->mon->my + c->mon->mh)
		c->y = c->mon->my + c->mon->mh - HEIGHT(c);
	c->x = MAX(c->x, c->mon->mx);
	/* only fix client y-offset, if the client center might cover the bar */
	c->y = MAX(c->y, ((c->mon->bar->by == c->mon->my) && (c->x + (c->w / 2) >= c->mon->wx)
		&& (c->x + (c->w / 2) < c->mon->wx + c->mon->ww)) ? bh : c->mon->my);

	wc.border_width = c->bw;
	XConfigureWindow(dpy, w, CWBorderWidth, &wc);
	configure(c); /* propagates border_width, if size doesn't change */
	updatesizehints(c);

	/* If the client indicates that it is in fullscreen, or if the FullScreen flag has been
	 * explictly set via client rules, then enable fullscreen now. */
	if (getatomprop(c, netatom[NetWMState]) == netatom[NetWMFullscreen] || ISFULLSCREEN(c)) {
		setflag(c, FullScreen, 0);
		setfullscreen(c, 1, 0);
	}
	updatewmhints(c);
	updatemotifhints(c);

	if (ISCENTERED(c)) {
		if (ISTRANSIENT(c)) {
			/* Transient windows are centered within the geometry of the parent window */
			c->x = t->x + WIDTH(t) / 2 - WIDTH(c) / 2;
			c->y = t->y + HEIGHT(t) / 2 - HEIGHT(c) / 2;
		} else {
			c->x = c->mon->wx + (c->mon->ww - WIDTH(c)) / 2;
			c->y = c->mon->wy + (c->mon->wh - HEIGHT(c)) / 2;
		}
	}

	c->sfx = -9999;
	c->sfy = -9999;
	c->sfw = c->w;
	c->sfh = c->h;

	XSelectInput(dpy, w, EnterWindowMask|FocusChangeMask|PropertyChangeMask|StructureNotifyMask);
	grabbuttons(c, 0);

	if (trans != None)
		c->prevflags |= Floating;
	if (!ISFLOATING(c) && (ISFIXED(c) || WASFLOATING(c) || getatomprop(c, clientatom[IsFloating])))
		SETFLOATING(c);

	if (ISFLOATING(c)) {
		XRaiseWindow(dpy, c->win);
		XSetWindowBorder(dpy, w, scheme[SchemeNorm][ColFloat].pixel);
	}
	attachx(c);
	attachstack(c);
	XChangeProperty(dpy, root, netatom[NetClientList], XA_WINDOW, 32, PropModeAppend,
		(unsigned char *) &(c->win), 1);
	XChangeProperty(dpy, root, netatom[NetClientListStacking], XA_WINDOW, 32, PropModePrepend,
		(unsigned char *) &(c->win), 1);
	XMoveResizeWindow(dpy, c->win, c->x + 2 * sw, c->y, c->w, c->h); /* some windows require this */

	if ((c->flags & Hidden) && !HIDDEN(c))
		hide(c);
	if (!HIDDEN(c))
		setclientstate(c, NormalState);
	if (c->mon == selmon)
		unfocus(selmon->sel, 0, c);
	c->mon->sel = c;
	if (!(term && swallow(term, c))) {
		arrange(c->mon);
		if (!HIDDEN(c))
			XMapWindow(dpy, c->win);
	}
	focus(NULL);
	setfloatinghint(c);
}

void
mappingnotify(XEvent *e)
{
	XMappingEvent *ev = &e->xmapping;

	XRefreshKeyboardMapping(ev);
	if (ev->request == MappingKeyboard)
		grabkeys(NULL);
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
	if (wa.override_redirect)
		return;
	if (!wintoclient(ev->window))
		manage(ev->window, &wa);
}

void
motionnotify(XEvent *e)
{
	static Monitor *mon = NULL;
	Monitor *m;
	Client *sel;
	XMotionEvent *ev = &e->xmotion;

	// temporary code
	// if (enabled(Debug)) {
	// sel = wintoclient(ev->window);
	// if (sel) {
	// 	fprintf(stderr, "motionnotify: received event x = %d, y = %d for client %s\n", ev->x_root, ev->y_root, sel->name);
	// } else if (ev->window == root) {
	// 	fprintf(stderr, "motionnotify: received event x = %d, y = %d for root window\n", ev->x_root, ev->y_root);
	// } else {
	// 	fprintf(stderr, "motionnotify: received event x = %d, y = %d for no window?\n", ev->x_root, ev->y_root);
	// }
	// }
	// temporary code

	if (ev->window != root)
		return;
	if ((m = recttomon(ev->x_root, ev->y_root, 1, 1)) != mon && mon) {
		sel = selmon->sel;
		selmon = m;
		unfocus(sel, 1, NULL);
		focus(NULL);
	}
	mon = m;
}

void
movemouse(const Arg *arg)
{
	int x, y, ocx, ocy, nx, ny;
	Client *c;
	Monitor *m;
	XEvent ev;
	Time lasttime = 0;

	if (!(c = selmon->sel))
		return;
	if (ISFULLSCREEN(c) && !ISFAKEFULLSCREEN(c)) /* no support moving fullscreen windows by mouse */
		return;
	restack(selmon);
	ocx = c->x;
	ocy = c->y;
	if (XGrabPointer(dpy, root, False, MOUSEMASK, GrabModeAsync, GrabModeAsync,
		None, cursor[CurMove]->cursor, CurrentTime) != GrabSuccess)
		return;
	if (!getrootptr(&x, &y))
		return;
	addflag(c, MoveResize);
	do {
		XMaskEvent(dpy, MOUSEMASK|ExposureMask|SubstructureRedirectMask, &ev);
		switch(ev.type) {
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
			if (abs(selmon->wx - nx) < snap)
				nx = selmon->wx;
			else if (abs((selmon->wx + selmon->ww) - (nx + WIDTH(c))) < snap)
				nx = selmon->wx + selmon->ww - WIDTH(c);
			if (abs(selmon->wy - ny) < snap)
				ny = selmon->wy;
			else if (abs((selmon->wy + selmon->wh) - (ny + HEIGHT(c))) < snap)
				ny = selmon->wy + selmon->wh - HEIGHT(c);
			if (!ISFLOATING(c) && selmon->lt[selmon->sellt]->arrange
					&& (abs(nx - c->x) > snap || abs(ny - c->y) > snap))
				togglefloating(NULL);
			if (!selmon->lt[selmon->sellt]->arrange || ISFLOATING(c)) {
				resize(c, nx, ny, c->w, c->h, 1);
				if (enabled(AutoSaveFloats))
					savefloats(NULL);
			}
			break;
		}
	} while (ev.type != ButtonRelease);
	XUngrabPointer(dpy, CurrentTime);
	if ((m = recttomon(c->x, c->y, c->w, c->h)) != selmon) {
		if (c->tags & SPTAGMASK) {
			c->mon->tagset[c->mon->seltags] ^= (c->tags & SPTAGMASK);
			m->tagset[m->seltags] |= (c->tags & SPTAGMASK);
		}
		sendmon(c, m);
		selmon = m;
		focus(NULL);
	}
	removeflag(c, MoveResize);
}

Client *
nexttiled(Client *c)
{
	for (; c && (ISFLOATING(c) || !ISVISIBLE(c) || HIDDEN(c)); c = c->next);
	return c;
}

Client *
prevtiled(Client *c)
{
	Client *p, *r;
	for (p = nexttiled(c->mon->clients), r = NULL; p && p != c && (r = p); p = nexttiled(p->next));
	return r;
}

void
pop(Client *c)
{
	detach(c);
	attach(c);
	focus(c);
	arrangemon(c->mon);
	restack(c->mon);
}

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

	if ((ev->window == root) && (ev->atom == XA_WM_NAME)) {
		updatestatus();
	} else if (ev->state == PropertyDelete) {
		if (enabled(Debug)) {
			if ((c = wintoclient(ev->window))) {
				fprintf(stderr, "propertynotify: ignored property delete event %s (%ld) for client %s\n", XGetAtomName(dpy, ev->atom), ev->atom, c->name);
			} else {
				fprintf(stderr, "propertynotify: ignored property delete event %s (%ld) for unknown client\n", XGetAtomName(dpy, ev->atom), ev->atom);
			}
		}
		return; /* ignore */
	} else if ((c = wintoclient(ev->window))) {

		if (enabled(Debug))
			fprintf(stderr, "propertynotify: received message type of %s (%ld) for client %s\n", XGetAtomName(dpy, ev->atom), ev->atom, c->name);

		switch(ev->atom) {
		default: break;
		case XA_WM_TRANSIENT_FOR:
			XGetTransientForHint(dpy, c->win, &trans);
			setflag(c, Floating, (wintoclient(trans)) != NULL);
			if (WASFLOATING(c) && ISFLOATING(c))
				arrange(c->mon);
			break;
		case XA_WM_NORMAL_HINTS:
			updatesizehints(c);
			break;
		case XA_WM_HINTS:
			updatewmhints(c);
			if (ISURGENT(c))
				drawbars();
			break;
		}
		if (ev->atom == XA_WM_NAME || ev->atom == netatom[NetWMName]) {
			updatetitle(c);
			if (c == c->mon->sel)
				drawbar(c->mon);
		}
		if (ev->atom == motifatom)
			updatemotifhints(c);
	}
}

void
quit(const Arg *arg)
{
	Monitor *m;
	Client *c;
	size_t i;
	if (arg->i)
		restart = 1;
	running = 0;

	/* kill child processes */
	for (i = 0; i < autostart_len; i++) {
		if (0 < autostart_pids[i]) {
			kill(autostart_pids[i], SIGTERM);
			waitpid(autostart_pids[i], NULL, 0);
		}
	}

	/* set dawn client atoms */
	for (m = mons; m; m = m->next)
		for (i = 1, c = m->clients; c; c = c->next, ++i) {
			c->id = i;
			setdawnclientflags(c);
			setdawnmonitortags(c);
		}
	XSync(dpy, False);
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

void
resize(Client *c, int x, int y, int w, int h, int interact)
{
	if (applysizehints(c, &x, &y, &w, &h, interact))
		resizeclient(c, x, y, w, h);
}

void
resizeclient(Client *c, int x, int y, int w, int h)
{
	XWindowChanges wc;

	if (!ISLOCKED(c)) {
		c->oldx = c->x;
		c->oldy = c->y;
		c->oldw = c->w;
		c->oldh = c->h;
	}
	c->x = wc.x = x;
	c->y = wc.y = y;
	c->w = wc.width = w;
	c->h = wc.height = h;
	wc.border_width = c->bw;
	if (enabled(NoBorder) && ((nexttiled(c->mon->clients) == c && !nexttiled(c->next)))
		&& (ISFAKEFULLSCREEN(c) || !ISFULLSCREEN(c))
		&& !ISFLOATING(c)
		&& c->mon->lt[c->mon->sellt]->arrange)
	{
		wc.width += c->bw * 2;
		wc.height += c->bw * 2;
		wc.border_width = 0;
	}
	XConfigureWindow(dpy, c->win, CWX|CWY|CWWidth|CWHeight|CWBorderWidth, &wc);
	configure(c);
	if (ISFAKEFULLSCREEN(c) && WASFULLSCREEN(c)) {
		/* Exception: if the client was in actual fullscreen and we exit out to fake fullscreen
		 * mode, then the focus would drift to whichever window is under the mouse cursor at the
		 * time. To avoid this we pass True to XSync which will make the X server disregard any
		 * other events in the queue thus cancelling the EnterNotify event that would otherwise
		 * have changed focus. */
		XSync(dpy, True);
	} else
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
	Monitor *m;
	XEvent ev;
	Time lasttime = 0;

	if (!(c = selmon->sel))
		return;
	if (ISFULLSCREEN(c) && !ISFAKEFULLSCREEN(c)) /* no support resizing fullscreen windows by mouse */
		return;
	restack(selmon);
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
		switch(ev.type) {
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

			if (c->mon->wx + nw >= selmon->wx && c->mon->wx + nw <= selmon->wx + selmon->ww
			&& c->mon->wy + nh >= selmon->wy && c->mon->wy + nh <= selmon->wy + selmon->wh)
			{
				if (!ISFLOATING(c) && selmon->lt[selmon->sellt]->arrange
				&& (abs(nw - c->w) > snap || abs(nh - c->h) > snap))
					togglefloating(NULL);
			}
			if (!selmon->lt[selmon->sellt]->arrange || ISFLOATING(c)) {
				resizeclient(c, nx, ny, nw, nh);
				if (enabled(AutoSaveFloats))
					savefloats(NULL);
			}
			break;
		}
	} while (ev.type != ButtonRelease);

	XUngrabPointer(dpy, CurrentTime);
	while (XCheckMaskEvent(dpy, EnterWindowMask, &ev));
	if ((m = recttomon(c->x, c->y, c->w, c->h)) != selmon) {
		if (c->tags & SPTAGMASK) {
			c->mon->tagset[c->mon->seltags] ^= (c->tags & SPTAGMASK);
			m->tagset[m->seltags] |= (c->tags & SPTAGMASK);
		}
		sendmon(c, m);
		selmon = m;
		focus(NULL);
	}
	removeflag(c, MoveResize);
}

void
restack(Monitor *m)
{
	Client *c;
	XEvent ev;
	XWindowChanges wc;
	int n;

	drawbar(m);
	if (!m->sel)
		return;
	if (ISFLOATING(m->sel) || !m->lt[m->sellt]->arrange)
		XRaiseWindow(dpy, m->sel->win);
	if (m->lt[m->sellt]->arrange) {
		wc.stack_mode = Below;
		wc.sibling = m->bar->win;
		for (c = m->stack; c; c = c->snext)
			if (!ISFLOATING(c) && ISVISIBLE(c)) {
				XConfigureWindow(dpy, c->win, CWSibling|CWStackMode, &wc);
				wc.sibling = c->win;
			}
	}
	XSync(dpy, False);
	while (XCheckMaskEvent(dpy, EnterWindowMask, &ev));

	if (enabled(Warp)) {
		for (n = 0, c = nexttiled(m->clients); c; c = nexttiled(c->next), n++);
		if (m == selmon && (m->tagset[m->seltags] & m->sel->tags) && (
			!(m->ltaxis[MASTER] == MONOCLE && (abs(m->ltaxis[LAYOUT] == NO_SPLIT || !m->nmaster || n <= m->nmaster)))
			|| ISFLOATING(m->sel))
		)
			warp(m->sel);
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
						NUMTAGS, layouts, LENGTH(layouts)) < 0) {
					fprintf(stderr, "Error handling IPC event on fd %d\n", event_fd);
				}
			} else {
				fprintf(stderr, "Got event from unknown fd %d, ptr %p, u32 %d, u64 %lu",
				event_fd, events[i].data.ptr, events[i].data.u32,
				events[i].data.u64);
				fprintf(stderr, " with events %d\n", events[i].events);
				return;
			}
		}
	}
}

void
scan(void)
{
	scanner = 1;
	char swin[256];
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
sendmon(Client *c, Monitor *m)
{
	if (c->mon == m)
		return;
	int hadfocus = (c == selmon->sel);
	unfocus(c, 1, NULL);
	detach(c);
	detachstack(c);
	arrange(c->mon);
	c->mon = m;
	c->id = 0;

	if (!(c->tags & SPTAGMASK))
		c->tags = m->tagset[m->seltags]; /* assign tags of target monitor */
	attachx(c);
	attachstack(c);
	arrange(m);

	if (hadfocus) {
		focus(c);
		restack(m);
	} else
		focus(NULL);
	if (c->reverttags)
		c->reverttags = 0;
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
	sendevent(c->win, wmatom[WMTakeFocus], NoEventMask, wmatom[WMTakeFocus], CurrentTime, 0, 0, 0);
}

void
setfullscreen(Client *c, int fullscreen, int restorefakefullscreen)
{
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
		resizeclient(c, c->mon->mx, c->mon->my, c->mon->mw, c->mon->mh);
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
			c->x = MAX(c->mon->wx, c->oldx);
			c->y = MAX(c->mon->wy, c->oldy);
			c->w = MIN(c->mon->ww - c->x + c->mon->wx - 2*c->bw, c->oldw);
			c->h = MIN(c->mon->wh - c->y + c->mon->wy - 2*c->bw, c->oldh);
			resizeclient(c, c->x, c->y, c->w, c->h);
			restack(c->mon);
		} else {
			arrangemon(c->mon);
			restack(c->mon);
		}
	}
}

void
setlayout(const Arg *arg)
{
	if (!arg || !arg->v || arg->v != selmon->lt[selmon->sellt]) {
		selmon->pertag->sellts[selmon->pertag->curtag] ^= 1;
		selmon->sellt = selmon->pertag->sellts[selmon->pertag->curtag];
	}
	if (arg && arg->v)
		selmon->pertag->ltidxs[selmon->pertag->curtag][selmon->sellt] = (Layout *)arg->v;
	selmon->lt[selmon->sellt] = selmon->pertag->ltidxs[selmon->pertag->curtag][selmon->sellt];

	if (selmon->lt[selmon->sellt]->preset.nmaster && selmon->lt[selmon->sellt]->preset.nmaster != -1)
		selmon->nmaster = selmon->lt[selmon->sellt]->preset.nmaster;
	if (selmon->lt[selmon->sellt]->preset.nstack && selmon->lt[selmon->sellt]->preset.nstack != -1)
		selmon->nstack = selmon->lt[selmon->sellt]->preset.nstack;

	selmon->ltaxis[LAYOUT] = selmon->lt[selmon->sellt]->preset.layout;
	selmon->ltaxis[MASTER] = selmon->lt[selmon->sellt]->preset.masteraxis;
	selmon->ltaxis[STACK]  = selmon->lt[selmon->sellt]->preset.stack1axis;
	selmon->ltaxis[STACK2] = selmon->lt[selmon->sellt]->preset.stack2axis;

	selmon->pertag->ltaxis[selmon->pertag->curtag][LAYOUT] = selmon->ltaxis[LAYOUT];
	selmon->pertag->ltaxis[selmon->pertag->curtag][MASTER] = selmon->ltaxis[MASTER];
	selmon->pertag->ltaxis[selmon->pertag->curtag][STACK]  = selmon->ltaxis[STACK];
	selmon->pertag->ltaxis[selmon->pertag->curtag][STACK2] = selmon->ltaxis[STACK2];

	strncpy(selmon->ltsymbol, selmon->lt[selmon->sellt]->symbol, sizeof selmon->ltsymbol);
	if (selmon->sel)
		arrangemon(selmon);
	else
		drawbar(selmon);
}

/* arg > 1.0 will set mfact absolutely */
void
setmfact(const Arg *arg)
{
	float f;

	if (!arg || !selmon->lt[selmon->sellt]->arrange)
		return;
	f = arg->f < 1.0 ? arg->f + selmon->mfact : arg->f - 1.0;
	if (f < 0.05 || f > 0.95)
		return;

	selmon->mfact = selmon->pertag->mfacts[selmon->pertag->curtag] = f;
	arrangemon(selmon);
}

void
setup(void)
{
	int i;
	XSetWindowAttributes wa;
	Atom utf8string;

	/* clean up any zombies immediately */
	sigchld(0);
	signal(SIGHUP, sighup);
	signal(SIGTERM, sigterm);

	enablefunc(functionality);

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

	updategeom();
	/* init atoms */
	utf8string = XInternAtom(dpy, "UTF8_STRING", False);
	wmatom[WMProtocols] = XInternAtom(dpy, "WM_PROTOCOLS", False);
	wmatom[WMDelete] = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
	wmatom[WMState] = XInternAtom(dpy, "WM_STATE", False);
	wmatom[WMTakeFocus] = XInternAtom(dpy, "WM_TAKE_FOCUS", False);
	wmatom[WMWindowRole] = XInternAtom(dpy, "WM_WINDOW_ROLE", False);
	wmatom[WMChangeState] = XInternAtom(dpy, "WM_CHANGE_STATE", False);
	netatom[NetActiveWindow] = XInternAtom(dpy, "_NET_ACTIVE_WINDOW", False);
	netatom[NetSupported] = XInternAtom(dpy, "_NET_SUPPORTED", False);
	netatom[NetSystemTray] = XInternAtom(dpy, "_NET_SYSTEM_TRAY_S0", False);
	netatom[NetSystemTrayOP] = XInternAtom(dpy, "_NET_SYSTEM_TRAY_OPCODE", False);
	netatom[NetSystemTrayOrientation] = XInternAtom(dpy, "_NET_SYSTEM_TRAY_ORIENTATION", False);
	netatom[NetSystemTrayOrientationHorz] = XInternAtom(dpy, "_NET_SYSTEM_TRAY_ORIENTATION_HORZ", False);
	netatom[NetSystemTrayVisual] = XInternAtom(dpy, "_NET_SYSTEM_TRAY_VISUAL", False);
	netatom[NetWMWindowTypeDock] = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_DOCK", False);
	xatom[Manager] = XInternAtom(dpy, "MANAGER", False);
	xatom[Xembed] = XInternAtom(dpy, "_XEMBED", False);
	xatom[XembedInfo] = XInternAtom(dpy, "_XEMBED_INFO", False);
	clientatom[IsFloating] = XInternAtom(dpy, "_IS_FLOATING", False);
	clientatom[DawnClientFlags] = XInternAtom(dpy, "_DAWN_CLIENT_FLAGS", False);
	clientatom[DawnMonitorTags] = XInternAtom(dpy, "_DAWN_MONITOR_TAGS", False);
	netatom[NetDesktopViewport] = XInternAtom(dpy, "_NET_DESKTOP_VIEWPORT", False);
	netatom[NetNumberOfDesktops] = XInternAtom(dpy, "_NET_NUMBER_OF_DESKTOPS", False);
	netatom[NetCurrentDesktop] = XInternAtom(dpy, "_NET_CURRENT_DESKTOP", False);
	netatom[NetDesktopNames] = XInternAtom(dpy, "_NET_DESKTOP_NAMES", False);
	netatom[NetWMActionClose] = XInternAtom(dpy, "_NET_WM_ACTION_CLOSE", False);
	netatom[NetWMFullPlacement] = XInternAtom(dpy, "_NET_WM_FULL_PLACEMENT", False); /* https://specifications.freedesktop.org/wm-spec/latest/ar01s07.html */
	netatom[NetWMName] = XInternAtom(dpy, "_NET_WM_NAME", False);
	netatom[NetWMState] = XInternAtom(dpy, "_NET_WM_STATE", False);
	netatom[NetWMMoveResize] = XInternAtom(dpy, "_NET_WM_MOVERESIZE", False);
	netatom[NetWMCheck] = XInternAtom(dpy, "_NET_SUPPORTING_WM_CHECK", False);
	netatom[NetWMFullscreen] = XInternAtom(dpy, "_NET_WM_STATE_FULLSCREEN", False);
	netatom[NetWMMaximizedVert] = XInternAtom(dpy, "_NET_WM_STATE_MAXIMIZED_VERT", False);
	netatom[NetWMMaximizedHorz] = XInternAtom(dpy, "_NET_WM_STATE_MAXIMIZED_HORZ", False);
	netatom[NetWMWindowOpacity] = XInternAtom(dpy, "_NET_WM_WINDOW_OPACITY", False);
	netatom[NetWMWindowType] = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE", False);
	netatom[NetClientList] = XInternAtom(dpy, "_NET_CLIENT_LIST", False);
	netatom[NetClientListStacking] = XInternAtom(dpy, "_NET_CLIENT_LIST_STACKING", False);
	motifatom = XInternAtom(dpy, "_MOTIF_WM_HINTS", False);
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
	scheme[LENGTH(colors)] = drw_scm_create(drw, colors[0], alphas[0], ColCount);

	for (i = 0; i < LENGTH(colors); i++)
		scheme[i] = drw_scm_create(drw, colors[i], alphas[i], ColCount);

	updatebars();
	updatestatus();

	/* supporting window for NetWMCheck */
	wmcheckwin = XCreateSimpleWindow(dpy, root, 0, 0, 1, 1, 0, 0, 0);
	XChangeProperty(dpy, wmcheckwin, netatom[NetWMCheck], XA_WINDOW, 32,
		PropModeReplace, (unsigned char *) &wmcheckwin, 1);
	XChangeProperty(dpy, wmcheckwin, netatom[NetWMName], utf8string, 8,
		PropModeReplace, (unsigned char *) "dawn", 3);
	XChangeProperty(dpy, root, netatom[NetWMCheck], XA_WINDOW, 32,
		PropModeReplace, (unsigned char *) &wmcheckwin, 1);
	/* EWMH support per view */
	XChangeProperty(dpy, root, netatom[NetSupported], XA_ATOM, 32,
		PropModeReplace, (unsigned char *) netatom, NetLast);
	setnumdesktops();
	setcurrentdesktop();
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
	grabkeys(NULL);
	focus(NULL);
	setupepoll();
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
showhide(Client *c)
{
	if (!c)
		return;
	if (ISVISIBLE(c)) {
		if (
			(c->tags & SPTAGMASK) &&
			ISFLOATING(c) &&
			(
				c->x < c->mon->mx ||
				c->x > c->mon->mx + c->mon->mw ||
				c->y < c->mon->my ||
				c->y > c->mon->my + c->mon->mh
			)
		) {
			c->x = c->mon->wx + (c->mon->ww / 2 - WIDTH(c) / 2);
			c->y = c->mon->wy + (c->mon->wh / 2 - HEIGHT(c) / 2);
		}
		/* show clients top down */
		if (!c->mon->lt[c->mon->sellt]->arrange && c->sfx != -9999 && !ISFULLSCREEN(c)) {
			XMoveWindow(dpy, c->win, c->sfx, c->sfy);
			resize(c, c->sfx, c->sfy, c->sfw, c->sfh, 0);
			showhide(c->snext);
			return;
		}
		if (NEEDRESIZE(c)) {
			removeflag(c, NeedResize);
			XMoveResizeWindow(dpy, c->win, c->x, c->y, c->w, c->h);
		} else {
			XMoveWindow(dpy, c->win, c->x, c->y);
		}
		if ((!c->mon->lt[c->mon->sellt]->arrange || ISFLOATING(c)) && !ISFULLSCREEN(c))
			resize(c, c->x, c->y, c->w, c->h, 0);
		showhide(c->snext);
	} else {
		/* hide clients bottom up */
		showhide(c->snext);
		XMoveWindow(dpy, c->win, WIDTH(c) * -2, c->y);
	}
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
spawn(const Arg *arg)
{
	if (fork() == 0) {
		if (dpy)
			close(ConnectionNumber(dpy));
		if (enabled(SpawnCwd) && selmon->sel) {
			const char* const home = getenv("HOME");
			assert(home && strchr(home, '/'));
			const size_t homelen = strlen(home);
			char *cwd, *pathbuf = NULL;
			struct stat statbuf;

			cwd = strtok(selmon->sel->name, SPAWN_CWD_DELIM);
			/* NOTE: strtok() alters selmon->sel->name in-place,
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
		setsid();
		execvp(((char **)arg->v)[0], (char **)arg->v);
		fprintf(stderr, "dawn: execvp %s", ((char **)arg->v)[0]);
		perror(" failed");
		exit(EXIT_SUCCESS);
	}
}

void
tag(const Arg *arg)
{
	if (selmon->sel && arg->ui & TAGMASK) {
		selmon->sel->tags = arg->ui & TAGMASK;

		if (selmon->sel->reverttags)
			selmon->sel->reverttags = 0;

		focus(NULL);
		arrange(selmon);
		if (enabled(ViewOnTag) && (arg->ui & TAGMASK) != selmon->tagset[selmon->seltags])
			view(arg);
	}
}

void
tagfittomon(Client *c, Monitor *m, int *cx, int *cy, int *cw, int *ch)
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
tagmon(const Arg *arg)
{
	Client *c = selmon->sel;
	if (!c || !mons->next)
		return;
	Monitor *n = dirtomon(arg->i);
	tagmonresize(c, c->mon, n);
	if (ISFULLSCREEN(c)) {
		setflag(c, FullScreen, 0);
		sendmon(c, n);
		setflag(c, FullScreen, 1);
		if (!ISFAKEFULLSCREEN(c)) {
			resizeclient(c, c->mon->mx, c->mon->my, c->mon->mw, c->mon->mh);
			XRaiseWindow(dpy, c->win);
		}
	} else {
		sendmon(c, dirtomon(arg->i));
		if (ISFLOATING(c))
			resizeclient(c, c->x, c->y, c->w, c->h);
	}
}


void
tagmonresize(Client *c, Monitor *o, Monitor *n)
{
	if (ISFLOATING(c) && (!ISFULLSCREEN(c) || ISFAKEFULLSCREEN(c)))
		tagrelposmon(c, o, n, &c->x, &c->y, &c->w, &c->h);
	else
		tagrelposmon(c, o, n, &c->oldx, &c->oldy, &c->oldw, &c->oldh);

	if (c->sfx != -9999)
		tagrelposmon(c, o, n, &c->sfx, &c->sfy, &c->sfw, &c->sfh);
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
tagrelposmon(Client *c, Monitor *o, Monitor *n, int *cx, int *cy, int *cw, int *ch)
{
	int ncw = MIN(*cw, MIN(o->ww, n->ww) - 2 * c->bw - 2 * n->gappov);
	int nch = MIN(*ch, MIN(o->wh, n->wh) - 2 * c->bw - 2 * n->gappoh);

	tagfittomon(c, o, cx, cy, cw, ch);

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

	tagfittomon(c, n, cx, cy, cw, ch);
}

void
togglebar(const Arg *arg)
{
	Bar *bar;
	selmon->showbar = !selmon->showbar;
	if (enabled(PerTagBar))
		selmon->pertag->showbars[selmon->pertag->curtag] = selmon->showbar;
	updatebarpos(selmon);
	for (bar = selmon->bar; bar; bar = bar->next)
		XMoveResizeWindow(dpy, bar->win, bar->bx, bar->by, bar->bw, bar->bh);
	arrangemon(selmon);
}

void
togglefloating(const Arg *arg)
{
	Client *c = selmon->sel;
	if (arg && arg->v)
		c = (Client*)arg->v;
	if (!c)
		return;
	if (ISFULLSCREEN(c) && !ISFAKEFULLSCREEN(c)) /* no support for fullscreen windows */
		return;
	setflag(c, Floating, !ISFLOATING(c) || ISFIXED(c));
	if (ISFLOATING(c) && !MOVERESIZE(c)) {
		if (c->sfx != -9999)
			/* restore last known float dimensions */
			resize(c, c->sfx, c->sfy, c->sfw, c->sfh, 0);
		else {
			setfloatpos(c, toggle_float_pos);
			resizeclient(c, c->x, c->y, c->w, c->h);
		}
	}
	arrangemon(c->mon);
	restack(c->mon);
	setfloatinghint(c);
}

void
togglemaximize(Client *c, int maximize_vert, int maximize_horz)
{
	if (!maximize_vert && !maximize_horz)
		return;

	if (ISFLOATING(c)) {
		if (maximize_vert && maximize_horz) {
			if (abs(c->x - c->mon->wx) <= c->mon->gappov && abs(c->y - c->mon->wy) <= c->mon->gappoh) {
				if (!WASFLOATING(c))
					togglefloating(&((Arg) { .v = c }));
				else
					resizeclient(c, c->sfx, c->sfy, c->sfw, c->sfh);
				return;
			}
		} else if (maximize_vert && abs(c->y - c->mon->wy) <= c->mon->gappoh) {
			resizeclient(c, c->x, c->sfy, c->w, c->sfh);
			return;
		} else if (maximize_horz && abs(c->x - c->mon->wx) <= c->mon->gappov) {
			resizeclient(c, c->sfx, c->y, c->sfw, c->h);
			return;
		}
		savefloats(&((Arg) { .v = c }));
	}

 	SETFLOATING(c);
 	XRaiseWindow(dpy, c->win);

	if (maximize_vert && maximize_horz)
		setfloatpos(c, "0% 0% 100% 100%");
	else if (maximize_vert)
		setfloatpos(c, "-1x 0% -1w 100%");
	else
		setfloatpos(c, "0% -1y 100% -1h");

	resizeclient(c, c->x, c->y, c->w, c->h);
}

void
toggletag(const Arg *arg)
{
	unsigned int newtags;

	if (!selmon->sel)
		return;
	newtags = selmon->sel->tags ^ (arg->ui & TAGMASK);
	if (newtags) {
		selmon->sel->tags = newtags;
		focus(NULL);
		arrange(selmon);
	}
	updatecurrentdesktop();
}

void
toggleview(const Arg *arg)
{
	unsigned int newtagset = selmon->tagset[selmon->seltags] ^ (arg->ui & TAGMASK);
	int i;

	if (enabled(TagIntoStack)) {
		Client *const selected = selmon->sel;

		// clients in the master area should be the same after we add a new tag
		Client **const masters = calloc(selmon->nmaster, sizeof(Client *));
		if (!masters) {
			die("fatal: could not calloc() %u bytes \n", selmon->nmaster * sizeof(Client *));
		}
		// collect (from last to first) references to all clients in the master area
		Client *c;
		size_t j;
		for (c = nexttiled(selmon->clients), j = 0; c && j < selmon->nmaster; c = nexttiled(c->next), ++j)
			masters[selmon->nmaster - (j + 1)] = c;
		// put the master clients at the front of the list
		// > go from the 'last' master to the 'first'
		for (j = 0; j < selmon->nmaster; ++j)
			if (masters[j])
				pop(masters[j]);
		free(masters);

		// we also want to be sure not to mutate the focus
		focus(selected);
	}

	if (newtagset) {
		selmon->tagset[selmon->seltags] = newtagset;

		if (newtagset == ~SPTAGMASK) {
			selmon->pertag->prevtag = selmon->pertag->curtag;
			selmon->pertag->curtag = 0;
		}
		/* test if the user did not select the same tag */
		if (!(newtagset & 1 << (selmon->pertag->curtag - 1))) {
			selmon->pertag->prevtag = selmon->pertag->curtag;
			for (i=0; !(newtagset & 1 << i); i++) ;
			selmon->pertag->curtag = i + 1;
		}

		/* apply settings for this view */
		selmon->nmaster = selmon->pertag->nmasters[selmon->pertag->curtag];
		selmon->mfact = selmon->pertag->mfacts[selmon->pertag->curtag];
		selmon->sellt = selmon->pertag->sellts[selmon->pertag->curtag];
		selmon->lt[selmon->sellt] = selmon->pertag->ltidxs[selmon->pertag->curtag][selmon->sellt];
		selmon->lt[selmon->sellt^1] = selmon->pertag->ltidxs[selmon->pertag->curtag][selmon->sellt^1];
		if (enabled(PerTagBar) && selmon->showbar != selmon->pertag->showbars[selmon->pertag->curtag])
			togglebar(NULL);
		focus(NULL);
		arrange(selmon);
	}
	updatecurrentdesktop();
}

void
unfocus(Client *c, int setfocus, Client *nextfocus)
{
	if (!c)
		return;
	if (ISFULLSCREEN(c) && ISVISIBLE(c) && c->mon == selmon && nextfocus && !ISFLOATING(nextfocus))
		if (!ISFAKEFULLSCREEN(c))
			setfullscreen(c, 0, 0);
	grabbuttons(c, 0);
	if (setfocus) {
		XSetInputFocus(dpy, root, RevertToPointerRoot, CurrentTime);
		XDeleteProperty(dpy, root, netatom[NetActiveWindow]);
	}
}

void
unmanage(Client *c, int destroyed)
{
	Monitor *m = c->mon;
	unsigned int reverttags = c->reverttags;
	XWindowChanges wc;

	if (c->swallowing) {
		unswallow(c);
		return;
	}

	Client *s = swallowingclient(c->win);
	if (s) {
		free(s->swallowing);
		s->swallowing = NULL;
		arrange(m);
		focus(NULL);
		return;
	}

	detach(c);
	detachstack(c);
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

	if (s)
		return;

	focus(NULL);
	updateclientlist();
	arrange(m);

	if (reverttags && ((reverttags & TAGMASK) != selmon->tagset[selmon->seltags]))
		view(&((Arg) { .ui = reverttags }));
}

void
unmapnotify(XEvent *e)
{
	/* https://tronche.com/gui/x/xlib/events/window-state-change/unmap.html
	typedef struct {
	    int type;             // UnmapNotify
	    unsigned long serial; // # of last request processed by server
	    Bool send_event;      // true if this came from a SendEvent request
	    Display *display;     // the display the event was read from
	    Window event;         // the window the notification originates from
	    Window window;        // the window the notification is for
	    Bool from_configure;  // true if the event was generated as a result of a resizing of the
	                          // window's parent when the window itself had a win_gravity of
	                          // UnmapGravity
	} XUnmapEvent; */

	Client *c;
	XUnmapEvent *ev = &e->xunmap;
	if (enabled(Debug))
		fprintf(stderr, "unmapnotify: received event type %s (%d), serial %ld, window %ld, event %ld, ev->send_event = %d, ev->from_configure = %d\n", XGetAtomName(dpy, ev->type), ev->type, ev->serial, ev->window, ev->event, ev->send_event, ev->from_configure);

	if ((c = wintoclient(ev->window))) {
		if (enabled(Debug))
			fprintf(stderr, "unmapnotify: window %ld --> client %s\n", ev->window, c->name);
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
updatebars(void)
{
	Bar *bar;
	Monitor *m;
	XSetWindowAttributes wa = {
		.override_redirect = True,
		.background_pixel = 0,
		.border_pixel = 0,
		.colormap = cmap,
		.event_mask = ButtonPressMask|ExposureMask
	};
	XClassHint ch = {"dawn", "dawn"};
	for (m = mons; m; m = m->next) {
		for (bar = m->bar; bar; bar = bar->next) {
			if (bar->external)
				continue;
			if (!bar->win) {
				bar->win = XCreateWindow(dpy, root, bar->bx, bar->by, bar->bw, bar->bh, 0, depth,
				                          InputOutput, visual,
				                          CWOverrideRedirect|CWBackPixel|CWBorderPixel|CWColormap|CWEventMask, &wa);
				XDefineCursor(dpy, bar->win, cursor[CurNormal]->cursor);
				XMapRaised(dpy, bar->win);
				XSetClassHint(dpy, bar->win, &ch);
			}
		}
	}
}

void
updatebarpos(Monitor *m)
{
	m->wx = m->mx;
	m->wy = m->my;
	m->ww = m->mw;
	m->wh = m->mh;
	Bar *bar;
	int y_pad = vertpad;
	int x_pad = sidepad;

	for (bar = m->bar; bar; bar = bar->next) {
		bar->bx = m->wx + x_pad;
		bar->bw = m->ww - 2 * x_pad;
	}

	for (bar = m->bar; bar; bar = bar->next)
		if (!m->showbar || !bar->showbar)
			bar->by = -bar->bh - y_pad;
	if (!m->showbar)
		return;
	for (bar = m->bar; bar; bar = bar->next) {
		if (!bar->showbar)
			continue;
		if (bar->topbar)
			m->wy = m->wy + bar->bh + y_pad;
		m->wh -= y_pad + bar->bh;
	}
	for (bar = m->bar; bar; bar = bar->next)
		bar->by = (bar->topbar ? m->wy - bar->bh : m->wy + m->wh);
}

void
updateclientlist()
{
	Client *c;
	Monitor *m;

	XDeleteProperty(dpy, root, netatom[NetClientList]);
	for (m = mons; m; m = m->next)
		for (c = m->clients; c; c = c->next)
			XChangeProperty(dpy, root, netatom[NetClientList],
				XA_WINDOW, 32, PropModeAppend,
				(unsigned char *) &(c->win), 1);

	XDeleteProperty(dpy, root, netatom[NetClientListStacking]);
	for (m = mons; m; m = m->next)
		for (c = m->stack; c; c = c->snext)
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
		Client *c;
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
			for (i = 0; i < (nn - n); i++) {
				for (m = mons; m && m->next; m = m->next);
				if (m)
					m->next = createmon();
				else
					mons = createmon();
			}
			for (i = 0, m = mons; i < nn && m; m = m->next, i++) {
				if (i >= n
				|| unique[i].x_org != m->mx || unique[i].y_org != m->my
				|| unique[i].width != m->mw || unique[i].height != m->mh)
				{
					dirty = 1;
					m->num = i;
					m->mx = m->wx = unique[i].x_org;
					m->my = m->wy = unique[i].y_org;
					m->mw = m->ww = unique[i].width;
					m->mh = m->wh = unique[i].height;
					updatebarpos(m);
				}
			}
		} else { /* less monitors available nn < n */
			for (i = nn; i < n; i++) {
				for (m = mons; m && m->next; m = m->next);
				while ((c = m->clients)) {
					dirty = 1;
					m->clients = c->next;
					detachstack(c);
					c->mon = mons;
					attach(c);
					attachstack(c);
				}
				if (m == selmon)
					selmon = mons;
				cleanupmon(m);
			}
		}
		for (i = 0, m = mons; m; m = m->next, i++)
			m->index = i;
		free(unique);
	} else
#endif /* XINERAMA */
	{ /* default monitor setup */
		if (!mons)
			mons = createmon();
		if (mons->mw != sw || mons->mh != sh) {
			dirty = 1;
			mons->mw = mons->ww = sw;
			mons->mh = mons->wh = sh;
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
	if (size.flags & PMinSize) {
		c->minw = size.min_width;
		c->minh = size.min_height;
	} else if (size.flags & PBaseSize) {
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
updatestatus(void)
{
	Monitor *m;
	if (!gettextprop(root, XA_WM_NAME, rawstext, sizeof(rawstext))) {
		strcpy(stext, "dawn-"VERSION);
		estext[0] = '\0';
	} else {
		char *e = strchr(rawstext, statussep);
		if (e) {
			*e = '\0'; e++;
			strncpy(rawestext, e, sizeof(estext) - 1);
			copyvalidchars(estext, rawestext);
		} else {
			estext[0] = '\0';
		}
		copyvalidchars(stext, rawstext);
	}
	for (m = mons; m; m = m->next)
		drawbar(m);
}

void
updatetitle(Client *c)
{
	char oldname[sizeof(c->name)];
	strcpy(oldname, c->name);

	if (!gettextprop(c->win, netatom[NetWMName], c->name, sizeof c->name))
		gettextprop(c->win, XA_WM_NAME, c->name, sizeof c->name);
	if (c->name[0] == '\0') /* hack to mark broken clients */
		strcpy(c->name, broken);

	for (Monitor *m = mons; m; m = m->next) {
		if (m->sel == c && strcmp(oldname, c->name) != 0)
			ipc_focused_title_change_event(m->num, c->win, oldname, c->name);
	}
}

void
updatewmhints(Client *c)
{
	XWMHints *wmh;

	if ((wmh = XGetWMHints(dpy, c->win))) {
		if (c == selmon->sel && wmh->flags & XUrgencyHint) {
			wmh->flags &= ~XUrgencyHint;
			XSetWMHints(dpy, c->win, wmh);
		} else
			setflag(c, Urgent, wmh->flags & XUrgencyHint);

		if (ISURGENT(c)) {
			if (ISFLOATING(c))
				XSetWindowBorder(dpy, c->win, scheme[SchemeUrg][ColFloat].pixel);
			else
				XSetWindowBorder(dpy, c->win, scheme[SchemeUrg][ColBorder].pixel);
		}
		setflag(c, NeverFocus, wmh->flags & InputHint ? !wmh->input : 0);
		XFree(wmh);
	}
}

void
view(const Arg *arg)
{
	if (arg->ui && (arg->ui & TAGMASK) == selmon->tagset[selmon->seltags])
	{
		view(&((Arg) { .ui = 0 }));
		return;
    }
	selmon->seltags ^= 1; /* toggle sel tagset */
	pertagview(arg);
	focus(NULL);
	arrange(selmon);
	updatecurrentdesktop();
}

Client *
wintoclient(Window w)
{
	Client *c;
	Monitor *m;

	for (m = mons; m; m = m->next)
		for (c = m->clients; c; c = c->next)
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
		return c->mon;
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
	fprintf(stderr, "dawn: fatal error: request code=%d, error code=%d\n",
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
	die("dawn: another window manager is already running");
	return -1;
}

void
zoom(const Arg *arg)
{
	Client *c = selmon->sel, *at = NULL, *cold, *cprevious = NULL, *p;;
	if (arg && arg->v)
		c = (Client*)arg->v;
	if (!c)
		return;

	if (c && ISFLOATING(c))
		togglefloating(&((Arg) { .v = c }));

	if (!c->mon->lt[c->mon->sellt]->arrange || (c && ISFLOATING(c)) || !c)
		return;

	if (c == nexttiled(c->mon->clients)) {
		p = c->mon->pertag->prevzooms[c->mon->pertag->curtag];
		at = findbefore(p);
		if (at)
			cprevious = nexttiled(at->next);
		if (!cprevious || cprevious != p) {
			c->mon->pertag->prevzooms[c->mon->pertag->curtag] = NULL;
			if (!c || !(c = nexttiled(c->next)))
				return;
		} else
			c = cprevious;
	}

	cold = nexttiled(c->mon->clients);
	if (c != cold && !at)
		at = findbefore(c);

	detach(c);
	attach(c);

	/* swap windows instead of pushing the previous one down */
	if (c != cold && at) {
		c->mon->pertag->prevzooms[c->mon->pertag->curtag] = cold;
		if (cold && at != cold) {
			detach(cold);
			cold->next = at->next;
			at->next = cold;
		}
	}

	arrangemon(c->mon);
}

int
main(int argc, char *argv[])
{
	if (argc == 2 && !strcmp("-v", argv[1]))
		die("dawn-"VERSION);
	else if (argc != 1)
		die("usage: dawn [-v]");
	if (!setlocale(LC_CTYPE, "") || !XSupportsLocale())
		fputs("warning: no locale support\n", stderr);
	if (!(dpy = XOpenDisplay(NULL)))
		die("dawn: cannot open display");

	if (!(xcon = XGetXCBConnection(dpy)))
		die("dawn: cannot get xcb connection\n");

	checkotherwm();
	XrmInitialize(); // needed for xrdb / Xresources
	if (enabled(Xresources)) {
		loadxrdb();
	}
	autostart_exec();
	setup();
#ifdef __OpenBSD__
	if (pledge("stdio rpath proc exec ps", NULL) == -1)
		die("pledge");
#endif /* __OpenBSD__ */
	scan();
	#if AUTOSTART_PATCH
	runautostart();
	#endif
	run();
	if (restart)
		execvp(argv[0], argv);
	cleanup();
	XCloseDisplay(dpy);
	return EXIT_SUCCESS;
}
