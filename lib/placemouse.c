void
placemouse(const Arg *arg)
{
	int x, y, px, py, ocx, ocy, nx = -9999, ny = -9999, freemove = 0;
	Client *c, *r = NULL, *prevr;
	Workspace *w, *ws = selws;
	XEvent ev;
	XWindowAttributes wa;
	double prevopacity;
	Time lasttime = 0;
	unsigned long attachmode, prevattachmode;
	attachmode = prevattachmode = AttachMaster;

	if (!(c = ws->sel) || !ws->layout->arrange) /* no support for placemouse when floating layout is used */
		return;
	if (ISFULLSCREEN(c) && !ISFAKEFULLSCREEN(c)) /* no support placing fullscreen windows by mouse */
		return;
	if (ISSTICKY(c))
		return;
	restack(ws);
	prevr = c;
	if (XGrabPointer(dpy, root, False, MOUSEMASK, GrabModeAsync, GrabModeAsync,
		None, cursor[CurMove]->cursor, CurrentTime) != GrabSuccess)
		return;

	addflag(c, MovePlace);
	removeflag(c, Floating);
	if (placeopacity) {
		prevopacity = c->opacity;
		opacity(c, placeopacity);
	}

	XGetWindowAttributes(dpy, c->win, &wa);
	ocx = wa.x;
	ocy = wa.y;

	if (arg->i == 2) // warp cursor to client center
		XWarpPointer(dpy, None, c->win, 0, 0, 0, 0, WIDTH(c) / 2, HEIGHT(c) / 2);

	if (!getrootptr(&x, &y))
		return;

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

			nx = ocx + (ev.xmotion.x - x);
			ny = ocy + (ev.xmotion.y - y);

			if (!freemove && (abs(nx - ocx) > snap || abs(ny - ocy) > snap))
				freemove = 1;

			if (freemove)
				XMoveWindow(dpy, c->win, nx, ny);

			if ((w = recttows(ev.xmotion.x, ev.xmotion.y, 1, 1)) && w != selws) {
				selws = w;
				selmon = w->mon;
			}

			if (arg->i == 1) { // tiled position is relative to the client window center point
				px = nx + wa.width / 2;
				py = ny + wa.height / 2;
			} else { // tiled position is relative to the mouse cursor
				px = ev.xmotion.x;
				py = ev.xmotion.y;
			}

			r = recttoclient(px, py, 1, 1, 0);

			if (!r || r == c)
				break;

			if ((((float)(r->y + r->h - py) / r->h) > ((float)(r->x + r->w - px) / r->w)
			    	&& (abs(r->y - py) < r->h / 2)) || (abs(r->x - px) < r->w / 2))
				attachmode = AttachAbove;
			else
				attachmode = AttachBelow;

			if ((r && r != prevr) || (attachmode != prevattachmode)) {
				detachstack(c);
				detach(c);
				if (c->ws != r->ws)
					arrangews(c->ws);

				r->ws->sel = r;
				attachx(c, attachmode, r->ws);
				attachstack(c);
				arrangews(r->ws);
				prevr = r;
				prevattachmode = attachmode;
			}
			break;
		}
	} while (ev.type != ButtonRelease);
	XUngrabPointer(dpy, CurrentTime);

	if ((w = recttows(ev.xmotion.x, ev.xmotion.y, 1, 1)) && w != c->ws) {
		detach(c);
		detachstack(c);
		arrangews(c->ws);
		attachx(c, AttachBottom, w);
		attachstack(c);
		selws = w;
		selmon = w->mon;
	}

	focus(c);
	removeflag(c, MovePlace);
	if (nx != -9999)
		resize(c, nx, ny, c->w, c->h, 0);
	arrangews(c->ws);
	if (placeopacity)
		opacity(c, prevopacity);
}
