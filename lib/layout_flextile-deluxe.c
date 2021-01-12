typedef struct {
	void (*arrange)(Monitor *, int, int, int, int, int, int, int);
} LayoutArranger;

typedef struct {
	void (*arrange)(Monitor *, int, int, int, int, int, int, int, int, int);
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
	{ arrange_gapplessgrid_alt1 },
	{ arrange_gapplessgrid_alt2 },
	{ arrange_gridmode },
	{ arrange_horizgrid },
	{ arrange_dwindle },
	{ arrange_spiral },
};

static void
getfactsforrange(Monitor *m, int an, int ai, int size, int *rest, float *fact)
{
	int i;
	float facts;
	Client *c;
	int total = 0;
	Workspace *ws = MWS(m);

	facts = 0;
	for (i = 0, c = nexttiled(ws->clients); c; c = nexttiled(c->next), i++)
		if (i >= ai && i < (ai + an))
			facts += c->cfact;

	for (i = 0, c = nexttiled(ws->clients); c; c = nexttiled(c->next), i++)
		if (i >= ai && i < (ai + an))
			total += size * (c->cfact / facts);

	*rest = size - total;
	*fact = facts;
}

static void
setlayoutaxisex(const Arg *arg)
{
	int axis, arr;
	Workspace *ws = selws;

	axis = arg->i & 0x3; // lower two bytes indicates layout, master or stack1-2
	arr = ((arg->i & 0xFC) >> 2); // remaining six upper bytes indicate arrangement

	if ((axis == 0 && abs(arr) > LAYOUT_LAST)
			|| (axis > 0 && (arr > AXIS_LAST || arr < 0)))
		arr = 0;

	ws->ltaxis[axis] = arr;
	arrangews(ws);
}

static void
setlayoutex(const Arg *arg)
{
	setlayout(&((Arg) { .v = &layouts[arg->i] }));
}

static void
layoutmonconvert(Workspace *ws, Monitor *from, Monitor *to)
{
	int from_orientation = from->mw < from->mh;
	int to_orientation = to->mw < to->mh;

	if (from_orientation == to_orientation)
		return;

	if (ws->layout->arrange != flextile)
		return;

	ws->ltaxis[LAYOUT] = convert_split(ws->ltaxis[LAYOUT]);
	ws->ltaxis[MASTER] = convert_arrange(ws->ltaxis[MASTER]);
	ws->ltaxis[STACK] = convert_arrange(ws->ltaxis[STACK]);
	ws->ltaxis[STACK2] = convert_arrange(ws->ltaxis[STACK2]);
}

static int
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

static int
convert_arrange(int arrange)
{
	if (arrange == TOP_TO_BOTTOM)
		return LEFT_TO_RIGHT;

	if (arrange == LEFT_TO_RIGHT)
		return TOP_TO_BOTTOM;

	return arrange;
}


static void
layout_no_split(Monitor *m, int x, int y, int h, int w, int ih, int iv, int n)
{
	Workspace *ws = MWS(m);
	(&flextiles[ws->ltaxis[ws->nmaster >= n ? MASTER : STACK]])->arrange(m, x, y, h, w, ih, iv, n, n, 0);
}

static void
layout_split_vertical(Monitor *m, int x, int y, int h, int w, int ih, int iv, int n)
{
	Workspace *ws = MWS(m);
	/* Split master into master + stack if we have enough clients */
	if (ws->nmaster && n > ws->nmaster) {
		layout_split_vertical_fixed(m, x, y, h, w, ih, iv, n);
	} else {
		layout_no_split(m, x, y, h, w, ih, iv, n);
	}
}

static void
layout_split_vertical_fixed(Monitor *m, int x, int y, int h, int w, int ih, int iv, int n)
{
	int sw, sx;
	Workspace *ws = MWS(m);

	sw = (w - iv) * (1 - ws->mfact);
	w = (w - iv) * ws->mfact;
	if (ws->ltaxis[LAYOUT] < 0) { // mirror
		sx = x;
		x += sw + iv;
	} else {
		sx = x + w + iv;
	}

	(&flextiles[ws->ltaxis[MASTER]])->arrange(m, x, y, h, w, ih, iv, n, ws->nmaster, 0);
	(&flextiles[ws->ltaxis[STACK]])->arrange(m, sx, y, h, sw, ih, iv, n, n - ws->nmaster, ws->nmaster);
}

