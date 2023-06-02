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
	addflag(c, Sticky);

	stickyws->next = c->ws;
	stickyws->mon = c->ws->mon;
	detach(c);
	detachstack(c);
	attachx(c, AttachBottom, stickyws);
	attachstack(c);
	stickyws->sel = c;
	restorewindowfloatposition(c, selws->mon);
	arrange(stickyws->next);
	selws = stickyws;
}

void
unsetsticky(Client *c)
{
	removeflag(c, Sticky);
	detach(c);
	detachstack(c);
	stickyws->sel = stickyws->stack;
	attachx(c, attachdefault, stickyws->next);
	attachstack(c);
	arrange(c->ws);
	selws = c->ws;
}
