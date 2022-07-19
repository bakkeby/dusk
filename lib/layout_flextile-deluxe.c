typedef struct {
	void (*arrange)(Workspace *, int, int, int, int, int, int, int);
} LayoutArranger;

typedef struct {
	void (*arrange)(Workspace *, int, int, int, int, int, int, int, int, int, int, int);
} TileArranger;

static const LayoutArranger flexlayouts[] = {
	{ layout_no_split },
	{ layout_split_vertical },
	{ layout_split_horizontal },
	{ layout_split_centered_vertical },
	{ layout_split_centered_horizontal },
	{ layout_split_vertical_dual_stack },
	{ layout_split_horizontal_dual_stack },
	{ layout_floating_master },
	{ layout_split_vertical_fixed },
	{ layout_split_horizontal_fixed },
	{ layout_split_centered_vertical_fixed },
	{ layout_split_centered_horizontal_fixed },
	{ layout_split_vertical_dual_stack_fixed },
	{ layout_split_horizontal_dual_stack_fixed },
	{ layout_floating_master_fixed },
};

static const TileArranger flextiles[] = {
	{ arrange_top_to_bottom },
	{ arrange_left_to_right },
	{ arrange_monocle },
	{ arrange_gapplessgrid },
	{ arrange_gapplessgrid_cfacts },
	{ arrange_gapplessgrid_alt1 },
	{ arrange_gapplessgrid_alt2 },
	{ arrange_gridmode },
	{ arrange_horizgrid },
	{ arrange_dwindle },
	{ arrange_dwindle_cfacts },
	{ arrange_spiral },
	{ arrange_spiral_cfacts },
	{ arrange_tatami },
	{ arrange_tatami_cfacts },
};

/* workspace  symbol     nmaster, nstack, split, master axis, stack axis, secondary stack axis  */
void
customlayout(const Arg args[], int num_args)
{
	Workspace *ws;

	if (num_args < 8) {
		fprintf(stderr, "customlayout: num_args < 7 : %d\n", num_args);
		return;
	}

	/* 0) Workspace */
	for (ws = workspaces; ws && ws->num != args[0].i; ws = ws->next);
	if (!ws)
		ws = selws;
	ws->prevlayout = ws->layout;

	/* 1) Layout symbol */
	if (args[1].v)
		strncpy(ws->ltsymbol, args[1].v, sizeof ws->ltsymbol);

	/* 2) nmaster */
	if (args[2].i > -1)
		ws->nmaster = args[2].i;

	/* 3) nstack */
	if (args[3].i > -1)
		ws->nstack = args[3].i;

	/* 4) split (layout), negative means mirror layout */
	if (labs(args[4].i) < LAYOUT_LAST)
		ws->ltaxis[LAYOUT] = args[4].i;

	/* 5) master axis */
	if (args[5].i > -1 && args[5].i < AXIS_LAST)
		ws->ltaxis[MASTER] = args[5].i;

	/* 6) stack 1 axis */
	if (args[6].i > -1 && args[6].i < AXIS_LAST)
		ws->ltaxis[STACK] = args[6].i;

	/* 7) stack 2 axis */
	if (args[7].i > -1 && args[7].i < AXIS_LAST)
		ws->ltaxis[STACK2] = args[7].i;

	arrange(ws);
}

void
getfactsforrange(Workspace *ws, int an, int ai, int size, int *rest, float *fact)
{
	int i;
	float facts;
	Client *c;
	int total = 0;

	facts = 0;
	for (i = 0, c = nexttiled(ws->clients); c && i < (ai + an); c = nexttiled(c->next), i++)
		if (i >= ai)
			facts += c->cfact;

	for (i = 0, c = nexttiled(ws->clients); c && i < (ai + an); c = nexttiled(c->next), i++)
		if (i >= ai)
			total += size * (c->cfact / facts);

	*rest = size - total;
	*fact = facts;
}

void
setlayoutaxisex(const Arg *arg)
{
	int axis, arr;
	Workspace *ws = selws;

	axis = arg->i & 0x3; // lower two bits indicates layout, master or stack1-2
	arr = arg->i >> 2;   // remaining upper bits indicate arrangement

	if ((axis == 0 && labs(arr) > LAYOUT_LAST)
			|| (axis > 0 && (arr > AXIS_LAST || arr < 0)))
		arr = 0;

	ws->ltaxis[axis] = arr;
	arrange(ws);
}

void
setlayoutex(const Arg *arg)
{
	setlayout(&((Arg) { .v = &layouts[arg->i % LENGTH(layouts)] }));
}

void
setwindowborders(Workspace *ws, Client *sel)
{
	for (Client *s = ws->stack; s; s = s->snext)
		XSetWindowBorder(dpy, s->win, scheme[clientscheme(s, sel)][ColBorder].pixel);
}

void
layoutmonconvert(Workspace *ws, Monitor *from, Monitor *to)
{
	int from_orientation = from->mw < from->mh;
	int to_orientation = to->mw < to->mh;

	if (from_orientation == to_orientation)
		return;

	if (ws->layout->arrange != flextile)
		return;

	layoutconvert(&((Arg) { .v = ws }));
}

void
layoutconvert(const Arg *arg)
{
	Workspace *ws = (arg && arg->v ? (Workspace *)arg->v : selws);
	ws->ltaxis[LAYOUT] = convert_split(ws->ltaxis[LAYOUT]);
	ws->ltaxis[MASTER] = convert_arrange(ws->ltaxis[MASTER]);
	ws->ltaxis[STACK] = convert_arrange(ws->ltaxis[STACK]);
	ws->ltaxis[STACK2] = convert_arrange(ws->ltaxis[STACK2]);

	if (!arg || !arg->v)
		arrange(ws);
}

int
convert_split(int split)
{
	switch (split) {
	case SPLIT_VERTICAL:
		return SPLIT_HORIZONTAL;
	case SPLIT_HORIZONTAL:
		return SPLIT_VERTICAL;
	case SPLIT_CENTERED_VERTICAL:
		return SPLIT_CENTERED_HORIZONTAL;
	case SPLIT_CENTERED_HORIZONTAL:
		return SPLIT_CENTERED_VERTICAL;
	case SPLIT_VERTICAL_DUAL_STACK:
		return SPLIT_HORIZONTAL_DUAL_STACK;
	case SPLIT_HORIZONTAL_DUAL_STACK:
		return SPLIT_VERTICAL_DUAL_STACK;
	case SPLIT_VERTICAL_FIXED:
		return SPLIT_HORIZONTAL_FIXED;
	case SPLIT_HORIZONTAL_FIXED:
		return SPLIT_VERTICAL_FIXED;
	case SPLIT_CENTERED_VERTICAL_FIXED:
		return SPLIT_CENTERED_HORIZONTAL_FIXED;
	case SPLIT_CENTERED_HORIZONTAL_FIXED:
		return SPLIT_CENTERED_VERTICAL_FIXED;
	case SPLIT_VERTICAL_DUAL_STACK_FIXED:
		return SPLIT_HORIZONTAL_DUAL_STACK_FIXED;
	case SPLIT_HORIZONTAL_DUAL_STACK_FIXED:
		return SPLIT_VERTICAL_DUAL_STACK_FIXED;
	}

	return split;
}

