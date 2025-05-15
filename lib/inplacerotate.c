void
insertclient(Client *item, Client *insertItem, int after)
{
	Client *c;
	Workspace *ws = selws;
	if (item == NULL || insertItem == NULL || item == insertItem)
		return;
	detach(insertItem);
	if (!after && ws->clients == item) {
		attach(insertItem);
		return;
	}
	if (after) {
		c = item;
	} else {
		for (c = ws->clients; c; c = c->next) {
			if (c->next == item)
				break;
		}
	}
	insertItem->next = c->next;
	c->next = insertItem;
}

void
inplacerotate(const Arg *arg)
{
	Workspace *ws = selws;
	if (!ws->sel || !ws->layout->arrange)
		return;

	int n, selidx = 0, i = 0, tidx, center, dualstack;
	Client *c = NULL,
		*shead = NULL, *stail = NULL,
		*thead = NULL, *ttail = NULL,
		*mhead = NULL, *mtail = NULL;

	for (n = 0, c = nexttiled(ws->clients); c; c = nexttiled(c->next), ++n);
	if (n < 2)
		return;

	tidx = ws->nmaster + (ws->nstack > 0 ? ws->nstack : (n - ws->nmaster) / 2 + ((n - ws->nmaster) % 2 > 0 ? 1 : 0));

	/* Shift client */
	for (c = ws->clients; c; c = c->next) {
		if (ISVISIBLE(c) && ISTILED(c)) {
			if (ws->sel == c)
				selidx = i;
			if (i == ws->nmaster - 1)
				mtail = c;
			if (i == ws->nmaster)
				shead = c;
			if (i == tidx - 1)
				stail = c;
			if (i == tidx)
				thead = c;
			if (mhead == NULL)
				mhead = c;
			ttail = c;
			i++;
		}
	}

	center = iscenteredlayout(ws, n);
	dualstack = isdualstacklayout(ws);
	if ((!center && !dualstack) || (center && n <= ws->nmaster + (ws->nstack ? ws->nstack : 1))) {
		if (arg->i < 0 && selidx >= ws->nmaster) insertclient(ttail, shead, 1);
		if (arg->i > 0 && selidx >= ws->nmaster) insertclient(shead, ttail, 0);
	} else {
		if (arg->i < 0 && selidx >= ws->nmaster && selidx < tidx) insertclient(stail, shead, 1);
		if (arg->i > 0 && selidx >= ws->nmaster && selidx < tidx) insertclient(shead, stail, 0);
		if (arg->i < 0 && selidx >= tidx) insertclient(ttail, thead, 1);
		if (arg->i > 0 && selidx >= tidx) insertclient(thead, ttail, 0);
	}
	if (arg->i < 0 && selidx < ws->nmaster) insertclient(mtail, mhead, 1);
	if (arg->i > 0 && selidx < ws->nmaster) insertclient(mhead, mtail, 0);

	/* Restore focus position */
	i = 0;
	for (c = ws->clients; c; c = c->next) {
		if (!ISVISIBLE(c) || ISFLOATING(c))
			continue;
		if (i == selidx) {
			focus(c);
			break;
		}
		i++;
	}
	arrangews(ws);
	focus(c);
}
