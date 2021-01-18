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
	Workspace *ws;
	int w, x = a->x;
	unsigned int inv, occ, urg;
	char *icon;
	Client *c;

	for (ws = workspaces; ws; ws = ws->next) {
		if (ws->mon != bar->mon)
			continue;

		icon = wsicon(ws);
		w = TEXTW(icon);
		if (w <= lrpad)
			continue;

		for (inv = urg = occ = 0, c = ws->clients; c; c = c->next, occ++)
			if (ISURGENT(c)) {
				urg++;
				break;
			}

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
		drawindicator(ws, NULL, ws->clients != NULL, x, a->y, w, a->h, -1, 0, wsindicatortype);
		drawindicator(ws, NULL, ws->pinned, x, a->y, w, a->h, -1, 0, wspinnedindicatortype);
		x += w;
	}
	return 1;
}

int
click_workspaces(Bar *bar, Arg *arg, BarArg *a)
{
	Workspace *ws = workspaces;
	int w, x = lrpad / 2;

	for (ws = workspaces; ws && ws->mon != bar->mon; ws = ws->next); // find first workspace for mon
	if (!ws)
		return ClkWorkspaceBar;

	do {
		if (ws->mon != bar->mon)
			continue;
		w = TEXTW(wsicon(ws));
		if (w <= lrpad)
			continue;
		x += w;
	} while (a->x >= x && (ws = ws->next));
	arg->v = ws;

	return ClkWorkspaceBar;
}
