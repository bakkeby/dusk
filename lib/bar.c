/* Compile-time check to make sure that the number of bar rules do not exceed the limit */
struct NumBarRules { char TooManyBarRules__Increase_BARRULES_macro_to_fix_this[LENGTH(barrules) > BARRULES ? -1 : 1]; };

void
barhover(XEvent *e, Bar *bar)
{
	if (!bar)
		return;

	const BarRule *br;
	Monitor *m = bar->mon;
	XMotionEvent *ev = &e->xmotion;
	BarArg barg = { 0, 0, 0, 0 };
	int r;

	for (r = 0; r < LENGTH(barrules); r++) {
		br = &barrules[r];
		if (br->bar != bar->idx || (br->monitor == 'A' && m != selmon) || br->hoverfunc == NULL)
			continue;
		if (br->monitor != 'A' && br->monitor != -1 && br->monitor != bar->mon->num)
			continue;
		if (bar->vert && (bar->p[r] > ev->y || ev->y > bar->p[r] + br->lpad + bar->s[r] + br->rpad))
			continue;
		if (!bar->vert && (bar->p[r] > ev->x || ev->x > bar->p[r] + br->lpad + bar->s[r] + br->rpad))
			continue;

		if (bar->vert) {
			barg.x = ev->x - bar->borderpx;
			barg.y = ev->y - (bar->p[r] + br->lpad);
			barg.w = bar->bw - 2 * bar->borderpx;
			barg.h = bar->s[r];
		} else {
			barg.x = ev->x - (bar->p[r] + br->lpad);
			barg.y = ev->y - bar->borderpx;
			barg.w = bar->s[r];
			barg.h = bar->bh - 2 * bar->borderpx;
		}
		barg.lpad = br->lpad;
		barg.rpad = br->rpad;
		barg.value = br->value;

		br->hoverfunc(bar, &barg, ev);
		break;
	}
}

void
barpress(XButtonPressedEvent *ev, Monitor *m, Arg *arg, int *click)
{
	Bar *bar;
	const BarRule *br;
	BarArg barg = { 0, 0, 0, 0 };
	int barclick, r;

	for (bar = selmon->bar; bar; bar = bar->next) {
		if (ev->window == bar->win) {
			for (r = 0; r < LENGTH(barrules); r++) {
				br = &barrules[r];
				if (br->bar != bar->idx || (br->monitor == 'A' && m != selmon) || br->clickfunc == NULL || !bar->s[r])
					continue;
				if (br->monitor != 'A' && br->monitor != -1 && br->monitor != bar->mon->num)
					continue;
				if (bar->vert && (bar->p[r] > ev->y || ev->y > bar->p[r] + br->lpad + bar->s[r] + br->rpad))
					continue;
				if (!bar->vert && (bar->p[r] > ev->x || ev->x > bar->p[r] + br->lpad + bar->s[r] + br->rpad))
					continue;

				if (bar->vert) {
					barg.x = ev->x - bar->borderpx;
					barg.y = ev->y - (bar->p[r] + br->lpad);
					barg.w = bar->bw - 2 * bar->borderpx;
					barg.h = bar->s[r];
				} else {
					barg.x = ev->x - (bar->p[r] + br->lpad);
					barg.y = ev->y - bar->borderpx;
					barg.w = bar->s[r];
					barg.h = bar->bh - 2 * bar->borderpx;
				}
				barg.lpad = br->lpad;
				barg.rpad = br->rpad;
				barg.value = br->value;

				barclick = br->clickfunc(bar, arg, &barg);
				if (barclick > -1)
					*click = barclick;
				return;
			}
			break;
		}
	}
}

void
createbars(Monitor *m)
{
	const BarDef *def;

	for (int i = 0; i < LENGTH(bars); i++) {
		def = &bars[i];
		if (def->monitor == m->num)
			createbar(def, m);
	}
}

void
createbar(const BarDef *def, Monitor *m)
{
	Bar *bar;
	bar = ecalloc(1, sizeof(Bar));
	bar->def = def;
	bar->win = 0;
	bar->mon = m;
	bar->idx = def->idx;
	bar->next = m->bar;
	bar->name = def->name;
	bar->vert = def->vert;
	bar->barpos = def->barpos;
	bar->showbar = 1;
	bar->external = 0;
	bar->borderpx = enabled(BarBorder) ? borderpx : 0;
	m->bar = bar;
}

