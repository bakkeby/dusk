void
settitle(const Arg *arg)
{
	if (!selws)
		return;

	setclienttitle(selws->sel, arg->v);
}

void
setwintitle(const Arg args[], int num_args)
{
	Window win = args[0].i;
	const char *title = args[1].v;

	setclienttitle(wintoclient(win), title);
}

void
setclienttitle(Client *c, const char *title)
{
	if (!c)
		return;

	strlcpy(c->altname, title, sizeof c->altname);
	drawbar(c->ws->mon);
}
