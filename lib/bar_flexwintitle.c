/* Flexwintitle properties, you can override these in your config.h if you want. */
#ifndef FLEXWINTITLE_BORDERS
#define FLEXWINTITLE_BORDERS 2       // 0 = off, otherwise width of border / client separator
#endif
#ifndef FLEXWINTITLE_SHOWFLOATING
#define FLEXWINTITLE_SHOWFLOATING 0  // whether to show titles for floating windows, hidden clients are always shown
#endif
#ifndef FLEXWINTITLE_MASTERWEIGHT
#define FLEXWINTITLE_MASTERWEIGHT 9  // master weight compared to stack, hidden and floating window titles
#endif
#ifndef FLEXWINTITLE_STACKWEIGHT
#define FLEXWINTITLE_STACKWEIGHT 3   // stack weight compared to master, hidden and floating window titles
#endif
#ifndef FLEXWINTITLE_HIDDENWEIGHT
#define FLEXWINTITLE_HIDDENWEIGHT 1  // hidden window title weight
#endif
#ifndef FLEXWINTITLE_FLOATWEIGHT
#define FLEXWINTITLE_FLOATWEIGHT 1   // floating window title weight, set to 0 to not show floating windows
#endif

#define SCHEMEFOR(c) getschemefor(m, c, groupactive == c)

enum { GRP_NOSELECTION, GRP_MASTER, GRP_STACK1, GRP_STACK2, GRP_FLOAT, GRP_HIDDEN };

int
width_flexwintitle(Bar *bar, BarArg *a)
{
	return a->w;
}

int
draw_flexwintitle(Bar *bar, BarArg *a)
{
	drw_rect(drw, a->x, a->y, a->w, a->h, 1, 1);
	return flextitlecalculate(bar, a->x, a->w, -1, flextitledraw, NULL, a);
}

int
click_flexwintitle(Bar *bar, Arg *arg, BarArg *a)
{
	flextitlecalculate(bar, 0, a->w, a->x, flextitleclick, arg, a);
	return ClkWinTitle;
}

Client *
flextitledrawarea(Monitor *m, Client *c, int x, int r, int w, int max_clients, int scheme, int draw_tiled, int draw_hidden, int draw_floating,
	int passx, void(*tabfn)(Monitor *, Client *, int, int, int, int, Arg *arg, BarArg *barg), Arg *arg, BarArg *barg)
{
	int i;
	for (i = 0; c && i < max_clients; c = c->next) {
		if (
			ISVISIBLE(c) &&
			(
				(draw_tiled && !ISFLOATING(c) && !HIDDEN(c)) ||
				(draw_floating && ISFLOATING(c) && !HIDDEN(c)) ||
				(draw_hidden && HIDDEN(c))
			)
		) {
			tabfn(m, c, passx, x, w + (i < r ? 1 : 0), scheme, arg, barg);
			x += w + (i < r ? 1 : 0);
			i++;
		}
	}
	return c;
}

int
getschemefor(Monitor *m, int group, int activegroup)
{
	Workspace *ws = MWS(m);
	switch (group) {
	case GRP_NOSELECTION:
		return SchemeTitleNorm;
	case GRP_MASTER:
	case GRP_STACK1:
	case GRP_STACK2:
		if (ws->layout->arrange == &flextile)
			return (activegroup ? SchemeFlexActTTB + ws->ltaxis[group] : SchemeFlexInaTTB + ws->ltaxis[group]);
		return SchemeTitleNorm;
	case GRP_HIDDEN:
		return SchemeHid;
	case GRP_FLOAT:
		return (activegroup ? SchemeFlexActFloat : SchemeFlexInaFloat);
	}
	return SchemeTitleNorm;
}

void
getclientcounts(Monitor *m, int *groupactive, int *n, int *clientsnmaster, int *clientsnstack, int *clientsnstack2, int *clientsnfloating, int *clientsnhidden)
{
	fprintf(stderr, "getclientcounts: -->\n");
	Workspace *ws = MWS(m);
	Client *c;
	int i, selidx = 0, cm = 0, cs1 = 0, cs2 = 0, cf = 0, ch = 0, center, dualstack;

	fprintf(stderr, "getclientcounts: ws == NULL? %d\n", ws == NULL);
	for (i = 0, c = ws->clients; c; c = c->next) {
		if (!ISVISIBLE(c))
			continue;
		if (HIDDEN(c)) {
			if (FLEXWINTITLE_HIDDENWEIGHT)
				ch++;
			continue;
		}

		if (ISFLOATING(c)) {
			if (FLEXWINTITLE_FLOATWEIGHT)
				cf++;
			continue;
		}

		if (ws->sel == c)
			selidx = i;

		if (!ws->nmaster || i < ws->nmaster || ws->ltaxis[LAYOUT] == NO_SPLIT)
			cm++;
		else if (ws->nstack) {
			if (cs1 < ws->nstack)
				cs1++;
			else
				cs2++;
		}
		else if ((i - ws->nmaster) % 2)
			cs2++;
		else
			cs1++;
		i++;
	}

	*n = cm + cs1 + cs2 + cf + ch;
	center = iscenteredlayout(m, *n);
	dualstack = isdualstacklayout(m);

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
	fprintf(stderr, "getclientcounts: <--\n");
}

