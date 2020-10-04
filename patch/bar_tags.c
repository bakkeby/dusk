int
width_tags(Bar *bar, BarArg *a)
{
	int w, i;
	Client *c;
	unsigned int occ = 0;
	if (enabled(HideVacantTags))
		for (c = bar->mon->clients; c; c = c->next)
			occ |= c->tags == 255 ? 0 : c->tags;

	for (w = 0, i = 0; i < NUMTAGS; i++) {
		if (enabled(HideVacantTags) && !(occ & 1 << i || bar->mon->tagset[bar->mon->seltags] & 1 << i))
			continue;
		w += TEXTW(tagicon(bar->mon, i));
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

	for (c = m->clients; c; c = c->next) {
		occ |= c->tags == 255 ? 0 : c->tags;
		if (ISURGENT(c))
			urg |= c->tags;
	}
	for (i = 0; i < NUMTAGS; i++) {
		/* do not draw vacant tags */
		if (enabled(HideVacantTags) && !(occ & 1 << i || m->tagset[m->seltags] & 1 << i))
			continue;

		icon = tagicon(bar->mon, i);
		invert = 0;
		w = TEXTW(icon);
		drw_setscheme(drw, scheme[
			m->tagset[m->seltags] & 1 << i
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
	int i = 0, x = lrpad / 2;
	Client *c;
	unsigned int occ = 0;
	if (enabled(HideVacantTags))
		for (c = bar->mon->clients; c; c = c->next)
			occ |= c->tags == 255 ? 0 : c->tags;

	do {
		if (enabled(HideVacantTags) && !(occ & 1 << i || bar->mon->tagset[bar->mon->seltags] & 1 << i))
			continue;
		x += TEXTW(tagicon(bar->mon, i));
	} while (a->x >= x && ++i < NUMTAGS);
	if (i < NUMTAGS) {
		arg->ui = 1 << i;
	}
	return ClkTagBar;
}
