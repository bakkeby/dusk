void
togglesticky(const Arg *arg)
{
	if (!selws->sel)
		return;
	setflag(selws->sel, Sticky, !ISSTICKY(selws->sel));
	arrange(selws);
}