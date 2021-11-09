Client *
cloneclient(Client *c)
{
	Client *clone = ecalloc(1, sizeof(Client));

	strcpy(clone->name, c->name);
	clone->mina = c->mina;
	clone->maxa = c->maxa;
	clone->cfact = c->cfact;
	clone->win = c->win;
	clone->flags = c->flags;
	clone->ws = c->ws;
	clone->pid = c->pid;
	clone->opacity = c->opacity;

	clone->sfx = c->sfx;
	clone->sfy = c->sfy;
	clone->sfh = c->sfh;
	clone->sfw = c->sfw;

	clone->x = c->x;
	clone->y = c->y;
	clone->h = c->h;
	clone->w = c->w;

	clone->oldx = c->oldx;
	clone->oldy = c->oldy;
	clone->oldh = c->oldh;
	clone->oldw = c->oldw;

	clone->basew = c->basew;
	clone->baseh = c->baseh;
	clone->incw = c->incw;
	clone->inch = c->inch;
	clone->maxw = c->maxw;
	clone->maxh = c->maxh;
	clone->minw = c->minw;
	clone->minh = c->minh;

	clone->bw = c->bw;
	clone->oldbw = c->oldbw;

	return clone;
}

void
initsemiscratchpad(Client *c)
{
	if (!SEMISCRATCHPAD(c))
		return;

	removeflag(c, Terminal); // disallow semi scratchpad from swallowing clients
	c->swallowing = cloneclient(c);
	c->swallowing->scratchkey = 0; // the cloned client is normal, not a scratchpad
	attachx(c->swallowing, 0, NULL);
	attachstack(c->swallowing);
	swapsemiscratchpadclients(c, c->swallowing); // always start in "normal" mode
}

void
unmanagesemiscratchpad(Client *c)
{
	if (!SEMISCRATCHPAD(c))
		return;

	Client *s;

	if (c->swallowing) {
		s = c->swallowing;
		c->swallowing = NULL;
	} else
		s = semisscratchpadforclient(c);
	if (s) {
		s->swallowing = NULL;
		removeflag(s, SemiScratchpad);
		unmanage(s, 1);
	}
}

void
swapsemiscratchpadclients(Client *o, Client *n)
{
	if (!o->win)
		return;

	n->win = o->win;
	n->icon = o->icon;
	n->icw = o->icw;
	n->ich = o->ich;
	o->win = o->icon = o->icw = o->ich = 0;

	strcpy(n->name, o->name);

	if (ISFLOATING(n)) {
		addflag(n, NeedResize);
		showwsclient(n);
	} else if (ISVISIBLE(n))
		XMoveResizeWindow(dpy, n->win, n->x, n->y, n->w, n->h);
	else
		XMoveResizeWindow(dpy, n->win, WIDTH(n) * -2, n->y, n->w, n->h);
}

Client *
semisscratchpadforclient(Client *s)
{
	Workspace *ws;
	Client *c;

	for (ws = workspaces; ws; ws = ws->next)
		for (c = ws->clients; c; c = c->next)
			if (c->swallowing && c->swallowing == s)
				return c;

	return NULL;
}