void
togglenomodbuttons(const Arg *arg)
{
	Workspace *ws = selws;
	togglefunc(AllowNoModifierButtons);
	if (ws->sel)
		grabbuttons(ws->sel, 1);
}