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
	drawbar(ws->mon);
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
	drawbar(ws->mon);
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

const StackerIcon *
getstackericonforclient(Client *c)
{
	int i, stacklevel;
	int fine = -1, good = -1, better = -1, best = -1; // prioritisation levels
	int nthc, ntht, nthf, nthm, nths; // nth client, tiled, floating, master, stack
	int nc, nt, nf, nm, ns;           // num clients, tiled, floating, master, stack
	getclientindices(c, &nthc, &ntht, &nthf, &nthm, &nths, &nc, &nt, &nf, &nm, &ns);

	for (i = 0; i < LENGTH(stackericons); i++) {
		stacklevel = stackericons[i].arg.i;

		/* MASTER, STACK and implicit client positions take precedence for icons */
		if (nthm && stacklevel == MASTER(nthm)) {
			best = i;
		} else if (nths && stacklevel == STACK(nths)) {
			best = i;
		} else if (stacklevel == nthc) {
			best = i;
		}

		/* Falls back to LASTTILED, if applicable */
		if (stacklevel == LASTTILED && ntht == nt) {
			better = i;
		}

		/* Falls back to INC, if applicable */
		if (stacklevel == INC(-1) && c->next == c->ws->sel) {
			good = i;
		}

		if (stacklevel == INC(+1) && c->ws->sel && c->ws->sel->next == c) {
			good = i;
		}

		/* Falls back to PREVSEL, if applicable */
		if (stacklevel == PREVSEL && c == prevsel()) {
			fine = i;
		}
	}

	i = best > -1 ? best : better > -1 ? better : good > -1 ? good : fine;

	if (i == -1)
		return NULL;

	return &stackericons[i];
}