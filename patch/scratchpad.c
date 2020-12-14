void
removescratch(const Arg *arg)
{
	Client *c = selws->sel;
	if (!c)
		return;
	unsigned int scratchtag = SPTAG(arg->ui);
	c->tags = c->ws->tagset[c->ws->seltags] ^ scratchtag;
	arrange(c->ws);
}

void
setscratch(const Arg *arg)
{
	Client *c = selws->sel;
	if (!c)
		return;
	unsigned int scratchtag = SPTAG(arg->ui);
	c->tags = scratchtag;
	arrange(c->ws);
}

void
togglescratch(const Arg *arg)
{
	Client *c = NULL, *next = NULL, *found = NULL;
	Monitor *mon;
	unsigned int scratchtag = SPTAG(arg->ui);
	unsigned int newtagset = 0;
	int nh = 0, nw = 0;
	Arg sparg = {.v = scratchpads[arg->ui].cmd};

	for (mon = mons; mon; mon = mon->next) {
		for (c = ws->clients; c; c = next) {
			next = c->next;
			if (!(c->tags & scratchtag))
				continue;

			found = c;

			if (HIDDEN(c)) {
				XMapWindow(dpy, c->win);
				setclientstate(c, NormalState);
				newtagset = 0;
			} else
				newtagset = selmon->tagset[selws->seltags] ^ scratchtag;

			if (c->ws != selmon) {
				if (c->ws->tagset[c->ws->seltags] & SPTAGMASK)
					c->ws->tagset[c->ws->seltags] ^= scratchtag;
				if (c->w > selmon->ww)
					nw = selmon->ww - c->bw * 2;
				if (c->h > selmon->wh)
					nh = selmon->wh - c->bw * 2;
				if (nw > 0 || nh > 0)
					resizeclient(c, c->x, c->y, nw ? nw : c->w, nh ? nh : c->h);
				sendmon(c, selmon);
			}
		}
	}

	if (found) {
		if (newtagset) {
			selmon->tagset[selws->seltags] = newtagset;
			focus(NULL);
			arrange(selws);
		}
		if (ISVISIBLE(found)) {
			focus(found);
			restack(selws);
		}
	} else {
		selmon->tagset[selws->seltags] |= scratchtag;
		spawn(&sparg);
	}
}