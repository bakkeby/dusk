static int num_marked = 0;
static int ignore_marked = 1; /* used to avoid marked clients when key functions are used internally */

static Client *nextmarked(Client *prev, Client *def);
static void mark(const Arg *arg);
static void markall(const Arg *arg);
static void markclient(Client *c);
static void markmouse(const Arg *arg);
static void togglemark(const Arg *arg);
static void unmark(const Arg *arg);
static void unmarkall(const Arg *arg);
static void unmarkclient(Client *c);
