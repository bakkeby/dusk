
Client *
findbefore(Client *c) {
	Client *p;
	if (!c || c == c->ws->clients)
		return NULL;
	for (p = c->ws->clients; p && p->next != c; p = p->next);
	return p;
}