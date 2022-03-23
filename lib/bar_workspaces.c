static const char *workspace_label_placeholder = "%s";
static int occupied_workspace_label_format_length = 0;
static int vacant_workspace_label_format_length = 0;

int
size_workspaces(Bar *bar, BarArg *a)
{
	Client *c;
	Workspace *ws;
	int s = 0, w;
	int plw = (bar->vert ? 0 : a->value ? drw->fonts->h / 2 + 1 : 0);
	int padding = lrpad - plw;

	for (ws = workspaces; ws; ws = ws->next) {
		if (ws->mon != bar->mon)
			continue;
		w = TEXT2DW(wsicon(ws));
		if (!w)
			continue;
		if (enabled(WorkspaceLabels)) {
			c = firsttiled(ws);
			if (c)
				w += occupied_workspace_label_format_length + TEXTW(c->label);
			else
				w += vacant_workspace_label_format_length;
		}
		if (bar->vert)
			s += bh;
		else
			s += w + plw + padding;
	}
	return s - plw;
}

int
draw_workspaces(Bar *bar, BarArg *a)
{
	Client *c;
	Workspace *ws = NULL, *nextws = NULL;
	int w = 0, nextw, x = a->x + a->lpad, y = a->y, h = (bar->vert ? bh : a->h);
	int plw = (bar->vert ? 0 : a->value ? drw->fonts->h / 2 + 1 : 0);
	int padding = lrpad - plw;
	unsigned int inv, occ, urg;
	char *icon, *nexticon;
	char label[128] = {0};
	int wsscheme = 0, nextscheme = 0;

	nextwsicon(bar, workspaces, &nextws, &nexticon, &nextw);

	while (ws || nextws) {

		if (nextws) {
			for (inv = urg = occ = 0, c = nextws->clients; c; c = c->next, occ++)
				if (ISURGENT(c)) {
					urg++;
					break;
				}

			nextscheme =
				nextws == nextws->mon->selws
				? nextws->scheme[SELECTED]
				: nextws->visible
				? nextws->scheme[VISIBLE]
				: urg
				? SchemeUrg
				: occ
				? nextws->scheme[OCCUPIED]
				: nextws->scheme[NORMAL];

			if (a->firstscheme == -1)
				a->firstscheme = nextscheme;
			a->lastscheme = nextscheme;
		}

		if (ws) {
			if (enabled(WorkspaceLabels)) {
				c = firsttiled(ws);
				if (c) {
					w += occupied_workspace_label_format_length + TEXTW(c->label);
					snprintf(label, 128, occupied_workspace_label_format, icon, c->label);
				} else {
					w += vacant_workspace_label_format_length;
					snprintf(label, 128, vacant_workspace_label_format, icon);
				}
				icon = label;
			}
			drw_2dtext(drw, x, y, w, h, padding / 2, icon, inv, 1, wsscheme);

			if (plw && nextws)
				drw_arrow(drw, x + w, y, plw, h, a->value, scheme[wsscheme][ColBg], scheme[nextscheme][ColBg], scheme[SchemeNorm][ColBg]);

			drawindicator(ws, NULL, hasclients(ws), x, y, w , h, -1, 0, wsindicatortype);
			drawindicator(ws, NULL, ws->pinned, x, y, w, h, -1, 0, wspinnedindicatortype);

			if (bar->vert) {
				y += bh;
			} else {
				x += w + plw;
			}
		}

		ws = nextws;
		icon = nexticon;
		wsscheme = nextscheme;
		w = nextw + padding;

		if (ws)
			nextwsicon(bar, ws->next, &nextws, &nexticon, &nextw);
	}

	return 1;
}

