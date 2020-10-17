void savefloats(const Arg *arg)
{
	Client *c = selmon->sel;
	if (arg && arg->v)
		c = (Client*)arg->v;
	if (!c)
		return;
	/* save last known float dimensions */
	c->sfx = c->x;
	c->sfy = c->y;
	c->sfw = c->w;
	c->sfh = c->h;
}