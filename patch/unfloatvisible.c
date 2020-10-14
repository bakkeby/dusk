void
unfloatvisible(const Arg *arg)
{
	Client *c;

	for (c = selmon->clients; c; c = c->next)
		if (ISVISIBLE(c) && ISFLOATING(c))
			setflag(c, Floating, ISFIXED(c));

	arrange(selmon);
}