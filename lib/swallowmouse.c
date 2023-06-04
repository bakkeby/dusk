void
swallowmouse(const Arg *arg)
{
	Client *r, *c = CLIENT;
	XEvent ev;
	int nx = 0, ny = 0, di;
	unsigned int dui;
	Window dummy;
	Time lasttime = 0;

	if (!c || disabled(Swallow))
		return;

	if (!XQueryPointer(dpy, c->win, &dummy, &dummy, &nx, &ny, &di, &di, &dui))
		return;

	if (XGrabPointer(dpy, root, False, MOUSEMASK, GrabModeAsync,
		GrabModeAsync, None, cursor[CurSwallow]->cursor, CurrentTime) != GrabSuccess) {
		return;
	}

	if (ISMARKED(c))
		ignore_marked = 0; // swallowmouse supports marked clients

	do {
		XMaskEvent(dpy, MOUSEMASK|ExposureMask|SubstructureRedirectMask, &ev);
		switch(ev.type) {
		case ConfigureRequest: /* fallthrough */
		case Expose: /* fallthrough */
		case MapRequest:
			handler[ev.type](&ev);
			break;
		case MotionNotify:
			if ((ev.xmotion.time - lasttime) <= (1000 / 60))
				continue;
			lasttime = ev.xmotion.time;
			nx = ev.xmotion.x;
			ny = ev.xmotion.y;
			break;
		}
	} while (ev.type != ButtonRelease);
	XUngrabPointer(dpy, CurrentTime);

	r = recttoclient(nx, ny, 1, 1, 1);
	if (r && r != c) {

		// mark(&((Arg) { .v = c }));
		swallow(&((Arg) { .v = r }));
	}

	/* Remove accumulated pending EnterWindow events caused by the mouse
	 * movements. */
	XCheckMaskEvent(dpy, EnterWindowMask, &ev);
}
