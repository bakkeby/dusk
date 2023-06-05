Client **
clientptr(Client *c)
{
	Client **tc;
	for (tc = &c->ws->clients; *tc && *tc != c; tc = &(*tc)->next);
	return tc;
}

int
ismasterclient(Client *client)
{
	Workspace *ws = client->ws;
	Client *c;
	int i;

	for (i = 0, c = nexttiled(ws->clients); c && i < ws->nmaster; c = nexttiled(c->next), ++i)
		if (c == client)
			return 1;

	return 0;
}

Client *
lasttiled(Client *c)
{
	Client *last = NULL;
	for (; c; c = c->next)
		if (TILED(c))
			last = c;

	return last;
}

Client *
nexttiled(Client *c)
{
	for (; c && !TILED(c); c = c->next);
	return c;
}

Client *
nthmaster(Client *c, int n, int reduce)
{
	return nthtiled(c, MIN(n, c->ws->nmaster), 1);
}

Client *
nthstack(Client *c, int n, int reduce)
{
	return nthtiled(c, n + c->ws->nmaster, 1);
}

Client *
nthtiled(Client *c, int n, int reduce)
{
	Client *prev = NULL;
	int i;

	for (i = 1, c = nexttiled(c); c && (i++ < n); prev = c, c = nexttiled(c->next));

	if (!c && reduce) {
		c = prev;
	}

	return c;
}

Client *
prevtiled(Client *c)
{
	Client *p, *r;
	for (p = nexttiled(c->ws->clients), r = NULL; p && p != c && (r = p); p = nexttiled(p->next));
	return r;
}

Client *
prevsel(void)
{
	Workspace *ws = selws;
	Client *c;

	if (!ws->clients)
		return NULL;

	for (c = ws->stack; c && (!ISVISIBLE(c) || c == ws->sel); c = c->snext);
	return c;
}

void
swap(Client *a, Client *b)
{
	Client **ap = clientptr(a);
	Client **bp = clientptr(b);
	Client *an = a->next;
	Client *bn = b->next;

	if (bn == a) {
		b->next = an;
		a->next = b;
		*bp = a;
	} else if (an == b) {
		b->next = a;
		a->next = bn;
		*ap = b;
	} else {
		b->next = an;
		a->next = bn;
		*ap = b;
		*bp = a;
	}
}
