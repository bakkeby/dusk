void
warp(const Client *c)
{
	Monitor *m;
	Client *f;
	Bar *bar;
	int x, y;

	if (ignore_warp) {
		fprintf(stderr, "warp: <-- ignoring warp \n");
		return;
	}

	if (!c) {
		fprintf(stderr, "warp: <-- !c \n");
		XWarpPointer(dpy, None, root, 0, 0, 0, 0, selmon->wx + selmon->ww / 2, selmon->wy + selmon->wh / 2);
		return;
	}

	if (!getrootptr(&x, &y)) {
		fprintf(stderr, "warp: <-- !getrootptr \n");
		return;
	}

	/* Avoid warping the cursor if it is already within the client's borders */
	if (!force_warp &&
		(x > c->x - c->bw &&
		 y > c->y - c->bw &&
		 x < c->x + c->w + c->bw*2 &&
		 y < c->y + c->h + c->bw*2)) {
		fprintf(stderr, "warp: <-- cursor within the client's border \n");
		return;
	}

	force_warp = 0;

	if (disabled(FocusedOnTop)) {
		/* Avoid warping the cursor if our client's midpoint is covered by a floating window */
		for (f = c->ws->stack; f; f = f->snext)
			if (f != c && ISVISIBLE(f) && !HIDDEN(f) &&
				f->x < c->x + c->w / 2 &&
				f->x + f->w > c->x + c->w / 2 &&
				f->y < c->y + c->h / 2 &&
				f->y + f->h > c->y + c->h / 2) {
				fprintf(stderr, "warp: <-- avoid warping if the client's midpoint is covered by a floating window \n");
				fprintf(stderr, "warp: .. that floating window is %s\n", f->name);
				return;
			}
	}

	/* Do not warp if cursor rests on one of the bars */
	for (m = mons; m; m = m->next)
		for (bar = m->bar; bar; bar = bar->next)
			if (x > bar->bx &&
				x < bar->bx + bar->bw &&
				y > bar->by &&
				y < bar->by + bar->bh) {
				fprintf(stderr, "%s\n", "not warping as mouse cursor on top of one of the bars");
				return;
			}

	XWarpPointer(dpy, None, c->win, 0, 0, 0, 0, c->w / 2, c->h / 2);
	fprintf(stderr, "warp: ran XWarpPointer\n");
}