void
drawbar(Monitor *m)
{
	Bar *bar;
	Workspace *ws;

	if (m->showbar) {
		for (ws = nextvismonws(m, workspaces); ws; ws = nextvismonws(m, ws->next))
			if (hasfullscreen(ws))
				return;
		for (bar = m->bar; bar; bar = bar->next)
			drawbarwin(bar);
	}
}

void
drawbars(void)
{
	Monitor *m;
	for (m = mons; m; m = m->next)
		drawbar(m);
}

void
drawbarwin(Bar *bar)
{
	if (!bar || !bar->win || bar->external)
		return;

	int r, w, mw, total_drawn = 0, idx;
	int rx, lx, rw, lw; // bar size, split between left and right if a center module is added
	const BarRule *br;
	Monitor *lastmon;

	if (enabled(BarActiveGroupBorderColor) && bar->mon->selws)
		bar->scheme = clientscheme(bar->mon->selws->sel, bar->mon->selws->sel);
	else if (enabled(BarMasterGroupBorderColor) && bar->mon->selws)
		bar->scheme = clientscheme(bar->mon->selws->clients, NULL);
	else
		bar->scheme = SchemeNorm;

	if (bar->borderpx) {
		idx = (enabled(BarBorderColBg) ? ColBg : ColBorder);
		XSetForeground(drw->dpy, drw->gc, scheme[bar->scheme][idx].pixel);
		XFillRectangle(drw->dpy, drw->drawable, drw->gc, 0, 0, bar->bw, bar->bh);
	}

	BarArg barg = { 0 };
	barg.h = bar->bh - 2 * bar->borderpx;

	rw = lw = (bar->vert ? bar->bh : bar->bw) - 2 * bar->borderpx;
	rx = lx = bar->borderpx;

	for (lastmon = mons; lastmon && lastmon->next; lastmon = lastmon->next);

	drw_setscheme(drw, scheme[SchemeNorm]);
	drw_rect(drw, lx, bar->borderpx, lw, bar->bh - 2 * bar->borderpx, 1, 1);

	for (r = 0; r < LENGTH(barrules); r++) {
		br = &barrules[r];
		bar->s[r] = 0;
		if (br->bar != bar->idx || !br->sizefunc || (br->monitor == 'A' && bar->mon != selmon))
			continue;
		if (br->monitor != 'A' && br->monitor != -1 && br->monitor != bar->mon->num &&
				!(br->drawfunc == draw_systray && br->monitor > lastmon->num && bar->mon->num == 0)) // hack: draw systray on first monitor if the designated one is not available
			continue;

		barg.lpad = br->lpad;
		barg.rpad = br->rpad;
		barg.value = br->value;
		barg.scheme = (br->scheme > -1 ? br->scheme : SchemeNorm);
		barg.firstscheme = -1;
		barg.lastscheme = -1;

		drw_setscheme(drw, scheme[barg.scheme]);

		mw = (br->alignment < BAR_ALIGN_RIGHT_LEFT ? lw : rw);
		barg.w = MAX(0, mw - br->lpad - br->rpad);
		w = br->sizefunc(bar, &barg);
		bar->s[r] = w = MIN(barg.w, w);

		if (!bar->s[r])
			continue;

		if (w) {
			w += br->lpad;
			if (w + br->rpad <= mw)
				w += br->rpad;
		}

		barg.w = w;

		/* If left is exhausted then switch to right side, and vice versa */
		if (lw <= 0) {
			lw = rw;
			lx = rx;
		} else if (rw <= 0) {
			rw = lw;
			rx = lx;
		}

		switch (br->alignment) {
		default:
		case BAR_ALIGN_NONE:
		case BAR_ALIGN_TOP_TOP:
		case BAR_ALIGN_LEFT_LEFT:
		case BAR_ALIGN_TOP:
		case BAR_ALIGN_LEFT:
			bar->p[r] = lx;
			break;
		case BAR_ALIGN_TOP_BOTTOM:
		case BAR_ALIGN_LEFT_RIGHT:
		case BAR_ALIGN_BOTTOM:
		case BAR_ALIGN_RIGHT:
			bar->p[r] = lx + lw - w;
			break;
		case BAR_ALIGN_TOP_CENTER:
		case BAR_ALIGN_LEFT_CENTER:
		case BAR_ALIGN_CENTER:
			bar->p[r] = lx + lw / 2 - w / 2;
			break;
		case BAR_ALIGN_BOTTOM_TOP:
		case BAR_ALIGN_RIGHT_LEFT:
			bar->p[r] = rx;
			break;
		case BAR_ALIGN_BOTTOM_BOTTOM:
		case BAR_ALIGN_RIGHT_RIGHT:
			bar->p[r] = rx + rw - w;
			break;
		case BAR_ALIGN_BOTTOM_CENTER:
		case BAR_ALIGN_RIGHT_CENTER:
			bar->p[r] = rx + rw / 2 - w / 2;
			break;
		}

		if (br->drawfunc == draw_powerline) {

			if (reducepowerline(bar, r)) {
				bar->s[r] = 0;
				continue;
			}

			/* If the powerline is at the start or end of the bar, then keep the powerline but
			 * reduce the size by half. When drawn this will be made a solid block rather than
			 * slashes or arrows. */
			if (bar->p[r] == bar->borderpx)
				barg.w = w = bar->s[r] = bar->s[r] / 2;
			else if (bar->p[r] + bar->s[r] + bar->borderpx == bar->bw) {
				bar->p[r] += bar->s[r] / 2 + bar->s[r] % 2;
				barg.w = w = bar->s[r] = bar->s[r] / 2;
			}
		}

		switch (br->alignment) {
		default:
		case BAR_ALIGN_NONE:
		case BAR_ALIGN_TOP_TOP:
		case BAR_ALIGN_LEFT_LEFT:
		case BAR_ALIGN_TOP:
		case BAR_ALIGN_LEFT:
			if (lx == rx) {
				rx += w;
				rw -= w;
			}
			lx += w;
			lw -= w;
			break;
		case BAR_ALIGN_TOP_BOTTOM:
		case BAR_ALIGN_LEFT_RIGHT:
		case BAR_ALIGN_BOTTOM:
		case BAR_ALIGN_RIGHT:
			if (lx == rx)
				rw -= w;
			lw -= w;
			break;
		case BAR_ALIGN_TOP_CENTER:
		case BAR_ALIGN_LEFT_CENTER:
		case BAR_ALIGN_CENTER:
			if (lx == rx) {
				rw = rx + rw - bar->p[r] - w;
				rx = bar->p[r] + w;
			}
			lw = bar->p[r] - lx;
			break;
		case BAR_ALIGN_BOTTOM_TOP:
		case BAR_ALIGN_RIGHT_LEFT:
			if (lx == rx) {
				lx += w;
				lw -= w;
			}
			rx += w;
			rw -= w;
			break;
		case BAR_ALIGN_BOTTOM_BOTTOM:
		case BAR_ALIGN_RIGHT_RIGHT:
			if (lx == rx)
				lw -= w;
			rw -= w;
			break;
		case BAR_ALIGN_BOTTOM_CENTER:
		case BAR_ALIGN_RIGHT_CENTER:
			if (lx == rx) {
				lw = lx + lw - bar->p[r] + w;
				lx = bar->p[r] + w;
			}
			rw = bar->p[r] - rx;
			break;
		}

		if (bar->vert) {
			barg.x = bar->borderpx + 5;
			barg.y = bar->p[r];
			barg.h = barg.w;
			barg.w = bar->bw - 2 * bar->borderpx;
		} else {
			barg.x = bar->p[r];
			barg.y = bar->borderpx;
			barg.h = bar->bh - 2 * bar->borderpx;
		}

		if (br->drawfunc && br->drawfunc != draw_powerline) {
			total_drawn += br->drawfunc(bar, &barg);
			bar->sscheme[r] = (barg.firstscheme != -1 ? barg.firstscheme : barg.scheme);
			bar->escheme[r] = (barg.lastscheme != -1 ? barg.lastscheme : barg.scheme);
		}
	}

	/* Draw powerline separators */
	for (r = 0; r < LENGTH(barrules); r++) {
		br = &barrules[r];
		if (!bar->s[r] || br->drawfunc != draw_powerline)
			continue;

		barg.lpad = br->lpad;
		barg.rpad = br->rpad;
		barg.value = br->value;
		barg.scheme = (br->scheme > -1 ? br->scheme : SchemeNorm);
		barg.firstscheme = schemeleftof(bar, r);
		barg.lastscheme = schemerightof(bar, r);
		barg.x = bar->p[r] + br->lpad;
		barg.y = bar->borderpx;
		barg.h = bar->bh - 2 * bar->borderpx;
		barg.w = bar->s[r];

		br->drawfunc(bar, &barg);
	}

	if (total_drawn == 0 && bar->showbar) {
		bar->showbar = 0;
		updatebarpos(bar->mon);
		showhidebar(bar);
		setworkspaceareasformon(bar->mon);
		arrangemon(bar->mon);
	} else if (total_drawn > 0 && !bar->showbar) {
		bar->showbar = 1;
		updatebarpos(bar->mon);
		showhidebar(bar);
		drw_map(drw, bar->win, 0, 0, bar->bw, bar->bh);
		setworkspaceareasformon(bar->mon);
		arrangemon(bar->mon);
	} else {
		drw_map(drw, bar->win, 0, 0, bar->bw, bar->bh);
	}
}

