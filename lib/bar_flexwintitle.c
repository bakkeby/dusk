int firstpwlwintitle = 0;
int prevscheme = 0;

int
size_flexwintitle(Bar *bar, BarArg *a)
{
	if (!bar->mon->selws->visible)
		return 0;
	return a->w;
}

int
draw_flexwintitle(Bar *bar, BarArg *a)
{
	if (!bar->mon->selws->visible)
		return 0;
	return flextitlecalculate(bar, a->x + a->lpad, a->w - a->lpad - a->rpad, -1, flextitledraw, NULL, a);
}

int
click_flexwintitle(Bar *bar, Arg *arg, BarArg *a)
{
	if (!bar->mon->selws->visible)
		return 0;
	if (flextitlecalculate(bar, 0, a->w, a->x, flextitleclick, arg, a))
		return ClkWinTitle;
	return -1;
}

void
getclientcounts(Workspace *ws, int *n, int *clientsnmaster, int *clientsnstack, int *clientsnstack2, int *clientsnfloating, int *clientsnhidden)
{
	Client *c;
	int cm = 0, cs1 = 0, cs2 = 0, cf = 0, ch = 0;

	for (c = (ws ? ws->clients : NULL); c; c = c->next) {
		if (ISINVISIBLE(c))
			continue;
		if (SKIPTASKBAR(c))
			continue;
		if (HIDDEN(c)) {
			if (flexwintitle_hiddenweight)
				ch++;
			continue;
		}
		if (!ISTILED(c)) {
			if (flexwintitle_floatweight)
				cf++;
			continue;
		}

		switch (c->area) {
		case MASTER:
			if (flexwintitle_masterweight)
				++cm;
			break;
		case STACK:
			if (flexwintitle_stackweight)
				++cs1;
			break;
		case STACK2:
			if (flexwintitle_stackweight)
				++cs2;
			break;
		}
	}
	*n = cm + cs1 + cs2 + cf + ch;

	*clientsnmaster = cm;
	*clientsnstack = cs1;
	*clientsnstack2 = cs2;
	*clientsnfloating = cf;
	*clientsnhidden = ch;
}

int
isdualstacklayout(Workspace *ws)
{
	if (!ws || ws->layout->arrange != &flextile)
		return 0;

	int layout = ws->ltaxis[LAYOUT];
	if (layout < 0)
		layout *= -1;

	return (
		layout == SPLIT_HORIZONTAL_DUAL_STACK ||
		layout == SPLIT_HORIZONTAL_DUAL_STACK_FIXED ||
		layout == SPLIT_VERTICAL_DUAL_STACK ||
		layout == SPLIT_VERTICAL_DUAL_STACK_FIXED
	);
}

int
iscenteredlayout(Workspace *ws, int n)
{
	if (!ws || ws->layout->arrange != &flextile)
		return 0;

	int layout = ws->ltaxis[LAYOUT];
	if (layout < 0)
		layout *= -1;

	return (
			(layout == SPLIT_CENTERED_VERTICAL && (n - ws->nmaster > 1)) ||
			layout == SPLIT_CENTERED_VERTICAL_FIXED ||
			(layout == SPLIT_CENTERED_HORIZONTAL && (n - ws->nmaster > 1)) ||
			layout == SPLIT_CENTERED_HORIZONTAL_FIXED ||
			layout == FLOATING_MASTER
	);
}

int
ismirroredlayout(Workspace *ws)
{
	if (!ws || ws->layout->arrange != &flextile)
		return 0;

	return ws->ltaxis[LAYOUT] < 0;
}

