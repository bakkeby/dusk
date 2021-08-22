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
	if (!c)
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
		if (ISFLOATING(last_hidden) || !last_hidden->ws->layout->arrange) {
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
