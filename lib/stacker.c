void
stackfocus(const Arg *arg)
{
	Workspace *ws = selws;
	Client *c = NULL;

	if (ISINC(arg)) {
		focusstack(&((Arg) { .i = GETINC(arg) }));
		return;
	}

	if (!ws->clients)
		return;

	stackposclient(arg, &c);

	if (c == ws->sel) {
		if (arg->i != PREVSEL) {
			stackfocus(&((Arg) { .i = PREVSEL }));
		}
		return;
	}

	focus(c);
	arrangews(ws);
	skipfocusevents();
	if (canwarp(ws))
		warp(c);
}

void
stackpush(const Arg *arg)
{
	Workspace *ws = selws;
	Client *c = NULL, *sel = ws->sel;

	if (ISINC(arg))
		return;

	if (!ws->clients)
		return;

	stackposclient(arg, &c);

	if (c == sel)
		return;

	detach(sel);
	attachabove(sel, c);

	arrangews(ws);
	skipfocusevents();
	if (canwarp(ws))
		warp(sel);
}

void
stackswap(const Arg *arg)
{
	Workspace *ws = selws;
	Client *c = NULL, *sel = ws->sel;

	if (ISINC(arg))
		return;

	if (!ws->clients)
		return;

	stackposclient(arg, &c);

	if (c == sel)
		return;

	swap(sel, c);

	if (ismasterclient(c)) {
		focus(c);
		sel = c;
	}

	arrangews(ws);
	skipfocusevents();
	if (canwarp(ws))
		warp(sel);
}

void
stackposclient(const Arg *arg, Client **f)
{
	Workspace *ws = selws;

	if (!ws->clients)
		return;

	if (ISMASTER(arg)) {
		*f = nthmaster(ws->clients, GETMASTER(arg), 1);
		return;
	}

	if (ISSTACK(arg)) {
		*f = nthstack(ws->clients, GETSTACK(arg), 1);
		return;
	}

	if (ISLAST(arg)) {
		*f = lasttiled(ws->clients);
		return;
	}

	if (ISPREVSEL(arg)) {
		*f = prevsel();
		return;
	}

	*f = nthtiled(ws->clients, arg->i, 1);
	return;
}
