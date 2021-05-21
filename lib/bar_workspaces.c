int
size_workspaces(Bar *bar, BarArg *a)
{
	Workspace *ws;
	int s = 0, tw;
	for (ws = workspaces; ws; ws = ws->next) {
		if (ws->mon != bar->mon)
			continue;
		tw = TEXTW(wsicon(ws)) + lrpad;
		if (tw <= lrpad)
			continue;
		if (bar->vert)
			s += bh;
		else
			s += tw;
	}
	return s;
}

int
draw_workspaces(Bar *bar, BarArg *a)
{
	Workspace *ws;
	int w, x = a->x, y = a->y, h = (bar->vert ? bh : a->h);
	unsigned int inv, occ, urg;
	char *icon;
	Client *c;

	for (ws = workspaces; ws; ws = ws->next) {
		if (ws->mon != bar->mon)
			continue;

		icon = wsicon(ws);
		w = TEXTW(icon) + lrpad;
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
			: occ
			? SchemeWsOcc
			: ws->pinned
			? SchemeWsPin
			: SchemeWsNorm
		]);
		drw_text(drw, x, y, w, h, lrpad / 2, icon, inv, False);
		drawindicator(ws, NULL, hasclients(ws), x, y, w, h, -1, 0, wsindicatortype);
		drawindicator(ws, NULL, ws->pinned, x, y, w, h, -1, 0, wspinnedindicatortype);
		if (bar->vert) {
			y += bh;
		} else {
			x += w;
		}
	}
	return 1;
}

int
click_workspaces(Bar *bar, Arg *arg, BarArg *a)
{
	Workspace *ws = workspaces;
	int w, s = 0, t = (bar->vert ? a->y : a->x);

	for (ws = workspaces; ws && ws->mon != bar->mon; ws = ws->next); // find first workspace for mon
	if (!ws)
		return ClkWorkspaceBar;

	do {
		if (ws->mon != bar->mon)
			continue;
		w = TEXTW(wsicon(ws)) + lrpad;
		if (w <= lrpad)
			continue;
		if (bar->vert)
			s += bh;
		else
			s += w;
	} while (t >= s && (ws = ws->next));

	if (!ws)
		return -1;

	arg->v = ws;

	return ClkWorkspaceBar;
}
