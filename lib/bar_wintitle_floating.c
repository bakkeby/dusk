int
size_wintitle_floating(Bar *bar, BarArg *a)
{
	if (!bar->mon->selws)
		return 0;
	return a->w;
}

int
draw_wintitle_floating(Bar *bar, BarArg *a)
{
	if (!bar->mon->selws)
		return 0;
	return calc_wintitle_floating(bar, bar->mon->selws, a->x, a->w, -1, flextitledraw, NULL, a);
}

int
click_wintitle_floating(Bar *bar, Arg *arg, BarArg *a)
{
	if (!bar->mon->selws)
		return 0;
	if (calc_wintitle_floating(bar, bar->mon->selws, 0, a->w, a->x, flextitleclick, arg, a))
		return ClkWinTitle;
	return -1;
}

int
calc_wintitle_floating(
	Bar *bar, Workspace *ws, int offx, int tabw, int passx,
	void(*tabfn)(Workspace *, Client *, int, int, int, int, Arg *arg, BarArg *barg),
	Arg *arg, BarArg *a
) {
	Client *c;
	int clientsnfloating = 0;
	int groupactive = ISFLOATING(ws->sel) ? GRP_FLOAT : 0;

	for (c = ws->clients; c; c = c->next) {
		if (!ISVISIBLE(c))
			continue;
		if (SKIPTASKBAR(c))
			continue;
		if (ISFLOATING(c))
			clientsnfloating++;
	}

	if (!clientsnfloating)
		return 0;

	/* This avoids drawing a separator on the left hand side of the wintitle section if
	 * there is a border and the wintitle module rests at the left border. */
	if (a->x > bar->borderpx) {
		offx += flexwintitle_separator;
		tabw -= flexwintitle_separator;
	}

	/* This avoids drawing a separator on the right hand side of the wintitle section if
	 * there is a border and the wintitle module rests at the right border. */
	if (a->x + a->w < bar->bw - bar->borderpx)
		tabw -= flexwintitle_separator;

	XSetForeground(drw->dpy, drw->gc, scheme[bar->scheme][ColBorder].pixel);
	XFillRectangle(drw->dpy, drw->drawable, drw->gc, a->x, a->y, a->w, a->h);

	c = flextitledrawarea(ws, ws->clients, offx, tabw, clientsnfloating, SCHEMEFOR(GRP_FLOAT), 0, 0, 1, passx, tabfn, arg, a);
	return 1;
}
