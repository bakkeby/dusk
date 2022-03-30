void
setborderpx(const Arg *arg)
{
	Workspace *ws = selws;
	Monitor *m = selmon;
	Client *c;
	int prev_borderpx = m->borderpx;
	m->borderpx = arg->i
		? MAX(m->borderpx + arg->i, 0)
		: m->borderpx == borderpx
		? 0
		: borderpx;
	int delta = 2 * (m->borderpx - prev_borderpx);

	for (c = ws->clients; c; c = c->next) {
		c->bw = m->borderpx;
		if (!ISVISIBLE(c))
			continue;
		if (ISFULLSCREEN(c) && !ISFAKEFULLSCREEN(c))
			continue;
		if (!ISFLOATING(c) && ws->layout->arrange)
			continue;

		resizeclient(c, c->x, c->y, c->w - delta, c->h - delta);
	}
	arrangews(ws);
}
