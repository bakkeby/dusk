int
width_workspaces(Bar *bar, BarArg *a)
{
	Workspace *ws;
	fprintf(stderr, "width_workspaces: -->\n");
	int w = 0, tw;
	for (ws = workspaces; ws; ws = ws->next) {
		if (ws->mon != bar->mon)
			continue;
		tw = TEXTW(wsicon(ws));
		if (tw <= lrpad)
			continue;
		w += tw;
	}
	fprintf(stderr, "width_workspaces: <--\n");
	return w;
}

int
draw_workspaces(Bar *bar, BarArg *a)
{
	fprintf(stderr, "draw_workspaces: -->\n");
	int w, x = a->x;
	unsigned int inv, occ, urg;
	char *icon;
	Client *c;
	Monitor *m = bar->mon;
	Workspace *ws = MWS(m);

	for (ws = workspaces; ws; ws = ws->next) {
		if (ws->mon != bar->mon)
			continue;
		for (inv = urg = occ = 0, c = ws->clients; c; c = c->next, occ++)
			if (ISURGENT(c))
				urg++;

		icon = wsicon(ws);
		w = TEXTW(icon);
		if (w <= lrpad)
			continue;

		drw_setscheme(drw, scheme[
			ws == ws->mon->selws
			? SchemeWsSel
			: urg
			? SchemeUrg
			: SchemeWsNorm
		]);
		drw_text(drw, x, a->y, w, a->h, lrpad / 2, icon, inv, False);
		drawindicator(m, NULL, ws->clients != NULL, x, a->y, w, a->h, -1, 0, wsindicatortype);
		x += w;
	}
	fprintf(stderr, "draw_workspaces: <--\n");
	return 1;
}

int
click_workspaces(Bar *bar, Arg *arg, BarArg *a)
{
	fprintf(stderr, "click_workspaces: -->\n");
	Workspace *ws = workspaces;
	int x = lrpad / 2;

	do {
		if (ws->mon != bar->mon)
			continue;
		x += TEXTW(ws->name);
		fprintf(stderr, "click_workspaces: click x = %d, x = %d, ws = %s\n", a->x, x, ws->name);
	} while (a->x >= x && (ws = ws->next));
	arg->v = ws;
	if (arg->v == NULL)
		fprintf(stderr, "click_workspaces: set arg->v to NULL\n");
	else
		fprintf(stderr, "click_workspaces: set arg->v to %s\n", ws->name);

	// int i = 0, tw, x = lrpad / 2;
	// do {
	// 	tw = TEXTW(tagicon(bar->mon, i));
	// 	if (tw <= lrpad)
	// 		continue;
	// 	x += tw;
	// } while (a->x >= x && ++i < NUMTAGS);
	// if (i < NUMTAGS)
	// 	arg->ui = 1 << i;
	fprintf(stderr, "click_workspaces: <--\n");
	return ClkWorkspaceBar;
}
