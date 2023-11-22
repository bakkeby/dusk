void
stackfocus(const Arg *arg)
{
	Workspace *ws = selws;
	Client *c;

	if (ISINC(arg)) {
		focusstack(&((Arg) { .i = GETINC(arg) }));
		return;
	}

	if (!ws->clients)
		return;

	c = stackposclient(arg);

	if (!c)
		return;

	if (c == ws->sel) {
		if (arg->i != PREVSEL) {
			stackfocus(&((Arg) { .i = PREVSEL }));
		}
		return;
	}

	focus(c);
	arrangews(ws);
	skipfocusevents();
	if (canwarp(c))
		warp(c);
}

void
stackpush(const Arg *arg)
{
	Workspace *ws = selws;
	Client *c, *sel = ws->sel;

	if (!ws->clients)
		return;

	if (ISINC(arg)) {
		stackswap(arg);
		return;
	}

	c = stackposclient(arg);

	if (!c)
		return;

	if (c == sel)
		return;

	detach(sel);
	attachabove(sel, c);

	arrangews(ws);
	skipfocusevents();
	if (canwarp(c))
		warp(sel);
}

void
stackswap(const Arg *arg)
{
	Workspace *ws = selws;
	Client *c, *sel = ws->sel;

	if (!ws->clients)
		return;

	c = stackposclient(arg);

	if (!c)
		return;

	if (c == sel)
		return;

	swap(sel, c);

	if (!ISINC(arg) && ismasterclient(c)) {
		focus(c);
		sel = c;
	}

	arrangews(ws);
	skipfocusevents();
	if (canwarp(sel))
		warp(sel);
}

Client *
stackposclient(const Arg *arg)
{
	Workspace *ws = selws;

	if (!ws->clients)
		return NULL;

	if (ISINC(arg))
		return inctiled(ws->sel, GETINC(arg));

	if (ISMASTER(arg))
		return nthmaster(ws->clients, GETMASTER(arg), 1);

	if (ISSTACK(arg))
		return nthstack(ws->clients, GETSTACK(arg), 1);

	if (ISLAST(arg))
		return lasttiled(ws->clients);

	if (ISPREVSEL(arg))
		return prevsel();

	return nthtiled(ws->clients, arg->i, 1);
}