int
convert_arrange(int arrange)
{
	if (arrange == TOP_TO_BOTTOM)
		return LEFT_TO_RIGHT;

	if (arrange == LEFT_TO_RIGHT)
		return TOP_TO_BOTTOM;

	return arrange;
}


void
layout_no_split(Workspace *ws, int x, int y, int h, int w, int ih, int iv, int n)
{
	if (ws->nmaster >= n)
		(&flextiles[ws->ltaxis[MASTER]])->arrange(ws, x, y, h, w, ih, iv, n, n, 0, ws->ltaxis[MASTER], MASTER);
	else
		(&flextiles[ws->ltaxis[STACK]])->arrange(ws, x, y, h, w, ih, iv, n, n, 0, ws->ltaxis[STACK], STACK);
}

void
layout_split_vertical(Workspace *ws, int x, int y, int h, int w, int ih, int iv, int n)
{
	/* Split master into master + stack if we have enough clients */
	if (ws->nmaster && n > ws->nmaster) {
		layout_split_vertical_fixed(ws, x, y, h, w, ih, iv, n);
	} else {
		layout_no_split(ws, x, y, h, w, ih, iv, n);
	}
}

void
layout_split_vertical_fixed(Workspace *ws, int x, int y, int h, int w, int ih, int iv, int n)
{
	int sw, sx;

	sw = (w - iv) * (1 - ws->mfact);
	w = (w - iv) * ws->mfact;
	if (ws->ltaxis[LAYOUT] < 0) { // mirror
		sx = x;
		x += sw + iv;
	} else {
		sx = x + w + iv;
	}

	(&flextiles[ws->ltaxis[MASTER]])->arrange(ws, x, y, h, w, ih, iv, n, ws->nmaster, 0, ws->ltaxis[MASTER], MASTER);
	(&flextiles[ws->ltaxis[STACK]])->arrange(ws, sx, y, h, sw, ih, iv, n, n - ws->nmaster, ws->nmaster, ws->ltaxis[STACK], STACK);
}

void
layout_split_vertical_dual_stack(Workspace *ws, int x, int y, int h, int w, int ih, int iv, int n)
{
	/* Split master into master + stack if we have enough clients */
	if (!ws->nmaster || n <= ws->nmaster) {
		layout_no_split(ws, x, y, h, w, ih, iv, n);
	} else if (n <= ws->nmaster + (ws->nstack ? ws->nstack : 1)) {
		layout_split_vertical(ws, x, y, h, w, ih, iv, n);
	} else {
		layout_split_vertical_dual_stack_fixed(ws, x, y, h, w, ih, iv, n);
	}
}

void
layout_split_vertical_dual_stack_fixed(Workspace *ws, int x, int y, int h, int w, int ih, int iv, int n)
{
	int sh, sw, sx, oy, sc;

	if (ws->nstack)
		sc = ws->nstack;
	else
		sc = (n - ws->nmaster) / 2 + ((n - ws->nmaster) % 2 > 0 ? 1 : 0);

	sw = (w - iv) * (1 - ws->mfact);
	sh = (h - ih) / 2;
	w = (w - iv) * ws->mfact;
	oy = y + sh + ih;
	if (ws->ltaxis[LAYOUT] < 0) { // mirror
		sx = x;
		x += sw + iv;
	} else {
		sx = x + w + iv;
	}

	(&flextiles[ws->ltaxis[MASTER]])->arrange(ws, x, y, h, w, ih, iv, n, ws->nmaster, 0, ws->ltaxis[MASTER], MASTER);
	(&flextiles[ws->ltaxis[STACK]])->arrange(ws, sx, y, sh, sw, ih, iv, n, sc, ws->nmaster, ws->ltaxis[STACK], STACK);
	(&flextiles[ws->ltaxis[STACK2]])->arrange(ws, sx, oy, sh, sw, ih, iv, n, n - ws->nmaster - sc, ws->nmaster + sc, ws->ltaxis[STACK2], STACK2);
}

void
layout_split_horizontal(Workspace *ws, int x, int y, int h, int w, int ih, int iv, int n)
{
	/* Split master into master + stack if we have enough clients */
	if (ws->nmaster && n > ws->nmaster) {
		layout_split_horizontal_fixed(ws, x, y, h, w, ih, iv, n);
	} else {
		layout_no_split(ws, x, y, h, w, ih, iv, n);
	}
}

void
layout_split_horizontal_fixed(Workspace *ws, int x, int y, int h, int w, int ih, int iv, int n)
{
	int sh, sy;

	sh = (h - ih) * (1 - ws->mfact);
	h = (h - ih) * ws->mfact;
	if (ws->ltaxis[LAYOUT] < 0) { // mirror
		sy = y;
		y += sh + ih;
	} else {
		sy = y + h + ih;
	}

	(&flextiles[ws->ltaxis[MASTER]])->arrange(ws, x, y, h, w, ih, iv, n, ws->nmaster, 0, ws->ltaxis[MASTER], MASTER);
	(&flextiles[ws->ltaxis[STACK]])->arrange(ws, x, sy, sh, w, ih, iv, n, n - ws->nmaster, ws->nmaster, ws->ltaxis[STACK], STACK);
}

void
layout_split_horizontal_dual_stack(Workspace *ws, int x, int y, int h, int w, int ih, int iv, int n)
{
	/* Split master into master + stack if we have enough clients */
	if (!ws->nmaster || n <= ws->nmaster) {
		layout_no_split(ws, x, y, h, w, ih, iv, n);
	} else if (n <= ws->nmaster + (ws->nstack ? ws->nstack : 1)) {
		layout_split_horizontal(ws, x, y, h, w, ih, iv, n);
	} else {
		layout_split_horizontal_dual_stack_fixed(ws, x, y, h, w, ih, iv, n);
	}
}

