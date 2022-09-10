void
setcfact(const Arg *arg)
{
	float f;
	Workspace *ws = NULL;
	Client *c = selws->sel;

	for (c = nextmarked(NULL, c); c; c = nextmarked(c->next, NULL)) {
		if (!arg || !c->ws->layout->arrange)
			continue;
		if (!arg->f)
			f = 1.0;
		else if (arg->f < 4.0)
			f = arg->f + c->cfact;
		else // set fact absolutely
			f = arg->f - 4.0;
		if (f < 0.25)
			f = 0.25;
		else if (f > 4.0)
			f = 4.0;
		c->cfact = f;

		if (ws && c->ws != ws)
			arrangews(c->ws);
		ws = c->ws;
	}

	if (ws)
		arrangews(ws);
}
