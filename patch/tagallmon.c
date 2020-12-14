void
tagallmon(const Arg *arg)
{
	Monitor *n;
	Client *c, *last, *slast, *next;

	if (!mons->next)
		return;

	n = dirtomon(arg->i);
	for (last = n->clients; last && last->next; last = last->next);
	for (slast = n->stack; slast && slast->snext; slast = slast->snext);

	for (c = selws->clients; c; c = next) {
		next = c->next;
		if (!ISVISIBLE(c))
			continue;
		tagmonresize(c, c->ws->mon, n);
		unfocus(c, 1, NULL);
		detach(c);
		detachstack(c);
		c->ws = n;
		c->tags = n->tagset[n->seltags]; /* assign tags of target monitor */
		c->next = NULL;
		c->snext = NULL;
		if (last)
			last = last->next = c;
		else
			n->clients = last = c;
		if (slast)
			slast = slast->snext = c;
		else
			n->stack = slast = c;
		if (ISFULLSCREEN(c)) {
			if (!ISFAKEFULLSCREEN(c)) {
				resizeclient(c, c->ws->mon->mx, c->ws->mon->my, c->ws->mon->mw, c->ws->mon->mh);
				XRaiseWindow(dpy, c->win);
			}
		}
	}

	focus(NULL);
	arrange(NULL);
}