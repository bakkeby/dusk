void
unfloatvisible(const Arg *arg)
{
	Workspace *ws = WS;
	Client *c;

	for (c = ws->clients; c; c = c->next)
		if (ISVISIBLE(c) && ISFLOATING(c))
			setflag(c, Floating, ISFIXED(c));

	arrangemon(ws->mon);
}