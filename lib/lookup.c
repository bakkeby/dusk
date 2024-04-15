Client **
clientptr(Client *c)
{
	Client **tc;
	for (tc = &c->ws->clients; *tc && *tc != c; tc = &(*tc)->next);
	return tc;
}

/* Calculates the position of a client in a workspace.
 *
 *   nth_client   - the position of the given client in the client list
 *   nth_tiled    - the tiled position of the client
 *   nth_floating - the floating position of the client
 *   nth_master   - the nth position of the client in the master stack
 *   nth_stack    - the nth position of the client in the stack
 *   num_clients  - total number of clients in the workspace
 *   num_tiled    - total number of tiled clients
 *   num_floating - total number of floating clients
 *   num_master   - total number of master clients
 *   num_stack    - total number of stack clients
 */
void
getclientindices(
	Client *c,
	int *nth_client,
	int *nth_tiled,
	int *nth_floating,
	int *nth_master,
	int *nth_stack,
	int *num_clients,
	int *num_tiled,
	int *num_floating,
	int *num_master,
	int *num_stack

) {
	Workspace *ws = c->ws;
	Client *s;
	int nc = 0, nt = 0, nf = 0, nm = 0, ns = 0;
	int nthc = 0, ntht = 0, nthf = 0, nthm = 0, nths = 0;

	for (s = ws->clients; s; s = s->next) {
		nc++;

		if (ISFLOATING(s)) {
			nf++;
		} else {
			nt++;
			if (nt <= ws->nmaster) {
				nm++;
			} else {
				ns++;
			}
		}

		if (c == s) {
			nthc = nc;
			if (ISFLOATING(c)) {
				nthf = nf;
			} else {
				ntht = nt;
				nthm = ns ? 0 : nm;
				nths = ns;
			}
		}
	}

	*nth_client = nthc;
	*nth_tiled = ntht;
	*nth_floating = nthf;
	*nth_master = nthm;
	*nth_stack = nths;
	*num_clients = nc;
	*num_tiled = nt;
	*num_floating = nf;
	*num_master = nm;
	*num_stack = ns;
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
lastclient(Client *c)
{
	Client *last;
	for (last = c; last && last->next; last = last->next);

	return last;
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
	if (!c)
		return NULL;

	return nthtiled(c, MIN(n, c->ws->nmaster), 1);
}

Client *
nthstack(Client *c, int n, int reduce)
{
	if (!c)
		return NULL;

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
inctiled(Client *c, int n)
{
	Client *f;

	if (!c)
		return NULL;

	if (n > 0) {
		f = nexttiled(c->next);
		if (!f) {
			f = nexttiled(c->ws->clients);
		}
	} else {
		f = prevtiled(c);
		if (!f) {
			f = lasttiled(c->ws->clients);
		}
	}
	return f;
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
