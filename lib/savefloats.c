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