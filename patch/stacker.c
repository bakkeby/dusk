void
stackfocus(const Arg *arg)
{
	int i = stackpos(arg);
	Client *c, *p;

	if (i < 0)
 		return;

	for (p = NULL, c = selws->clients; c && (i || !ISVISIBLE(c));
	    i -= ISVISIBLE(c) ? 1 : 0, p = c, c = c->next);
	focus(c ? c : p);
	restack(selws);
	arrangemon(selmon);
}

void
stackpush(const Arg *arg)
{
	int i = stackpos(arg);
	Client *sel = selws->sel, *c, *p;

	if (i < 0)
		return;
	else if (i == 0) {
		detach(sel);
		attach(sel);
	}
	else {
		for (p = NULL, c = selws->clients; c; p = c, c = c->next)
			if (!(i -= (ISVISIBLE(c) && c != sel)))
				break;
		c = c ? c : p;
		detach(sel);
		sel->next = c->next;
		c->next = sel;
	}
	arrangemon(selmon);
}

int
stackpos(const Arg *arg)
{
	int n, i;
	Client *c, *l;

	if (!selws->clients)
		return -1;

	if (arg->i == PREVSEL) {
		for (l = selws->stack; l && (!ISVISIBLE(l) || l == selws->sel); l = l->snext);
		if (!l)
			return -1;
		for (i = 0, c = selws->clients; c != l; i += ISVISIBLE(c) ? 1 : 0, c = c->next);
		return i;
	}
	else if (ISINC(arg->i)) {
		if (!selws->sel)
			return -1;
		for (i = 0, c = selws->clients; c != selws->sel; i += ISVISIBLE(c) ? 1 : 0, c = c->next);
		for (n = i; c; n += ISVISIBLE(c) ? 1 : 0, c = c->next);
		return MOD(i + GETINC(arg->i), n);
	}
	else if (arg->i < 0) {
		for (i = 0, c = selws->clients; c; i += ISVISIBLE(c) ? 1 : 0, c = c->next);
		return MAX(i + arg->i, 0);
	}
	else
		return arg->i;
}
