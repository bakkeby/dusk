static void comboviewwsbyname(const Arg *arg);
static void createworkspaces();
static Workspace *createworkspace(int num);

static char * wsicon(Workspace *ws);
static int hasclients(Workspace *ws);
static void adjustwsformonitor(Workspace *ws, Monitor *m);
static Workspace * getwsbyname(const Arg *arg);
static Workspace * getwsbyindex(int index);
static unsigned int getwsmask(Monitor *m);
static void viewwsmask(Monitor *m, unsigned int wsmask);

static void enablews(const Arg *arg);
static void enablewsbyname(const Arg *arg);
static void hidews(Workspace *ws);
static void hidewsclients(Workspace *ws);
static void showws(Workspace *ws);
static void showwsclients(Workspace *ws);
static void drawws(Workspace *ws, Monitor *m, int enablews, int arrangeall, int do_warp);

static void movews(const Arg *arg);
static void movewsdir(const Arg *arg);
static void movetows(Client *c, Workspace *ws);
static void movetowsbyname(const Arg *arg);
static void moveallclientstows(Workspace *from, Workspace *to);
static void movealltowsbyname(const Arg *arg);

static void swapws(const Arg *arg);
static void swapwsclients(Workspace *ws, Workspace *ows);
static void swapwsbyname(const Arg *arg);

static void togglepinnedws(const Arg *arg);

static void viewallwsonmon(const Arg *arg);
static void viewalloccwsonmon(const Arg *arg);
static void viewselws(const Arg *arg);
static void viewws(const Arg *arg);
static void viewwsbyname(const Arg *arg);
static void viewwsdir(const Arg *arg);
static void viewwsonmon(Workspace *ws, Monitor *m, int enablews);

static void setworkspaceareas();
static void setworkspaceareasformon(Monitor *mon);
static Workspace * nextmonws(Monitor *mon, Workspace *ws);
