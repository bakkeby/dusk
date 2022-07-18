void savefloats(Client *c)
{
	if (ISSTICKY(c))
		return;

	int x, y, w, h;
	Workspace *ws = c->ws;
	Monitor *m = ws->mon;

	/* Save last known workspace float dimensions relative to monitor */
	x = m->wx + (c->x - ws->wx) * m->ww / ws->ww;
	y = m->wy + (c->y - ws->wy) * m->wh / ws->wh;
	w = c->w * m->ww / ws->ww;
	h = c->h * m->wh / ws->wh;

	/* If the client is outside of / larger than the workspace then use absolute monitor position */
	if (x < 0 || y < 0 || w > ws->ww || h > ws->wh) {
		x = m->wx + c->x - ws->wx;
		y = m->wy + c->y - ws->wy;
		w = c->w;
		h = c->h;
	}

	c->sfx = x;
	c->sfy = y;
	c->sfw = w;
	c->sfh = h;
}

void
restorefloats(Client *c)
{
	if (!ISVISIBLE(c) || ISSTICKY(c))
		return;
	if (c->sfx == -9999)
		savefloats(c);

	c->area = -99;
	Workspace *ws = c->ws;
	Monitor *m = ws->mon;

	int x = ws->wx + (c->sfx - m->wx) * ws->ww / m->ww;
	int y = ws->wy + (c->sfy - m->wy) * ws->wh / m->wh;
	int w = c->sfw * ws->ww / m->ww;
	int h = c->sfh * ws->wh / m->wh;

	if (w <= 0 || h <= 0) {
		fprintf(stderr, "restorefloats: bad float values x = %d, y = %d, w = %d, h = %d for client = %s\n", x, y, w, h, c->name);
		return;
	}

	resize(c, x, y, w, h, 0);
}

void
restorewsfloats(Workspace *ws)
{
	Client *c;
	/* restore last known float dimensions for all visible clients */
	for (c = ws->stack; c; c = c->snext)
		restorefloats(c);
}
