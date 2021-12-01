int size_flexwintitle(Bar *bar, BarArg *a);
int draw_flexwintitle(Bar *bar, BarArg *a);
int click_flexwintitle(Bar *bar, Arg *arg, BarArg *a);

void flextitledraw(Workspace *ws, Client *c, int unused, int x, int w, int groupactive, Arg *arg, BarArg *barg);
void flextitledrawarea(Workspace *ws, Client *c, int x, int w, int num_clients, int tabscheme, int draw_tiled, int draw_hidden, int draw_floating, int passx, void(*tabfn)(Workspace *, Client *, int, int, int, int, Arg *arg, BarArg *barg), Arg *arg, BarArg *barg);
void flextitleclick(Workspace *ws, Client *c, int passx, int x, int w, int unused, Arg *arg, BarArg *barg);
int flextitlecalculate(Bar *bar, int offx, int w, int passx, void(*tabfn)(Workspace *, Client *, int, int, int, int, Arg *arg, BarArg *barg), Arg *arg, BarArg *barg);
int getschemefor(Workspace *ws, int group, int activegroup);
int getselschemefor(int scheme);
void getclientcounts(Workspace *ws, int *groupactive, int *n, int *clientsnmaster, int *clientsnstack, int *clientsnstack2, int *clientsnfloating, int *clientsnhidden);
int isdualstacklayout(Workspace *ws);
int iscenteredlayout(Workspace *ws, int n);
int ismirroredlayout(Workspace *ws);