static void
layout_split_vertical_dual_stack(Monitor *m, int x, int y, int h, int w, int ih, int iv, int n)
{
	Workspace *ws = MWS(m);
	/* Split master into master + stack if we have enough clients */
	if (!ws->nmaster || n <= ws->nmaster) {
		layout_no_split(m, x, y, h, w, ih, iv, n);
	} else if (n <= ws->nmaster + (ws->nstack ? ws->nstack : 1)) {
		layout_split_vertical(m, x, y, h, w, ih, iv, n);
	} else {
		layout_split_vertical_dual_stack_fixed(m, x, y, h, w, ih, iv, n);
	}
}

static void
layout_split_vertical_dual_stack_fixed(Monitor *m, int x, int y, int h, int w, int ih, int iv, int n)
{
	int sh, sw, sx, oy, sc;
	Workspace *ws = MWS(m);

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

	(&flextiles[ws->ltaxis[MASTER]])->arrange(m, x, y, h, w, ih, iv, n, ws->nmaster, 0);
	(&flextiles[ws->ltaxis[STACK]])->arrange(m, sx, y, sh, sw, ih, iv, n, sc, ws->nmaster);
	(&flextiles[ws->ltaxis[STACK2]])->arrange(m, sx, oy, sh, sw, ih, iv, n, n - ws->nmaster - sc, ws->nmaster + sc);
}

static void
layout_split_horizontal(Monitor *m, int x, int y, int h, int w, int ih, int iv, int n)
{
	Workspace *ws = MWS(m);
	/* Split master into master + stack if we have enough clients */
	if (ws->nmaster && n > ws->nmaster) {
		layout_split_horizontal_fixed(m, x, y, h, w, ih, iv, n);
	} else {
		layout_no_split(m, x, y, h, w, ih, iv, n);
	}
}

static void
layout_split_horizontal_fixed(Monitor *m, int x, int y, int h, int w, int ih, int iv, int n)
{
	int sh, sy;
	Workspace *ws = MWS(m);

	sh = (h - ih) * (1 - ws->mfact);
	h = (h - ih) * ws->mfact;
	if (ws->ltaxis[LAYOUT] < 0) { // mirror
		sy = y;
		y += sh + ih;
	} else {
		sy = y + h + ih;
	}

	(&flextiles[ws->ltaxis[MASTER]])->arrange(m, x, y, h, w, ih, iv, n, ws->nmaster, 0);
	(&flextiles[ws->ltaxis[STACK]])->arrange(m, x, sy, sh, w, ih, iv, n, n - ws->nmaster, ws->nmaster);
}

static void
layout_split_horizontal_dual_stack(Monitor *m, int x, int y, int h, int w, int ih, int iv, int n)
{
	Workspace *ws = MWS(m);
	/* Split master into master + stack if we have enough clients */
	if (!ws->nmaster || n <= ws->nmaster) {
		layout_no_split(m, x, y, h, w, ih, iv, n);
	} else if (n <= ws->nmaster + (ws->nstack ? ws->nstack : 1)) {
		layout_split_horizontal(m, x, y, h, w, ih, iv, n);
	} else {
		layout_split_horizontal_dual_stack_fixed(m, x, y, h, w, ih, iv, n);
	}
}

static void
layout_split_horizontal_dual_stack_fixed(Monitor *m, int x, int y, int h, int w, int ih, int iv, int n)
{
	int sh, sy, ox, sc;
	Workspace *ws = MWS(m);

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

	(&flextiles[ws->ltaxis[MASTER]])->arrange(m, x, y, h, w, ih, iv, n, ws->nmaster, 0);
	(&flextiles[ws->ltaxis[STACK]])->arrange(m, x, sy, sh, sw, ih, iv, n, sc, ws->nmaster);
	(&flextiles[ws->ltaxis[STACK2]])->arrange(m, ox, sy, sh, sw, ih, iv, n, n - ws->nmaster - sc, ws->nmaster + sc);
}

static void
layout_split_centered_vertical(Monitor *m, int x, int y, int h, int w, int ih, int iv, int n)
{
	Workspace *ws = MWS(m);
	/* Split master into master + stack if we have enough clients */
	if (!ws->nmaster || n <= ws->nmaster) {
		layout_no_split(m, x, y, h, w, ih, iv, n);
	} else if (n <= ws->nmaster + (ws->nstack ? ws->nstack : 1)) {
		layout_split_vertical(m, x, y, h, w, ih, iv, n);
	} else {
		layout_split_centered_vertical_fixed(m, x, y, h, w, ih, iv, n);
	}
}

