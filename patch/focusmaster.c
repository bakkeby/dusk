void
focusmaster(const Arg *arg)
{
	Client *c;

	if (selmon->nmaster < 1)
		return;

	c = nexttiled(selws->clients);

	if (c)
		focus(c);
}