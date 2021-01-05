void
attachx(Client *c)
{
	Client *at;
	unsigned int n;
	unsigned long attachmode
		= c->flags & AttachMaster
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

	if (c->id > 0) { /* then the client has a designated position in the client list */
		for (at = c->ws->clients; at; at = at->next)
			if (c->id < at->id) {
				c->next = at;
				c->ws->clients = c;
				return;
			} else if (at->id <= c->id && (!at->next || c->id <= at->next->id)) {
				c->next = at->next;
				at->next = c;
				return;
			}
	}

	if (attachmode == AttachAbove) {
		if (!(c->ws->sel == NULL || c->ws->sel == c->ws->clients || ISFLOATING(c->ws->sel))) {
			for (at = c->ws->clients; at->next != c->ws->sel; at = at->next);
			c->next = at->next;
			at->next = c;
			return;
		}
	} else if (attachmode == AttachAside) {
		for (at = c->ws->clients, n = 0; at; at = at->next)
			if (!ISFLOATING(at))
				if (++n >= c->ws->nmaster)
					break;

		if (at && c->ws->nmaster) {
			c->next = at->next;
			at->next = c;
			return;
		}
	} else if (attachmode == AttachBelow) {
		if (!(c->ws->sel == NULL || c->ws->sel == c || ISFLOATING(c->ws->sel))) {
			c->next = c->ws->sel->next;
			c->ws->sel->next = c;
			return;
		}
	} else if (attachmode == AttachBottom) {
		for (at = c->ws->clients; at && at->next; at = at->next);
		if (at) {
			at->next = c;
			c->next = NULL;
			return;
		}
	}
	attach(c); // master (default)
}