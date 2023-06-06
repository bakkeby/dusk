void
resizemouse(const Arg *arg)
{
	int ocx, ocy, h, w, nw, nh, sw, sh;
	int opx, opy, och, ocw, nx, ny, sx, sy;
	int horizcorner, vertcorner, vsnap, hsnap;
	int i;
	unsigned int dui;
	Window dummy;
	Client *c, *s;
	Monitor *m;
	XEvent ev;
	Workspace *ws;
	Time lasttime = 0;
	double prevopacity;

	if (!(c = selws->sel))
		return;
	if (ISFULLSCREEN(c) && !ISFAKEFULLSCREEN(c)) /* no support resizing fullscreen windows by mouse */
		return;

	/* Snap girders */
	int gap = gappfl;
	int ngirders = 0;
	int lgirder[100] = {0};
	int rgirder[100] = {0};
	int tgirder[100] = {0};
	int bgirder[100] = {0};

	for (m = mons; m; m = m->next) {
		lgirder[ngirders] = m->mx + (enabled(BarPadding) ? sidepad : 0);
		rgirder[ngirders] = m->mx + m->mw - (enabled(BarPadding) ? sidepad : 0);
		tgirder[ngirders] = m->my + (enabled(BarPadding) ? vertpad : 0);
		bgirder[ngirders] = m->my + m->mh - (enabled(BarPadding) ? vertpad : 0);
		ngirders++;
	}

	for (ws = workspaces; ws; ws = ws->next) {

		if (!ws->visible)
			continue;

		lgirder[ngirders] = ws->wx + gappov;
		rgirder[ngirders] = ws->wx + ws->ww - gappov;
		tgirder[ngirders] = ws->wy + gappoh;
		bgirder[ngirders] = ws->wy + ws->wh - gappoh;
		ngirders++;

		if (disabled(SnapToWindows) || arg->i == 11) {
			if (ws == stickyws)
				break;
			continue;
		}

		for (s = ws->stack; s; s = s->snext) {
			if ((!ISFLOATING(s) && ws->layout->arrange) || !ISVISIBLE(s) || s == c)
				continue;

			h = HEIGHT(s);
			w = WIDTH(s);
			lgirder[ngirders] = s->x;
			rgirder[ngirders] = s->x + w;
			tgirder[ngirders] = s->y;
			bgirder[ngirders] = s->y + h;
			ngirders++;
			lgirder[ngirders] = s->x + w + gap;
			rgirder[ngirders] = s->x - gap;
			tgirder[ngirders] = s->y + h + gap;
			bgirder[ngirders] = s->y - gap;
			ngirders++;
		}
	}

	if (resizeopacity) {
		prevopacity = c->opacity;
		opacity(c, resizeopacity);
	}

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

			sx = nx = horizcorner ? (ocx + ev.xmotion.x - opx) : c->x;
			sy = ny = vertcorner ? (ocy + ev.xmotion.y - opy) : c->y;
			sw = nw = MAX(horizcorner ? (ocx + ocw - nx) : (ocw + (ev.xmotion.x - opx)), 1);
			sh = nh = MAX(vertcorner ? (ocy + och - ny) : (och + (ev.xmotion.y - opy)), 1);
			vsnap = hsnap = snap;

			if (c->ws->mon->wx + nw >= selmon->wx && c->ws->mon->wx + nw <= selmon->wx + selmon->ww
			&& c->ws->mon->wy + nh >= selmon->wy && c->ws->mon->wy + nh <= selmon->wy + selmon->wh)
			{
				if (!ISFLOATING(c) && selws->layout->arrange
				&& (abs(nw - c->w) > snap || abs(nh - c->h) > snap))
					togglefloating(NULL);
			}

			for (i = 0; i < ngirders; i++) {
				if (horizcorner && abs(lgirder[i] - nx) < vsnap) {
					sx = lgirder[i];
					sw = nw + nx - sx;
					vsnap = abs(sx - nx);
				}
				if (!horizcorner && abs(rgirder[i] - nx - nw) < vsnap) {
					sx = nx;
					sw = rgirder[i] - nx - 2 * c->bw;
					vsnap = abs(sw - nw);
				}
				if (vertcorner && abs(tgirder[i] - ny) < hsnap) {
					sy = tgirder[i];
					sh = nh + ny - sy;
					hsnap = abs(sy - ny);
				}
				if (!vertcorner && abs(bgirder[i] - ny - nh) < hsnap) {
					sy = ny;
					sh = bgirder[i] - ny - 2 * c->bw;
					hsnap = abs(sh - nh);
				}
			}

			nx = sx;
			ny = sy;
			nw = sw;
			nh = sh;

			if (!selws->layout->arrange || ISFLOATING(c)) {
				resize(c, nx, ny, nw, nh, 1);
				savefloats(c);
			}
			break;
		}
	} while (ev.type != ButtonRelease);

	XUngrabPointer(dpy, CurrentTime);
	skipfocusevents();

	if (!ISSTICKY(c) && (ws = recttows(c->x, c->y, c->w, c->h)) && ws != selws) {
		detach(c);
		detachstack(c);
		attachx(c, AttachBottom, ws);
		attachstack(c);
		selws = ws;
		selmon = ws->mon;
		focus(c);
	}
	removeflag(c, MoveResize);
	if (resizeopacity)
		opacity(c, prevopacity);
}

void
resizeorfacts(const Arg *arg)
{
	if (!selws || !selws->sel)
		return;

	if (!selws->layout->arrange || ISFLOATING(selws->sel))
		resizemouse(arg);
	else
		dragfact(arg);
}
