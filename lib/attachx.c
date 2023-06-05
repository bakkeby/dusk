void
attachx(Client *c, uint64_t mode, Workspace *ws)
{
	if (!c)
		return;

	Client *i;
	uint64_t attachmode
		= mode
		? mode
		: c->flags & AttachFlag
		? c->flags & AttachFlag
		: attachdefault;

	if (!ws)
		ws = c->ws;
	else if (c->ws != ws) {
		for (i = c; i; i = i->next) {
			i->ws = ws;
			i->revertws = NULL;
			updateclientdesktop(i);
		}
	}

	if (c->idx > 0) { /* then the client has a designated position in the client list */
		for (i = ws->clients; i; i = i->next) {
			if (c->idx < i->idx) {
				attachabove(c, i);
				return;
			} else if (i->idx <= c->idx && (!i->next || c->idx <= i->next->idx)) {
				attachbelow(c, i);
				return;
			}
		}
	}

	if (attachmode == AttachAbove) {
		attachabove(c, ws->sel);
	} else if (attachmode == AttachAside) {
		attachaside(c);
	} else if (attachmode == AttachBelow) {
		attachbelow(c, ws->sel);
	} else if (attachmode == AttachBottom) {
		attachbottom(c);
	} else {
		/* Attach master (default) */
		attachabove(c, ws->clients);
	}
}

void
attachabove(Client *c, Client *target)
{
	Client **tp;
	Client *last;

	if (target) {
		last = lastclient(c);
		last->next = target;
		tp = clientptr(target);
		*tp = c;
		return;
	}

	attach(c);
}

void
attachaside(Client *c)
{
	Client *target = nthmaster(c->ws->clients, c->ws->nmaster, 1);

	if (target) {
		attachbelow(c, target);
		return;
	}

	attach(c);
}

void
attachbelow(Client *c, Client *target)
{
	Client *last;

	if (target) {
		last = lastclient(c);
		last->next = target->next;
		target->next = c;
		return;
	}

	attach(c);
}

void
attachbottom(Client *c)
{
	attachbelow(c, lastclient(c->ws->clients));
}

void
attachstackx(Client *c, uint64_t mode, Workspace *ws)
{
	if (!c)
		return;

	Client *at, *last;
	unsigned int n;
	uint64_t attachmode
		= mode
		? mode
		: c->flags & AttachFlag
		? c->flags & AttachFlag
		: attachdefault;

	if (!ws)
		ws = c->ws;

	for (last = c; last && last->snext; last = last->snext);

	if (attachmode == AttachAbove) {
		if (!(ws->sel == NULL || ws->sel == ws->stack || ISFLOATING(ws->sel))) {
			for (at = ws->stack; at->snext != ws->sel; at = at->snext);
			last->snext = at->snext;
			at->snext = c;
			return;
		}
	} else if (attachmode == AttachAside) {
		for (at = ws->stack, n = 0; at; at = at->snext)
			if (!ISFLOATING(at))
				if (++n >= ws->nmaster)
					break;

		if (at && ws->nmaster) {
			last->snext = at->snext;
			at->snext = c;
			return;
		}
	} else if (attachmode == AttachBelow) {
		if (!(ws->sel == NULL || ws->sel == c || ISFLOATING(ws->sel))) {
			last->snext = ws->sel->snext;
			ws->sel->snext = c;
			return;
		}
	} else if (attachmode == AttachBottom) {
		for (at = ws->stack; at && at->snext; at = at->snext);
		if (at) {
			at->snext = c;
			last->snext = NULL;
			return;
		}
	}

	/* Attach master (default) */
	last->snext = ws->stack;
	ws->stack = c;
}

void
setattachdefault(const Arg *arg)
{
	attachdefault = getflagbyname(arg->v) & AttachFlag;
}
