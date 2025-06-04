typedef struct {
	void (*layout)(Workspace *, FlexDim d);
} LayoutArranger;

typedef struct {
	void (*arrange)(Workspace *, FlexDim d);
} TileArranger;

#define ARRANGE(AREA, WS, FLEXDIM) FLEXDIM.grp = AREA; (&flextiles[ws->ltaxis[AREA]])->arrange(WS, FLEXDIM)
#define MIRROR (ws->ltaxis[LAYOUT] < 0)

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
	{ arrange_gaplessgrid },
	{ arrange_gaplessgrid_cfacts },
	{ arrange_gaplessgrid_alt1 },
	{ arrange_gaplessgrid_alt2 },
	{ arrange_gridmode },
	{ arrange_horizgrid },
	{ arrange_dwindle },
	{ arrange_dwindle_cfacts },
	{ arrange_spiral },
	{ arrange_spiral_cfacts },
	{ arrange_tatami },
	{ arrange_tatami_cfacts },
	{ arrange_aspectgrid },
	{ arrange_top_to_bottom_aspect },
	{ arrange_left_to_right_aspect },
};

/* workspace  symbol     nmaster, nstack, split, master axis, stack axis, secondary stack axis  */
void
customlayout(
	int ws_num,
	char *symbol,
	int nmaster,
	int nstack,
	int split,
	int master_axis,
	int stack_axis,
	int secondary_stack_axis
) {
	Workspace *ws;

	for (ws = workspaces; ws && ws->num != ws_num; ws = ws->next);
	if (!ws)
		ws = selws;
	ws->prevlayout = ws->layout;

	if (symbol != NULL)
		strlcpy(ws->ltsymbol, symbol, sizeof ws->ltsymbol);

	if (nmaster > -1)
		ws->nmaster = nmaster;

	if (nstack > -1)
		ws->nstack = nstack;

	/* split (layout), negative means mirror layout */
	if (labs(split) < LAYOUT_LAST)
		ws->ltaxis[LAYOUT] = split;

	if (master_axis > -1 && master_axis < AXIS_LAST)
		ws->ltaxis[MASTER] = master_axis;

	if (stack_axis > -1 && stack_axis < AXIS_LAST)
		ws->ltaxis[STACK] = stack_axis;

	if (secondary_stack_axis > -1 && secondary_stack_axis < AXIS_LAST)
		ws->ltaxis[STACK2] = secondary_stack_axis;

	arrange(ws);
}

