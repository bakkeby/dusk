void
shiftviewclients(const Arg *arg)
{
	Workspace *ws = WS;
	Arg shifted;
	Client *c;
	unsigned int tagmask = 0;

	for (c = ws->clients; c; c = c->next)
		if (!(c->tags & SPTAGMASK))
			tagmask = tagmask | c->tags;

	shifted.ui = ws->tags & ~SPTAGMASK;
	if (arg->i > 0) // left circular shift
		do {
			shifted.ui = (shifted.ui << arg->i)
			   | (shifted.ui >> (NUMTAGS - arg->i));
			shifted.ui &= ~SPTAGMASK;
		} while (tagmask && !(shifted.ui & tagmask));
	else // right circular shift
		do {
			shifted.ui = (shifted.ui >> -arg->i)
			   | (shifted.ui << (NUMTAGS + arg->i));
			shifted.ui &= ~SPTAGMASK;
		} while (tagmask && !(shifted.ui & tagmask));

	view(&shifted);
}