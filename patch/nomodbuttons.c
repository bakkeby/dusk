void
togglenomodbuttons(const Arg *arg)
{
	Workspace *ws = WS;
	togglefunc(AllowNoModifierButtons);
	if (ws->sel)
		grabbuttons(ws->sel, 1);
}