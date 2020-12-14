void
swaptags(const Arg *arg)
{
	unsigned int newtag = arg->ui & TAGMASK;
	unsigned int curtag = selmon->tagset[selws->seltags];

	if (newtag == curtag || !curtag || (curtag & (curtag-1)))
		return;

	for (Client *c = selws->clients; c != NULL; c = c->next) {
		if ((c->tags & newtag) || (c->tags & curtag))
			c->tags ^= curtag ^ newtag;

		if (!c->tags)
			c->tags = newtag;
	}

	selmon->tagset[selws->seltags] = newtag;

	focus(NULL);
	arrange(selws);
}