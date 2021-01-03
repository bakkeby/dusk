void
swaptags(const Arg *arg)
{
	Workspace *ws = WS;
	unsigned int newtag = arg->ui & TAGMASK;
	unsigned int curtag = ws->tags;

	if (newtag == curtag || !curtag || (curtag & (curtag-1)))
		return;

	for (Client *c = ws->clients; c != NULL; c = c->next) {
		if ((c->tags & newtag) || (c->tags & curtag))
			c->tags ^= curtag ^ newtag;

		if (!c->tags)
			c->tags = newtag;
	}

	ws->tags = newtag;

	focus(NULL);
	arrange(ws);
}