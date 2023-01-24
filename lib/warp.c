int
canwarp(Workspace *ws)
{
	/* Do not warp if the functionality is disabled */
	if (disabled(Warp))
		return 0;

	/* Do not warp if we are currently ignoring warp (e.g. when using drag functionality) */
	if (ignore_warp)
		return 0;

	/* Do not warp if this is not the currently selected workspace (to avoid the mouse cursor
	 * jumping around when moving windows between visible workspaces for example). */
	if (ws != selws)
		return 0;

	/* Do warp if the client is floating or floating layout is used */
	if (ISFLOATING(ws->sel) || !ws->layout->arrange)
		return 1;

	/* Do not warp if the monocle layout used */
	if (
		ws->ltaxis[MASTER] == MONOCLE && (
			abs(ws->ltaxis[LAYOUT]) == NO_SPLIT ||
			!ws->nmaster ||
			numtiled(ws) <= ws->nmaster
		)
	)
		return 0;

	/* Fine to warp in other situations */
	return 1;
}

void
warp(const Client *c)
{
	Monitor *m;
	Client *f;
	Bar *bar;
	int x, y;

	if (ignore_warp)
		return;

	if (!c) {
		XWarpPointer(dpy, None, root, 0, 0, 0, 0, selmon->wx + selmon->ww / 2, selmon->wy + selmon->wh / 2);
		return;
	}

	if (!getrootptr(&x, &y))
		return;

	/* Avoid warping the cursor if it is already within the client's borders */
	if (!force_warp &&
		(x > c->x - c->bw &&
		 y > c->y - c->bw &&
		 x < c->x + c->w + c->bw*2 &&
		 y < c->y + c->h + c->bw*2))
		return;

	force_warp = 0;

	if (disabled(FocusedOnTop) && !ALWAYSONTOP(c)) {
		/* Avoid warping the cursor if our client's midpoint is covered by a floating window */
		for (f = c->ws->stack; f && f != c; f = f->snext)
			if (ISVISIBLE(f) &&
				f->x < c->x + c->w / 2 &&
				f->x + f->w > c->x + c->w / 2 &&
				f->y < c->y + c->h / 2 &&
				f->y + f->h > c->y + c->h / 2)
				return;
	}

	/* Do not warp if cursor rests on one of the bars */
	for (m = mons; m; m = m->next)
		for (bar = m->bar; bar; bar = bar->next)
			if (x > bar->bx &&
				x < bar->bx + bar->bw &&
				y > bar->by &&
				y < bar->by + bar->bh)
				return;

	XWarpPointer(dpy, None, c->win, 0, 0, 0, 0, c->w / 2, c->h / 2);
}