void
drawbarmodule(const BarRule *br, int r)
{
	Monitor *m;
	Bar *bar;
	BarArg barg = { 0 };
	barg.lpad = br->lpad;
	barg.rpad = br->rpad;
	barg.value = br->value;
	barg.scheme = (br->scheme > -1 ? br->scheme : SchemeNorm);

	for (m = mons; m; m = m->next) {
		if ((br->monitor > -1 && br->monitor != m->num) || !m->showbar)
			continue;
		for (bar = m->bar; bar; bar = bar->next) {
			if ((br->bar > -1 && br->bar != bar->idx) || bar->external)
				continue;

			if (bar->vert) {
				barg.x = bar->borderpx + 5;
				barg.y = bar->p[r];
				barg.h = bar->s[r] + barg.lpad + barg.rpad;
				barg.w = bar->bw - 2 * bar->borderpx;
			} else {
				barg.y = bar->borderpx;
				barg.x = bar->p[r];
				barg.w = bar->s[r] + barg.lpad + barg.rpad;
				barg.h = bar->bh - 2 * bar->borderpx;
			}

			/* Optimisation, if the bar module size has not changed then we can just
			   update the designated part of the bar rather than drawing the entire
			   bar, otherwise only update the bars that have this module. */
			if (bar->s[r] == br->sizefunc(bar, &barg)) {
				if (!bar->s[r])
					continue;
				br->drawfunc(bar, &barg);
				drw_map(drw, bar->win, barg.x, barg.y, barg.w, barg.h);
			} else {
				drawbarwin(bar);
			}
		}
	}
}

