void
togglenomodbuttons(const Arg *arg)
{
	togglefunc(AllowNoModifierButtons);
	if (selmon->sel)
		grabbuttons(selmon->sel, 1);
}