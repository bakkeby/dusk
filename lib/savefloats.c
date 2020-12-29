void savefloats(const Arg *arg)
{
	Workspace *ws = WS;
	Client *c = ws->sel;
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