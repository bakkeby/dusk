int
width_workspaces(Bar *bar, BarArg *a)
{
	Workspace *ws;
	int w = 0, tw;
	for (ws = workspaces; ws; ws = ws->next) {
		if (ws->mon != bar->mon)
			continue;
		tw = TEXTW(wsicon(ws));
		if (tw <= lrpad)
			continue;
		w += tw;
	}
	return w;
}

int
draw_workspaces(Bar *bar, BarArg *a)
{
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
			: ws->visible
			? SchemeWsVisible
			: urg
			? SchemeUrg
			: SchemeWsNorm
		]);
		drw_text(drw, x, a->y, w, a->h, lrpad / 2, icon, inv, False);
		drawindicator(m, NULL, ws->clients != NULL, x, a->y, w, a->h, -1, 0, wsindicatortype);
		x += w;
	}
	return 1;
}

int
click_workspaces(Bar *bar, Arg *arg, BarArg *a)
{
	Workspace *ws = workspaces;
	int x = lrpad / 2;

	for (ws = workspaces; ws && ws->mon != bar->mon; ws = ws->next); // find first workspace for mon

	do {
		if (ws->mon != bar->mon)
			continue;
		x += TEXTW(ws->name);
	} while (a->x >= x && (ws = ws->next));
	arg->v = ws;

	return ClkWorkspaceBar;
}
