void
tagallmon(const Arg *arg)
{
	Workspace *ws = WS, *nws;
	Monitor *n;
	Client *c, *last, *slast, *next;

	if (!mons->next)
		return;

	n = dirtomon(arg->i);
	nws = MWS(n);
	for (last = nws->clients; last && last->next; last = last->next);
	for (slast = nws->stack; slast && slast->snext; slast = slast->snext);

	for (c = ws->clients; c; c = next) {
		next = c->next;
		if (!ISVISIBLE(c))
			continue;
		tagmonresize(c, c->ws->mon, n);
		unfocus(c, 1, NULL);
		detach(c);
		detachstack(c);
		c->ws = nws;
		c->tags = nws->tags; /* assign tags of target monitor */
		c->next = NULL;
		c->snext = NULL;
		if (last)
			last = last->next = c;
		else
			nws->clients = last = c;
		if (slast)
			slast = slast->snext = c;
		else
			nws->stack = slast = c;
		if (ISFULLSCREEN(c)) {
			if (!ISFAKEFULLSCREEN(c)) {
				resizeclient(c, n->mx, n->my, n->mw, n->mh);
				XRaiseWindow(dpy, c->win);
			}
		}
	}

	focus(NULL);
	arrange(NULL);
}