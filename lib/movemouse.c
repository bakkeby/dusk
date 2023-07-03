void
moveorplace(const Arg *arg)
{
	if (!selws || !selws->sel)
		return;

	if (!selws->layout->arrange || ISFLOATING(selws->sel))
		movemouse(arg);
	else
		placemouse(arg);
}

void
movemouse(const Arg *arg)
{
	int i, g, x, y, w, h, nx, ny, sx, sy, vsnap, hsnap, xoff, yoff, group_after;
	Client *c, *s;
	Workspace *ws;
	Monitor *m;
	XEvent ev;
	Time lasttime = 0;
	double prevopacity;

	if (!(c = selws->sel))
		return;
	if (ISFULLSCREEN(c) && !ISFAKEFULLSCREEN(c)) /* no support moving fullscreen windows by mouse */
		return;

	group_after = c->group;
	if (ISMARKED(c)) {
		ignore_marked = 0; // movemouse supports marked clients
		group(NULL);
	}

	/* Snap girders */
	int gap = gappfl;
	int ngirders = 0;
	int lgirder[100] = {0};
	int rgirder[100] = {0};
	int tgirder[100] = {0};
	int bgirder[100] = {0};

	/* Grouped floating windows */
	Client *group[10] = {c};
	int ngroup = 1;
	int ocx[10] = {c->x};
	int ocy[10] = {c->y};
	int ocw[10] = {WIDTH(c)};
	int och[10] = {HEIGHT(c)};

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
			continue;
		}

		for (s = ws->stack; s; s = s->snext) {
			if ((!ISFLOATING(s) && ws->layout->arrange) || !ISVISIBLE(s) || s == c)
				continue;
			if (c->group && s->group == c->group) {
				group[ngroup] = s;
				ocx[ngroup] = s->x;
				ocy[ngroup] = s->y;
				och[ngroup] = HEIGHT(s);
				ocw[ngroup] = WIDTH(s);
				ngroup++;
				if (moveopacity)
					opacity(s, moveopacity);
				continue;
			}

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

	if (XGrabPointer(dpy, root, False, MOUSEMASK, GrabModeAsync, GrabModeAsync,
		None, cursor[CurMove]->cursor, CurrentTime) != GrabSuccess)
		return;
	if (!getrootptr(&x, &y))
		return;

	for (g = 0; g < ngroup; g++) {
		if (moveopacity) {
			prevopacity = group[0]->opacity;
			opacity(group[g], moveopacity);
		}
		addflag(group[g], MoveResize);
	}

	do {
		XMaskEvent(dpy, MOUSEMASK|ExposureMask|SubstructureRedirectMask, &ev);
		switch (ev.type) {
		case ConfigureRequest:
		case Expose:
		case MapRequest:
			handler[ev.type](&ev);
			break;
		case MotionNotify:
			if ((ev.xmotion.time - lasttime) <= (1000 / dpyHz))
				continue;
			lasttime = ev.xmotion.time;

			sx = nx = ocx[0] + (ev.xmotion.x - x);
			sy = ny = ocy[0] + (ev.xmotion.y - y);
			vsnap = hsnap = snap;

			if (!ISFLOATING(c) && selws->layout->arrange) {
				if (abs(nx - c->x) <= snap && abs(ny - c->y) <= snap)
					continue;
				togglefloating(NULL);
			}

			for (i = 0; i < ngirders; i++) {
				for (g = 0; g < ngroup; g++) {
					xoff = (g == 0 ? 0 : ocx[g] - ocx[0]);
					yoff = (g == 0 ? 0 : ocy[g] - ocy[0]);

					if (abs(lgirder[i] - nx - xoff) < vsnap) {
						sx = lgirder[i] - xoff;
						vsnap = abs(sx - nx);
					}
					if (abs(rgirder[i] - nx - ocw[g] - xoff) < vsnap) {
						sx = rgirder[i] - ocw[g] - xoff;
						vsnap = abs(sx - nx);
					}
					if (abs(tgirder[i] - ny - yoff) < hsnap) {
						sy = tgirder[i] - yoff;
						hsnap = abs(sy - ny);
					}
					if (abs(bgirder[i] - ny - och[g] - yoff) < hsnap) {
						sy = bgirder[i] - och[g] - yoff;
						hsnap = abs(sy - ny);
					}
				}
			}

			nx = sx;
			ny = sy;

			for (g = 0; g < ngroup; g++) {
				xoff = (g == 0 ? 0 : ocx[g] - ocx[0]);
				yoff = (g == 0 ? 0 : ocy[g] - ocy[0]);
				resize(group[g], nx + xoff, ny + yoff, group[g]->w, group[g]->h, 1);
				savefloats(group[g]);
			}
			break;
		}
	} while (ev.type != ButtonRelease);
	XUngrabPointer(dpy, CurrentTime);

	for (g = ngroup - 1; g > -1; g--) {
		c = group[g];

		ws = recttows(c->x, c->y, c->w, c->h);
		if (ws && ISSTICKY(c)) {
			stickyws->mon = ws->mon;
			drawbars();
		} else if (ws && ws != c->ws) {
			detach(c);
			detachstack(c);
			attachx(c, AttachBottom, ws);
			attachstack(c);
			if (g == 0) {
				selws = ws;
				selmon = ws->mon;
			}
		}

		removeflag(c, MoveResize);
		if (moveopacity)
			opacity(c, prevopacity);
		c->group = group_after;
	}

	focus(group[0]);
	drawbars();
}