void
layout_split_horizontal_dual_stack_fixed(Workspace *ws, int x, int y, int h, int w, int ih, int iv, int n)
{
	int sh, sy, ox, sc;

	if (ws->nstack)
		sc = ws->nstack;
	else
		sc = (n - ws->nmaster) / 2 + ((n - ws->nmaster) % 2 > 0 ? 1 : 0);

	sh = (h - ih) * (1 - ws->mfact);
	h = (h - ih) * ws->mfact;
	sw = (w - iv) / 2;
	ox = x + sw + iv;
	if (ws->ltaxis[LAYOUT] < 0) { // mirror
		sy = y;
		y += sh + ih;
	} else {
		sy = y + h + ih;
	}

	(&flextiles[ws->ltaxis[MASTER]])->arrange(ws, x, y, h, w, ih, iv, n, ws->nmaster, 0, ws->ltaxis[MASTER], MASTER);
	(&flextiles[ws->ltaxis[STACK]])->arrange(ws, x, sy, sh, sw, ih, iv, n, sc, ws->nmaster, ws->ltaxis[STACK], STACK);
	(&flextiles[ws->ltaxis[STACK2]])->arrange(ws, ox, sy, sh, sw, ih, iv, n, n - ws->nmaster - sc, ws->nmaster + sc, ws->ltaxis[STACK2], STACK2);
}

void
layout_split_centered_vertical(Workspace *ws, int x, int y, int h, int w, int ih, int iv, int n)
{
	/* Split master into master + stack if we have enough clients */
	if (!ws->nmaster || n <= ws->nmaster) {
		layout_no_split(ws, x, y, h, w, ih, iv, n);
	} else if (n <= ws->nmaster + (ws->nstack ? ws->nstack : 1)) {
		layout_split_vertical(ws, x, y, h, w, ih, iv, n);
	} else {
		layout_split_centered_vertical_fixed(ws, x, y, h, w, ih, iv, n);
	}
}

void
layout_split_centered_vertical_fixed(Workspace *ws, int x, int y, int h, int w, int ih, int iv, int n)
{
	int sw, sx, ox, sc;

	if (ws->nstack)
		sc = ws->nstack;
	else
		sc = (n - ws->nmaster) / 2 + ((n - ws->nmaster) % 2 > 0 ? 1 : 0);

	sw = (w - 2*iv) * (1 - ws->mfact) / 2;
	w = (w - 2*iv) * ws->mfact;
	if (ws->ltaxis[LAYOUT] < 0)  { // mirror
		sx = x;
		x += sw + iv;
		ox = x + w + iv;
	} else {
		ox = x;
		x += sw + iv;
		sx = x + w + iv;
	}

	(&flextiles[ws->ltaxis[MASTER]])->arrange(ws, x, y, h, w, ih, iv, n, ws->nmaster, 0, ws->ltaxis[MASTER], MASTER);
	(&flextiles[ws->ltaxis[STACK]])->arrange(ws, sx, y, h, sw, ih, iv, n, sc, ws->nmaster, ws->ltaxis[STACK], STACK);
	(&flextiles[ws->ltaxis[STACK2]])->arrange(ws, ox, y, h, sw, ih, iv, n, n - ws->nmaster - sc, ws->nmaster + sc, ws->ltaxis[STACK2], STACK2);
}

void
layout_split_centered_horizontal(Workspace *ws, int x, int y, int h, int w, int ih, int iv, int n)
{
	/* Split master into master + stack if we have enough clients */
	if (!ws->nmaster || n <= ws->nmaster) {
		layout_no_split(ws, x, y, h, w, ih, iv, n);
	} else if (n <= ws->nmaster + (ws->nstack ? ws->nstack : 1)) {
		layout_split_horizontal(ws, x, y, h, w, ih, iv, n);
	} else {
		layout_split_centered_horizontal_fixed(ws, x, y, h, w, ih, iv, n);
	}
}

void
layout_split_centered_horizontal_fixed(Workspace *ws, int x, int y, int h, int w, int ih, int iv, int n)
{
	int sh, sy, oy, sc;

	if (ws->nstack)
		sc = ws->nstack;
	else
		sc = (n - ws->nmaster) / 2 + ((n - ws->nmaster) % 2 > 0 ? 1 : 0);

	sh = (h - 2*ih) * (1 - ws->mfact) / 2;
	h = (h - 2*ih) * ws->mfact;
	if (ws->ltaxis[LAYOUT] < 0)  { // mirror
		sy = y;
		y += sh + ih;
		oy = y + h + ih;
	} else {
		oy = y;
		y += sh + ih;
		sy = y + h + ih;
	}

	(&flextiles[ws->ltaxis[MASTER]])->arrange(ws, x, y, h, w, ih, iv, n, ws->nmaster, 0, ws->ltaxis[MASTER], MASTER);
	(&flextiles[ws->ltaxis[STACK]])->arrange(ws, x, sy, sh, w, ih, iv, n, sc, ws->nmaster, ws->ltaxis[STACK], STACK);
	(&flextiles[ws->ltaxis[STACK2]])->arrange(ws, x, oy, sh, w, ih, iv, n, n - ws->nmaster - sc, ws->nmaster + sc, ws->ltaxis[STACK2], STACK2);
}

void
layout_floating_master(Workspace *ws, int x, int y, int h, int w, int ih, int iv, int n)
{
	/* Split master into master + stack if we have enough clients */
	if (!ws->nmaster || n <= ws->nmaster) {
		layout_no_split(ws, x, y, h, w, ih, iv, n);
	} else {
		layout_floating_master_fixed(ws, x, y, h, w, ih, iv, n);
	}
}

void
layout_floating_master_fixed(Workspace *ws, int x, int y, int h, int w, int ih, int iv, int n)
{
	int mh, mw;

	/* Draw stack area first */
	(&flextiles[ws->ltaxis[STACK]])->arrange(ws, x, y, h, w, ih, iv, n, n - ws->nmaster, ws->nmaster, ws->ltaxis[STACK], STACK);

	if (w > h) {
		mw = w * ws->mfact;
		mh = h * 0.9;
	} else {
		mw = w * 0.9;
		mh = h * ws->mfact;
	}
	x = x + (w - mw) / 2;
	y = y + (h - mh) / 2;

	(&flextiles[ws->ltaxis[MASTER]])->arrange(ws, x, y, mh, mw, ih, iv, n, ws->nmaster, 0, ws->ltaxis[MASTER], MASTER);
}

void
arrange_left_to_right(Workspace *ws, int x, int y, int h, int w, int ih, int iv, int n, int an, int ai, int arr, int grp)
{
	int i, rest, cw;
	float facts, fact = 1;
	Client *c;

	if (ai + an > n)
		an = n - ai;

	w -= iv * (an - 1);
	getfactsforrange(ws, an, ai, w, &rest, &facts);
	for (i = 0, c = nexttiled(ws->clients); c && i < (ai + an); c = nexttiled(c->next), i++) {
		if (i >= ai) {
			c->area = grp;
			c->arr = arr;
			fact = c->cfact;
			cw = w * (fact / facts) + ((i - ai) < rest ? 1 : 0);
			resize(c, x, y, cw - (2 * c->bw), h - (2 * c->bw), 0);
			x += cw + iv;
		}
	}
}

