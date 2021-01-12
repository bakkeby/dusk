void
focusurgent(const Arg *arg)
{
	Workspace *ws = selws;
	Client *c;
	for (c = ws->clients; c && !ISURGENT(c); c = c->next);
	if (c) {
		if (!c->ws->visible)
			viewwsonmon(c->ws, c->ws->mon);
		focus(c);
	}
}