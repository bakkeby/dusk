void
focusurgent(const Arg *arg)
{
	Workspace *ws;
	Client *c = NULL;
	for (ws = workspaces; ws && !c; ws = ws->next)
		for (c = ws->clients; c && !ISURGENT(c); c = c->next);
	if (c) {
		if (!c->ws->visible)
			viewwsonmon(c->ws, c->ws->mon, 0);
		focus(c);
	}
}
