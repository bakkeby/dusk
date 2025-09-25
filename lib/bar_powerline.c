int
size_powerline(Bar *bar, BarArg *a)
{
	return (bar->vert ? 0 : a->value ? drw->fonts->h / 2 + 1 : 0);
}

/* Conditional powerline that only appears if the selected workspace has both floating and hidden
   clients */
int
size_pwrl_ifhidfloat(Bar *bar, BarArg *a)
{
	if (hasfloating(bar->mon->selws) && hashidden(bar->mon->selws))
		return size_powerline(bar, a);
	return 0;
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

	/* If the powerline is at the start or end of the bar, then get rid of it. */
	if (bar->p[r_idx] == bar->borderpx)
		return 1;

	if (bar->p[r_idx] + bar->s[r_idx] + bar->borderpx == bar->bw)
		return 1;

	for (r = 0; r < r_idx; r++) {
		br = &_cfg_barrules[r];
		if (!bar->s[r] || br->drawfunc != draw_powerline)
			continue;
		if (br->bar != bar->idx || !br->sizefunc || (br->monitor == 'A' && bar->mon != selmon))
			continue;
		/* If the powerline overlaps with another powerline, then get rid of it. */
		if (bar->p[r] + bar->s[r] == bar->p[r_idx] || bar->p[r_idx] + bar->s[r_idx] == bar->p[r])
			return 1;
	}

	return 0;
}

int
schemeleftof(Bar *bar, int r_idx)
{
	const BarRule *br;
	for (int r = 0; r < num_barrules; r++) {
		br = &_cfg_barrules[r];
		if (!bar->s[r] || br->drawfunc == draw_powerline)
			continue;
		if (br->bar != bar->idx || !br->sizefunc || (br->monitor == 'A' && bar->mon != selmon))
			continue;

		if (bar->p[r] + bar->s[r] + br->lpad + br->rpad == bar->p[r_idx]) {
			return bar->escheme[r];
		}
	}

	return SchemeNorm;
}

int
schemerightof(Bar *bar, int r_idx)
{
	const BarRule *br;
	for (int r = 0; r < num_barrules; r++) {
		br = &_cfg_barrules[r];
		if (!bar->s[r] || br->drawfunc == draw_powerline)
			continue;
		if (br->bar != bar->idx || !br->sizefunc || (br->monitor == 'A' && bar->mon != selmon))
			continue;

		if (bar->p[r_idx] + bar->s[r_idx] == bar->p[r]) {
			return bar->sscheme[r];
		}
	}

	return SchemeNorm;
}
