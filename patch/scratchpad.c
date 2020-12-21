void
removescratch(const Arg *arg)
{
	Workspace *ws = WS;
	Client *c = ws->sel;
	if (!c)
		return;
	unsigned int scratchtag = SPTAG(arg->ui);
	c->tags = c->ws->tags ^ scratchtag;
	arrange(c->ws->mon);
}

void
setscratch(const Arg *arg)
{
	Workspace *ws = WS;
	Client *c = ws->sel;
	if (!c)
		return;
	unsigned int scratchtag = SPTAG(arg->ui);
	c->tags = scratchtag;
	arrange(c->ws->mon);
}

void
togglescratch(const Arg *arg)
{
	Client *c = NULL, *next = NULL, *found = NULL;
	Monitor *mon;
	Workspace *ws = WS;
	unsigned int scratchtag = SPTAG(arg->ui);
	unsigned int newtags = 0;
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
				newtags = 0;
			} else
				newtags = ws->tags ^ scratchtag;

			if (c->ws->mon != selmon) {
				if (c->ws->tags & SPTAGMASK)
					c->ws->tags ^= scratchtag;
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
		if (newtags) {
			ws->tags = newtags;
			focus(NULL);
			arrange(ws->mon);
		}
		if (ISVISIBLE(found)) {
			focus(found);
			restack(ws);
		}
	} else {
		ws->tags |= scratchtag;
		spawn(&sparg);
	}
}