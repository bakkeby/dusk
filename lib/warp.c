int
canwarp(Client *c)
{
	Workspace *ws;

	/* Do not warp if the client is NULL */
	if (!c)
		return 0;

	ws = c->ws;

	/* Do not warp if the functionality is disabled */
	if (disabled(Warp))
		return 0;

	/* Do not warp if the client says not to */
	if (NOWARP(c))
		return 0;

	/* Do not warp if we are currently ignoring warp (e.g. when using drag functionality) */
	if (ignore_warp)
		return 0;

	/* Do not warp if this is not the currently selected workspace (to avoid the mouse cursor
	 * jumping around when moving windows between visible workspaces for example). */
	if (ws != selws)
		return 0;

	/* Do warp if the client is floating or floating layout is used */
	if (FREEFLOW(c))
		return 1;

	/* Do not warp if monocle layout is used */
	if (
		ws->ltaxis[MASTER] == MONOCLE && (
			abs(ws->ltaxis[LAYOUT]) == NO_SPLIT ||
			!ws->nmaster ||
			numtiled(ws) <= ws->nmaster
		)
	) {
		return 0;
	}

	/* Fine to warp in other situations */
	return 1;
}

void
warp(Client *c)
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

	/* Do not warp if cursor rests on one of the bars */
	for (m = mons; m; m = m->next)
		for (bar = m->bar; bar; bar = bar->next)
			if (x >= bar->bx &&
				x < bar->bx + bar->bw &&
				y >= bar->by &&
				y < bar->by + bar->bh)
				return;

	/* Avoid warping the cursor if it is already within the client's borders */
	if (!force_warp &&
		(x > c->x - c->bw &&
		 y > c->y - c->bw &&
		 x < c->x + c->w + c->bw*2 &&
		 y < c->y + c->h + c->bw*2)
	) {
		if (enabled(BanishMouseCursorToCorner) && cursor_hidden) {
			mouse_x = x;
			mouse_y = y;
			ban_to_corner(c);
		}
		return;
	}

	force_warp = 0;

	if (enabled(BanishMouseCursor) && cursor_hidden) {
		mouse_x = c->x + c->w / 2;
		mouse_y = c->y + c->h / 2;
		if (enabled(BanishMouseCursorToCorner)) {
			ban_to_corner(c);
			return;
		}
	}

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

	XWarpPointer(dpy, None, c->win, 0, 0, 0, 0, c->w / 2, c->h / 2);
}
