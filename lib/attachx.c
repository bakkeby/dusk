void
attachx(Client *c, unsigned long mode, Workspace *ws)
{
	if (!c)
		return;

	Client *at, *last;
	unsigned int n;
	unsigned long attachmode
		= mode
		? mode
		: c->flags & AttachMaster
		? AttachMaster
		: c->flags & AttachAbove
		? AttachAbove
		: c->flags & AttachAside
		? AttachAside
		: c->flags & AttachBelow
		? AttachBelow
		: c->flags & AttachBottom
		? AttachBottom
		: attachdefault;

	for (last = c; last && last->next; last = last->next);

	if (!ws)
		ws = c->ws;
	else if (c->ws != ws) {
		for (at = c; at; at = at->next) {
			at->ws = ws;
			at->revertws = NULL;
			updateclientdesktop(at);
		}
	}

	if (c->id > 0) { /* then the client has a designated position in the client list */
		for (at = ws->clients; at; at = at->next)
			if (c->id < at->id) {
				last->next = at;
				ws->clients = c;
				return;
			} else if (at->id <= c->id && (!at->next || c->id <= at->next->id)) {
				last->next = at->next;
				at->next = c;
				return;
			}
	}

	if (attachmode == AttachAbove) {
		if (!(ws->sel == NULL || ws->sel == ws->clients || ISFLOATING(ws->sel))) {
			for (at = ws->clients; at->next != ws->sel; at = at->next);
			last->next = at->next;
			at->next = c;
			return;
		}
	} else if (attachmode == AttachAside) {
		for (at = ws->clients, n = 0; at; at = at->next)
			if (!ISFLOATING(at))
				if (++n >= ws->nmaster)
					break;

		if (at && ws->nmaster) {
			last->next = at->next;
			at->next = c;
			return;
		}
	} else if (attachmode == AttachBelow) {
		if (!(ws->sel == NULL || ws->sel == c || ISFLOATING(ws->sel))) {
			last->next = ws->sel->next;
			ws->sel->next = c;
			return;
		}
	} else if (attachmode == AttachBottom) {
		for (at = ws->clients; at && at->next; at = at->next);
		if (at) {
			at->next = c;
			last->next = NULL;
			return;
		}
	}

	/* Attach master (default) */
	last->next = ws->clients;
	ws->clients = c;
}

void
attachstackx(Client *c, unsigned long mode, Workspace *ws)
{
	if (!c)
		return;

	Client *at, *last;
	unsigned int n;
	unsigned long attachmode
		= mode
		? mode
		: c->flags & AttachMaster
		? AttachMaster
		: c->flags & AttachAbove
		? AttachAbove
		: c->flags & AttachAside
		? AttachAside
		: c->flags & AttachBelow
		? AttachBelow
		: c->flags & AttachBottom
		? AttachBottom
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