void
updatebarpos(Monitor *m)
{
	Bar *bar;
	for (bar = m->bar; bar; bar = bar->next)
		setbarpos(bar);
	reducewindowarea(m);
}

void
setbarpos(Bar *bar)
{
	if (!bar)
		return;

	char xCh, yCh, wCh, hCh;
	float w, h;
	float x, y;

	int y_pad = (enabled(BarPadding) && !bar->external ? vertpad : 0);
	int x_pad = (enabled(BarPadding) && !bar->external ? sidepad : 0);
	Monitor *m = bar->mon;

	switch (sscanf(bar->barpos, "%f%c %f%c %f%c %f%c", &x, &xCh, &y, &yCh, &w, &wCh, &h, &hCh)) {
	case 8:
		// all good
		break;
	default:
		fprintf(stderr, "Bar %s (%d) on monitor %d, bad barpos '%s' - can't place bar\n", bar->name, bar->idx, bar->mon->num, bar->barpos);
		return;
	}

	bar->bx = m->mx + x_pad;
	bar->by = m->my + y_pad;
	if (bar->external) {
		getbarsize(bar, &bar->bw, &bar->bh);
	} else if (bar->vert) {
		bar->bh = m->mh - 2 * y_pad;
		bar->bw = bh + bar->borderpx * 2;
	} else {
		bar->bh = bh + bar->borderpx * 2;
		bar->bw = m->mw - 2 * x_pad;
	}

	if (wCh == '%') {
		bar->bw = (m->mw - 2 * x_pad) * w / 100;
	} else if (wCh == 'w') {
		if (w > 0)
			bar->bw = w;
	}
	if (hCh == '%') {
		bar->bh = (m->mh - 2 * y_pad) * h / 100;
	} else if (hCh == 'h') {
		if (h > 0)
			bar->bh = h;
	}

	if (xCh == '%') {
		bar->bx += (m->mw - 2 * x_pad - bar->bw) * x / 100;
	} else if (xCh == 'x') {
		if (x >= 0)
			bar->bx = m->mx + x;
	}

	if (yCh == '%') {
		bar->by += (m->mh - 2 * y_pad - bar->bh) * y / 100;
	} else if (yCh == 'y') {
		if (y >= 0)
			bar->by = m->my + y;
	}
}

