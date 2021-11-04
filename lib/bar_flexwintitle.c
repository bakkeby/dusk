#define SCHEMEFOR(c) getschemefor(ws, c, groupactive == c)

int
size_flexwintitle(Bar *bar, BarArg *a)
{
	if (!bar->mon->selws)
		return 0;
	return a->w;
}

int
draw_flexwintitle(Bar *bar, BarArg *a)
{
	if (!bar->mon->selws)
		return 0;
	return flextitlecalculate(bar, a->x, a->w, -1, flextitledraw, NULL, a);
}

int
click_flexwintitle(Bar *bar, Arg *arg, BarArg *a)
{
	if (!bar->mon->selws)
		return 0;
	if (flextitlecalculate(bar, 0, a->w, a->x, flextitleclick, arg, a))
		return ClkWinTitle;
	return -1;
}

Client *
flextitledrawarea(Workspace *ws, Client *c, int x, int w, int num_clients, int scheme, int draw_tiled, int draw_hidden, int draw_floating,
	int passx, void(*tabfn)(Workspace *, Client *, int, int, int, int, Arg *arg, BarArg *barg), Arg *arg, BarArg *barg)
{
	int i, rw, cw;
	cw = (w - flexwintitle_separator * (num_clients - 1)) / num_clients;
	rw = (w - flexwintitle_separator * (num_clients - 1)) % num_clients;
	for (i = 0; c && i < num_clients; c = c->next) {
		if (
			!SKIPTASKBAR(c) &&
			!ISINVISIBLE(c) &&
			(
				(draw_tiled && !ISFLOATING(c) && !HIDDEN(c)) ||
				(draw_floating && ISFLOATING(c) && !HIDDEN(c)) ||
				(draw_hidden && HIDDEN(c))
			)
		) {
			tabfn(ws, c, passx, x, cw + (i < rw ? 1 : 0), scheme, arg, barg);
			x += cw + (i < rw ? 1 : 0) + flexwintitle_separator;
			i++;
		}
	}
	return c;
}

int
getschemefor(Workspace *ws, int group, int activegroup)
{
	switch (group) {
	case GRP_NOSELECTION:
		return SchemeTitleNorm;
	case GRP_MASTER:
	case GRP_STACK1:
	case GRP_STACK2:
		if (ws && ws->layout->arrange == &flextile)
			return (activegroup ? SchemeFlexActTTB + ws->ltaxis[group] : SchemeFlexInaTTB + ws->ltaxis[group]);
		return SchemeTitleNorm;
	case GRP_HIDDEN:
		return SchemeHidNorm;
	case GRP_FLOAT:
		return (activegroup ? SchemeFlexActFloat : SchemeFlexInaFloat);
	}
	return SchemeTitleNorm;
}