int
click_workspaces(Bar *bar, Arg *arg, BarArg *a)
{
	Client *c;
	Workspace *ws;
	int w, s = 0, t = (bar->vert ? a->y : a->x);
	int plw = (bar->vert ? 0 : a->value ? drw->fonts->h / 2 + 1 : 0);
	int padding = lrpad - plw;

	/* This avoids clicks to the immediate left of the leftmost workspace (e.g. 2) to evaluate
	 * as workspace 1 (which can be on a different monitor). */
	for (ws = workspaces; ws && ws->mon != bar->mon; ws = ws->next); // find first workspace for mon
	if (!ws)
		return ClkWorkspaceBar;

	do {
		if (ws->mon != bar->mon)
			continue;
		w = TEXT2DW(wsicon(ws));
		if (!w)
			continue;
		if (enabled(WorkspaceLabels)) {
			c = firsttiled(ws);
			if (c)
				w += occupied_workspace_label_format_length + TEXTW(c->label);
			else
				w += vacant_workspace_label_format_length;
		}
		if (bar->vert)
			s += bh;
		else
			s += w + plw + padding;
	} while (t >= s && (ws = ws->next));

	if (!ws)
		return -1;

	arg->v = ws;

	return ClkWorkspaceBar;
}

int
hover_workspaces(Bar *bar, BarArg *a, XMotionEvent *ev)
{
	Client *c;
	Workspace *ws;
	Monitor *m = bar->mon;
	int x, y, w, s = 0, t = (bar->vert ? a->y : a->x);
	int plw = (bar->vert ? 0 : a->value ? drw->fonts->h / 2 + 1 : 0);
	int padding = lrpad - plw;

	/* This avoids clicks to the immediate left of the leftmost workspace (e.g. 2) to evaluate
	 * as workspace 1 (which can be on a different monitor). */
	for (ws = workspaces; ws && ws->mon != m; ws = ws->next); // find first workspace for mon
	if (!ws)
		return 0;

	do {
		if (ws->mon != m)
			continue;
		w = TEXT2DW(wsicon(ws));
		if (!w)
			continue;
		if (enabled(WorkspaceLabels)) {
			c = firsttiled(ws);
			if (c)
				w += occupied_workspace_label_format_length + TEXTW(c->label);
			else
				w += vacant_workspace_label_format_length;
		}
		if (bar->vert)
			s += bh;
		else
			s += w + plw + padding;
	} while (t >= s && (ws = ws->next));

	if (!ws) {
		hidewspreview(m);
		return 0;
	}

	if (bar->vert) {
		if (bar->bx > m->mx + m->mw / 2) // right bar
			x = bar->bx - m->mw / scalepreview - gappov;
		else // left bar
			x = bar->bx + bar->bw + gappov;
		y = bar->by + ev->y - m->mh / scalepreview / 2;
		if (y + m->mh / scalepreview > m->wy + m->wh)
			y = m->wy + m->wh - m->mh / scalepreview - gappoh;
		else if (y < bar->by)
			y = m->wy + gappoh;
	} else {
		if (bar->by > m->my + m->mh / 2) // bottom bar
			y = bar->by - m->mh / scalepreview - gappoh;
		else // top bar
			y = bar->by + bar->bh + gappoh;
		x = bar->bx + ev->x - m->mw / scalepreview / 2;
		if (x + m->mw / scalepreview > m->mx + m->mw)
			x = m->wx + m->ww - m->mw / scalepreview - gappov;
		else if (x < bar->bx)
			x = m->wx + gappov;
	}

	if (m->preview->show != (ws->num + 1) && m->selws != ws) {
		m->preview->show = ws->num + 1;
		showwspreview(ws, x, y);
	} else if (m->selws == ws)
		hidewspreview(m);

	return 1;
}

void
saveclientclass(Client *c)
{
	XClassHint ch = { NULL, NULL };
	XGetClassHint(dpy, c->win, &ch);
	strcpy(c->label, ch.res_class ? ch.res_class : broken);
	if (lowercase_workspace_labels)
		c->label[0] = tolower(c->label[0]);
}

void
nextwsicon(Bar *bar, Workspace *ws, Workspace **next, char **nexticon, int *nextw)
{
	char *icon;
	int w;

	*next = NULL;
	*nexticon = NULL;
	*nextw = 0;

	for (; ws; ws = ws->next) {
		if (ws->mon != bar->mon)
			continue;

		icon = wsicon(ws);
		w = TEXT2DW(icon);
		if (w <= 0)
			continue;

		*next = ws;
		*nexticon = icon;
		*nextw = w;
		break;
	}
}

Client *
firsttiled(Workspace *ws)
{
	Client *c;
	for (c = ws->clients; c && (ISFLOATING(c) || ISINVISIBLE(c)); c = c->next);
	if (!c)
		for (c = ws->stack; c && ISINVISIBLE(c); c = c->snext);
	return c;
}