void
getbarsize(Bar *bar, int *w, int *h)
{
	XWindowAttributes wa;

	if (!bar->win)
		return;

	if (!XGetWindowAttributes(dpy, bar->win, &wa))
		return;

	*w = wa.width;
	*h = wa.height;
}

void
recreatebar(Bar *bar)
{
	const BarDef *def = bar->def;
	Monitor *m = bar->mon;
	int setsystraybar = (systray && bar == systray->bar);
	removebar(bar);
	createbar(def, m);

	if (setsystraybar)
		systray->bar = m->bar;

	updatebarpos(m);
	updatebars();
}

void
reducewindowarea(Monitor *m)
{
	Bar *bar;

	m->wx = m->mx;
	m->wy = m->my;
	m->ww = m->mw;
	m->wh = m->mh;

	for (bar = m->bar; bar; bar = bar->next) {
		if (!bar->showbar || !bar->mon->showbar)
			continue;

		if (bar->vert) { // vertical bar
			if (bar->bx < m->mx + m->mw / 2) { // left aligned
				if (m->wx < bar->bx + bar->bw) {
					m->ww -= (bar->bx + bar->bw - m->wx);
					m->wx = bar->bx + bar->bw;
				}
			} else { // right aligned
				if (bar->bx < m->wx + m->ww) {
					m->ww = bar->bx - m->wx;
				}
			}
		} else { // horizontal bar
			if (bar->by < m->my + m->mh / 2) { // top bar
				if (m->wy < bar->by + bar->bh) {
					m->wh -= (bar->by + bar->bh - m->wy);
					m->wy = bar->by + bar->bh;
				}
			} else { // bottom bar
				if (bar->by < m->wy + m->wh) {
					m->wh = bar->by - m->wy;
				}
			}
		}
	}
}

void
removebar(Bar *bar)
{
	Bar *b;
	Monitor *m = bar->mon;

	for (b = m->bar; b; b = bar->next) {
		if (b == bar)
			m->bar = bar->next;
		else if (b->next == bar)
			b->next = bar->next;
		else
			continue;

		if (!bar->external) {
			XUnmapWindow(dpy, bar->win);
			XDestroyWindow(dpy, bar->win);
		}
		if (systray && bar == systray->bar)
			systray->bar = NULL;
		free(bar);
		break;
	}
}

void
updatebars(void)
{
	Bar *bar;
	Monitor *m;
	XSetWindowAttributes wa = {
		.override_redirect = True,
		.background_pixel = 0,
		.border_pixel = 0,
		.colormap = cmap,
		.event_mask = ButtonPressMask|ExposureMask|PointerMotionMask
	};
	XClassHint ch = {"dusk", "dusk"};
	for (m = mons; m; m = m->next) {
		for (bar = m->bar; bar; bar = bar->next) {
			if (bar->external)
				continue;
			if (!bar->win) {
				bar->win = XCreateWindow(dpy, root, bar->bx, bar->by, bar->bw, bar->bh, 0, depth,
					InputOutput, visual,
					CWOverrideRedirect|CWBackPixel|CWBorderPixel|CWColormap|CWEventMask, &wa);
				restackwin(bar->win, Above, wmcheckwin);
				XDefineCursor(dpy, bar->win, cursor[CurNormal]->cursor);
				XSetClassHint(dpy, bar->win, &ch);
				XMapWindow(dpy, bar->win);
			}
		}
	}
}

void
showbar(const Arg *arg)
{
	if (!selmon->showbar) {
		togglebar(arg);
		selmon->showbar = 2;
	}
}

void
hidebar(const Arg *arg)
{
	if (selmon->showbar == 2) {
		selmon->showbar = 1;
		togglebar(arg);
	}
}

void
showhidebar(Bar *bar)
{
	int x = bar->bx;
	int y = bar->by;
	int w = bar->bw;
	int h = bar->bh;

	if (!bar->showbar || !bar->mon->showbar) {
		if (bar->vert) {
			x = -bar->bw;
		} else {
			y = -bar->bh;
		}
	}

	XMoveResizeWindow(dpy, bar->win, x, y, w, h);
}

void
teardownbars(Monitor *m)
{
	Bar *bar, *next;

	for (bar = m->bar; bar; bar = next) {
		next = bar->next;

		if (!bar->external) {
			XUnmapWindow(dpy, bar->win);
			XDestroyWindow(dpy, bar->win);
		}

		if (systray && bar == systray->bar)
			systray->bar = NULL;
		free(bar);
	}

	m->bar = NULL;
}

