void
settitle(const Arg *arg)
{
	if (!selws)
		return;

	setclienttitle(selws->sel, arg->v);
}

void
setwintitle(Window win, const char *title)
{
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
