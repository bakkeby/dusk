int
size_ltsymbol(Bar *bar, BarArg *a)
{
	if (!bar->mon->selws)
		return 0;
	return (bar->vert ? bh : TEXT2DW(bar->mon->selws->ltsymbol));
}

int
draw_ltsymbol(Bar *bar, BarArg *a)
{
	if (!bar->mon->selws)
		return 0;
	return drw_2dtext(drw, a->x, a->y, a->w, a->h, 0, bar->mon->selws->ltsymbol, 0, 1, a->scheme);
}

int
click_ltsymbol(Bar *bar, Arg *arg, BarArg *a)
{
	return ClkLtSymbol;
}
