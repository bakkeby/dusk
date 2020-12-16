int
width_tags(Bar *bar, BarArg *a)
{
	int w, i, tw;
	for (w = 0, i = 0; i < NUMTAGS; i++) {
		tw = TEXTW(tagicon(bar->mon, i));
		if (tw <= lrpad)
			continue;
		w += tw;
	}
	return w;
}

int
draw_tags(Bar *bar, BarArg *a)
{
	int invert;
	int w, x = a->x;
	unsigned int i, occ = 0, urg = 0;
	char *icon;
	Client *c;
	Monitor *m = bar->mon;
	Workspace *ws = MWS(m);

	for (c = m->selws->clients; c; c = c->next) {
		occ |= c->tags == 255 ? 0 : c->tags;
		if (ISURGENT(c))
			urg |= c->tags;
	}
	for (i = 0; i < NUMTAGS; i++) {
		icon = tagicon(bar->mon, i);
		invert = 0;
		w = TEXTW(icon);
		if (w <= lrpad)
			continue;

		drw_setscheme(drw, scheme[
			ws->tags & 1 << i
			? SchemeTagsSel
			: urg & 1 << i
			? SchemeUrg
			: SchemeTagsNorm
		]);
		drw_text(drw, x, a->y, w, a->h, lrpad / 2, icon, invert, False);
		drawindicator(m, NULL, occ, x, a->y, w, a->h, i, -1, invert, tagindicatortype);
		x += w;
	}

	return 1;
}

int
click_tags(Bar *bar, Arg *arg, BarArg *a)
{
	int i = 0, tw, x = lrpad / 2;
	do {
		tw = TEXTW(tagicon(bar->mon, i));
		if (tw <= lrpad)
			continue;
		x += tw;
	} while (a->x >= x && ++i < NUMTAGS);
	if (i < NUMTAGS)
		arg->ui = 1 << i;
	return ClkTagBar;
}