void
getfactsforrange(Client *f, int n, int size, int *rest, float *fact, int include_mina)
{
	int i;
	float facts = 0;
	int total = 0;
	Client *c;

	for (i = 0, c = f; c && i < n; c = nexttiled(c->next), i++)
		if (include_mina || !c->mina)
			facts += c->cfact;

	for (i = 0, c = f; c && i < n; c = nexttiled(c->next), i++)
		if (include_mina || !c->mina)
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
setwindowborders(Workspace *ws, Client *sel)
{
	for (Client *s = ws->stack; s; s = s->snext)
		XSetWindowBorder(dpy, s->win, scheme[clientscheme(s, sel)][ColBorder].pixel);
}

void
layoutconvert(const Arg *arg)
{
	Workspace *ws = (arg && arg->v ? (Workspace*)arg->v : selws);
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

	/* Intentionally not converting TOP_TO_BOTTOM_AR and LEFT_TO_RIGHT_AR here because
	 * aspect ratios of windows do not change */

	return arrange;
}


void
layout_no_split(Workspace *ws, FlexDim d)
{
	int area = (ws->nmaster >= d.n ? MASTER : STACK);
	ARRANGE(area, ws, d);
}

void
layout_split_vertical(Workspace *ws, FlexDim d)
{
	/* Split master into master + stack if we have enough clients */
	if (ws->nmaster && d.n > ws->nmaster) {
		layout_split_vertical_fixed(ws, d);
	} else {
		layout_no_split(ws, d);
	}
}

void
layout_split_vertical_fixed(Workspace *ws, FlexDim d)
{
	int mx, mw, sx, sw;
	int iv = d.iv, w = d.w, x = d.x, n = d.n;
	FlexDim master = d, stack = d;

	/* Work out the size of the stack area vs the size of the master area */
	mw = (w - iv) * ws->mfact;
	sw = (w - iv) * (1 - ws->mfact);

	if (MIRROR) {
		sx = x;
		mx = sx + sw + iv;
	} else {
		mx = x;
		sx = mx + mw + iv;
	}

	/* Tile master area */
	master.an = ws->nmaster;
	master.x = mx;
	master.w = mw;
	ARRANGE(MASTER, ws, master);

	/* Tile stack area */
	stack.an = n - ws->nmaster;
	stack.ai = ws->nmaster;
	stack.x = sx;
	stack.w = sw;
	ARRANGE(STACK, ws, stack);
}

void
layout_split_vertical_dual_stack(Workspace *ws, FlexDim d)
{
	/* Split master into master + stack if we have enough clients */
	if (!ws->nmaster || d.n <= ws->nmaster) {
		layout_no_split(ws, d);
	} else if (d.n <= ws->nmaster + (ws->nstack ? ws->nstack : 1)) {
		layout_split_vertical(ws, d);
	} else {
		layout_split_vertical_dual_stack_fixed(ws, d);
	}
}

void
layout_split_vertical_dual_stack_fixed(Workspace *ws, FlexDim d)
{
	int mh, mw, mx, sh, sw, sx, sc;
	int n = d.n, w = d.w, h = d.h, x = d.x, ih = d.ih, iv = d.iv;
	FlexDim master = d, stack = d, stack2 = d;

	if (ws->nstack)
		sc = ws->nstack;
	else
		sc = (n - ws->nmaster) / 2 + ((n - ws->nmaster) % 2 > 0 ? 1 : 0);

	/* Work out the size of the stack areas vs the size of the master area */
	sw = (w - iv) * (1 - ws->mfact);
	mw = (w - iv) * ws->mfact;
	sh = (h - ih) / 2;
	mh = h;

	if (MIRROR) {
		sx = x;
		mx = sx + sw + iv;
	} else {
		mx = x;
		sx = mx + mw + iv;
	}

	/* Tile master area */
	master.an = ws->nmaster;
	master.x = mx;
	master.w = mw;
	master.h = mh;
	ARRANGE(MASTER, ws, master);

	/* Tile stack area */
	stack.an = sc;
	stack.ai = ws->nmaster;
	stack.x = sx;
	stack.w = sw;
	stack.h = sh;
	ARRANGE(STACK, ws, stack);

	/* Tile secondary stack area */
	stack2.an = n - ws->nmaster - sc;
	stack2.ai = ws->nmaster + sc;
	stack2.x = sx;
	stack2.y = stack.y + stack.h + ih;
	stack2.w = sw;
	stack2.h = sh;
	ARRANGE(STACK2, ws, stack2);
}

void
layout_split_horizontal(Workspace *ws, FlexDim d)
{
	/* Split master into master + stack if we have enough clients */
	if (ws->nmaster && d.n > ws->nmaster) {
		layout_split_horizontal_fixed(ws, d);
	} else {
		layout_no_split(ws, d);
	}
}

void
layout_split_horizontal_fixed(Workspace *ws, FlexDim d)
{
	int mh, my, sh, sy;
	int h = d.h, ih = d.ih, y = d.y, n = d.n;
	FlexDim master = d, stack = d;

	sh = (h - ih) * (1 - ws->mfact);
	mh = (h - ih) * ws->mfact;

	if (MIRROR) {
		sy = y;
		my = sy + sh + ih;
	} else {
		my = y;
		sy = my + mh + ih;
	}

	/* Tile master area */
	master.an = ws->nmaster;
	master.y = my;
	master.h = mh;
	ARRANGE(MASTER, ws, master);

	/* Tile stack area */
	stack.an = n - ws->nmaster;
	stack.ai = ws->nmaster;
	stack.y = sy;
	stack.h = sh;
	ARRANGE(STACK, ws, stack);
}

void
layout_split_horizontal_dual_stack(Workspace *ws, FlexDim d)
{
	/* Split master into master + stack if we have enough clients */
	if (!ws->nmaster || d.n <= ws->nmaster) {
		layout_no_split(ws, d);
	} else if (d.n <= ws->nmaster + (ws->nstack ? ws->nstack : 1)) {
		layout_split_horizontal(ws, d);
	} else {
		layout_split_horizontal_dual_stack_fixed(ws, d);
	}
}

void
layout_split_horizontal_dual_stack_fixed(Workspace *ws, FlexDim d)
{
	int mh, my, sw, sh, sy, sc;
	int n = d.n, h = d.h, ih = d.ih, iv = d.iv, y = d.y;
	FlexDim master = d, stack = d, stack2 = d;

	if (ws->nstack)
		sc = ws->nstack;
	else
		sc = (n - ws->nmaster) / 2 + ((n - ws->nmaster) % 2 > 0 ? 1 : 0);

	sh = (h - ih) * (1 - ws->mfact);
	mh = (h - ih) * ws->mfact;
	sw = (d.w - d.iv) / 2;

	if (MIRROR) {
		sy = y;
		my = sy + sh + ih;
	} else {
		my = y;
		sy = my + mh + ih;
	}

	/* Tile master area */
	master.an = ws->nmaster;
	master.y = my;
	master.h = mh;
	ARRANGE(MASTER, ws, master);

	/* Tile stack area */
	stack.an = sc;
	stack.ai = ws->nmaster;
	stack.y = sy;
	stack.w = sw;
	stack.h = sh;
	ARRANGE(STACK, ws, stack);

	/* Tile secondary stack area */
	stack2.an = n - ws->nmaster - sc;
	stack2.ai = ws->nmaster + sc;
	stack2.x = stack.x + stack.w + iv;
	stack2.y = sy;
	stack2.w = sw;
	stack2.h = sh;
	ARRANGE(STACK2, ws, stack2);
}

void
layout_split_centered_vertical(Workspace *ws, FlexDim d)
{
	/* Split master into master + stack if we have enough clients */
	if (!ws->nmaster || d.n <= ws->nmaster) {
		layout_no_split(ws, d);
	} else if (d.n <= ws->nmaster + (ws->nstack ? ws->nstack : 1)) {
		layout_split_vertical(ws, d);
	} else {
		layout_split_centered_vertical_fixed(ws, d);
	}
}

void
layout_split_centered_vertical_fixed(Workspace *ws, FlexDim d)
{
	int mw, mx, sw, sx, ox, sc;
	int n = d.n, x = d.x, w = d.w, iv = d.iv;
	FlexDim master = d, stack = d, stack2 = d;

	if (ws->nstack)
		sc = ws->nstack;
	else
		sc = (n - ws->nmaster) / 2 + ((n - ws->nmaster) % 2 > 0 ? 1 : 0);

	mw = (w - 2*iv) * ws->mfact;
	sw = (w - 2*iv) * (1 - ws->mfact) / 2;

	if (MIRROR)  {
		sx = x;
		mx = sx + sw + iv;
		ox = mx + mw + iv;
	} else {
		ox = x;
		mx = ox + sw + iv;
		sx = mx + mw + iv;
	}

	/* Tile master area */
	master.an = ws->nmaster;
	master.x = mx;
	master.w = mw;
	ARRANGE(MASTER, ws, master);

	/* Tile stack area */
	stack.an = sc;
	stack.ai = ws->nmaster;
	stack.x = sx;
	stack.w = sw;
	ARRANGE(STACK, ws, stack);

	/* Tile secondary stack area */
	stack2.an = n - ws->nmaster - sc;
	stack2.ai = ws->nmaster + sc;
	stack2.x = ox;
	stack2.w = sw;
	ARRANGE(STACK2, ws, stack2);
}

void
layout_split_centered_horizontal(Workspace *ws, FlexDim d)
{
	/* Split master into master + stack if we have enough clients */
	if (!ws->nmaster || d.n <= ws->nmaster) {
		layout_no_split(ws, d);
	} else if (d.n <= ws->nmaster + (ws->nstack ? ws->nstack : 1)) {
		layout_split_horizontal(ws, d);
	} else {
		layout_split_centered_horizontal_fixed(ws, d);
	}
}

void
layout_split_centered_horizontal_fixed(Workspace *ws, FlexDim d)
{
	int mh, my, sh, sy, oy, sc;
	int n = d.n, y = d.y, h = d.h, ih = d.ih;
	FlexDim master = d, stack = d, stack2 = d;

	if (ws->nstack)
		sc = ws->nstack;
	else
		sc = (n - ws->nmaster) / 2 + ((n - ws->nmaster) % 2 > 0 ? 1 : 0);

	mh = (h - 2*ih) * ws->mfact;
	sh = (h - 2*ih) * (1 - ws->mfact) / 2;

	if (MIRROR) {
		sy = y;
		my = sy + sh + ih;
		oy = my + mh + ih;
	} else {
		oy = y;
		my = oy + sh + ih;
		sy = my + mh + ih;
	}

	/* Tile master area */
	master.an = ws->nmaster;
	master.y = my;
	master.h = mh;
	ARRANGE(MASTER, ws, master);

	/* Tile stack area */
	stack.an = sc;
	stack.ai = ws->nmaster;
	stack.y = sy;
	stack.h = sh;
	ARRANGE(STACK, ws, stack);

	/* Tile secondary stack area */
	stack2.an = n - ws->nmaster - sc;
	stack2.ai = ws->nmaster + sc;
	stack2.y = oy;
	stack2.h = sh;
	ARRANGE(STACK2, ws, stack2);
}

void
layout_floating_master(Workspace *ws, FlexDim d)
{
	/* Split master into master + stack if we have enough clients */
	if (!ws->nmaster || d.n <= ws->nmaster) {
		layout_no_split(ws, d);
	} else {
		layout_floating_master_fixed(ws, d);
	}
}

void
layout_floating_master_fixed(Workspace *ws, FlexDim d)
{
	int mx, my, mw, mh;
	int n = d.n, x = d.x, y = d.y, w = d.w, h = d.h;
	FlexDim master = d, stack = d;

	/* Tile stack area first */
	stack.an = n - ws->nmaster;
	stack.ai = ws->nmaster;
	ARRANGE(STACK, ws, stack);

	if (w > h) {
		mw = w * ws->mfact;
		mh = h * 0.9;
	} else {
		mw = w * 0.9;
		mh = h * ws->mfact;
	}

	mx = x + (w - mw) / 2;
	my = y + (h - mh) / 2;

	/* Tile master area */
	master.an = ws->nmaster;
	master.x = mx;
	master.y = my;
	master.w = mw;
	master.h = mh;
	ARRANGE(MASTER, ws, master);
}

void
arrange_left_to_right(Workspace *ws, FlexDim d)
{
	int i, rest, cw;
	int n = d.n, an = d.an, ai = d.ai, x = d.x, y = d.y, w = d.w, h = d.h, iv = d.iv;
	float facts;
	Client *c;

	if (ai + an > n)
		an = n - ai;

	/* Skip ahead to the first client. */
	for (i = 0, c = nexttiled(ws->clients); c && i < ai; c = nexttiled(c->next), i++);

	w -= iv * (an - 1);
	getfactsforrange(c, an, w, &rest, &facts, 1);
	for (i = 0; c && i < an; c = nexttiled(c->next), i++) {
		c->area = d.grp;
		cw = w * (c->cfact / facts) + (i < rest ? 1 : 0);
		resize(c, x, y, cw - (2 * c->bw), h - (2 * c->bw), 0);
		x += cw + iv;
	}
}

void
arrange_top_to_bottom(Workspace *ws, FlexDim d)
{
	int i, rest, ch;
	int n = d.n, an = d.an, ai = d.ai, x = d.x, y = d.y, w = d.w, h = d.h, ih = d.ih;
	float facts;
	Client *c;

	if (ai + an > n)
		an = n - ai;

	/* Skip ahead to the first client. */
	for (i = 0, c = nexttiled(ws->clients); c && i < ai; c = nexttiled(c->next), i++);

	h -= ih * (an - 1);
	getfactsforrange(c, an, h, &rest, &facts, 1);
	for (i = 0, c = nexttiled(ws->clients); c && i < an; c = nexttiled(c->next), i++) {
		c->area = d.grp;
		ch = h * (c->cfact / facts) + (i < rest ? 1 : 0);
		resize(c, x, y, w - (2 * c->bw), ch - (2 * c->bw), 0);
		y += ch + ih;
	}
}

void
arrange_monocle(Workspace *ws, FlexDim d)
{
	int i;
	int n = d.n, an = d.an, ai = d.ai, x = d.x, y = d.y, w = d.w, h = d.h;
	Client *c, *s, *focused = NULL;

	/* Find the most recently focused client among the clients tiled in monocle arrangement and
	 * move it into view. */
	for (s = ws->stack; s && !focused; s = s->snext) {
		for (i = 0, c = nexttiled(ws->clients); c && i < (ai + an); c = nexttiled(c->next), i++) {
			if (i < ai) {
				if (c == s)
					break;
				continue;
			}

			if (c != s)
				continue;

			/* If this is full monocle then draw the client without a border if relevant
			 * (it still needs to be drawn with a border in a deck layout for example). */
			if (enabled(NoBorders) && n == an)
				addflag(c, NoBorder);
			resize(c, x, y, w - (2 * c->bw), h - (2 * c->bw), 0);
			focused = c;
			break;
		}
	}

	/* Hide other windows after focused have been moved into view, avoids background flickering */
	for (i = 0, c = nexttiled(ws->clients); c && i < (ai + an); c = nexttiled(c->next), i++) {
		if (i < ai)
			continue;

		c->area = d.grp;

		if (c != focused)
			hide(c);
	}

	skipfocusevents();
}

void
arrange_gridmode(Workspace *ws, FlexDim d)
{
	int i, cols, rows, ch, cw, cx, cy, cc, cr, chrest, cwrest; // counters
	int an = d.an, ai = d.ai, x = d.x, y = d.y, w = d.w, h = d.h, ih = d.ih, iv = d.iv;
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
		c->area = d.grp;
		cc = ((i - ai) / rows); // client column number
		cr = ((i - ai) % rows); // client row number
		cx = x + cc * (cw + iv) + MIN(cc, cwrest);
		cy = y + cr * (ch + ih) + MIN(cr, chrest);
		resize(c, cx, cy, cw + (cc < cwrest ? 1 : 0) - 2 * c->bw, ch + (cr < chrest ? 1 : 0) - 2 * c->bw, False);
	}
}

