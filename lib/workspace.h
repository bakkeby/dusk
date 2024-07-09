enum {
	NORMAL,
	VISIBLE,
	SELECTED,
	OCCUPIED,
};

static void attachws(Workspace *ws, Workspace *target);
static void detachws(Workspace *ws);

static void comboviewwsbyindex(const Arg *arg);
static void comboviewwsbyname(const Arg *arg);
static void createworkspaces(void);
static Workspace *createworkspace(int num, const WorkspaceRule *r);

static char * wsicon(Workspace *ws);
static int hasclients(Workspace *ws);
static int hashidden(Workspace *ws);
static int hasfloating(Workspace *ws);
static int hasfullscreen(Workspace *ws);
static void handleabandoned(Workspace *ws);
static int noborder(Client *c);
static void adjustwsformonitor(Workspace *ws, Monitor *m);
static Workspace * getwsbyindex(const Arg *arg);
static Workspace * getwsbyname(const Arg *arg);
static Workspace * getwsbynum(int num);
static uint64_t getwsmask(Monitor *m);
static uint64_t getallwsmask(Monitor *m);
static void viewwsmask(Monitor *m, uint64_t wsmask);
static void storewsmask(void);
static void togglewsmask(void);

static void enablews(const Arg *arg);
static void enablewsbyindex(const Arg *arg);
static void enablewsbyname(const Arg *arg);
static void hidews(Workspace *ws);
static void hidewsclients(Client *c);
static void showws(Workspace *ws);
static void showwsclient(Client *c);
static void showwsclients(Client *c);
static void drawws(Workspace *ws, Monitor *m, uint64_t prevwsmask, int enablews, int arrangeall, int do_warp);

static void movews(const Arg *arg);
static void movewsdir(const Arg *arg);
static void movetows(Client *c, Workspace *ws, int view_workspace);
static void movetowsbyindex(const Arg *arg);
static void movetowsbyname(const Arg *arg);
static unsigned int numtiled(Workspace *ws);
static void sendtowsbyindex(const Arg *arg);
static void sendtowsbyname(const Arg *arg);
static void moveallclientstows(Workspace *from, Workspace *to, int view_workspace);
static void moveallfromwsbyindex(const Arg *arg);
static void moveallfromwsbyname(const Arg *arg);
static void movealltowsbyindex(const Arg *arg);
static void movealltowsbyname(const Arg *arg);

static void swapws(const Arg *arg);
static void swapwsclients(Workspace *ws, Workspace *ows);
static void swapwsbyindex(const Arg *arg);
static void swapwsbyname(const Arg *arg);

static void togglepinnedws(const Arg *arg);
static void togglews(const Arg *arg);

static void viewallwsonmon(const Arg *arg);
static void viewalloccwsonmon(const Arg *arg);
static void viewselws(const Arg *arg);
static void viewws(const Arg *arg);
static void viewwsbyindex(const Arg *arg);
static void viewwsbyname(const Arg *arg);
static void viewwsdir(const Arg *arg);
static void viewwsonmon(Workspace *ws, Monitor *m, int enablews);

static void assignworkspacetomonitor(Workspace *ws, Monitor *m);
static void redistributeworkspaces(void);
static void reorientworkspaces(void);
static void reorientworkspace(Workspace *ws, int orientation);
static void reviewworkspaces(void);
static void setwfact(const Arg *arg);
static void setworkspaceareas(void);
static void setworkspaceareasformon(Monitor *mon);
static Workspace * nextmonws(Monitor *mon, Workspace *ws);
static Workspace * nextoccmonws(Monitor *mon, Workspace *ws);
static Workspace * nextvismonws(Monitor *mon, Workspace *ws);
static Workspace * selectmonws(Monitor *mon);
