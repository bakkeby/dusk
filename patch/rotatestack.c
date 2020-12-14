void
enqueue(Client *c)
{
	Client *l;
	for (l = c->ws->clients; l && l->next; l = l->next);
	if (l) {
		l->next = c;
		c->next = NULL;
	}
}

void
enqueuestack(Client *c)
{
	Client *l;
	for (l = c->ws->stack; l && l->snext; l = l->snext);
	if (l) {
		l->snext = c;
		c->snext = NULL;
	}
}

void
rotatestack(const Arg *arg)
{
	Client *c = NULL, *f;

	if (!selws->sel)
		return;
	f = selws->sel;
	if (arg->i > 0) {
		for (c = nexttiled(selws->clients); c && nexttiled(c->next); c = nexttiled(c->next));
		if (c) {
			detach(c);
			attach(c);
			detachstack(c);
			attachstack(c);
		}
	} else {
		if ((c = nexttiled(selws->clients))) {
			detach(c);
			enqueue(c);
			detachstack(c);
			enqueuestack(c);
		}
	}
	if (c) {
		arrange(selws);
		focus(f);
		restack(selws);
	}
}