int
isdualstacklayout(Monitor *m)
{
	Workspace *ws = MWS(m);
	if (ws->layout->arrange != &flextile)
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
iscenteredlayout(Monitor *m, int n)
{
	Workspace *ws = MWS(m);
	if (ws->layout->arrange != &flextile)
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
isfixedlayout(Monitor *m)
{
	Workspace *ws = MWS(m);
	if (ws->layout->arrange != &flextile)
		return 0;

	int layout = ws->ltaxis[LAYOUT];
	if (layout < 0)
		layout *= -1;

	return layout > FLOATING_MASTER;
}

int
ismirroredlayout(Monitor *m)
{
	Workspace *ws = MWS(m);
	if (ws->layout->arrange != &flextile)
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
flextitledraw(Monitor *m, Client *c, int unused, int x, int w, int tabscheme, Arg *arg, BarArg *barg)
{
	if (!c)
		return;
	Workspace *ws = MWS(m);

	int i, nclienttags = 0, nviewtags = 0, pad = lrpad / 2;
	int clientscheme = (
		c == ws->sel
		? getselschemefor(tabscheme)
		: HIDDEN(c)
		? SchemeHid
		: ISURGENT(c)
		? SchemeUrg
		: tabscheme
	);
	drw_setscheme(drw, scheme[clientscheme]);
	XSetWindowBorder(dpy, c->win, scheme[clientscheme][ColBorder].pixel);
	if (w <= TEXTW("A") - lrpad + pad) // reduce text padding if wintitle is too small
		pad = (w - TEXTW("A") + lrpad < 0 ? 0 : (w - TEXTW("A") + lrpad) / 2);
	else if (enabled(CenteredWindowName) && TEXTW(c->name) < w)
		pad = (w - TEXTW(c->name) + lrpad) / 2;

	drw_text(drw, x, barg->y, w, barg->h, pad, c->name, 0, False);
	drawstateindicator(m, c, 1, x + 2, barg->y, w, barg->h, 0, 0, 0);

	if (FLEXWINTITLE_BORDERS) {
		XSetForeground(drw->dpy, drw->gc, scheme[SchemeSel][ColBorder].pixel);
		XFillRectangle(drw->dpy, drw->drawable, drw->gc, x, barg->y, FLEXWINTITLE_BORDERS, barg->h);
		XFillRectangle(drw->dpy, drw->drawable, drw->gc, x + w - (x + w >= barg->w ? 1 : 0), barg->y, FLEXWINTITLE_BORDERS, barg->h);
	}
	/* Optional tags icons */
	for (i = 0; i < NUMTAGS; i++) {
		if ((ws->tags >> i) & 1)
			nviewtags++;
		if ((c->tags >> i) & 1)
			nclienttags++;
	}

	if (TAGSINDICATOR == 2 || nclienttags > 1 || nviewtags > 1)
		drawindicator(m, c, 1, x, barg->y, w, barg->h, 0, 0, 0, INDICATOR_RIGHT_TAGS);
}

void
flextitleclick(Monitor *m, Client *c, int passx, int x, int w, int unused, Arg *arg, BarArg *barg)
{
	if (passx >= x && passx <= x + w)
		arg->v = c;
}

int
flextitlecalculate(
	Bar *bar, int offx, int tabw, int passx,
	void(*tabfn)(Monitor *, Client *, int, int, int, int, Arg *arg, BarArg *barg),
	Arg *arg, BarArg *barg
) {
	Client *c;
	Monitor *m = bar->mon;
	Workspace *ws = MWS(m);
	int n, center = 0, mirror = 0, fixed = 0; // layout configuration
	int groupactive = 0, clientsnmaster = 0, clientsnstack = 0, clientsnstack2 = 0, clientsnfloating = 0, clientsnhidden = 0;
	int w, r, num = 0, den, fulllayout = 0;
	int rw, rr;

	int mas_x = offx, st1_x = offx, st2_x = offx, hid_x = offx, flt_x = offx;
	int mas_w, st1_w, st2_w, hid_w;

	getclientcounts(m, &groupactive, &n, &clientsnmaster, &clientsnstack, &clientsnstack2, &clientsnfloating, &clientsnhidden);

	if (n == 0)
	 	return 0;
	else if (ws->layout->arrange == &flextile) {
		mirror = ismirroredlayout(m);
		fixed = isfixedlayout(m);
		center = iscenteredlayout(m, n);
	}

	/* Certain layouts have no master / stack areas */
	if (!ws->layout->arrange                            // floating layout
		|| (!n || (!fixed && ws->nmaster && n <= ws->nmaster)) // no master
		|| (ws->layout->arrange == &flextile && ws->ltaxis[LAYOUT] == NO_SPLIT)
	)
		fulllayout = 1;

	num = tabw;
	c = ws->clients;

	/* floating mode */
	if ((fulllayout && FLEXWINTITLE_FLOATWEIGHT > 0) || clientsnmaster + clientsnstack == 0 || !ws->layout->arrange) {
		den = clientsnmaster + clientsnstack + clientsnstack2 + clientsnfloating + clientsnhidden;
		w = num / den;
		r = num % den; // rest
		c = flextitledrawarea(m, c, mas_x, r, w, den, !ws->layout->arrange ? SchemeFlexActFloat : SCHEMEFOR(GRP_MASTER), 1, FLEXWINTITLE_HIDDENWEIGHT, FLEXWINTITLE_FLOATWEIGHT, passx, tabfn, arg, barg); // floating
	/* no master and stack mode, e.g. monocole, grid layouts, fibonacci */
	} else if (fulllayout) {
		den = clientsnmaster + clientsnstack + clientsnstack2 + clientsnhidden;
		w = num / den;
		r = num % den; // rest
		c = flextitledrawarea(m, c, mas_x, r, w, den, SCHEMEFOR(GRP_MASTER), 1, FLEXWINTITLE_HIDDENWEIGHT, 0, passx, tabfn, arg, barg); // full
	/* tiled mode */
	} else {
		den = clientsnmaster * FLEXWINTITLE_MASTERWEIGHT + (clientsnstack + clientsnstack2) * FLEXWINTITLE_STACKWEIGHT + clientsnfloating * FLEXWINTITLE_FLOATWEIGHT + clientsnhidden * FLEXWINTITLE_HIDDENWEIGHT;
		w = num / den; // weight width per client
		r = num % den; // weight rest width
		rw = r / n; // rest incr per client
		rr = r % n; // rest rest

		mas_w = clientsnmaster * rw + w * clientsnmaster * FLEXWINTITLE_MASTERWEIGHT + (rr > 0 ? MIN(rr, clientsnmaster) : 0);
		rr -= clientsnmaster;
		st1_w = clientsnstack * (rw + w * FLEXWINTITLE_STACKWEIGHT) + (rr > 0 ? MIN(rr, clientsnstack) : 0);
		rr -= clientsnstack;
		st2_w = clientsnstack2 * (rw + w * FLEXWINTITLE_STACKWEIGHT) + (rr > 0 ? MIN(rr, clientsnstack2) : 0);
		rr -= clientsnstack2;
		hid_w = clientsnhidden * (rw + w * FLEXWINTITLE_HIDDENWEIGHT) + (rr > 0 ? MIN(rr, clientsnhidden) : 0);
		rr -= clientsnhidden;
		rr = r % n;

		if (mirror) {
			if (center && clientsnstack2) {
				mas_x = st1_x + st1_w;
				st2_x = mas_x + mas_w;
				hid_x = st2_x + st2_w;
			} else {
				if (clientsnstack2) {
					st2_x = st1_x + st1_w;
					mas_x = st2_x + st2_w;
				} else
					mas_x = st1_x + st1_w;
				hid_x = mas_x + mas_w;
			}
		} else {
			if (center && clientsnstack2) {
				mas_x = st2_x + st2_w;
				st1_x = mas_x + mas_w;
				hid_x = st1_x + st1_w;
			} else {
				st1_x = mas_x + mas_w;
				if (clientsnstack2) {
					st2_x = st1_x + st1_w;
					hid_x = st2_x + st2_w;
				} else
					hid_x = st1_x + st1_w;
			}
		}

		flt_x = hid_x + hid_w;
		c = flextitledrawarea(m, c, mas_x, rr, w * FLEXWINTITLE_MASTERWEIGHT + rw, clientsnmaster, SCHEMEFOR(GRP_MASTER), 1, 0, 0, passx, tabfn, arg, barg); // master
		rr -= clientsnmaster;
		c = flextitledrawarea(m, c, st1_x, rr, w * FLEXWINTITLE_STACKWEIGHT + rw, clientsnstack, SCHEMEFOR(GRP_STACK1), 1, 0, 0, passx, tabfn, arg, barg); // stack1
		rr -= clientsnstack;
		if (clientsnstack2) {
			c = flextitledrawarea(m, c, st2_x, rr, w * FLEXWINTITLE_STACKWEIGHT + rw, clientsnstack2, SCHEMEFOR(GRP_STACK2), 1, 0, 0, passx, tabfn, arg, barg); // stack2
			rr -= clientsnstack2;
		}
		c = flextitledrawarea(m, ws->clients, hid_x, rr, w * FLEXWINTITLE_HIDDENWEIGHT + rw, clientsnhidden, SCHEMEFOR(GRP_HIDDEN), 0, 1, 0, passx, tabfn, arg, barg); // hidden
		rr -= clientsnhidden;
		c = flextitledrawarea(m, ws->clients, flt_x, rr, w * FLEXWINTITLE_FLOATWEIGHT + rw, clientsnfloating, SCHEMEFOR(GRP_FLOAT), 0, 0, 1, passx, tabfn, arg, barg); // floating
	}
	return 1;
}