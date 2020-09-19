void
evilgrid(Monitor *m)
{
	unsigned int i, n;
	int x, y, cols, rows, ch, cw, cn, rn, rrest, crest; // counters
	int oh, ov, ih, iv;
	Client *c;
	Monitor *mon;

	getgaps(m, &oh, &ov, &ih, &iv, &n);
	n = 0;
	for (mon = mons; mon; mon = mon->next)
		for (c = mon->clients; c; c = c->next)
			if (HIDDEN(c))
				n++;
	snprintf(m->ltsymbol, sizeof m->ltsymbol, "E%dE", n);
	if (n == 0)
		return;

	/* grid dimensions */
	for (cols = 0; cols <= n/2; cols++)
		if (cols*cols >= n)
			break;
	if (n == 5) /* set layout against the general calculation: not 1:2:2, but 2:3 */
		cols = 2;
	rows = n/cols;
	cn = rn = 0; // reset column no, row no, client count

	ch = (m->wh - 2*oh - ih * (rows - 1)) / rows;
	cw = (m->ww - 2*ov - iv * (cols - 1)) / cols;
	rrest = (m->wh - 2*oh - ih * (rows - 1)) - ch * rows;
	crest = (m->ww - 2*ov - iv * (cols - 1)) - cw * cols;
	x = m->wx + ov;
	y = m->wy + oh;

	for (i = 0, mon = mons; mon; mon = mon->next)
		for (c = mon->clients; c; c = c->next) {
			if (!HIDDEN(c))
				continue;
			i++;
			if (i/rows + 1 > cols - n%cols) {
				rows = n/cols + 1;
				ch = (m->wh - 2*oh - ih * (rows - 1)) / rows;
				rrest = (m->wh - 2*oh - ih * (rows - 1)) - ch * rows;
			}
			resize(c,
				x,
				y + rn*(ch + ih) + MIN(rn, rrest),
				cw + (cn < crest ? 1 : 0) - 2*c->bw,
				ch + (rn < rrest ? 1 : 0) - 2*c->bw,
				0);
			rn++;
			if (rn >= rows) {
				rn = 0;
				x += cw + ih + (cn < crest ? 1 : 0);
				cn++;
		}
	}
}