void
arrange_top_to_bottom(Workspace *ws, int x, int y, int h, int w, int ih, int iv, int n, int an, int ai, int arr, int grp)
{
	int i, rest, ch;
	float facts, fact = 1;
	Client *c;

	if (ai + an > n)
		an = n - ai;

	h -= ih * (an - 1);
	getfactsforrange(ws, an, ai, h, &rest, &facts);
	for (i = 0, c = nexttiled(ws->clients); c && i < (ai + an); c = nexttiled(c->next), i++) {
		if (i >= ai) {
			c->area = grp;
			c->arr = arr;
			fact = c->cfact;
			ch = h * (fact / facts) + ((i - ai) < rest ? 1 : 0);
			resize(c, x, y, w - (2 * c->bw), ch - (2 * c->bw), 0);
			y += ch + ih;
		}
	}
}

void
arrange_monocle(Workspace *ws, int x, int y, int h, int w, int ih, int iv, int n, int an, int ai, int arr, int grp)
{
	int i, stackno, minstackno = 0xFFFFFF;
	Client *c, *s, *f = NULL;

	for (i = 0, c = nexttiled(ws->clients); c && i < (ai + an); c = nexttiled(c->next), i++) {
		if (i < ai)
			continue;

		for (stackno = 0, s = ws->stack; s && s != c; s = s->snext, ++stackno);
		if (stackno < minstackno) {
			f = s;
			minstackno = stackno;
		}
	}

	for (i = 0, c = nexttiled(ws->clients); c && i < (ai + an); c = nexttiled(c->next), i++) {
		if (i < ai)
			continue;

		c->area = grp;
		c->arr = arr;
		if (c == f) {
			XMoveWindow(dpy, c->win, x, y);
			resize(c, x, y, w - (2 * c->bw), h - (2 * c->bw), 0);
		} else {
			hide(c);
		}
	}

	skipfocusevents();
}

void
arrange_gridmode(Workspace *ws, int x, int y, int h, int w, int ih, int iv, int n, int an, int ai, int arr, int grp)
{
	int i, cols, rows, ch, cw, cx, cy, cc, cr, chrest, cwrest; // counters
	Client *c;

	/* grid dimensions */
	for (rows = 0; rows <= an/2; rows++)
		if (rows*rows >= an)
			break;
	cols = (rows && (rows - 1) * rows >= an) ? rows - 1 : rows;

	/* window geoms (cell height/width) */
	ch = (h - ih * (rows - 1)) / (rows ? rows : 1);
	cw = (w - iv * (cols - 1)) / (cols ? cols : 1);
	chrest = h - ih * (rows - 1) - ch * rows;
	cwrest = w - iv * (cols - 1) - cw * cols;
	for (i = 0, c = nexttiled(ws->clients); c && i < (ai + an); c = nexttiled(c->next), i++) {
		if (i < ai)
			continue;
		c->area = grp;
		c->arr = arr;
		cc = ((i - ai) / rows); // client column number
		cr = ((i - ai) % rows); // client row number
		cx = x + cc * (cw + iv) + MIN(cc, cwrest);
		cy = y + cr * (ch + ih) + MIN(cr, chrest);
		resize(c, cx, cy, cw + (cc < cwrest ? 1 : 0) - 2 * c->bw, ch + (cr < chrest ? 1 : 0) - 2 * c->bw, False);
	}
}

void
arrange_horizgrid(Workspace *ws, int x, int y, int h, int w, int ih, int iv, int n, int an, int ai, int arr, int grp)
{
	int ntop, nbottom, rh, rest;

	/* Exception when there is only one client; don't split into two rows */
	if (an == 1) {
		arrange_monocle(ws, x, y, h, w, ih, iv, n, an, ai, arr, grp);
		return;
	}

	ntop = an / 2;
	nbottom = an - ntop;
	rh = (h - ih) / 2;
	rest = h - ih - rh * 2;
	arrange_left_to_right(ws, x, y, rh + rest, w, ih, iv, n, ntop, ai, arr, grp);
	arrange_left_to_right(ws, x, y + rh + ih + rest, rh, w, ih, iv, n, nbottom, ai + ntop, arr, grp);
}

void
arrange_gapplessgrid(Workspace *ws, int x, int y, int h, int w, int ih, int iv, int n, int an, int ai, int arr, int grp)
{
	int i, cols, rows, ch, cw, cn, rn, cc, rrest, crest; // counters
	Client *c;

	/* grid dimensions */
	for (cols = 1; cols <= an/2; cols++)
		if (cols*cols >= an)
			break;
	if (an == 5) /* set layout against the general calculation: not 1:2:2, but 2:3 */
		cols = 2;
	rows = an/cols;
	cn = rn = cc = 0; // reset column no, row no, client count

	ch = (h - ih * (rows - 1)) / rows;
	rrest = (h - ih * (rows - 1)) - ch * rows;
	cw = (w - iv * (cols - 1)) / cols;
	crest = (w - iv * (cols - 1)) - cw * cols;

	for (i = 0, c = nexttiled(ws->clients); c && i < (ai + an); c = nexttiled(c->next), i++) {
		if (i < ai)
			continue;

		c->area = grp;
		c->arr = arr;
		if (cc/rows + 1 > cols - an%cols) {
			rows = an/cols + 1;
			ch = (h - ih * (rows - 1)) / rows;
			rrest = (h - ih * (rows - 1)) - ch * rows;
		}
		resize(c,
			x,
			y + rn*(ch + ih) + MIN(rn, rrest),
			cw + (cn < crest ? 1 : 0) - 2 * c->bw,
			ch + (rn < rrest ? 1 : 0) - 2 * c->bw,
			0);
		rn++;
		cc++;
		if (rn >= rows) {
			rn = 0;
			x += cw + ih + (cn < crest ? 1 : 0);
			cn++;
		}
	}
}

