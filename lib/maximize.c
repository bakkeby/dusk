void
maximize(const Arg *arg)
{
	Client *c = CLIENT;

	if (!c)
		return;

	togglemaximize(c, 1, 1);
}

void
maximizevert(const Arg *arg)
{
	Client *c = CLIENT;

	if (!c)
		return;

	togglemaximize(c, 1, 0);
}

void
maximizehorz(const Arg *arg)
{
	Client *c = CLIENT;

	if (!c)
		return;

	togglemaximize(c, 0, 1);
}

void
maximize_client(Client *c, int maximize_vert, int maximize_horz)
{
	if (!maximize_vert && !maximize_horz)
		return;
	Workspace *ws = c->ws;

	SETFLOATING(c);
	XRaiseWindow(dpy, c->win);

	if (maximize_vert && maximize_horz)
		setfloatpos(c, "0% 0% 100% 100%", 1, 0);
	else if (maximize_vert)
		setfloatpos(c, "-1x 0% -1w 100%", 1, 0);
	else
		setfloatpos(c, "0% -1y 100% -1h", 1, 0);

	if (noborder(c, 0, 0, 0, 0)) {
		addflag(c, NoBorder);
	}

	resizeclient(c, c->x, c->y, c->w, c->h);
	drawbar(ws->mon);
}

void
togglemaximize(Client *c, int maximize_vert, int maximize_horz)
{
	if (!maximize_vert && !maximize_horz)
		return;

	Workspace *ws = c->ws;
	Monitor *m = ws->mon;
	int is_maximized_vert = 0, is_maximized_horz = 0;

	if (ISFLOATING(c)) {
		is_maximized_vert = abs(c->y - m->wy) <= m->gappoh;
		is_maximized_horz = abs(c->x - m->wx) <= m->gappov;

		if (
			(maximize_vert && maximize_horz && is_maximized_vert && is_maximized_horz) ||
			(!maximize_horz && is_maximized_vert) ||
			(!maximize_vert && is_maximized_horz)
		) {
			if (!WASFLOATING(c))
				togglefloating(&((Arg) { .v = c }));
			else
				restorefloats(c);
			return;
		}
		savefloats(c);
	}

	maximize_client(c, maximize_vert, maximize_horz);
}