static void
layout_split_centered_vertical_fixed(Monitor *m, int x, int y, int h, int w, int ih, int iv, int n)
{
	int sw, sx, ox, sc;
	Workspace *ws = MWS(m);

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

	(&flextiles[ws->ltaxis[MASTER]])->arrange(m, x, y, h, w, ih, iv, n, ws->nmaster, 0);
	(&flextiles[ws->ltaxis[STACK]])->arrange(m, sx, y, h, sw, ih, iv, n, sc, ws->nmaster);
	(&flextiles[ws->ltaxis[STACK2]])->arrange(m, ox, y, h, sw, ih, iv, n, n - ws->nmaster - sc, ws->nmaster + sc);
}

static void
layout_split_centered_horizontal(Monitor *m, int x, int y, int h, int w, int ih, int iv, int n)
{
	Workspace *ws = MWS(m);
	/* Split master into master + stack if we have enough clients */
	if (!ws->nmaster || n <= ws->nmaster) {
		layout_no_split(m, x, y, h, w, ih, iv, n);
	} else if (n <= ws->nmaster + (ws->nstack ? ws->nstack : 1)) {
		layout_split_horizontal(m, x, y, h, w, ih, iv, n);
	} else {
		layout_split_centered_horizontal_fixed(m, x, y, h, w, ih, iv, n);
	}
}

static void
layout_split_centered_horizontal_fixed(Monitor *m, int x, int y, int h, int w, int ih, int iv, int n)
{
	int sh, sy, oy, sc;
	Workspace *ws = MWS(m);

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

	(&flextiles[ws->ltaxis[MASTER]])->arrange(m, x, y, h, w, ih, iv, n, ws->nmaster, 0);
	(&flextiles[ws->ltaxis[STACK]])->arrange(m, x, sy, sh, w, ih, iv, n, sc, ws->nmaster);
	(&flextiles[ws->ltaxis[STACK2]])->arrange(m, x, oy, sh, w, ih, iv, n, n - ws->nmaster - sc, ws->nmaster + sc);
}

static void
layout_floating_master(Monitor *m, int x, int y, int h, int w, int ih, int iv, int n)
{
	Workspace *ws = MWS(m);
	/* Split master into master + stack if we have enough clients */
	if (!ws->nmaster || n <= ws->nmaster) {
		layout_no_split(m, x, y, h, w, ih, iv, n);
	} else {
		layout_floating_master_fixed(m, x, y, h, w, ih, iv, n);
	}
}

static void
layout_floating_master_fixed(Monitor *m, int x, int y, int h, int w, int ih, int iv, int n)
{
	int mh, mw;
	Workspace *ws = MWS(m);

	/* Draw stack area first */
	(&flextiles[ws->ltaxis[STACK]])->arrange(m, x, y, h, w, ih, iv, n, n - ws->nmaster, ws->nmaster);

	if (w > h) {
		mw = w * ws->mfact;
		mh = h * 0.9;
	} else {
		mw = w * 0.9;
		mh = h * ws->mfact;
	}
	x = x + (w - mw) / 2;
	y = y + (h - mh) / 2;

	(&flextiles[ws->ltaxis[MASTER]])->arrange(m, x, y, mh, mw, ih, iv, n, ws->nmaster, 0);
}

static void
arrange_left_to_right(Monitor *m, int x, int y, int h, int w, int ih, int iv, int n, int an, int ai)
{
	int i, rest;
	float facts, fact = 1;
	Client *c;
	Workspace *ws = MWS(m);

	if (ai + an > n)
		an = n - ai;

	w -= iv * (an - 1);
	getfactsforrange(m, an, ai, w, &rest, &facts);
	for (i = 0, c = nexttiled(ws->clients); c; c = nexttiled(c->next), i++) {
		if (i >= ai && i < (ai + an)) {
			fact = c->cfact;
			resize(c, x, y, w * (fact / facts) + ((i - ai) < rest ? 1 : 0) - (2 * c->bw), h - (2 * c->bw), 0);
			x += WIDTH(c) + iv;
		}
	}
}

static void
arrange_top_to_bottom(Monitor *m, int x, int y, int h, int w, int ih, int iv, int n, int an, int ai)
{
	int i, rest;
	float facts, fact = 1;
	Client *c;
	Workspace *ws = MWS(m);

	if (ai + an > n)
		an = n - ai;

	h -= ih * (an - 1);
	getfactsforrange(m, an, ai, h, &rest, &facts);
	for (i = 0, c = nexttiled(ws->clients); c; c = nexttiled(c->next), i++) {
		if (i >= ai && i < (ai + an)) {
			fact = c->cfact;
			resize(c, x, y, w - (2 * c->bw), h * (fact / facts) + ((i - ai) < rest ? 1 : 0) - (2 * c->bw), 0);
			y += HEIGHT(c) + ih;
		}
	}
}