void
arrange_gapplessgrid_cfacts(Workspace *ws, int x, int y, int h, int w, int ih, int iv, int n, int an, int ai, int arr, int grp)
{
	int i, cols, rows, ch, cw, cy, cn, rn, cc, crest, colw; // counters
	float cfacts_total = 0;
	Client *c;

	/* grid dimensions */
	for (cols = 1; cols <= an/2; cols++)
		if (cols*cols >= an)
			break;
	if (an == 5) /* set layout against the general calculation: not 1:2:2, but 2:3 */
		cols = 2;
	rows = an/cols;

	crest = colw = w - iv * (cols - 1);

	float cfacts[cols];
	int rrests[cols];
	for (i = 0; i < cols; i++) {
		cfacts[i] = 0;
		rrests[i] = 0;
	}

	/* Sum cfacts for columns */
	for (i = cn = rn = cc = 0, c = nexttiled(ws->clients); c && i < (ai + an); c = nexttiled(c->next), ++i) {
		if (i < ai)
			continue;

		if (cc/rows + 1 > cols - an%cols)
			rows = an/cols + 1;
		cfacts[cn] += c->cfact;
		cfacts_total += c->cfact;
		rn++;
		cc++;
		if (rn >= rows) {
			rn = 0;
			cn++;
		}
	}

	/* Work out cfact remainders */
	for (i = cn = rn = cc = 0, rows = an/cols, c = nexttiled(ws->clients); c && i < (ai + an); c = nexttiled(c->next), ++i) {
		if (i < ai)
			continue;

		if (cc/rows + 1 > cols - an%cols)
			rows = an/cols + 1;
		rrests[cn] += (h - ih * (rows - 1)) * (c->cfact / cfacts[cn]) + (rn == 0 ? 0 : ih);
		rn++;
		cc++;
		if (rn >= rows) {
			rn = 0;
			cn++;
		}
	}

	for (i = 0; i < cols; i++) {
		crest -= (int)(colw * (cfacts[i] / cfacts_total));
		rrests[i] = h - rrests[i];
	}

	for (i = cn = rn = cc = 0, cy = y, rows = an/cols, c = nexttiled(ws->clients); c && i < (ai + an); c = nexttiled(c->next), ++i) {
		if (i < ai)
			continue;

		c->area = grp;
		c->arr = arr;
		if (cc/rows + 1 > cols - an%cols)
			rows = an/cols + 1;
		cw = (int)(colw * (cfacts[cn] / cfacts_total)) + (cn < crest ? 1 : 0);
		ch = (h - ih * (rows - 1)) * ((double)c->cfact / (double)cfacts[cn]) + (rn < rrests[cn] ? 1 : 0);
		resize(c,
			x,
			cy,
			cw - 2 * c->bw,
			ch - 2 * c->bw,
			0);
		rn++;
		cc++;
		cy += ch + ih;
		if (rn >= rows) {
			rn = 0;
			x += cw + ih;
			cn++;
			cy = y;
		}
	}
}


/* This version of gappless grid fills rows first */
void
arrange_gapplessgrid_alt1(Workspace *ws, int x, int y, int h, int w, int ih, int iv, int n, int an, int ai, int arr, int grp)
{
	int i, cols, rows, rest, ch;

	/* grid dimensions */
	for (cols = 1; cols <= an/2; cols++)
		if (cols*cols >= an)
			break;
	rows = (cols && (cols - 1) * cols >= an) ? cols - 1 : cols;
	ch = (h - ih * (rows - 1)) / (rows ? rows : 1);
	rest = (h - ih * (rows - 1)) - ch * rows;

	for (i = 0; i < rows; i++) {
		arrange_left_to_right(ws, x, y, ch + (i < rest ? 1 : 0), w, ih, iv, n, MIN(cols, an - i*cols), ai + i*cols, arr, grp);
		y += ch + (i < rest ? 1 : 0) + ih;
	}
}

/* This version of gappless grid fills columns first */
void
arrange_gapplessgrid_alt2(Workspace *ws, int x, int y, int h, int w, int ih, int iv, int n, int an, int ai, int arr, int grp)
{
	int i, cols, rows, rest, cw;

	/* grid dimensions */
	for (rows = 0; rows <= an/2; rows++)
		if (rows*rows >= an)
			break;
	cols = (rows && (rows - 1) * rows >= an) ? rows - 1 : rows;
	cw = (w - iv * (cols - 1)) / (cols ? cols : 1);
	rest = (w - iv * (cols - 1)) - cw * cols;

	for (i = 0; i < cols; i++) {
		arrange_top_to_bottom(ws, x, y, h, cw + (i < rest ? 1 : 0), ih, iv, n, MIN(rows, an - i*rows), ai + i*rows, arr, grp);
		x += cw + (i < rest ? 1 : 0) + iv;
	}
}

void
arrange_fibonacci(Workspace *ws, int x, int y, int h, int w, int ih, int iv, int n, int an, int ai, int s, int arr, int grp)
{
	int i, j, nv, hrest = 0, wrest = 0, nx = x, ny = y, nw = w, nh = h, r = 1;
	Client *c;

	for (i = 0, j = 0, c = nexttiled(ws->clients); c && j < (ai + an); c = nexttiled(c->next), j++) {
		if (j < ai)
			continue;

		if (r) {
			if ((i % 2 && ((nh - ih) / 2) <= (bh + 2 * c->bw)) || (!(i % 2) && ((nw - iv) / 2) <= (bh + 2 * c->bw))) {
				r = 0;
			}
			if (r && i < an - 1) {
				if (i % 2) {
					nv = (nh - ih) / 2;
					hrest = nh - 2*nv - ih;
					nh = nv;
				} else {
					nv = (nw - iv) / 2;
					wrest = nw - 2*nv - iv;
					nw = nv;
				}

				if ((i % 4) == 2 && !s)
					nx += nw + iv;
				else if ((i % 4) == 3 && !s)
					ny += nh + ih;
			}
			if ((i % 4) == 0) {
				if (s) {
					ny += nh + ih;
					nh += hrest;
				} else {
					nh -= hrest;
					ny -= nh + ih;
				}
			} else if ((i % 4) == 1) {
				nx += nw + iv;
				nw += wrest;
			} else if ((i % 4) == 2) {
				ny += nh + ih;
				nh += hrest;
				if (i < n - 1)
					nw += wrest;
			} else if ((i % 4) == 3) {
				if (s) {
					nx += nw + iv;
					nw -= wrest;
				} else {
					nw -= wrest;
					nx -= nw + iv;
					nh += hrest;
				}
			}
			if (i == 0)	{
				if (an != 1) {
					nw = (w - iv) - (w - iv) * (1 - ws->mfact);
					wrest = 0;
				}
				ny = y;
			} else if (i == 1)
				nw = w - nw - iv;
			i++;
		}

		c->area = grp;
		c->arr = arr;
		resize(c, nx, ny, nw - 2 * c->bw, nh - 2 * c->bw, False);
	}
}

