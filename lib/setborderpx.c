void
setborderpx(const Arg *arg)
{
	if (!selws)
		return;

	Workspace *ws = selws;
	Monitor *m = selmon;
	Client *c;
	int prev_borderpx = m->borderpx;
	int next_borderpx = arg->i > 1000
		? arg->i - 1000
		: arg->i
		? MAX(m->borderpx + arg->i, 0)
		: m->borderpx == borderpx
		? 0
		: borderpx;
	int delta = 2 * (next_borderpx - prev_borderpx);
	m->borderpx = next_borderpx;

	for (c = ws->clients; c; c = c->next) {
		if (c->w - delta < MIN(c->minw, 1) || c->h - delta < MIN(c->minh, 1))
			continue;

		c->bw = m->borderpx;

		if (!ISVISIBLE(c))
			continue;
		if (ISTRUEFULLSCREEN(c))
			continue;
		if (ISTILED(c) && ws->layout->arrange)
			continue;

		resizeclient(c, c->x, c->y, c->w - delta, c->h - delta);
	}

	arrangews(ws);
}

void
setclientborderpx(const Arg *arg)
{
	if (!selws || !selws->sel)
		return;

	Client *c = selws->sel;
	int prev_bw = c->bw;
	int next_bw = arg->i > 1000
		? arg->i - 1000
		: arg->i
		? MAX(c->bw + arg->i, 0)
		: c->bw == borderpx
		? 0
		: borderpx;
	int delta = 2 * (next_bw - prev_bw);

	if (c->w - delta < MIN(c->minw, 1) || c->h - delta < MIN(c->minh, 1))
		return;

	c->bw = next_bw;
	resizeclient(c, c->x, c->y, c->w - delta, c->h - delta);
}
