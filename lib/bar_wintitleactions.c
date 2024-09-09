void
conceal(Client *c)
{
	addflag(c, Hidden);
	hide(c);
}

void
reveal(Client *c)
{
	if (getstate(c->win) == IconicState)
		setclientstate(c, NormalState);

	removeflag(c, Hidden);

	if (!ISVISIBLE(c))
		return;

	if (ISFULLSCREEN(c))
		setfullscreen(c, 1, 0);
	else
		show(c);
}

void
focuswin(const Arg *arg)
{
	Client *c = (Client*)arg->v;
	if (!c)
		return;

	focus(c);
}

Client *
prevvisible(Client *c)
{
	Client *p, *i;
	for (p = NULL, i = c->ws->clients; c && i != c; i = i->next)
		if (ISVISIBLE(i))
			p = i;
	return p;
}

void
showhideclient(const Arg *arg)
{

	Client *c = CLIENT, *n, *last_shown = NULL, *last_hidden = NULL;
	if (!c && !num_marked)
		return;

	Workspace *ws = c->ws;
	force_warp = 1;

	for (n = nextmarked(NULL, c); n; n = nextmarked(n->next, NULL)) {
		if (HIDDEN(n)) {
			reveal(n);
			last_shown = n;
		} else {
			conceal(n);
			last_hidden = n;
		}
		if (n->ws != c->ws)
			ws = NULL;
	}

	if (last_shown) {
		focus(last_shown);
		restack(last_shown->ws);
	} else if (last_hidden) {
		if (FREEFLOW(last_hidden)) {
			for (n = last_hidden->snext; n && !ISVISIBLE(n); n = n->snext);
			if (!n)
				for (n = last_hidden->ws->stack; n && !ISVISIBLE(n); n = n->snext);
		} else {
			n = nexttiled(last_hidden);
			if (!n)
				n = prevvisible(last_hidden);
		}
		focus(n);
	}

	arrange(ws);
	force_warp = 0;
}

void
unhideall(const Arg *arg)
{
	if (num_marked)
		unmarkall(NULL);
	markall(&((Arg) { .i = MARKALL_HIDDEN }));
	if (num_marked)
		showhideclient(NULL);
}

void
unhidepop(const Arg *arg)
{
	Client *c;
	int prev_ignore_marked = ignore_marked;

	if (!selws)
		return;

	c = snexthidden(selws->stack);

	if (c) {
		ignore_marked = 1;
		showhideclient(&((Arg) { .v = c }));
		ignore_marked = prev_ignore_marked;
	}
}

/* Variant of focusstack that only focuses on hidden clients (if any) on the current workspace. */
void
focushidden(const Arg *arg)
{
	Client *c = NULL, *i;
	Workspace *ws = selws;

	if (!ws)
		return;

	if (arg->i > 0) {
		c = nexthidden(ws->sel->next);
		if (!c)
			c = nexthidden(ws->clients);
	} else {
		for (i = ws->clients; i && i != ws->sel; i = i->next)
			if (HIDDEN(i))
				c = i;
		if (!c)
			for (; i; i = i->next)
				if (HIDDEN(i))
					c = i;
	}

	if (c)
		focus(c);
}