void
arrange_fibonacci_cfacts(Workspace *ws, int x, int y, int h, int w, int ih, int iv, int n, int an, int ai, int s, int arr, int grp)
{
	Client *clients[4] = { NULL, NULL, NULL, NULL };
	int i, j, q, nx, ny, nw, nh, tnw, tnh;
	Client *t, *a, *b, *c, *d;

	nx = x;
	ny = y;
	nw = w;
	nh = h;

	for (i = 0, j = 0, q = 0, t = nexttiled(ws->clients); t && j < (ai + an); t = nexttiled(t->next), j++) {
		if (j < ai)
			continue;

		t->area = grp;
		t->arr = arr;
		clients[q] = t;
		++q;

		if (q < 4 && (j + 1 < ai + an) && j - ai < 7) // magic number 7 limits to 9 tiled clients
			continue;

		a = clients[0];
		b = clients[1];
		c = clients[2];
		d = clients[3];

		switch (q) {
		case 1:
			resize(a, nx, ny, nw - 2 * a->bw, nh - 2 * a->bw, False);
			q = 0;
			break;
		case 2:
			tnw = (nw - iv) * (a->cfact / (a->cfact + b->cfact));
			resize(a, nx, ny, tnw - 2 * a->bw, nh - 2 * a->bw, False);
			resize(b, nx + tnw + iv, ny, nw - iv - tnw - 2 * b->bw, nh - 2 * b->bw, False);
			nx += tnw + iv;
			q = 0;
			break;
		case 3:
			tnw = (nw - iv) * (a->cfact / (a->cfact + b->cfact));
			tnh = (nh - ih) * (b->cfact / (b->cfact + c->cfact));
			if (!s && i % 2) {
				resize(a, nx + nw - tnw, ny, tnw - 2 * a->bw, nh - 2 * a->bw, False);
				resize(b, nx, ny + nh - tnh, nw - iv - tnw - 2 * b->bw, tnh - 2 * b->bw, False);
				resize(c, nx, ny, nw - iv - tnw - 2 * c->bw, nh - ih - tnh - 2 * c->bw, False);

			} else {
				resize(a, nx, ny, tnw - 2 * a->bw, nh - 2 * a->bw, False);
				resize(b, nx + tnw + iv, ny, nw - iv - tnw - 2 * b->bw, tnh - 2 * b->bw, False);
				resize(c, nx + tnw + iv, ny + tnh + ih, nw - iv - tnw - 2 * c->bw, nh - ih - tnh - 2 * c->bw, False);
				nx += tnw + iv;
				ny += tnh + ih;
			}
			nw -= tnw + iv;
			nh -= tnh + ih;
			q = 0;
			break;
		case 4:
			if (!s && i % 2) {
				tnw = (nw - iv) * (a->cfact / (a->cfact + b->cfact));
				tnh = (nh - ih) * (b->cfact / (b->cfact + c->cfact));
				resize(a, nx + nw - tnw, ny, tnw - 2 * a->bw, nh - 2 * a->bw, False);
				resize(b, nx, ny + nh - tnh, nw - iv - tnw - 2 * b->bw, tnh - 2 * b->bw, False);
			} else {
				tnw = (nw - iv) * (a->cfact / (a->cfact + b->cfact));
				tnh = (nh - ih) * (b->cfact / (b->cfact + c->cfact));
				resize(a, nx, ny, tnw - 2 * a->bw, nh - 2 * a->bw, False);
				resize(b, nx + tnw + iv, ny, nw - iv - tnw - 2 * b->bw, tnh - 2 * b->bw, False);
				nx += tnw + iv;
				ny += tnh + ih;
			}
			nw -= tnw + iv;
			nh -= tnh + ih;

			if (j + 1 == ai + an) {
				tnw = (nw - iv) * (c->cfact / (c->cfact + d->cfact));
				resize(c, nx, ny, tnw - 2 * c->bw, nh - 2 * c->bw, False);
				resize(d, nx + tnw + iv, ny, nw - iv - tnw - 2 * d->bw, nh - 2 * d->bw, False);
				q = 0;
				break;
			}

			clients[0] = c;
			clients[1] = d;
			q = 2;
			break;
		}
		i++;
	}
}

void
arrange_dwindle(Workspace *ws, int x, int y, int h, int w, int ih, int iv, int n, int an, int ai, int arr, int grp)
{
	arrange_fibonacci(ws, x, y, h, w, ih, iv, n, an, ai, 1, arr, grp);
}

void
arrange_dwindle_cfacts(Workspace *ws, int x, int y, int h, int w, int ih, int iv, int n, int an, int ai, int arr, int grp)
{
	arrange_fibonacci_cfacts(ws, x, y, h, w, ih, iv, n, an, ai, 1, arr, grp);
}

void
arrange_spiral(Workspace *ws, int x, int y, int h, int w, int ih, int iv, int n, int an, int ai, int arr, int grp)
{
	arrange_fibonacci(ws, x, y, h, w, ih, iv, n, an, ai, 0, arr, grp);
}

void
arrange_spiral_cfacts(Workspace *ws, int x, int y, int h, int w, int ih, int iv, int n, int an, int ai, int arr, int grp)
{
	arrange_fibonacci_cfacts(ws, x, y, h, w, ih, iv, n, an, ai, 0, arr, grp);
}

