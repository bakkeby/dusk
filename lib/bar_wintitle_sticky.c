int
size_wintitle_sticky(Bar *bar, BarArg *a)
{
	Client *c;
	int i;

	for (c = stickyws->clients, i = 0; c; c = c->next, i++);
	return i * 300;
}

int
draw_wintitle_sticky(Bar *bar, BarArg *a)
{
	if (!stickyws->clients)
		return 0;
	return calc_wintitle_sticky(bar, stickyws, a->x, a->w, -1, flextitledraw, NULL, a);
}

int
click_wintitle_sticky(Bar *bar, Arg *arg, BarArg *a)
{
	if (!stickyws->clients)
		return 0;
	if (calc_wintitle_sticky(bar, stickyws, 0, a->w, a->x, flextitleclick, arg, a))
		return ClkWinTitle;
	return -1;
}

int
calc_wintitle_sticky(
	Bar *bar, Workspace *ws, int offx, int tabw, int passx,
	void(*tabfn)(Workspace *, Client *, int, int, int, int, Arg *arg, BarArg *barg),
	Arg *arg, BarArg *a
) {
	Client *c;
	int clientsnsticky = 0;
	int groupactive = GRP_FLOAT;

	for (c = ws->clients; c; c = c->next, clientsnsticky++);
	if (!clientsnsticky)
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

	c = flextitledrawarea(ws, ws->clients, offx, tabw, clientsnsticky, SCHEMEFOR(GRP_FLOAT), 0, 1, 1, passx, tabfn, arg, a);
	return 1;
}
