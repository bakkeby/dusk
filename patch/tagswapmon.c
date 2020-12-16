void
tagswapmon(const Arg *arg)
{
	Workspace *ws = WS, *nws;
	Monitor *n;
	Client *c, *sc = NULL, *mc = NULL, *next;

	if (!mons->next)
		return;

	n = dirtomon(arg->i);
	nws = MWS(n);

	for (c = ws->clients; c; c = next) {
		next = c->next;
		if (!ISVISIBLE(c))
			continue;
		unfocus(c, 1, NULL);
		detach(c);
		detachstack(c);
		c->next = sc;
		sc = c;
	}

	for (c = nws->clients; c; c = next) {
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
		c->ws = nws;
		c->tags = nws->tags; /* assign tags of target monitor */
		attach(c);
		attachstack(c);
		if (ISFULLSCREEN(c)) {
			if (!ISFAKEFULLSCREEN(c)) {
				resizeclient(c, n->mx, n->my, n->mw, n->mh);
				XRaiseWindow(dpy, c->win);
			}
		}
	}

	for (c = mc; c; c = next) {
		tagmonresize(c, n, selmon);
		next = c->next;
		c->ws = ws;
		c->tags = ws->tags; /* assign tags of target monitor */
		attach(c);
		attachstack(c);
		if (ISFULLSCREEN(c)) {
			if (!ISFAKEFULLSCREEN(c)) {
				resizeclient(c, selmon->mx, selmon->my, selmon->mw, selmon->mh);
				XRaiseWindow(dpy, c->win);
			}
		}
	}

	focus(NULL);
	arrange(NULL);
}