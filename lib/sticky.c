void
togglesticky(const Arg *arg)
{
	Workspace *ws = selws;
	if (!ws->sel)
		return;
	if (ISSTICKY(ws->sel))
		unsetsticky(ws->sel);
	else
		setsticky(ws->sel);
}

void
setsticky(Client *c)
{
	if (ISSTICKY(c))
		return;

	int wastiled = TILED(c);
	addflag(c, Sticky);

	stickyws->mon = c->ws->mon;
	detachws(stickyws);
	attachws(stickyws, c->ws);

	detach(c);
	detachstack(c);
	attachx(c, AttachBottom, stickyws);
	attachstack(c);
	stickyws->sel = c;
	restorewindowfloatposition(c, selws->mon);
	selws = stickyws;

	if (wastiled) {
		raiseclient(c);
		arrange(stickyws->next);
	} else {
		drawbar(stickyws->mon);
	}
}

void
unsetsticky(Client *c)
{
	if (!ISSTICKY(c))
		return;

	removeflag(c, Sticky);
	detach(c);
	detachstack(c);
	stickyws->sel = stickyws->stack;
	attachx(c, attachdefault, stickyws->next);
	attachstack(c);

	if (TILED(c)) {
		arrange(c->ws);
	} else {
		drawbar(c->ws->mon);
	}
	selws = c->ws;
}