void
flextitledrawarea(Workspace *ws, Client *c, int x, int w, int num_clients, int titlescheme, int draw_tiled, int draw_hidden, int draw_floating,
	int passx, void(*tabfn)(Workspace *, Client *, int, int, int, int, Arg *arg, BarArg *barg), Arg *arg, BarArg *barg)
{
	int i, rw, cw, padw;
	int drawpowerline = barg->value;
	int sepw = drawpowerline ? drw->fonts->h / 2 + 1 : flexwintitle_separator;
	int num_separators = (num_clients - (!drawpowerline || firstpwlwintitle ? 1 : 0));
	cw = (w - sepw * num_separators) / num_clients;
	rw = (w - sepw * num_separators) % num_clients;

	/* If we have run out of space to draw something, then just fill the remaining background */
	if (cw <= sepw) {
		XSetForeground(drw->dpy, drw->gc, drw->scheme[ColBg].pixel);
		XFillRectangle(drw->dpy, drw->drawable, drw->gc, x, barg->y, w, barg->h);
		return;
	}

	for (i = 0; c && i < num_clients; c = c->next) {
		if (
			!SKIPTASKBAR(c) &&
			!ISINVISIBLE(c) &&
			(
				(draw_tiled && ISTILED(c) && !HIDDEN(c)) ||
				(draw_floating && !ISTILED(c) && !HIDDEN(c)) ||
				(draw_hidden && HIDDEN(c))
			)
		) {
			padw = (!drawpowerline || firstpwlwintitle ? 0 : sepw);
			tabfn(ws, c, passx, x + padw, cw + (i < rw ? 1 : 0), titlescheme, arg, barg);

			if (drawpowerline && tabfn == flextitledraw && !firstpwlwintitle && prevscheme > -1 && barg->lastscheme > -1) {
				drw_arrow(drw, x, barg->y, sepw, barg->h, barg->value, scheme[prevscheme][ColBg], scheme[barg->lastscheme][ColBg], scheme[SchemeNorm][ColBg]);
			}

			firstpwlwintitle = 0;
			if (drawpowerline)
				x += cw + (i < rw ? 1 : 0) + padw;
			else
				x += cw + (i < rw ? 1 : 0) + flexwintitle_separator;
			i++;
		}
	}
}

void
flextitledraw(Workspace *ws, Client *c, int unused, int x, int w, int tabscheme, Arg *arg, BarArg *barg)
{
	if (!c)
		return;

	if (w <= 0)
		return;

	/* icon padding, left padding */
	int ipad = enabled(WinTitleIcons) && c->icon ? c->icw + iconspacing : 0;
	int lpad = 0;
	int tx = x;
	int tw = w;
	int icon2dwidth = 0;
	int titlewidth;
	int max_items = 4;
	int order[max_items];
	int idx = 0;
	int i;
	char *name = (enabled(AltWindowTitles) && c->altname[0] ? c->altname : c->name);
	titlewidth = TEXTW(name);

	const StackerIcon *stackericon = NULL;
	static int textw_single_char = 0;
	if (!textw_single_char)
		textw_single_char = TEXTW("A");

	if (enabled(StackerIcons) && c->ws == selws && ISVISIBLE(c)) {
		if ((stackericon = getstackericonforclient(c))) {
			icon2dwidth = TEXT2DW(stackericon->icon);
		}
	}

	/* This works out the colour scheme to use and fills the background with a solid block */
	prevscheme = barg->lastscheme;
	barg->lastscheme = clientscheme(c, c->ws->sel);

	drw_setscheme(drw, scheme[barg->lastscheme]);

	if (barg->firstscheme == -1)
		barg->firstscheme = barg->lastscheme;

	if (barg->lastscheme != SchemeMarked && c != ws->sel)
		c->scheme = barg->lastscheme;

	XSetForeground(drw->dpy, drw->gc, drw->scheme[ColBg].pixel);
	XFillRectangle(drw->dpy, drw->drawable, drw->gc, x, barg->y, w, barg->h);

	/* This works out how much padding to add on the left side before drawing the window title */
	if (w < textw_single_char + lrpad) { // reduce text padding if title is too small
		lpad = MAX(0, (w - textw_single_char) / 2);
		tx += lpad;
		tw -= lpad;
	} else if (enabled(CenteredWindowName) && titlewidth + icon2dwidth + lrpad + ipad < w) {
		lpad = (w - titlewidth - icon2dwidth - ipad) / 2;
		tx += lpad;
		tw -= lpad;
	} else {
		tx += lrpad / 2;
		tw -= lrpad;
	}

	enum {
		window_icon = 1,
		icon_spacing = 2,
		stacker_icon = 3,
		window_title = 4,
	};

	/* The below determines the order of the window title components */
	if (stackericon && stackericon->pos == StackerLeftOfWindowIcon)
		order[idx++] = stacker_icon;

	if (ipad)
		order[idx++] = window_icon;

	if (stackericon && stackericon->pos == StackerRightOfWindowIcon) {
		order[idx++] = stacker_icon;
		order[idx++] = icon_spacing;
	} else if (ipad) {
		order[idx++] = icon_spacing;
	}

	if (stackericon && stackericon->pos == StackerTitlePrefix)
		order[idx++] = stacker_icon;

	order[idx++] = window_title;

	if (stackericon && stackericon->pos == StackerTitleSuffix)
		order[idx++] = stacker_icon;

	if (stackericon && stackericon->pos == StackerTitleEllipsis)
		order[idx++] = stacker_icon;

	/* Then loop through and draw the title components in the given order */
	for (i = 0; i < max_items && order[i]; i++) {

		switch (order[i]) {
		case window_icon:
			drw_pic(drw, tx, barg->y + (barg->h - c->ich) / 2, c->icw, c->ich, c->icon);
			tx += c->icw;
			tw -= c->icw;
			break;
		case icon_spacing:
			tx += iconspacing;
			tw -= iconspacing;
			break;
		case window_title:
			drw_text(drw, tx, barg->y, tw, barg->h, 0, name, 0, 1);
			tx += titlewidth;
			tw -= titlewidth;
			break;
		case stacker_icon:
			if (tw < textw_single_char) {
				if (stackericon->pos != StackerTitleEllipsis)
					break;
				tx = x + w - icon2dwidth;
				tw = icon2dwidth;
			}

			drw_2dtext(drw, tx, barg->y, tw, barg->h, 0, stackericon->icon, 0, 1, barg->lastscheme);
			tx += icon2dwidth;
			tw -= icon2dwidth;
			break;
		}

	}

	drawstateindicator(ws, c, 1, x, barg->y, w, barg->h, 0, 0);
}

