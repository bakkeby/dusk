void
focusmaster(const Arg *arg)
{
	Client *c;
	Workspace *ws = selws;

	if (ws->nmaster < 1)
		return;

	c = nexttiled(ws->clients);

	if (c)
		focus(c);
}