void
tagswapmon(const Arg *arg)
{
	Monitor *m;
	Client *c, *sc = NULL, *mc = NULL, *next;

	if (!mons->next)
		return;

	m = dirtomon(arg->i);

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

	for (c = m->clients; c; c = next) {
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
		next = c->next;
		c->mon = m;
		c->tags = m->tagset[m->seltags]; /* assign tags of target monitor */
		attach(c);
		attachstack(c);
		if (ISFULLSCREEN(c)) {
			if (c->fakefullscreen != 1) {
				resizeclient(c, c->mon->mx, c->mon->my, c->mon->mw, c->mon->mh);
				XRaiseWindow(dpy, c->win);
			}
		}
	}

	for (c = mc; c; c = next) {
		next = c->next;
		c->mon = selmon;
		c->tags = selmon->tagset[selmon->seltags]; /* assign tags of target monitor */
		attach(c);
		attachstack(c);
		if (ISFULLSCREEN(c)) {
			if (c->fakefullscreen != 1) {
				resizeclient(c, c->mon->mx, c->mon->my, c->mon->mw, c->mon->mh);
				XRaiseWindow(dpy, c->win);
			}
		}
	}

	focus(NULL);
	arrange(NULL);
}