void
arrange_tatami(Workspace *ws, int x, int y, int h, int w, int ih, int iv, int n, int an, int ai, int arr, int grp)
{
	unsigned int i, j, nx, ny, nw, nh, tnx, tny, tnw, tnh, nhrest, hrest, wrest, areas, mats, cats;
	Client *c;

	nx = x;
	ny = y;
	nw = w;
	nh = h;

	mats = an / 5;
	cats = an % 5;
	hrest = 0;
	wrest = 0;

	areas = mats + (cats > 0);
	nh = (h - ih * (areas - 1)) / areas;
	nhrest = (h - ih * (areas - 1)) % areas;

	for (i = 0, j = 0, c = nexttiled(ws->clients); c && j < (ai + an); c = nexttiled(c->next), j++) {
		if (j < ai)
			continue;

		c->area = grp;
		c->arr = arr;
		tnw = nw;
		tnx = nx;
		tnh = nh;
		tny = ny;

		if (j < ai + cats) {
			/* Arrange cats (all excess clients that can't be tiled as mats). Cats sleep on mats. */

 			switch (cats) {
			case 1: // fill
				break;
			case 2: // up and down
				if ((i % 5) == 0) //up
					tnh = (nh - ih) / 2 + (nh - ih) % 2;
				else if ((i % 5) == 1) { //down
					tny += (nh - ih) / 2 + (nh - ih) % 2 + ih;
					tnh = (nh - ih) / 2;
				}
				break;
			case 3: // bottom, up-left and up-right
				if ((i % 5) == 0) { // up-left
					tnw = (nw - iv) / 2 + (nw - iv) % 2;
					tnh = (nh - ih) * 2 / 3 + (nh - ih) * 2 % 3;
				} else if ((i % 5) == 1) { // up-right
					tnx += (nw - iv) / 2 + (nw - iv) % 2 + iv;
					tnw = (nw - iv) / 2;
					tnh = (nh - ih) * 2 / 3 + (nh - ih) * 2 % 3;
				} else if ((i % 5) == 2) { // bottom
					tnh = (nh - ih) / 3;
					tny += (nh - ih) * 2 / 3 + (nh - ih) * 2 % 3 + ih;
				}
				break;
			case 4: // bottom, left, right and top
				if ((i % 5) == 0) { // top
					hrest = (nh - 2 * ih) % 4;
					tnh = (nh - 2 * ih) / 4 + (hrest ? 1 : 0);
				} else if ((i % 5) == 1) { // left
					tnw = (nw - iv) / 2 + (nw - iv) % 2;
					tny += (nh - 2 * ih) / 4 + (hrest ? 1 : 0) + ih;
					tnh = (nh - 2 * ih) * 2 / 4 + (hrest > 1 ? 1 : 0);
				} else if ((i % 5) == 2) { // right
					tnx += (nw - iv) / 2 + (nw - iv) % 2 + iv;
					tnw = (nw - iv) / 2;
					tny += (nh - 2 * ih) / 4 + (hrest ? 1 : 0) + ih;
					tnh = (nh - 2 * ih) * 2 / 4 + (hrest > 1 ? 1 : 0);
				} else if ((i % 5) == 3) { // bottom
					tny += (nh - 2 * ih) / 4 + (hrest ? 1 : 0) + (nh - 2 * ih) * 2 / 4 + (hrest > 1 ? 1 : 0) + 2 * ih;
					tnh = (nh - 2 * ih) / 4 + (hrest > 2 ? 1 : 0);
				}
				break;
			}

		} else {
			/* Arrange mats. One mat is a collection of five clients arranged tatami style */

			if (((i - cats) % 5) == 0) {
				if ((cats > 0) || ((i - cats) >= 5)) {
					tny = ny = ny + nh + (nhrest > 0 ? 1 : 0) + ih;
					--nhrest;
				}
			}

			switch ((i - cats) % 5) {
			case 0: // top-left-vert
				wrest = (nw - 2 * iv) % 3;
				hrest = (nh - 2 * ih) % 3;
				tnw = (nw - 2 * iv) / 3 + (wrest ? 1 : 0);
				tnh = (nh - 2 * ih) * 2 / 3 + hrest + iv;
				break;
			case 1: // top-right-hor
				tnx += (nw - 2 * iv) / 3 + (wrest ? 1 : 0) + iv;
				tnw = (nw - 2 * iv) * 2 / 3 + (wrest > 1 ? 1 : 0) + iv;
				tnh = (nh - 2 * ih) / 3 + (hrest ? 1 : 0);
				break;
			case 2: // center
				tnx += (nw - 2 * iv) / 3 + (wrest ? 1 : 0) + iv;
				tnw = (nw - 2 * iv) / 3 + (wrest > 1 ? 1 : 0);
				tny += (nh - 2 * ih) / 3 + (hrest ? 1 : 0) + ih;
				tnh = (nh - 2 * ih) / 3 + (hrest > 1 ? 1 : 0);
				break;
			case 3: // bottom-right-vert
				tnx += (nw - 2 * iv) * 2 / 3 + wrest + 2 * iv;
				tnw = (nw - 2 * iv) / 3;
				tny += (nh - 2 * ih) / 3 + (hrest ? 1 : 0) + ih;
				tnh = (nh - 2 * ih) * 2 / 3 + hrest + iv;
				break;
			case 4: // (oldest) bottom-left-hor
				tnw = (nw - 2 * iv) * 2 / 3 + wrest + iv;
				tny += (nh - 2 * ih) * 2 / 3 + hrest + 2 * iv;
				tnh = (nh - 2 * ih) / 3;
				break;
			}

		}

		resize(c, tnx, tny, tnw - 2 * c->bw, tnh - 2 * c->bw, False);
		++i;
	}
}

void
arrange_tatami_cfacts(Workspace *ws, int x, int y, int h, int w, int ih, int iv, int n, int an, int ai, int arr, int grp)
{
	Client *clients[5] = { NULL, NULL, NULL, NULL, NULL };
	unsigned int j, s, nx, ny, nw, nh, tnw, tmw, tnh, tmh, areas, mats, cats, nhrest;
	Client *t, *a, *b, *c, *d, *e;

	nx = x;
	ny = y;
	nw = w;
	nh = h;

	mats = an / 5;
	cats = an % 5;

	areas = mats + (cats > 0);
	if (cats == 0)
		cats = 5;
	nh = (h - ih * (areas - 1)) / areas;
	nhrest = (h - ih * (areas - 1)) % areas;

	for (j = 0, s = 0, t = nexttiled(ws->clients); t && j < (ai + an); t = nexttiled(t->next), j++) {
		if (j < ai)
			continue;

		t->area = grp;
		t->arr = arr;
		clients[s] = t;
		++s;

		if (s < cats)
			continue;

		a = clients[0];
		b = clients[1];
		c = clients[2];
		d = clients[3];
		e = clients[4];
		s = 0;

		if (cats < 5) {
			/* Arrange cats (all excess clients that can't be tiled as mats). Cats sleep on mats. */
			switch (cats) {
			case 1: // fill
				resize(a, nx, ny, nw - 2 * a->bw, nh - 2 * a->bw, False);
				break;
			case 2: // up and down
				tnh = (nh - ih) * (a->cfact / (a->cfact + b->cfact));
				resize(a, nx, ny, nw - 2 * a->bw, tnh - 2 * a->bw, False);
				resize(b, nx, ny + tnh + ih, nw - 2 * b->bw, (nh - tnh - ih) - 2 * b->bw, False);
				break;
			case 3: // bottom, up-left and up-right
				tnw = (nw - iv) * (a->cfact / (a->cfact + b->cfact));
				tnh = (nh - ih) * (c->cfact / (a->cfact + b->cfact + c->cfact));
				resize(a, nx, ny, tnw - 2 * a->bw, nh - ih - tnh - 2 * a->bw, False);
				resize(b, nx + tnw + iv, ny, nw - iv - tnw - 2 * b->bw, nh - ih - tnh - 2 * b->bw, False);
				resize(c, nx, ny + nh - tnh, nw - 2 * c->bw, tnh - 2 * c->bw, False);
				break;
			case 4: // bottom, left, right and top
				tnw = (nw - iv) * (b->cfact / (b->cfact + c->cfact));
				tnh = (nh - 2 * ih) * (a->cfact / (a->cfact + b->cfact + c->cfact + d->cfact));
				tmh = (nh - 2 * ih) * ((b->cfact + c->cfact) / (a->cfact + b->cfact + c->cfact + d->cfact));
				resize(a, nx, ny, nw - 2 * a->bw, tnh - 2 * a->bw, False);
				resize(b, nx, ny + tnh + iv, tnw - 2 * a->bw, tmh - 2 * a->bw, False);
				resize(c, nx + iv + tnw, ny + tnh + iv, nw - iv - tnw - 2 * a->bw, tmh - 2 * a->bw, False);
				resize(d, nx, ny + tnh + 2 * iv + tmh, nw - 2 * a->bw, nh - 2 * iv - tnh - tmh - 2 * a->bw, False);
				break;
			}

			cats = 5;
		} else {
			/* Arrange mats. One mat is a collection of five clients arranged tatami style */
			tnw = (nw - 2 * ih) * (a->cfact / (a->cfact + c->cfact + d->cfact));
			tmw = (nw - 2 * ih) * (d->cfact / (a->cfact + c->cfact + d->cfact));
			tnh = (nh - 2 * ih) * (e->cfact / (b->cfact + c->cfact + e->cfact));
			tmh = (nh - 2 * ih) * (b->cfact / (b->cfact + c->cfact + e->cfact));
			resize(a, nx, ny, tnw - 2 * a->bw, nh - tnh - ih - 2 * a->bw, False);
			resize(b, nx + tnw + iv, ny, nw - iv - tnw - 2 * a->bw, tmh - 2 * a->bw, False);
			resize(c, nx + tnw + iv, ny + tmh + ih, nw - tnw - tmw - 2 * iv - 2 * c->bw, nh - tnh - tmh - 2 * ih - 2 * c->bw, False);
			resize(d, nx + nw - tmw, ny + tmh + ih, tmw - 2 * d->bw, nh - tmh - ih - 2 * d->bw, False);
			resize(e, nx, ny + nh - tnh, nw - tmw - iv - 2 * e->bw, tnh - 2 * e->bw, False);
		}

		ny += nh + ih + (nhrest > 0 ? 1 : 0);
		--nhrest;
	}
}

