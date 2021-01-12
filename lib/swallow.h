// static void copysizehints(Client *from, Client *to);
static pid_t getparentprocess(pid_t p);
static int isdescprocess(pid_t p, pid_t c);
static void replaceclient(Client *old, Client *new);
static int swallow(Client *p, Client *c);
static Client *swallowingclient(Window w);
static Client *termforwin(const Client *c);
static void unswallow(Client *c);
static pid_t winpid(Window w);