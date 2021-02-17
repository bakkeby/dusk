int
width_wintitle_hidden(Bar *bar, BarArg *a)
{
	if (!bar->mon->selws)
		return 0;
	return a->w;
}

int
draw_wintitle_hidden(Bar *bar, BarArg *a)
{
	if (!bar->mon->selws)
		return 0;
	drw_rect(drw, a->x, a->y, a->w, a->h, 1, 1);
	return calc_wintitle_hidden(bar->mon->selws, a->x, a->w, -1, flextitledraw, NULL, a);
}

int
click_wintitle_hidden(Bar *bar, Arg *arg, BarArg *a)
{
	if (!bar->mon->selws)
		return 0;
	calc_wintitle_hidden(bar->mon->selws, 0, a->w, a->x, flextitleclick, arg, a);
	return ClkWinTitle;
}

int
calc_wintitle_hidden(
	Workspace *ws, int offx, int tabw, int passx,
	void(*tabfn)(Workspace *, Client *, int, int, int, int, Arg *arg, BarArg *barg),
	Arg *arg, BarArg *barg
) {
	Client *c;
	int clientsnhidden = 0, w, r;
	int groupactive = GRP_HIDDEN;

	for (c = ws->clients; c; c = c->next) {
		if (!ISINVISIBLE(c))
			continue;
		if (HIDDEN(c))
			clientsnhidden++;
	}

	if (!clientsnhidden)
		return 0;

	w = tabw / clientsnhidden;
	r = tabw % clientsnhidden;
	c = flextitledrawarea(ws, ws->clients, offx, r, w, clientsnhidden, SCHEMEFOR(GRP_HIDDEN), 0, 1, 0, passx, tabfn, arg, barg);
	return 1;
}