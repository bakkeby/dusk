int
size_powerline(Bar *bar, BarArg *a)
{
	return (bar->vert ? 0 : a->value ? drw->fonts->h / 2 + 1 : 0);
}

int
draw_powerline(Bar *bar, BarArg *a)
{
	int style =
		a->x == bar->borderpx
		? PwrlSolid
		: a->x + a->w + bar->borderpx == bar->bw
		? PwrlSolidRev
		: a->value;

	drw_arrow(drw, a->x, a->y, a->w, a->h, style, scheme[a->firstscheme][ColBg], scheme[a->lastscheme][ColBg], scheme[SchemeNorm][ColBg]);
	return a->w;
}

int
reducepowerline(Bar *bar, int r_idx)
{
	int r;
	const BarRule *br;

	/* If the powerline overlaps with another powerline, then get rid of it. */
	for (r = 0; r < r_idx; r++) {
		br = &barrules[r];
		if (!bar->s[r] || br->drawfunc != draw_powerline)
			continue;
		if (br->bar != bar->idx || !br->sizefunc || (br->monitor == 'A' && bar->mon != selmon))
			continue;
		if (bar->p[r] + bar->s[r] == bar->p[r_idx] || bar->p[r_idx] + bar->s[r_idx] == bar->p[r])
			return 1;
	}

	return 0;
}

int
schemeleftof(Bar *bar, int r_idx)
{
	int r, max_x = 0, max_r = -1;
	const BarRule *br;
	for (r = 0; r < LENGTH(barrules); r++) {
		br = &barrules[r];
		if (!bar->s[r] || br->drawfunc == draw_powerline)
			continue;
		if (br->bar != bar->idx || !br->sizefunc || (br->monitor == 'A' && bar->mon != selmon))
			continue;

		if (bar->p[r] > max_x && bar->p[r] < bar->p[r_idx]) {
			max_r = r;
			max_x = bar->p[r];
		}
	}

	return max_r == -1 ? SchemeNorm : bar->escheme[max_r];
}

int
schemerightof(Bar *bar, int r_idx)
{
	int r, min_x = INT_MAX, min_r = -1;
	const BarRule *br;
	for (r = 0; r < LENGTH(barrules); r++) {
		br = &barrules[r];
		if (!bar->s[r] || br->drawfunc == draw_powerline)
			continue;
		if (br->bar != bar->idx || !br->sizefunc || (br->monitor == 'A' && bar->mon != selmon))
			continue;

		if (bar->p[r] < min_x && bar->p[r] > bar->p[r_idx]) {
			min_r = r;
			min_x = bar->p[r];
		}
	}

	return min_r == -1 ? SchemeNorm : bar->sscheme[min_r];
}