void
flextitleclick(Workspace *ws, Client *c, int passx, int x, int w, int unused, Arg *arg, BarArg *barg)
{
	if (passx >= x && passx <= x + w)
		arg->v = c;
}

int
flextitlecalculate(
	Bar *bar, int offx, int tabw, int passx,
	void(*tabfn)(Workspace *, Client *, int, int, int, int, Arg *arg, BarArg *barg),
	Arg *arg, BarArg *a
) {
	Workspace *ws = bar->mon->selws;
	firstpwlwintitle = 1;

	int drawpowerline = a->value;
	int sepw = drawpowerline ? 0 : flexwintitle_separator;
	int n, center = 0, mirror = 0; // layout configuration
	int clientsnmaster = 0, clientsnstack = 0, clientsnstack2 = 0, clientsnfloating = 0, clientsnhidden = 0;
	int i, w, r, x, den;
	int rw, rr;

	int mas_w = 0, st1_w = 0, st2_w = 0, hid_w = 0, flt_w = 0;
	int order[5] = { 1, 2, 3, 4, 5 };

	if (!ws)
		return 0;

	/* This avoids drawing a separator on the left hand side of the wintitle section if
	 * there is a border and the wintitle module rests at the left border. */
	if (!drawpowerline && a->x > bar->borderpx) {
		offx += flexwintitle_separator;
		tabw -= flexwintitle_separator;
	}

	/* This avoids drawing a separator on the right hand side of the wintitle section if
	 * there is a border and the wintitle module rests at the right border. */
	if (!drawpowerline && a->x + a->w < bar->bw - bar->borderpx)
		tabw -= flexwintitle_separator;

	getclientcounts(ws, &n, &clientsnmaster, &clientsnstack, &clientsnstack2, &clientsnfloating, &clientsnhidden);

	if (n == 0)
	 	return 0;

	if (ws->layout->arrange == &flextile) {
		mirror = ismirroredlayout(ws);
		center = iscenteredlayout(ws, n);
	}

	XSetForeground(drw->dpy, drw->gc, scheme[bar->scheme][ColBorder].pixel);
	XFillRectangle(drw->dpy, drw->drawable, drw->gc, a->x + a->lpad, a->y, a->w - a->lpad - a->rpad, a->h);

	/* floating mode - all window titles are allocated equal width */
	if (!ws->layout->arrange) {
		flextitledrawarea(
			ws,
			ws->clients,
			offx,
			tabw,
			n,
			0,
			flexwintitle_masterweight + flexwintitle_stackweight,
			flexwintitle_hiddenweight,
			flexwintitle_floatweight,
			passx,
			tabfn,
			arg,
			a
		);
		return 1;
	}

	/* tiled mode */
	den = clientsnmaster * flexwintitle_masterweight
	    + (clientsnstack + clientsnstack2) * flexwintitle_stackweight
	    + clientsnhidden * flexwintitle_hiddenweight
	    + clientsnfloating * flexwintitle_floatweight;

	/* Return if there is nothing to draw */
	if (!den) {
		return 1;
	}

	w = (tabw - (n - 1) * sepw) / den;
	r = (tabw - (n - 1) * sepw) % den;

	rw = r / n; // rest incr per client
	rr = r % n; // rest rest

	if (mirror) {
		if (!center && clientsnstack2) {
			order[0] = 3;
			order[1] = 2;
			order[2] = 1;
		} else {
			order[0] = 2;
			order[1] = 1;
		}
	} else if (center && clientsnstack2) {
		order[0] = 3;
		order[1] = 1;
		order[2] = 2;
	}

	if (clientsnmaster)
		mas_w = clientsnmaster * rw + w * clientsnmaster * flexwintitle_masterweight + (rr > 0 ? MIN(rr, clientsnmaster) : 0) + (clientsnmaster - 1) * sepw;
	rr -= clientsnmaster;
	if (clientsnstack)
		st1_w = clientsnstack * (rw + w * flexwintitle_stackweight) + (rr > 0 ? MIN(rr, clientsnstack) : 0) + (clientsnstack - 1) * sepw;
	rr -= clientsnstack;
	if (clientsnstack2)
		st2_w = clientsnstack2 * (rw + w * flexwintitle_stackweight) + (rr > 0 ? MIN(rr, clientsnstack2) : 0) + (clientsnstack2 - 1) * sepw;
	rr -= clientsnstack2;
	if (clientsnhidden)
		hid_w = clientsnhidden * (rw + w * flexwintitle_hiddenweight) + (rr > 0 ? MIN(rr, clientsnhidden) : 0) + (clientsnhidden - 1) * sepw;
	rr -= clientsnhidden;
	if (clientsnfloating)
		flt_w = clientsnfloating * (rw + w * flexwintitle_floatweight) + (rr > 0 ? MIN(rr, clientsnfloating) : 0) + (clientsnfloating - 1) * sepw;
	if (rr > 0)
		mas_w += rr;

	for (i = 0, x = offx; i < LENGTH(order); i++) {
		w = 0;
		switch (order[i]) {
		case 1:
			if (clientsnmaster) {
				w = mas_w;
				flextitledrawarea(ws, ws->clients, x, w, clientsnmaster, 0, 1, 0, 0, passx, tabfn, arg, a); // master
			}
			break;
		case 2:
			if (clientsnstack) {
				w = st1_w;
				flextitledrawarea(ws, nthtiled(ws->clients, clientsnmaster + 1, 0), x, w, clientsnstack, 0, 1, 0, 0, passx, tabfn, arg, a); // stack1
			}
			break;
		case 3:
			if (clientsnstack2) {
				w = st2_w;
				flextitledrawarea(ws, nthtiled(ws->clients, clientsnmaster + clientsnstack + 1, 0), x, w, clientsnstack2, 0, 1, 0, 0, passx, tabfn, arg, a); // stack2
			}
			break;
		case 4:
			if (clientsnhidden) {
				w = hid_w;
				flextitledrawarea(ws, ws->clients, x, w, clientsnhidden, 0, 0, 1, 0, passx, tabfn, arg, a); // hidden
			}
			break;
		case 5:
			if (clientsnfloating) {
				w = flt_w;
				flextitledrawarea(ws, ws->clients, x, w, clientsnfloating, 0, 0, 0, 1, passx, tabfn, arg, a); // floating
			}
			break;
		}

		x += w;
		if (!drawpowerline && w) {
			x += flexwintitle_separator;
		}
	}
	return 1;
}
