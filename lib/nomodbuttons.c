void
togglenomodbuttons(const Arg *arg)
{
	Workspace *ws = selws;
	togglefunc(AllowNoModifierButtons);
	if (ws->sel)
		grabbuttons(ws->sel, 1);
}

void
togglekeybindings(const Arg *arg)
{
	static int keysenabled = 1;

	XUngrabKey(dpy, AnyKey, AnyModifier, root);
	if ((keysenabled = !keysenabled)) {
		grabkeys();
	}
}
