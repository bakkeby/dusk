void savefloats(const Arg *arg)
{
	Client *c = CLIENT;
	for (c = nextmarked(NULL, c); c; c = nextmarked(c->next, NULL)) {
		/* save last known float dimensions */
		c->sfx = c->x;
		c->sfy = c->y;
		c->sfw = c->w;
		c->sfh = c->h;
	}
}

void
restorefloats(Workspace *ws)
{
	Client *c;
	/* restore last known float dimensions for all visible clients */
	for (c = ws->clients; c; c = c->next)
		if (ISVISIBLE(c) && c->sfx != -9999)
			resize(c, c->sfx, c->sfy, c->sfw, c->sfh, 0);
}