enum {
	BAR_ALIGN_TOP,
	BAR_ALIGN_LEFT,
	BAR_ALIGN_CENTER,
	BAR_ALIGN_BOTTOM,
	BAR_ALIGN_RIGHT,
	BAR_ALIGN_TOP_TOP,
	BAR_ALIGN_TOP_CENTER,
	BAR_ALIGN_TOP_BOTTOM,
	BAR_ALIGN_LEFT_LEFT,
	BAR_ALIGN_LEFT_RIGHT,
	BAR_ALIGN_LEFT_CENTER,
	BAR_ALIGN_NONE,
	BAR_ALIGN_RIGHT_LEFT,
	BAR_ALIGN_RIGHT_RIGHT,
	BAR_ALIGN_RIGHT_CENTER,
	BAR_ALIGN_BOTTOM_BOTTOM,
	BAR_ALIGN_BOTTOM_CENTER,
	BAR_ALIGN_BOTTOM_TOP,
	BAR_ALIGN_LAST
}; /* bar alignment */

typedef struct BarDef BarDef;
struct BarDef {
	int monitor;
	int idx;
	int vert;
	char *barpos;
	char *name;
	char *extclass;
	char *extinstance;
	char *extname;
};

struct Bar {
	Window win;
	Monitor *mon;
	Bar *next;
	const BarDef *def;
	const char *barpos;
	const char *name;
	int vert;
	int idx;
	int showbar;
	int external;
	int borderpx;
	int scheme;
	int groupactive;
	int bx, by, bw, bh; /* bar geometry */
	int *s; // width/height, array length == barrules, then use r index for lookup purposes
	int *p; // x/y position, array length == ^
	int *sscheme; // start scheme
	int *escheme; // end scheme
};

typedef struct {
	int x;
	int y;
	int h;
	int w;
	int lpad;
	int rpad;
	int value;
	int scheme;
	int firstscheme;
	int lastscheme;
} BarArg;

typedef struct {
	int monitor;
	int bar;
	int scheme;
	int lpad;
	int rpad;
	int value;
	int alignment; // see bar alignment enum
	int (*sizefunc)(Bar *bar, BarArg *a);
	int (*drawfunc)(Bar *bar, BarArg *a);
	int (*clickfunc)(Bar *bar, Arg *arg, BarArg *a);
	int (*hoverfunc)(Bar *bar, BarArg *a, XMotionEvent *ev);
	char *name; // for debugging
} BarRule;

static int bh; /* bar geometry */
int num_bars, num_barrules;

static void barhover(XEvent *e, Bar *bar);
static void barpress(XButtonPressedEvent *ev, Monitor *m, Arg *arg, int *click);
static void createbars(Monitor *m);
static void createbar(const BarDef *def, Monitor *m);
static void drawbar(Monitor *m);
static void drawbars(void);
static void drawbarwin(Bar *bar);
static void drawbarmodule(const BarRule *br, int r);
static void updatebarpos(Monitor *m);
static void updatebars(void);
static void recreatebar(Bar *bar);
static void reducewindowarea(Monitor *m);
static void removebar(Bar *bar);
static void setbarpos(Bar *bar);
static void getbarsize(Bar *bar, int *w, int *h);
static void showbar(const Arg *arg);
static void hidebar(const Arg *arg);
static void showhidebar(Bar *bar);
static void teardownbars(Monitor *m);
static void togglebar(const Arg *arg);
static void togglebarpadding(const Arg *arg);
static void togglecompact(const Arg *arg);
static Bar * wintobar(Window win);
static Bar * mapexternalbar(Window win);
static int matchextbar(Bar *bar, Window win);