void
arrange_horizgrid(Workspace *ws, FlexDim d)
{
	int rh, rest;
	int an = d.an, h = d.h, ih = d.ih;

	/* Exception when there is only one client; do not split into two rows */
	if (an == 1) {
		arrange_monocle(ws, d);
		return;
	}

	FlexDim top_row = d, bottom_row = d;

	rh = (h - ih) / 2;
	rest = h - ih - rh * 2;

	top_row.an = an / 2;
	top_row.h = rh + rest;
	bottom_row.an = an - top_row.an;
	bottom_row.y = top_row.y + top_row.h + ih;
	bottom_row.h = rh;
	bottom_row.ai = top_row.ai + top_row.an;

	arrange_left_to_right(ws, top_row);
	arrange_left_to_right(ws, bottom_row);
}

void
arrange_gaplessgrid(Workspace *ws, FlexDim d)
{
	int i, cols, rows, ch, cw, cn, rn, cc, rrest, crest; // counters
	int an = d.an, ai = d.ai, x = d.x, y = d.y, w = d.w, h = d.h, ih = d.ih, iv = d.iv;
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

		c->area = d.grp;
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
arrange_gaplessgrid_cfacts(Workspace *ws, FlexDim d)
{
	int i, cols, rows, ch, cw, cy, cn, rn, cc, crest, colw; // counters
	int an = d.an, ai = d.ai, x = d.x, y = d.y, w = d.w, h = d.h, ih = d.ih, iv = d.iv;
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

		c->area = d.grp;
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

/* This version of gapless grid fills rows first */
void
arrange_gaplessgrid_alt1(Workspace *ws, FlexDim d)
{
	int i, cols, rows, rest, ch;
	int an = d.an, ai = d.ai, h = d.h, ih = d.ih;

	/* grid dimensions */
	for (cols = 1; cols <= an/2; cols++)
		if (cols*cols >= an)
			break;
	rows = (cols && (cols - 1) * cols >= an) ? cols - 1 : cols;
	ch = (h - ih * (rows - 1)) / (rows ? rows : 1);
	rest = (h - ih * (rows - 1)) - ch * rows;

	for (i = 0; i < rows; i++) {
		d.h = ch + (i < rest ? 1 : 0);
		d.an = MIN(cols, an - i*cols);
		d.ai = ai + i*cols;
		arrange_left_to_right(ws, d);
		d.y += d.h + ih;
	}
}

/* This version of gapless grid fills columns first */
void
arrange_gaplessgrid_alt2(Workspace *ws, FlexDim d)
{
	int i, cols, rows, rest, cw;
	int an = d.an, ai = d.ai, w = d.w, iv = d.iv;

	/* grid dimensions */
	for (rows = 0; rows <= an/2; rows++)
		if (rows*rows >= an)
			break;
	cols = (rows && (rows - 1) * rows >= an) ? rows - 1 : rows;
	cw = (w - iv * (cols - 1)) / (cols ? cols : 1);
	rest = (w - iv * (cols - 1)) - cw * cols;

	for (i = 0; i < cols; i++) {
		d.w = cw + (i < rest ? 1 : 0);
		d.an = MIN(rows, an - i*rows);
		d.ai = ai + i*rows;
		arrange_top_to_bottom(ws, d);
		d.x += cw + (i < rest ? 1 : 0) + iv;
	}
}

void
arrange_fibonacci(Workspace *ws, FlexDim d, int s)
{
	int n = d.n, an = d.an, ai = d.ai, x = d.x, y = d.y, w = d.w, h = d.h, ih = d.ih, iv = d.iv;
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

		c->area = d.grp;
		resize(c, nx, ny, nw - 2 * c->bw, nh - 2 * c->bw, False);
	}
}

void
arrange_fibonacci_cfacts(Workspace *ws, FlexDim f, int s)
{
	Client *clients[4] = { NULL, NULL, NULL, NULL };
	int i, j, q, nx, ny, nw, nh, tnw, tnh;
	int an = f.an, ai = f.ai, x = f.x, y = f.y, w = f.w, h = f.h, ih = f.ih, iv = f.iv;
	Client *t, *a, *b, *c, *d;

	nx = x;
	ny = y;
	nw = w;
	nh = h;

	for (i = 0, j = 0, q = 0, t = nexttiled(ws->clients); t && j < (ai + an); t = nexttiled(t->next), j++) {
		if (j < ai)
			continue;

		t->area = f.grp;
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
arrange_dwindle(Workspace *ws, FlexDim d)
{
	arrange_fibonacci(ws, d, 1);
}

void
arrange_dwindle_cfacts(Workspace *ws, FlexDim d)
{
	arrange_fibonacci_cfacts(ws, d, 1);
}

void
arrange_spiral(Workspace *ws, FlexDim d)
{
	arrange_fibonacci(ws, d, 0);
}

void
arrange_spiral_cfacts(Workspace *ws, FlexDim d)
{
	arrange_fibonacci_cfacts(ws, d, 0);
}

void
arrange_tatami(Workspace *ws, FlexDim d)
{
	unsigned int i, j, nx, ny, nw, nh, tnx, tny, tnw, tnh, nhrest, hrest, wrest, areas, mats, cats;
	int an = d.an, ai = d.ai, x = d.x, y = d.y, w = d.w, h = d.h, ih = d.ih, iv = d.iv;
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

		c->area = d.grp;
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
arrange_tatami_cfacts(Workspace *ws, FlexDim f)
{
	Client *clients[5] = { NULL, NULL, NULL, NULL, NULL };
	unsigned int j, s, nx, ny, nw, nh, tnw, tmw, tnh, tmh, areas, mats, cats, nhrest;
	int an = f.an, ai = f.ai, x = f.x, y = f.y, w = f.w, h = f.h, ih = f.ih, iv = f.iv;
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

		t->area = f.grp;
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
arrange_aspectgrid(Workspace *ws, FlexDim d)
{
	int i, target_an, layout = 0;
	int an = d.an, ai = d.ai;
	FlexDim next = d;  /* Copy of FlexDim, to be passed on recursively */
	Client *c, *f;
	float height_aspect = 0.0;  /* H/W, portrait, used to work out the width based on the height */
	float width_aspect = 0.0;   /* W/H, landscape, used to work out the height based on the width */
	float width_pct, height_pct, target_pct;  /* Percentages */
	float ar;
	int height, width, height_row_width, width_col_height;
	int iv, ih, rh, rw;

	/* Skip ahead to the first client. */
	for (i = 0, f = nexttiled(ws->clients); f && i < ai; f = nexttiled(f->next), i++);

	for (c = f, target_an = 1; target_an <= an; target_an++) {

		/* Calculate gap sizes depending on the number of target clients */
		iv = (target_an - 1) * d.iv;
		ih = (target_an - 1) * d.ih;

		/* The remaining height and width after deducting gaps */
		rh = d.h - ih;
		rw = d.w - iv;

		/* Sum up the aspect ratio for each target client. c->mina is portrait aspect, H/W. */
		if (c->mina) {
			height_aspect += c->mina;
			width_aspect += 1.0/c->mina;
		} else {
			/* Use workspace aspect ratio for non-aspect ratio restricted windows */
			ar = (float)ws->wh/ws->ww;
			height_aspect += ar;
			width_aspect += 1.0/ar;
		}

		/* Calculate the width for a top to bottom layout and the height for a left to right
		 * layout on the basis of the combined aspect ratio for all target clients. */
		width = (float)rh / height_aspect;
		height = (float)rw / width_aspect;

		/* Cap the width and height to the available size */
		width = MIN(width, rw);
		height = MIN(height, rh);

		/* Calculate the complementary height and width based on the capped size */
		width_col_height = (float)width * height_aspect;
		height_row_width = (float)height * width_aspect;

		/* in order to work out how much space would be used compared to the available space */
		width_pct = 100.0 * (float)(width * width_col_height) / (rh * rw);
		height_pct = 100.0 * (float)(height * height_row_width) / (rh * rw);
		target_pct = 100.0 * (float)target_an / an;

		/* If the amount of space taken up is less than the proportional amount of target clients,
		 * e.g. 3 out of 6 clients taking up ~50% of the tiling area, then we allow the best fit
		 * to determine whether we arrange the clients top to bottom or left to right. */
		if (width_pct <= target_pct && (width >= height_row_width || width_pct >= height_pct || height_pct > target_pct)) {
			layout = TOP_TO_BOTTOM;
			break;
		}

		if (height_pct <= target_pct && (height >= width_col_height || height_pct >= width_pct || width_pct > target_pct)) {
			layout = LEFT_TO_RIGHT;
			break;
		}

		c = nexttiled(c->next);
	}

	target_an = MIN(target_an, an);  /* Safeguard in case the for-loop above overflows */

	/* Prepare the next recursive call and trigger the arrangement of clients */
	next.an -= target_an;
	next.ai += target_an;

	/* Use up the remaining space if this is the last batch of clients */
	if (abs(d.w - width) < 5 || d.w < width || next.an == 0) {
		width = d.w;
	}

	if (abs(d.h - height) < 5 || d.h < height || next.an == 0) {
		height = d.h;
	}

	if (layout == LEFT_TO_RIGHT) {
		next.h -= height + d.ih;
		next.y += height + d.ih;
		d.h = height;
		d.an = target_an;
		arrange_left_to_right_aspect(ws, d);
	} else {
		next.w -= width + d.iv;
		next.x += width + d.iv;
		d.w = width;
		d.an = target_an;
		arrange_top_to_bottom_aspect(ws, d);
	}

	/* Only do the recursive call if we have more clients to process */
	if (target_an && next.an > 0)
		arrange_aspectgrid(ws, next);
}

void
arrange_top_to_bottom_aspect(Workspace *ws, FlexDim d)
{
	arrange_aspect_tiles(ws, d, TOP_TO_BOTTOM);
}

void
arrange_left_to_right_aspect(Workspace *ws, FlexDim d)
{
	arrange_aspect_tiles(ws, d, LEFT_TO_RIGHT);
}

void
arrange_aspect_tiles(Workspace *ws, FlexDim d, int arrange)
{
	int ai = d.ai, an = d.an;
	int i, j, s, rest, length, pos, gap;
	Client *c, *f;
	float facts;

	if (arrange == TOP_TO_BOTTOM) {
		gap = d.ih;
		length = d.h;
		pos = d.y;
	} else {
		gap = d.iv;
		length = d.w;
		pos = d.x;
	}

	int remaining_size = length - (gap * (an - 1));
	int remaining_clients = an;
	int target_size;
	float leeway = 1.20;
	int size[an];
	int num_normal_windows = 0, num_aspect_restricted_windows = 0;

	/* Skip ahead to the first client. */
	for (i = 0, f = nexttiled(ws->clients); f && i < ai; f = nexttiled(f->next), i++);


	/* Get a count of aspect restricted clients vs not. */
	for (i = 0, c = f; c && i < an; c = nexttiled(c->next), i++) {
		if (c->mina) {
			num_aspect_restricted_windows++;
		} else {
			num_normal_windows++;
		}
	}

	/* Adjust leeway depending on the number of clients */
	leeway += (0.4 * num_normal_windows / MAX(num_aspect_restricted_windows, 1));

	/* Size aspect restricted windows first. Client.mina = H/W. */
	for (i = 0, c = f; c && i < an; c = nexttiled(c->next), i++) {
		if (c->mina) {
			target_size = remaining_size / remaining_clients;
			s = (arrange == TOP_TO_BOTTOM ? d.w * c->mina : d.h / c->mina) + 2 * c->bw;
			if (c->cfact < 1.0) {
				s *= c->cfact;
			}
			if (s > target_size * leeway) {
				s = target_size * leeway;
			}
			size[i] = s;
			remaining_size -= s;
			remaining_clients--;
		}
	}

	/* Calculate size of remaining windows. */
	if (remaining_clients) {
		getfactsforrange(f, an, remaining_size, &rest, &facts, 0);

		for (i = 0, j = 0, c = f; c && j < remaining_clients; c = nexttiled(c->next), i++) {
			if (!c->mina) {
				target_size = remaining_size * (c->cfact / facts) + (j < rest ? 1 : 0);
				s = target_size;
				size[i] = s;
				j++;

			}
		}
	}

	/* Recalculate the remaining size in case we need to distribute a remainder */
	remaining_size = length - (gap * (an - 1));
	for (i = 0; i < an; i++) {
		remaining_size -= size[i];
	}

	/* Distribute the remainder, if any. */
	for (i = 0; i < remaining_size && i < an; i++) {
		size[i]++;
	}

	/* Adjust if the remainder is negative. */
	if (remaining_size < 0) {
		for (i = 0; i < an; i++) {
			target_size = abs(remaining_size / (an - i));
			size[i] -= target_size;
			remaining_size += target_size;
		}
	}

	/* Now resize and place clients */
	for (i = 0, c = f; c && i < an; c = nexttiled(c->next), i++) {
		if (arrange == TOP_TO_BOTTOM) {
			resize(c, d.x, pos, d.w - (2 * c->bw), size[i] - (2 * c->bw), 0);
		} else {
			resize(c, pos, d.y, size[i] - (2 * c->bw), d.h - (2 * c->bw), 0);
		}

		pos += size[i] + gap;
	}
}

void
flextile(Workspace *ws)
{
	FlexDim d = {0};
	int oh = 0, ov = 0; // outer horizontal/vertical gaps

	getgaps(ws, &oh, &ov, &d.ih, &d.iv, &d.n);
	updatelayoutsymbols(ws, d.n);

	if (d.n == 0) {
		setwindowborders(ws, ws->sel);
		return;
	}

	if (enabled(SmartGapsMonocle)) {
		/* Apply outer gap factor if full screen monocle */
		if (abs(ws->ltaxis[MASTER]) == MONOCLE && (abs(ws->ltaxis[LAYOUT]) == NO_SPLIT || d.n <= ws->nmaster)) {
			oh = ws->mon->oh * smartgaps_fact;
			ov = ws->mon->ov * smartgaps_fact;
		}
	}

	d.x = ws->wx + ov;
	d.y = ws->wy + oh;
	d.h = ws->wh - 2*oh;
	d.w = ws->ww - 2*ov;
	d.an = d.n;

	(&flexlayouts[abs(ws->ltaxis[LAYOUT])])->layout(ws, d);
	setwindowborders(ws, ws->sel);
}

void
updatelayoutsymbols(Workspace *ws, int n)
{
	LayoutPreset preset = ws->layout->preset;

	/* If the layout has changed from the preset then call setflexsymbols to
	 * generate the layout symbol. */
	if (
		preset.layout != ws->ltaxis[LAYOUT] ||
		preset.masteraxis != ws->ltaxis[MASTER] ||
		preset.stack1axis != ws->ltaxis[STACK] ||
		preset.stack2axis != ws->ltaxis[STACK2]
	) {
		setflexsymbols(ws, n);
		return;
	}

	/* Fall back to calling the symbol function for the layout, if one is set. */
	if (preset.symbolfunc != NULL) {
		preset.symbolfunc(ws, n);
	}
}

void
setflexsymbols(Workspace *ws, int n)
{
	int l;
	char sym1, sym2, sym3;

	if (!n)
		n = numtiled(ws);

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
		sym1 = tilesymb[ws->ltaxis[(MIRROR ? STACK : MASTER)]];
		sym2 = layoutsymb[l];
		sym3 = tilesymb[ws->ltaxis[(MIRROR ? MASTER : STACK)]];
	}

	snprintf(ws->ltsymbol, sizeof ws->ltsymbol, "%c%c%c", sym1, sym2, sym3);
}

void
monoclesymbols(Workspace *ws, int n)
{
	if (n > 0)
		snprintf(ws->ltsymbol, sizeof ws->ltsymbol, "[%d]", n);
	else
		snprintf(ws->ltsymbol, sizeof ws->ltsymbol, "[M]");
}

void
decksymbols(Workspace *ws, int n)
{
	if (n > ws->nmaster)
		snprintf(ws->ltsymbol, sizeof ws->ltsymbol, "[]%d", n - ws->nmaster);
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
	int mirror, value;
	Workspace *ws = selws;

	if (!ws->layout->arrange || axis < 0 || axis >= LTAXIS_LAST)
		return;

	value = abs(ws->ltaxis[axis]) + incr;

	if (axis == LAYOUT) {
		mirror = MIRROR ? -1 : 1;
		ws->ltaxis[axis] = WRAP(value, 0, LAYOUT_LAST - 1) * mirror;
	} else {
		ws->ltaxis[axis] = WRAP(value, 0, AXIS_LAST - 1);
	}

	arrange(ws);
}
