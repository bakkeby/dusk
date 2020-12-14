void
switchcol(const Arg *arg)
{
	Client *c, *t;
	int col = 0;
	int i;

	if (!selws->sel)
		return;
	for (i = 0, c = nexttiled(selws->clients); c ;
	     c = nexttiled(c->next), i++) {
		if (c == selws->sel)
			col = (i + 1) > selmon->nmaster;
	}
	if (i <= selmon->nmaster)
		return;
	for (c = selws->stack; c; c = c->snext) {
		if (!ISVISIBLE(c))
			continue;
		for (i = 0, t = nexttiled(selws->clients); t && t != c;
		     t = nexttiled(t->next), i++);
		if (t && (i + 1 > selmon->nmaster) != col) {
			focus(c);
			restack(selws);
			break;
		}
	}
}