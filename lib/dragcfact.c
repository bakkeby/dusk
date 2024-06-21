void
dragcfact(const Arg *arg)
{
	int prev_x, prev_y, dist_x, dist_y;
	float fact;
	Client *c;
	XEvent ev;
	Time lasttime = 0;
	Workspace *ws = selws;

	if (!(c = ws->sel))
		return;
	if (FREEFLOW(c)) {
		resizemouse(arg);
		return;
	}

	if (ISTRUEFULLSCREEN(c)) /* no support resizing fullscreen windows by mouse */
		return;
	restack(ws);

	if (XGrabPointer(dpy, root, False, MOUSEMASK, GrabModeAsync, GrabModeAsync,
		None, cursor[CurIronCross]->cursor, CurrentTime) != GrabSuccess)
		return;
	ignore_warp = 1;
	XWarpPointer(dpy, None, c->win, 0, 0, 0, 0, c->w/2, c->h/2);

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
			if ((ev.xmotion.time - lasttime) <= (1000 / DRAGCFACT_HZ))
				continue;
			lasttime = ev.xmotion.time;
			if (prev_x == -999999) {
				prev_x = ev.xmotion.x_root;
				prev_y = ev.xmotion.y_root;
			}

			dist_x = ev.xmotion.x - prev_x;
			dist_y = ev.xmotion.y - prev_y;

			if (abs(dist_x) > abs(dist_y)) {
				fact = (float) 4.0 * dist_x / c->ws->mon->ww;
			} else {
				fact = (float) -4.0 * dist_y / c->ws->mon->wh;
			}

			if (fact)
				setcfact(&((Arg) { .f = fact }));

			prev_x = ev.xmotion.x;
			prev_y = ev.xmotion.y;
			break;
		}
	} while (ev.type != ButtonRelease);

	ignore_warp = 0;
	XWarpPointer(dpy, None, c->win, 0, 0, 0, 0, c->w/2, c->h/2);

	XUngrabPointer(dpy, CurrentTime);
	skipfocusevents();
}
