void
unfloatvisible(const Arg *arg)
{
	Workspace *ws = selws;
	Client *c;

	for (c = ws->clients; c; c = c->next)
		if (ISVISIBLE(c) && FLOATING(c))
			setflag(c, Floating, ISFIXED(c));

	arrange(ws);
}
