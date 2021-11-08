static Client * cloneclient(Client *c);
static Client * semisscratchpadforclient(Client *s);
static void removescratch(const Arg *arg);
static void setscratch(const Arg *arg);
static void swapsemiscratchpadclients(Client *o, Client *n);
static void togglescratch(const Arg *arg);