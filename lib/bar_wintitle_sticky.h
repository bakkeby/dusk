static int size_wintitle_sticky(Bar *bar, BarArg *a);
static int draw_wintitle_sticky(Bar *bar, BarArg *a);
static int click_wintitle_sticky(Bar *bar, Arg *arg, BarArg *a);
static int calc_wintitle_sticky(
	Bar *bar, Workspace *ws, int offx, int tabw, int passx,
	void(*tabfn)(Workspace *, Client *, int, int, int, int, Arg *arg, BarArg *barg),
	Arg *arg, BarArg *barg
);
