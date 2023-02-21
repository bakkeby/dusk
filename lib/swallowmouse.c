void
swallowmouse(const Arg *arg)
{
	Client *r, *c = CLIENT;
	XEvent ev;
	int nx = 0, ny = 0, di;
	unsigned int dui;
	Window dummy;
	Time lasttime = 0;
	ignore_marked = 0; // swallowmouse supports marked clients

	fprintf(stderr, "swallowmouse: c = %s\n", c ? c->name : "NULL");
	if (!c || disabled(Swallow)) {
		fprintf(stderr, "bailing due to something\n" );
		return;
	}

	if (!XQueryPointer(dpy, c->win, &dummy, &dummy, &nx, &ny, &di, &di, &dui)) {
		fprintf(stderr, "can't query pointer" );
		return;
	}
	fprintf(stderr, "original nx and ny = %d and %d\n", nx, ny);

	if (XGrabPointer(dpy, root, False, MOUSEMASK, GrabModeAsync,
		GrabModeAsync, None, cursor[CurSwallow]->cursor, CurrentTime) != GrabSuccess) {
		fprintf(stderr, "cant grab pointer\n" );
		return;
	}

	do {
		XMaskEvent(dpy, MOUSEMASK|ExposureMask|SubstructureRedirectMask, &ev);
		switch(ev.type) {
		case ConfigureRequest: /* fallthrough */
		case Expose: /* fallthrough */
		case MapRequest:
			handler[ev.type](&ev);
			break;
		case MotionNotify:
			if ((ev.xmotion.time - lasttime) <= (1000 / dpyHz))
				continue;
			lasttime = ev.xmotion.time;
			nx = ev.xmotion.x;
			ny = ev.xmotion.y;
			fprintf(stderr, "set nx to %d and ny to %d\n", nx, ny);
			break;
		}
	} while (ev.type != ButtonRelease);
	XUngrabPointer(dpy, CurrentTime);

	r = recttoclient(nx, ny, 1, 1, 1);
	fprintf(stderr, "cursor rests on client %s at position %d, %d\n", r ? r->name : "NULL", nx, ny);
	if (r && r != c) {
		mark(&((Arg) { .v = c }));
		fprintf(stderr, "swalloing r %s\n", c->name);
		swallow(&((Arg) { .v = r }));
		fprintf(stderr, "after swallow\n" );
	}

	/* Remove accumulated pending EnterWindow events caused by the mouse
	 * movements. */
	XCheckMaskEvent(dpy, EnterWindowMask, &ev);
	fprintf(stderr, "swallowmouse <--\n" );
}
