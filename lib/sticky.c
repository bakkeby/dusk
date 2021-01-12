void
togglesticky(const Arg *arg)
{
	Workspace *ws = selws;
	if (!ws->sel)
		return;
	setflag(ws->sel, Sticky, !ISSTICKY(ws->sel));
	arrange(ws);
}