void
dragwfact(const Arg *arg)
{
	int prev_x, prev_y, dist_x, dist_y;
	int nw = 0;
	float fact;
	XEvent ev;
	Time lasttime = 0;
	Workspace *ws;
	Monitor *m;

	/* If we have no selected workspace then deny the dragwfact action. */
	if (!selws)
		return;

	m = selws->mon;

	/* If we are not viewing more than one workspace then deny the dragwfact action. */
	for (ws = nextvismonws(m, workspaces); ws; ws = nextvismonws(m, ws->next), ++nw);
	if (nw <= 1)
		return;

	ws = selws;

	if (XGrabPointer(dpy, root, False, MOUSEMASK, GrabModeAsync, GrabModeAsync,
		None, cursor[CurIronCross]->cursor, CurrentTime) != GrabSuccess)
		return;

	prev_x = prev_y = -999999;

	do {
		XMaskEvent(dpy, MOUSEMASK|ExposureMask|SubstructureRedirectMask, &ev);
		switch (ev.type) {
		case ConfigureRequest:
		case Expose:
		case MapRequest:
			handler[ev.type](&ev);
			break;
		case MotionNotify:
			if ((ev.xmotion.time - lasttime) <= (1000 / 60))
				continue;
			lasttime = ev.xmotion.time;
			if (prev_x == -999999) {
				prev_x = ev.xmotion.x_root;
				prev_y = ev.xmotion.y_root;
			}

			dist_x = ev.xmotion.x - prev_x;
			dist_y = ev.xmotion.y - prev_y;

			if (abs(dist_x) > abs(dist_y)) {
				fact = (float) 4.0 * dist_x / ws->mon->ww;
			} else {
				fact = (float) -4.0 * dist_y / ws->mon->wh;
			}

			if (fact)
				setwfact(&((Arg) { .f = fact }));

			prev_x = ev.xmotion.x;
			prev_y = ev.xmotion.y;
			break;
		}
	} while (ev.type != ButtonRelease);

	XUngrabPointer(dpy, CurrentTime);
	skipfocusevents();
}
