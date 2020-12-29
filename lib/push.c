void
pushup(const Arg *arg)
{
	Workspace *ws = WS;
	Client *sel = ws->sel;
	Client *c;

	if (!sel || (ISFLOATING(sel) && !arg->f))
		return;
	if ((c = prevtiled(sel))) {
		/* attach before c */
		detach(sel);
		sel->next = c;
		if (ws->clients == c)
			ws->clients = sel;
		else {
			for (c = ws->clients; c->next != sel->next; c = c->next);
			c->next = sel;
		}
	} else {
		/* move to the end */
		for (c = sel; c->next; c = c->next);
		detach(sel);
		sel->next = NULL;
		c->next = sel;
	}
	focus(sel);
	arrangemon(selmon);
}

void
pushdown(const Arg *arg)
{
	Workspace *ws = WS;
	Client *sel = ws->sel;
	Client *c;

	if (!sel || (ISFLOATING(sel) && !arg->f))
		return;
	if ((c = nexttiled(sel->next))) {
		/* attach after c */
		detach(sel);
		sel->next = c->next;
		c->next = sel;
	} else {
		/* move to the front */
		detach(sel);
		attach(sel);
	}
	focus(sel);
	arrangemon(selmon);
}