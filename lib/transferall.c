void
transferall(const Arg *arg)
{
	Workspace *ws = selws;
	Client *c, *n = ws->clients, *attachfrom = NULL;
	int i = 0, nstackclients = 0;
	while (n) {
		c = n;
		n = c->next;
		if (!ISVISIBLE(c) || ISFLOATING(c))
			continue;
		if (i >= ws->nmaster) {
			detach(c);
			if (!attachfrom) {
				attach(c);
			} else {
				c->next = attachfrom->next;
				attachfrom->next = c;
			}
			attachfrom = c;
			nstackclients++;
		}
		i++;
	}
	ws->nmaster = nstackclients;
	arrange(ws);
}