static void
arrange_monocle(Monitor *m, int x, int y, int h, int w, int ih, int iv, int n, int an, int ai)
{
	int i, stackno, minstackno = 0xFFFFFF;
	Client *c, *s, *f = NULL;
	Workspace *ws = MWS(m);

	for (i = 0, c = nexttiled(ws->clients); c; c = nexttiled(c->next), i++)
		if (i >= ai && i < (ai + an)) {
			for (stackno = 0, s = ws->stack; s && s != c; s = s->snext, ++stackno);
			if (stackno < minstackno) {
				f = s;
				minstackno = stackno;
			}
		}

	for (i = 0, c = nexttiled(ws->clients); c; c = nexttiled(c->next), i++)
		if (i >= ai && i < (ai + an)) {
			if (c == f) {
				XMoveWindow(dpy, c->win, x, y);
				resize(c, x, y, w - (2 * c->bw), h - (2 * c->bw), 0);
			} else {
				XMoveWindow(dpy, c->win, WIDTH(c) * -2, c->y);
			}
		}
}

static void
arrange_gridmode(Monitor *m, int x, int y, int h, int w, int ih, int iv, int n, int an, int ai)
{
	int i, cols, rows, ch, cw, cx, cy, cc, cr, chrest, cwrest; // counters
	Client *c;
	Workspace *ws = MWS(m);

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
	for (i = 0, c = nexttiled(ws->clients); c; c = nexttiled(c->next), i++) {
		if (i >= ai && i < (ai + an)) {
			cc = ((i - ai) / rows); // client column number
			cr = ((i - ai) % rows); // client row number
			cx = x + cc * (cw + iv) + MIN(cc, cwrest);
			cy = y + cr * (ch + ih) + MIN(cr, chrest);
			resize(c, cx, cy, cw + (cc < cwrest ? 1 : 0) - 2 * c->bw, ch + (cr < chrest ? 1 : 0) - 2 * c->bw, False);
		}
	}
}

static void
arrange_horizgrid(Monitor *m, int x, int y, int h, int w, int ih, int iv, int n, int an, int ai)
{
	int ntop, nbottom, rh, rest;

	/* Exception when there is only one client; don't split into two rows */
	if (an == 1) {
		arrange_monocle(m, x, y, h, w, ih, iv, n, an, ai);
		return;
	}

	ntop = an / 2;
	nbottom = an - ntop;
	rh = (h - ih) / 2;
	rest = h - ih - rh * 2;
	arrange_left_to_right(m, x, y, rh + rest, w, ih, iv, n, ntop, ai);
	arrange_left_to_right(m, x, y + rh + ih + rest, rh, w, ih, iv, n, nbottom, ai + ntop);
}

static void
arrange_gapplessgrid(Monitor *m, int x, int y, int h, int w, int ih, int iv, int n, int an, int ai)
{
	int i, cols, rows, ch, cw, cn, rn, cc, rrest, crest; // counters
	Client *c;
	Workspace *ws = MWS(m);

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

	for (i = 0, c = nexttiled(ws->clients); c; c = nexttiled(c->next), i++) {
		if (i >= ai && i < (ai + an)) {
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
}

/* This version of gappless grid fills rows first */
static void
arrange_gapplessgrid_alt1(Monitor *m, int x, int y, int h, int w, int ih, int iv, int n, int an, int ai)
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
		arrange_left_to_right(m, x, y, ch + (i < rest ? 1 : 0), w, ih, iv, n, MIN(cols, an - i*cols), ai + i*cols);
		y += ch + (i < rest ? 1 : 0) + ih;
	}
}

/* This version of gappless grid fills columns first */
static void
arrange_gapplessgrid_alt2(Monitor *m, int x, int y, int h, int w, int ih, int iv, int n, int an, int ai)
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
		arrange_top_to_bottom(m, x, y, h, cw + (i < rest ? 1 : 0), ih, iv, n, MIN(rows, an - i*rows), ai + i*rows);
		x += cw + (i < rest ? 1 : 0) + iv;
	}
}

