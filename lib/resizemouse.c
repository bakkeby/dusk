void
resizemouse(const Arg *arg)
{
	int ocx, ocy, nw, nh;
	int opx, opy, och, ocw, nx, ny;
	int horizcorner, vertcorner;
	unsigned int dui;
	Window dummy;
	Client *c;
	XEvent ev;
	Workspace *w, *ws = selws;
	Time lasttime = 0;
	double prevopacity;

	if (!(c = ws->sel))
		return;
	if (ISFULLSCREEN(c) && !ISFAKEFULLSCREEN(c)) /* no support resizing fullscreen windows by mouse */
		return;

	if (resizeopacity) {
		prevopacity = c->opacity;
		opacity(c, resizeopacity);
	}
	restack(selws);
	ocx = c->x;
	ocy = c->y;
	och = c->h;
	ocw = c->w;
	if (!XQueryPointer(dpy, c->win, &dummy, &dummy, &opx, &opy, &nx, &ny, &dui))
		return;
	horizcorner = nx < c->w / 2;
	vertcorner  = ny < c->h / 2;
	if (XGrabPointer(dpy, root, False, MOUSEMASK, GrabModeAsync, GrabModeAsync,
		None, cursor[horizcorner | (vertcorner << 1)]->cursor, CurrentTime) != GrabSuccess)
		return;
	addflag(c, MoveResize);
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

			nx = horizcorner ? (ocx + ev.xmotion.x - opx) : c->x;
			ny = vertcorner ? (ocy + ev.xmotion.y - opy) : c->y;
			nw = MAX(horizcorner ? (ocx + ocw - nx) : (ocw + (ev.xmotion.x - opx)), 1);
			nh = MAX(vertcorner ? (ocy + och - ny) : (och + (ev.xmotion.y - opy)), 1);

			if (c->ws->mon->wx + nw >= selmon->wx && c->ws->mon->wx + nw <= selmon->wx + selmon->ww
			&& c->ws->mon->wy + nh >= selmon->wy && c->ws->mon->wy + nh <= selmon->wy + selmon->wh)
			{
				if (!ISFLOATING(c) && ws->layout->arrange
				&& (abs(nw - c->w) > snap || abs(nh - c->h) > snap))
					togglefloating(NULL);
			}
			if (!ws->layout->arrange || ISFLOATING(c)) {
				resize(c, nx, ny, nw, nh, 1);
				savefloats(c);
			}
			break;
		}
	} while (ev.type != ButtonRelease);

	XUngrabPointer(dpy, CurrentTime);
	skipfocusevents();

	if (!ISSTICKY(c) && (w = recttows(c->x, c->y, c->w, c->h)) && w != selws) {
		detach(c);
		detachstack(c);
		attachx(c, AttachBottom, w);
		attachstack(c);
		selws = w;
		selmon = w->mon;
		focus(c);
	}
	removeflag(c, MoveResize);
	if (resizeopacity)
		opacity(c, prevopacity);
}

void
resizeorcfacts(const Arg *arg)
{
	if (!selws || !selws->sel)
		return;

	if (!selws->layout->arrange || ISFLOATING(selws->sel))
		resizemouse(arg);
	else
		dragcfact(arg);
}