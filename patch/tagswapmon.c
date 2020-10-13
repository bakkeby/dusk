void
tagswapmon(const Arg *arg)
{
	Monitor *n;
	Client *c, *sc = NULL, *mc = NULL, *next;

	if (!mons->next)
		return;

	n = dirtomon(arg->i);

	for (c = selmon->clients; c; c = next) {
		next = c->next;
		if (!ISVISIBLE(c))
			continue;
		unfocus(c, 1, NULL);
		detach(c);
		detachstack(c);
		c->next = sc;
		sc = c;
	}

	for (c = n->clients; c; c = next) {
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
		c->mon = n;
		c->tags = n->tagset[n->seltags]; /* assign tags of target monitor */
		attach(c);
		attachstack(c);
		if (ISFULLSCREEN(c)) {
			if (!ISFAKEFULLSCREEN(c)) {
				resizeclient(c, c->mon->mx, c->mon->my, c->mon->mw, c->mon->mh);
				XRaiseWindow(dpy, c->win);
			}
		}
	}

	for (c = mc; c; c = next) {
		tagmonresize(c, n, selmon);
		next = c->next;
		c->mon = selmon;
		c->tags = selmon->tagset[selmon->seltags]; /* assign tags of target monitor */
		attach(c);
		attachstack(c);
		if (ISFULLSCREEN(c)) {
			if (!ISFAKEFULLSCREEN(c)) {
				resizeclient(c, c->mon->mx, c->mon->my, c->mon->mw, c->mon->mh);
				XRaiseWindow(dpy, c->win);
			}
		}
	}

	focus(NULL);
	arrange(NULL);
}