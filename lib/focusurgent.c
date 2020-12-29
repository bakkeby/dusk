void
focusurgent(const Arg *arg)
{
	Workspace *ws = WS;
	Client *c;
	int i;
	for (c = ws->clients; c && !ISURGENT(c); c = c->next);
	if (c) {
		for (i = 0; i < NUMTAGS && !((1 << i) & c->tags); i++);
		if (i < NUMTAGS) {
			if (((1 << i) & TAGMASK) != ws->tags)
				view(&((Arg) { .ui = 1 << i }));
			focus(c);
		}
	}
}