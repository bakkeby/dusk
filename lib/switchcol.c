void
switchcol(const Arg *arg)
{
	Workspace *ws = selws;
	Client *c, *t;
	int col = 0;
	int i;

	if (!ws->sel)
		return;
	for (i = 0, c = nexttiled(ws->clients); c ;
	     c = nexttiled(c->next), i++) {
		if (c == ws->sel)
			col = (i + 1) > ws->nmaster;
	}
	if (i <= ws->nmaster)
		return;
	for (c = ws->stack; c; c = c->snext) {
		if (!ISVISIBLE(c))
			continue;
		for (i = 0, t = nexttiled(ws->clients); t && t != c;
		     t = nexttiled(t->next), i++);
		if (t && (i + 1 > ws->nmaster) != col) {
			focus(c);
			restack(ws);
			break;
		}
	}
}