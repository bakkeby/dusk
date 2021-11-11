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
	clone->scratchkey = c->scratchkey;

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
	Client *s;
	if (!SEMISCRATCHPAD(c))
		return;

	/* If the semi scratchpad already has a clone, then keep that one. */
	if (c->linked && SEMISCRATCHPAD(c->linked))
		return;

	s = cloneclient(c);
	c->linked = s;
	s->win = 0;
	s->linked = c; // circular dependency, what could possibly go wrong?
	c->scratchkey = 0; // the original client is normal, not a scratchpad
	attachx(s, 0, NULL);
	attachstack(s);
}

Client *
unmanagesemiscratchpad(Client *c)
{
	if (!SEMISCRATCHPAD(c) || !c->linked)
		return c;

	Client *s, *n;

	s = (c->scratchkey ? c : c->linked); // scratch client
	n = (c->scratchkey ? c->linked : c); // normal client

	if (s->win)
		swapsemiscratchpadclients(s, n);

	s->linked = NULL;
	n->linked = NULL;

	removeflag(s, SemiScratchpad);
	unmanage(s, 1);

	return n;
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