void
getclientcounts(Workspace *ws, int *groupactive, int *n, int *clientsnmaster, int *clientsnstack, int *clientsnstack2, int *clientsnfloating, int *clientsnhidden)
{
	Client *c;
	int i, selidx = 0, cm = 0, cs1 = 0, cs2 = 0, cf = 0, ch = 0, center, dualstack;

	if (!ws) {
		*groupactive = GRP_FLOAT;
		*clientsnmaster = cm;
		*clientsnstack = cs1;
		*clientsnstack2 = cs2;
		*clientsnfloating = cf;
		*clientsnhidden = ch;
		return;
	}

	for (i = 0, c = (ws ? ws->clients : NULL); c; c = c->next) {
		if (ISINVISIBLE(c))
			continue;
		if (SKIPTASKBAR(c))
			continue;
		if (HIDDEN(c)) {
			if (flexwintitle_hiddenweight)
				ch++;
			continue;
		}

		if (ISFLOATING(c)) {
			if (flexwintitle_floatweight)
				cf++;
			continue;
		}

		if (ws->sel == c)
			selidx = i;

		if (!ws->nmaster || i < ws->nmaster || ws->ltaxis[LAYOUT] == NO_SPLIT) {
			if (flexwintitle_masterweight)
				++cm;
		}
		else if (!flexwintitle_stackweight)
			continue;
		else if (ws->nstack) {
			if (cs1 < ws->nstack)
				++cs1;
			else
				++cs2;
		}
		else if ((i - ws->nmaster) % 2)
			++cs2;
		else
			++cs1;
		i++;
	}
	*n = cm + cs1 + cs2 + cf + ch;
	center = iscenteredlayout(ws, *n);
	dualstack = isdualstacklayout(ws);

	if ((!center && !dualstack) || (center && *n <= ws->nmaster + (ws->nstack ? ws->nstack : 1))) {
		cs1 += cs2;
		cs2 = 0;
	}

	if (!ws->sel)
		*groupactive = GRP_NOSELECTION;
	else if (HIDDEN(ws->sel))
		*groupactive = GRP_HIDDEN;
	else if (ISFLOATING(ws->sel))
		*groupactive = GRP_FLOAT;
	else if (selidx < cm)
		*groupactive = GRP_MASTER;
	else if (selidx < cm + cs1)
		*groupactive = GRP_STACK1;
	else if (selidx < cm + cs1 + cs2)
		*groupactive = GRP_STACK2;

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
isfixedlayout(Workspace *ws)
{
	if (!ws || ws->layout->arrange != &flextile)
		return 0;

	int layout = ws->ltaxis[LAYOUT];
	if (layout < 0)
		layout *= -1;

	return layout > FLOATING_MASTER;
}

int
ismirroredlayout(Workspace *ws)
{
	if (!ws || ws->layout->arrange != &flextile)
		return 0;

	return ws->ltaxis[LAYOUT] < 0;
}

int
getselschemefor(int scheme)
{
	if (scheme == SchemeFlexActFloat || scheme == SchemeFlexInaFloat)
		return SchemeFlexSelFloat;
	if (scheme >= SchemeFlexInaTTB)
		return scheme + SchemeFlexInaTTB - SchemeFlexActTTB;
	if (scheme >= SchemeFlexActTTB)
		return scheme + SchemeFlexSelTTB - SchemeFlexActTTB;
	return SchemeTitleSel;
}

void
flextitledraw(Workspace *ws, Client *c, int unused, int x, int w, int tabscheme, Arg *arg, BarArg *barg)
{
	if (!c)
		return;

	int pad = lrpad / 2;
	int ipad = enabled(WinTitleIcons) && c->icon ? c->icw + iconspacing : 0;
	int clientscheme = (
		ISMARKED(c)
		? SchemeMarked
		: c == ws->sel && HIDDEN(c)
		? SchemeHidSel
		: HIDDEN(c)
		? SchemeHidNorm
		: c->scratchkey != 0 && c == ws->sel
		? SchemeScratchSel
		: c->scratchkey != 0
		? SchemeScratchNorm
		: c == ws->sel
		? getselschemefor(tabscheme)
		: ISURGENT(c)
		? SchemeUrg
		: tabscheme
	);

	drw_setscheme(drw, scheme[clientscheme]);
	XSetWindowBorder(dpy, c->win, scheme[clientscheme][ColBorder].pixel);
	if (clientscheme != SchemeMarked && c != ws->sel)
		c->scheme = clientscheme;

	if (w <= TEXTW("A") + pad) // reduce text padding if wintitle is too small
		pad = (w - TEXTW("A") < 0 ? 0 : (w - TEXTW("A")) / 2);
	else if (enabled(CenteredWindowName) && TEXTW(c->name) + ipad < w)
		pad = (w - TEXTW(c->name) - ipad) / 2;

	drw_text(drw, x, barg->y, w, barg->h, pad + ipad, c->name, 0, False, 1);

 	if (ipad)
		drw_pic(drw, x + pad, barg->y + (barg->h - c->ich) / 2, c->icw, c->ich, c->icon);

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
	Client *c;
	Workspace *ws = bar->mon->selws;
	int n, center = 0, mirror = 0, fixed = 0; // layout configuration
	int groupactive = 0, clientsnmaster = 0, clientsnstack = 0, clientsnstack2 = 0, clientsnfloating = 0, clientsnhidden = 0;
	int w, r, den, fulllayout = 0;
	int rw, rr;

	int mas_x, st1_x, st2_x, hid_x, flt_x;
	int mas_w = 0, st1_w = 0, st2_w = 0, hid_w = 0, flt_w = 0;

	/* This avoids drawing a separator on the left hand side of the wintitle section if
	 * there is a border and the wintitle module rests at the left border. */
	if (a->x > bar->borderpx) {
		offx += flexwintitle_separator;
		tabw -= flexwintitle_separator;
	}

	/* This avoids drawing a separator on the right hand side of the wintitle section if
	 * there is a border and the wintitle module rests at the right border. */
	if (a->x + a->w < bar->bw - bar->borderpx)
		tabw -= flexwintitle_separator;

	mas_x = st1_x = st2_x = hid_x = flt_x = offx;

	getclientcounts(ws, &groupactive, &n, &clientsnmaster, &clientsnstack, &clientsnstack2, &clientsnfloating, &clientsnhidden);

	if (n == 0)
	 	return 0;
	else if (ws->layout->arrange == &flextile) {
		mirror = ismirroredlayout(ws);
		fixed = isfixedlayout(ws);
		center = iscenteredlayout(ws, n);
	}

	XSetForeground(drw->dpy, drw->gc, scheme[bar->scheme][ColBorder].pixel);
	XFillRectangle(drw->dpy, drw->drawable, drw->gc, a->x, a->y, a->w, a->h);

	/* Certain layouts have no master / stack areas */
	if (!ws->layout->arrange                            // floating layout
		|| (!n || (!fixed && ws->nmaster && n <= ws->nmaster)) // no master
		|| (ws->layout->arrange == &flextile && ws->ltaxis[LAYOUT] == NO_SPLIT)
	)
		fulllayout = 1;

	c = ws->clients;

	/* floating mode */
	if ((fulllayout && flexwintitle_floatweight > 0) || clientsnmaster + clientsnstack == 0 || !ws->layout->arrange) {
		den = clientsnmaster + clientsnstack + clientsnstack2 + clientsnfloating + clientsnhidden;
		c = flextitledrawarea(ws, c, mas_x, tabw, den, !ws->layout->arrange ? SchemeFlexActFloat : SCHEMEFOR(GRP_MASTER), 1, flexwintitle_hiddenweight, flexwintitle_floatweight, passx, tabfn, arg, a); // floating
	/* no master and stack mode, e.g. monocole, grid layouts, fibonacci */
	} else if (fulllayout) {
		den = clientsnmaster + clientsnstack + clientsnstack2 + clientsnhidden;
		c = flextitledrawarea(ws, c, mas_x, tabw, den, SCHEMEFOR(GRP_MASTER), 1, flexwintitle_hiddenweight, 0, passx, tabfn, arg, a); // full
	/* tiled mode */
	} else {
		den = clientsnmaster * flexwintitle_masterweight
		    + (clientsnstack + clientsnstack2) * flexwintitle_stackweight
		    + clientsnhidden * flexwintitle_hiddenweight
		    + clientsnfloating * flexwintitle_floatweight;
		w = (tabw - (n - 1) * flexwintitle_separator) / den;
		r = (tabw - (n - 1) * flexwintitle_separator) % den;

		rw = r / n; // rest incr per client
		rr = r % n; // rest rest

		if (clientsnmaster && flexwintitle_masterweight)
			mas_w = clientsnmaster * rw + w * clientsnmaster * flexwintitle_masterweight + (rr > 0 ? MIN(rr, clientsnmaster) : 0) + (clientsnmaster - 1) * flexwintitle_separator;
		rr -= clientsnmaster;
		if (clientsnstack && flexwintitle_stackweight)
			st1_w = clientsnstack * (rw + w * flexwintitle_stackweight) + (rr > 0 ? MIN(rr, clientsnstack) : 0) + (clientsnstack - 1) * flexwintitle_separator;
		rr -= clientsnstack;
		if (clientsnstack2 && flexwintitle_stackweight)
			st2_w = clientsnstack2 * (rw + w * flexwintitle_stackweight) + (rr > 0 ? MIN(rr, clientsnstack2) : 0) + (clientsnstack2 - 1) * flexwintitle_separator;
		rr -= clientsnstack2;
		if (clientsnhidden && flexwintitle_hiddenweight)
			hid_w = clientsnhidden * (rw + w * flexwintitle_hiddenweight) + (rr > 0 ? MIN(rr, clientsnhidden) : 0) + (clientsnhidden - 1) * flexwintitle_separator;
		rr -= clientsnhidden;
		if (clientsnfloating && flexwintitle_floatweight)
			flt_w = clientsnfloating * (rw + w * flexwintitle_floatweight) + (rr > 0 ? MIN(rr, clientsnfloating) : 0) + (clientsnfloating - 1) * flexwintitle_separator;
		if (rr > 0)
			mas_w += rr;

		if (mirror) {
			if (center && clientsnstack2) {
				mas_x = st1_x + (st1_w ? st1_w + flexwintitle_separator : 0);
				st2_x = mas_x + (mas_w ? mas_w + flexwintitle_separator : 0);
				hid_x = st2_x + (st2_w ? st2_w + flexwintitle_separator : 0);
			} else {
				if (clientsnstack2) {
					st2_x = st1_x + (st1_w ? st1_w + flexwintitle_separator : 0);
					mas_x = st2_x + (st2_w ? st2_w + flexwintitle_separator : 0);
				} else
					mas_x = st1_x + (st1_w ? st1_w + flexwintitle_separator : 0);
				hid_x = mas_x + (mas_w ? mas_w + flexwintitle_separator : 0);
			}
		} else {
			if (center && clientsnstack2) {
				mas_x = st2_x + (st2_w ? st2_w + flexwintitle_separator : 0);
				st1_x = mas_x + (mas_w ? mas_w + flexwintitle_separator : 0);
				hid_x = st1_x + (st1_w ? st1_w + flexwintitle_separator : 0);
			} else {
				st1_x = mas_x + (mas_w ? mas_w + flexwintitle_separator : 0);
				if (clientsnstack2) {
					st2_x = st1_x + (st1_w ? st1_w + flexwintitle_separator : 0);
					hid_x = st2_x + (st2_w ? st2_w + flexwintitle_separator : 0);
				} else
					hid_x = st1_x + (st1_w ? st1_w + flexwintitle_separator : 0);
			}
		}
		flt_x = hid_x + (hid_w ? hid_w + flexwintitle_separator : 0);

		if (clientsnmaster)
			c = flextitledrawarea(ws, c, mas_x, mas_w, clientsnmaster, SCHEMEFOR(GRP_MASTER), 1, 0, 0, passx, tabfn, arg, a); // master1
		if (clientsnstack)
			c = flextitledrawarea(ws, c, st1_x, st1_w, clientsnstack, SCHEMEFOR(GRP_STACK1), 1, 0, 0, passx, tabfn, arg, a); // stack1
		if (clientsnstack2)
			c = flextitledrawarea(ws, c, st2_x, st2_w, clientsnstack2, SCHEMEFOR(GRP_STACK2), 1, 0, 0, passx, tabfn, arg, a); // stack2
		if (clientsnhidden)
			c = flextitledrawarea(ws, ws->clients, hid_x, hid_w, clientsnhidden, SCHEMEFOR(GRP_HIDDEN), 0, 1, 0, passx, tabfn, arg, a); // hidden
		if (clientsnfloating)
			c = flextitledrawarea(ws, ws->clients, flt_x, flt_w, clientsnfloating, SCHEMEFOR(GRP_FLOAT), 0, 0, 1, passx, tabfn, arg, a); // floating
	}
	return 1;
}
