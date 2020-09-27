void
togglesticky(const Arg *arg)
{
	if (!selmon->sel)
		return;
	setflag(selmon->sel, Sticky, !ISSTICKY(selmon->sel));
	arrange(selmon);
}