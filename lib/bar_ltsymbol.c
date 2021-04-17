int
size_ltsymbol(Bar *bar, BarArg *a)
{
	if (!bar->mon->selws)
		return 0;
	return (bar->vert ? bh : TEXTW(bar->mon->selws->ltsymbol));
}

int
draw_ltsymbol(Bar *bar, BarArg *a)
{
	if (!bar->mon->selws)
		return 0;
	return drw_text(drw, a->x, a->y, a->w, a->h, 0, bar->mon->selws->ltsymbol, 0, False);
}

int
click_ltsymbol(Bar *bar, Arg *arg, BarArg *a)
{
	return ClkLtSymbol;
}