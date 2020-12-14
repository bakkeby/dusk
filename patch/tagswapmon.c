void
tagswapmon(const Arg *arg)
{
	Monitor *n;
	Client *c, *sc = NULL, *mc = NULL, *next;

	if (!mons->next)
		return;

	n = dirtomon(arg->i);

	for (c = selws->clients; c; c = next) {
		next = c->next;
		if (!ISVISIBLE(c))
			continue;
		unfocus(c, 1, NULL);
		detach(c);
		detachstack(c);
		c->next = sc;
		sc = c;
	}

	for (c = n->ws->clients; c; c = next) {
		next = c->next;
		if (!ISVISIBLE(c))
			continue;
		unfocus(c, 1, NULL);
		detach(c);
		detachstack(c);
		c->next = mc;
		mc = c;
	}

	for (c = sc; c; c = next) {
		tagmonresize(c, selmon, n);
		next = c->next;
		c->ws = n->ws;
		c->tags = n->ws->tagset[n->ws->seltags]; /* assign tags of target monitor */
		attach(c);
		attachstack(c);
		if (ISFULLSCREEN(c)) {
			if (!ISFAKEFULLSCREEN(c)) {
				resizeclient(c, c->ws->mon->mx, c->ws->mon->my, c->ws->mon->mw, c->ws->mon->mh);
				XRaiseWindow(dpy, c->win);
			}
		}
	}

	for (c = mc; c; c = next) {
		tagmonresize(c, n, selmon);
		next = c->next;
		c->ws = selws;
		c->tags = selws->tagset[selws->seltags]; /* assign tags of target monitor */
		attach(c);
		attachstack(c);
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