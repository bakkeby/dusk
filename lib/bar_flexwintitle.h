static int width_flexwintitle(Bar *bar, BarArg *a);
static int draw_flexwintitle(Bar *bar, BarArg *a);
static int click_flexwintitle(Bar *bar, Arg *arg, BarArg *a);

static void flextitledraw(Workspace *ws, Client *c, int unused, int x, int w, int groupactive, Arg *arg, BarArg *barg);
static void flextitleclick(Workspace *ws, Client *c, int passx, int x, int w, int unused, Arg *arg, BarArg *barg);
static int flextitlecalculate(Bar *bar, int offx, int w, int passx, void(*tabfn)(Workspace *, Client *, int, int, int, int, Arg *arg, BarArg *barg), Arg *arg, BarArg *barg);
static int getschemefor(Workspace *ws, int group, int activegroup);
static int getselschemefor(int scheme);
static void getclientcounts(Workspace *ws, int *groupactive, int *n, int *clientsnmaster, int *clientsnstack, int *clientsnstack2, int *clientsnfloating, int *clientsnhidden);
static int isdualstacklayout(Workspace *ws);
static int iscenteredlayout(Workspace *ws, int n);
static int isfixedlayout(Workspace *ws);
static int ismirroredlayout(Workspace *ws);

static Client *flextitledrawarea(Workspace *ws, Client *c, int x, int r, int w, int max_clients, int tabscheme, int draw_tiled, int draw_hidden, int draw_floating, int passx, void(*tabfn)(Workspace *, Client *, int, int, int, int, Arg *arg, BarArg *barg), Arg *arg, BarArg *barg);