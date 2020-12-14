void
togglenomodbuttons(const Arg *arg)
{
	togglefunc(AllowNoModifierButtons);
	if (selws->sel)
		grabbuttons(selws->sel, 1);
}