void
flextile(Workspace *ws)
{
	unsigned int n;
	int oh = 0, ov = 0, ih = 0, iv = 0; // gaps outer/inner horizontal/vertical
	int x, y, h, w;

	getgaps(ws, &oh, &ov, &ih, &iv, &n);

	if (ws->layout->preset.layout != ws->ltaxis[LAYOUT] ||
			ws->layout->preset.masteraxis != ws->ltaxis[MASTER] ||
			ws->layout->preset.stack1axis != ws->ltaxis[STACK] ||
			ws->layout->preset.stack2axis != ws->ltaxis[STACK2])
		setflexsymbols(ws, n);
	else if (ws->layout->preset.symbolfunc != NULL)
		ws->layout->preset.symbolfunc(ws, n);
	if (n == 0) {
		setwindowborders(ws, ws->sel);
		return;
	}

	if (enabled(SmartGapsMonocle)) {
		/* Apply outer gap factor if full screen monocle */
		if (abs(ws->ltaxis[MASTER]) == MONOCLE && (abs(ws->ltaxis[LAYOUT]) == NO_SPLIT || n <= ws->nmaster)) {
			oh = ws->mon->gappoh * smartgaps_fact;
			ov = ws->mon->gappov * smartgaps_fact;
		}
	}

	x = ws->wx + ov;
	y = ws->wy + oh;
	h = ws->wh - 2*oh;
	w = ws->ww - 2*ov;

	(&flexlayouts[abs(ws->ltaxis[LAYOUT])])->arrange(ws, x, y, h, w, ih, iv, n);
	setwindowborders(ws, ws->sel);
	return;
}

void
setflexsymbols(Workspace *ws, unsigned int n)
{
	int l;
	char sym1, sym2, sym3;
	Client *c;

	if (n == 0)
		for (c = nexttiled(ws->clients); c; c = nexttiled(c->next), n++);

	l = abs(ws->ltaxis[LAYOUT]);
	if (ws->ltaxis[MASTER] == MONOCLE && (l == NO_SPLIT || !ws->nmaster || n <= ws->nmaster)) {
		monoclesymbols(ws, n);
		return;
	}

	if (ws->ltaxis[STACK] == MONOCLE && (l == SPLIT_VERTICAL || l == SPLIT_HORIZONTAL_FIXED)) {
		decksymbols(ws, n);
		return;
	}

	/* Layout symbols */
	if (l == NO_SPLIT || !ws->nmaster) {
		sym1 = sym2 = sym3 = (int)tilesymb[ws->ltaxis[MASTER]];
	} else {
		sym2 = layoutsymb[l];
		if (ws->ltaxis[LAYOUT] < 0) {
			sym1 = tilesymb[ws->ltaxis[STACK]];
			sym3 = tilesymb[ws->ltaxis[MASTER]];
		} else {
			sym1 = tilesymb[ws->ltaxis[MASTER]];
			sym3 = tilesymb[ws->ltaxis[STACK]];
		}
	}

	snprintf(ws->ltsymbol, sizeof ws->ltsymbol, "%c%c%c", sym1, sym2, sym3);
}

void
monoclesymbols(Workspace *ws, unsigned int n)
{
	if (n > 0)
		snprintf(ws->ltsymbol, sizeof ws->ltsymbol, "[%d]", n);
	else
		snprintf(ws->ltsymbol, sizeof ws->ltsymbol, "[M]");
}

void
decksymbols(Workspace *ws, unsigned int n)
{
	if (n > ws->nmaster)
		snprintf(ws->ltsymbol, sizeof ws->ltsymbol, "[]%d", n);
	else
		snprintf(ws->ltsymbol, sizeof ws->ltsymbol, "[D]");
}

/* Mirror layout axis for flextile */
void
mirrorlayout(const Arg *arg)
{
	Workspace *ws = selws;
	if (!ws->layout->arrange)
		return;
	ws->ltaxis[LAYOUT] *= -1;
	arrange(ws);
}

/* Rotate layout axis for flextile */
void
rotatelayoutaxis(const Arg *arg)
{
	int incr = (arg->i > 0 ? 1 : -1);
	int axis = labs(arg->i) - 1;
	Workspace *ws = selws;

	if (!ws->layout->arrange || arg->i == 0)
		return;
	if (axis == LAYOUT) {
		if (ws->ltaxis[LAYOUT] >= 0) {
			ws->ltaxis[LAYOUT] += incr;
			if (ws->ltaxis[LAYOUT] >= LAYOUT_LAST)
				ws->ltaxis[LAYOUT] = 0;
			else if (ws->ltaxis[LAYOUT] < 0)
				ws->ltaxis[LAYOUT] = LAYOUT_LAST - 1;
		} else {
			ws->ltaxis[LAYOUT] -= incr;
			if (ws->ltaxis[LAYOUT] <= -LAYOUT_LAST)
				ws->ltaxis[LAYOUT] = 0;
			else if (ws->ltaxis[LAYOUT] > 0)
				ws->ltaxis[LAYOUT] = -LAYOUT_LAST + 1;
		}
	} else {
		ws->ltaxis[axis] += incr;
		if (ws->ltaxis[axis] >= AXIS_LAST)
			ws->ltaxis[axis] = 0;
		else if (ws->ltaxis[axis] < 0)
			ws->ltaxis[axis] = AXIS_LAST - 1;
	}
	arrange(ws);
	setflexsymbols(ws, 0);
}
