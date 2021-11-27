int
size_wintitle_sticky(Bar *bar, BarArg *a)
{
	Client *c;
	int w = 0;

	for (c = stickyws->clients; c; c = c->next) {
		if (!c->icon)
			continue;
		w += c->icw + iconspacing;
	}
	return w ? w - iconspacing : 0;
}

int
draw_wintitle_sticky(Bar *bar, BarArg *a)
{
	Client *c;
	int x = a->x;

	drw_setscheme(drw, scheme[bar->scheme]);
	drw_rect(drw, a->x, a->y, a->w, a->h, 1, 1);
	a->firstscheme = bar->scheme;
	a->lastscheme = bar->scheme;

	for (c = stickyws->clients; c; c = c->next) {
		if (!c->icon)
			continue;
		drw_pic(drw, x, a->y + (a->h - c->ich) / 2, c->icw, c->ich, c->icon);
		x += c->icw + iconspacing;
	}
	return x;
}

int
click_wintitle_sticky(Bar *bar, Arg *arg, BarArg *a)
{
	Client *c;
	if (!stickyws->clients)
		return 0;
	int x = 0, w;

	for (c = stickyws->clients; c; c = c->next) {
		if (!c->icon)
			continue;
		w = c->icw + iconspacing;
		x += w;
		if (x > a->x) {
			arg->v =c;
			return ClkWinTitle;
		}
	}
	return -1;
}
