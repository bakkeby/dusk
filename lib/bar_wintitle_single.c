int
size_wintitle_single(Bar *bar, BarArg *a)
{
	if (!bar->mon->selws->visible || !bar->mon->selws->sel)
		return 0;
	return a->w;
}

int
draw_wintitle_single(Bar *bar, BarArg *a)
{
	Workspace *ws = bar->mon->selws;
	if (!ws->visible || !ws->sel)
		return 0;

	flextitledraw(ws, ws->sel, 0, a->x, a->w, a->scheme, NULL, a);
	return 1;
}

int
click_wintitle_single(Bar *bar, Arg *arg, BarArg *a)
{
	Workspace *ws = bar->mon->selws;
	if (!ws->visible)
		return 0;
	arg->v = ws->sel;
	return ClkWinTitle;
}
