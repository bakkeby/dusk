int
width_ltsymbol(Bar *bar, BarArg *a)
{
	if (!bar->mon->selws)
		return 0;
	return TEXTW(bar->mon->selws->ltsymbol);
}

int
draw_ltsymbol(Bar *bar, BarArg *a)
{
	if (!bar->mon->selws)
		return 0;
	return drw_text(drw, a->x, a->y, a->w, a->h, lrpad / 2, bar->mon->selws->ltsymbol, 0, False);
}

int
click_ltsymbol(Bar *bar, Arg *arg, BarArg *a)
{
	return ClkLtSymbol;
}