static void
arrange_fibonacci(Monitor *m, int x, int y, int h, int w, int ih, int iv, int n, int an, int ai, int s)
{
	int i, j, nv, hrest = 0, wrest = 0, nx = x, ny = y, nw = w, nh = h, r = 1;
	Client *c;
	Workspace *ws = MWS(m);

	for (i = 0, j = 0, c = nexttiled(ws->clients); c; c = nexttiled(c->next), j++) {
		if (j >= ai && j < (ai + an)) {
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
					}
					else {
						nh -= hrest;
						ny -= nh + ih;
					}
				}
				else if ((i % 4) == 1) {
					nx += nw + iv;
					nw += wrest;
				}
				else if ((i % 4) == 2) {
					ny += nh + ih;
					nh += hrest;
					if (i < n - 1)
						nw += wrest;
				}
				else if ((i % 4) == 3) {
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
				}
				else if (i == 1)
					nw = w - nw - iv;
				i++;
			}

			resize(c, nx, ny, nw - 2 * c->bw, nh - 2 * c->bw, False);
		}
	}
}

static void
arrange_dwindle(Monitor *m, int x, int y, int h, int w, int ih, int iv, int n, int an, int ai)
{
	arrange_fibonacci(m, x, y, h, w, ih, iv, n, an, ai, 1);
}

static void
arrange_spiral(Monitor *m, int x, int y, int h, int w, int ih, int iv, int n, int an, int ai)
{
	arrange_fibonacci(m, x, y, h, w, ih, iv, n, an, ai, 0);
}

static void
flextile(Monitor *m)
{
	fprintf(stderr, "flextile: -->\n");
	unsigned int n;
	int oh = 0, ov = 0, ih = 0, iv = 0; // gaps outer/inner horizontal/vertical
	fprintf(stderr, "flextile: %d\n", 1);
	Workspace *ws = MWS(m);
	fprintf(stderr, "flextile: %d - ws = %s\n", 2, ws ? ws->name : "NULL");
	getgaps(ws, &oh, &ov, &ih, &iv, &n);

	fprintf(stderr, "flextile: %d\n", 3);
	if (ws->layout->preset.layout != ws->ltaxis[LAYOUT] ||
			ws->layout->preset.masteraxis != ws->ltaxis[MASTER] ||
			ws->layout->preset.stack1axis != ws->ltaxis[STACK] ||
			ws->layout->preset.stack2axis != ws->ltaxis[STACK2])
		setflexsymbols(m, n);
	else if (ws->layout->preset.symbolfunc != NULL)
		ws->layout->preset.symbolfunc(m, n);
	fprintf(stderr, "flextile: %d\n", 7);
	if (n == 0)
		return;

	if (enabled(SmartGapsMonocle)) {
		/* Apply outer gap factor if full screen monocle */
		if (abs(ws->ltaxis[MASTER]) == MONOCLE && (abs(ws->ltaxis[LAYOUT]) == NO_SPLIT || n <= ws->nmaster)) {
			oh = m->gappoh * smartgaps_fact;
			ov = m->gappov * smartgaps_fact;
		}
	}
	m = ws->mon;
	fprintf(stderr, "flextile: %d\n", 9);
	(&flexlayouts[abs(ws->ltaxis[LAYOUT])])->arrange(m, m->wx + ov, m->wy + oh, m->wh - 2*oh, m->ww - 2*ov, ih, iv, n);
	fprintf(stderr, "flextile: <--\n");
	return;
}

static void
setflexsymbols(Monitor *m, unsigned int n)
{
	int l;
	char sym1, sym2, sym3;
	Client *c;
	Workspace *ws = MWS(m);

	if (n == 0)
		for (c = nexttiled(ws->clients); c; c = nexttiled(c->next), n++);

	l = abs(ws->ltaxis[LAYOUT]);
	if (ws->ltaxis[MASTER] == MONOCLE && (l == NO_SPLIT || !ws->nmaster || n <= ws->nmaster)) {
		monoclesymbols(m, n);
		return;
	}

	if (ws->ltaxis[STACK] == MONOCLE && (l == SPLIT_VERTICAL || l == SPLIT_HORIZONTAL_FIXED)) {
		decksymbols(m, n);
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

static void
monoclesymbols(Monitor *m, unsigned int n)
{
	Workspace *ws = MWS(m);
	if (n > 0)
		snprintf(ws->ltsymbol, sizeof ws->ltsymbol, "[%d]", n);
	else
		snprintf(ws->ltsymbol, sizeof ws->ltsymbol, "[M]");
}

static void
decksymbols(Monitor *m, unsigned int n)
{
	Workspace *ws = MWS(m);
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
	arrangews(ws);
}

/* Rotate layout axis for flextile */
void
rotatelayoutaxis(const Arg *arg)
{
	int incr = (arg->i > 0 ? 1 : -1);
	int axis = abs(arg->i) - 1;
	Workspace *ws = selws;

	if (!ws->layout->arrange)
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
	arrangews(ws);
	setflexsymbols(selmon, 0);
}
