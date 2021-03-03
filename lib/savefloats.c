void savefloats(Client *c)
{
	Workspace *ws = c->ws;
	Monitor *m = ws->mon;

	/* save last known workspace float dimensions relative to monitor */
	c->sfx = m->wx + (c->x - ws->wx) * m->ww / ws->ww;
	c->sfy = m->wy + (c->y - ws->wy) * m->wh / ws->wh;
	c->sfw = c->w * m->ww / ws->ww;
	c->sfh = c->h * m->wh / ws->wh;
}

void
restorefloats(Client *c)
{
	if (!ISVISIBLE(c))
		return;
	if (c->sfx == -9999)
		savefloats(c);

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
	XMoveResizeWindow(dpy, c->win, x, y, w, h);
	resize(c, x, y, w, h, 0);
}

void
restorewsfloats(Workspace *ws)
{
	Client *c;
	/* restore last known float dimensions for all visible clients */
	for (c = ws->clients; c; c = c->next)
		restorefloats(c);
}