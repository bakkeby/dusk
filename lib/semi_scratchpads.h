static Client * cloneclient(Client *c);
static Client * semisscratchpadforclient(Client *s);
static void initsemiscratchpad(Client *c);
static void unmanagesemiscratchpad(Client *c);
static void swapsemiscratchpadclients(Client *o, Client *n);