void
togglebar(const Arg *arg)
{
	Bar *bar;
	selmon->showbar = (selmon->showbar == 2 ? 1 : !selmon->showbar);
	updatebarpos(selmon);
	for (bar = selmon->bar; bar; bar = bar->next)
		showhidebar(bar);
	if (!selmon->showbar && systray)
		XMoveWindow(dpy, systray->win, -32000, -32000);
	setworkspaceareasformon(selmon);
	arrangemon(selmon);
	drawbars();
}

void
togglebarpadding(const Arg *arg)
{
	Bar *bar;
	togglefunc(BarPadding);
	for (Monitor *m = mons; m; m = m->next) {
		updatebarpos(m);
		for (bar = m->bar; bar; bar = bar->next)
			showhidebar(bar);
		setworkspaceareasformon(m);
		drawbar(m);
		arrangemon(m);
	}
}

void
togglecompact(const Arg *arg)
{
	Bar *bar;
	Monitor *m;
	Workspace *ws;
	int gaps = 1, px = enabled(BarBorder) ? borderpx : 0;

	if (enabled(BarPadding))
		gaps = px = 0;

	for (ws = workspaces; ws; ws = ws->next)
		ws->enablegaps = gaps;

	for (m = mons; m; m = m->next)
		for (bar = m->bar; bar; bar = bar->next)
			bar->borderpx = px;

	togglebarpadding(arg);
}

Bar *
wintobar(Window win)
{
	Monitor *m;
	Bar *bar;
	for (m = mons; m; m = m->next)
		for (bar = m->bar; bar; bar = bar->next)
			if (bar->win == win)
				return bar;
	return NULL;
}

Bar *
mapexternalbar(Window win)
{
	Monitor *m;
	Bar *bar;

	for (m = mons; m; m = m->next) {
		for (bar = m->bar; bar; bar = bar->next) {
			if (matchextbar(bar, win)) {
				if (bar->win && bar->win != win) {
					XUnmapWindow(dpy, bar->win);
					XDestroyWindow(dpy, bar->win);
				}

				bar->win = win;
				bar->showbar = 1;
				bar->external = 1;
				bar->borderpx = 0;
				updatebarpos(m);
				showhidebar(bar);
				setworkspaceareasformon(m);
				restackwin(win, Above, wmcheckwin);
				XMapWindow(dpy, win);
				restack(m->selws);
				arrangemon(m);
				drawbars();
				return bar;
			}
		}
	}
	return NULL;
}

int
matchextbar(Bar *bar, Window win)
{
	if (bar->win == win)
		return 1;

	const BarDef *def = bar->def;

	if (def->extclass == NULL && def->extinstance == NULL && def->extname == NULL)
		return 0;

	const char *class, *instance;
	XClassHint ch = { NULL, NULL };
	if (!XGetClassHint(dpy, win, &ch))
		return 0;

	int matched = 0;
	class    = ch.res_class ? ch.res_class : broken;
	instance = ch.res_name  ? ch.res_name  : broken;
	char name[256] = {0};
	if (!gettextprop(win, netatom[NetWMName], name, sizeof name))
		gettextprop(win, XA_WM_NAME, name, sizeof name);

	if (enabled(Debug)) {
		fprintf(stderr, "matchextbar: checking new window %s (%ld), class = '%s', instance = '%s'\n", name, win, class, instance);
		fprintf(stderr, "             against external bar %d monitor %d class = '%s', instance = '%s', name = '%s'\n",
			def->idx,
			def->monitor,
			NVL(def->extclass, "NULL"),
			NVL(def->extinstance, "NULL"),
			NVL(def->extname, "NULL")
		);
	}

	if (def->extclass != NULL && strcmp(def->extclass, class))
		goto bail;

	if (def->extinstance != NULL && strcmp(def->extinstance, instance))
		goto bail;

	if (def->extname != NULL && strcmp(def->extname, name))
		goto bail;

	matched = 1;

bail:
	if (ch.res_class)
		XFree(ch.res_class);
	if (ch.res_name)
		XFree(ch.res_name);

	if (enabled(Debug)) {
		fprintf(stderr, "matchextbar: window %ld %s a match for external bar %d monitor %d\n", win, matched ? "is" : "is not", def->idx, def->monitor);
	}

	return matched;
}
