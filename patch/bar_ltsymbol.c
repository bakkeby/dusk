int
width_ltsymbol(Bar *bar, BarArg *a)
{
	Workspace *ws = MWS(bar->mon);
	return TEXTW(ws->ltsymbol);
}

int
draw_ltsymbol(Bar *bar, BarArg *a)
{
	Workspace *ws = MWS(bar->mon);
	return drw_text(drw, a->x, a->y, a->w, a->h, lrpad / 2, ws->ltsymbol, 0, False);
}

int
click_ltsymbol(Bar *bar, Arg *arg, BarArg *a)
{
	return ClkLtSymbol;
}