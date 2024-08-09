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

	if (ISFLOATING(c)) {
		if (maximize_vert && maximize_horz) {
			if (abs(c->x - m->wx) <= m->gappov && abs(c->y - m->wy) <= m->gappoh) {
				if (!WASFLOATING(c))
					togglefloating(&((Arg) { .v = c }));
				else
					restorefloats(c);
				return;
			}
		} else if (maximize_vert && abs(c->y - m->wy) <= m->gappoh) {
			resizeclient(c,
				c->x,
				ws->wy + (c->sfy - m->wy) * ws->wh / m->wh,
				c->w,
				c->sfh * ws->wh / m->wh
			);
			return;
		} else if (maximize_horz && abs(c->x - m->wx) <= m->gappov) {
			resizeclient(
				c,
				ws->wx + (c->sfx - m->wx) * ws->ww / m->ww,
				c->y,
				c->sfw * ws->ww / m->ww,
				c->h
			);
			return;
		}
		savefloats(c);
	}

	maximize_client(c, maximize_vert, maximize_horz);
}
