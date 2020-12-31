static char * wsicon(Workspace *ws);
static void hidews(Workspace *ws);
static void hidewsclients(Workspace *ws);
static void showws(Workspace *ws);
static void showwsclients(Workspace *ws);

static void movews(const Arg *arg);
static void movetows(Client *c, Workspace *ws);
static void movetowsbyname(const Arg *arg);

static void viewws(const Arg *arg);
static void viewwsbyname(const Arg *arg);