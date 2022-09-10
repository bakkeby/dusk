void
dragfact(const Arg *arg)
{
	unsigned int n;
	int px, py; // pointer coordinates
	int dist_x, dist_y;
	int horizontal = 0; // layout configuration
	float mfact, cfact, cf, cw, ch, mw, mh;
	Client *c;
	Workspace *ws = selws;
	XEvent ev;
	Time lasttime = 0;

	for (n = 0, c = nexttiled(ws->clients); c; c = nexttiled(c->next), n++);
	if (!(c = ws->sel) || !n || !ws->layout->arrange)
		return;

	if (ws->layout->arrange == &flextile) {
		int layout = ws->ltaxis[LAYOUT];
		if (layout < 0)
			layout *= -1;
		if (layout > FLOATING_MASTER)
			layout -= FLOATING_MASTER;

		if (layout == SPLIT_HORIZONTAL ||
			layout == SPLIT_HORIZONTAL_DUAL_STACK ||
			layout == SPLIT_CENTERED_HORIZONTAL ||
			(layout == FLOATING_MASTER && ws->ww < ws->wh)
		) {
			horizontal = 1;
		}
	}

	if (XGrabPointer(dpy, root, False, MOUSEMASK, GrabModeAsync, GrabModeAsync,
		None, cursor[CurResize]->cursor, CurrentTime) != GrabSuccess)
		return;

	if (!getrootptr(&px, &py))
		return;

	ignore_warp = 1;
	cf = c->cfact;
	ch = c->h;
	cw = c->w;
	mw = ws->ww * ws->mfact;
	mh = ws->wh * ws->mfact;

	do {
		XMaskEvent(dpy, MOUSEMASK|ExposureMask|SubstructureRedirectMask, &ev);
		switch (ev.type) {
		case ConfigureRequest:
		case Expose:
		case MapRequest:
			handler[ev.type](&ev);
			break;
		case MotionNotify:
			if ((ev.xmotion.time - lasttime) <= (1000 / 40))
				continue;
			lasttime = ev.xmotion.time;

			dist_x = ev.xmotion.x - px;
			dist_y = ev.xmotion.y - py;

			if (horizontal) {
				cfact = (float) cf * (cw + dist_x) / cw;
				mfact = (float) (mh + dist_y) / ws->wh;
			} else {
				cfact = (float) cf * (ch - dist_y) / ch;
				mfact = (float) (mw + dist_x) / ws->ww;
			}

			setcfact(&((Arg) { .f = 4.0 + MAX(cfact, 0.0) }));
			setmfact(&((Arg) { .f = 1.0 + MAX(mfact, 0) }));

			break;
		}
	} while (ev.type != ButtonRelease);

	ignore_warp = 0;
	XUngrabPointer(dpy, CurrentTime);
	skipfocusevents();
}