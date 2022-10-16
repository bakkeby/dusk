static Workspace *stickyws;

void
comboviewwsbyname(const Arg *arg)
{
	viewwsonmon(getwsbyname(arg), NULL, combo);
	combo = 1;
}

void
createworkspaces()
{
	Workspace *pws, *ws;
	Monitor *m;
	int i;

	/* find the floating layout for the sticky rule */
	for (i = 0; i < LENGTH(layouts); i++)
		if ((&layouts[i])->arrange == NULL)
			break;

	const WorkspaceRule stickywsrule = { .name = "Sticky", .layout = i };
	stickyws = createworkspace(LENGTH(wsrules), &stickywsrule);
	stickyws->visible = 1;
	stickyws->mon = mons; // not sure about how to handle mon
	stickyws->wh = sh;
	stickyws->ww = sw;

	stickyws->next = pws = selws = workspaces = createworkspace(0, &wsrules[0]);
	for (i = 1; i < LENGTH(wsrules); i++)
		pws = pws->next = createworkspace(i, &wsrules[i]);

	num_workspaces = i;

	for (m = mons, ws = workspaces; ws; ws = ws->next) {
		if (ws->mon == NULL)
			ws->mon = m;

		ws->wx = ws->mon->wx;
		ws->wy = ws->mon->wy;
		ws->wh = ws->mon->wh;
		ws->ww = ws->mon->ww;

		if (m->selws == NULL) {
			m->selws = ws;
			m->selws->visible = 1;
		}
		m = (m->next == NULL ? mons : m->next);
	}
	setworkspaceareas();
}

Workspace *
createworkspace(int num, const WorkspaceRule *r)
{
	Monitor *m = NULL;
	Workspace *ws;

	ws = ecalloc(1, sizeof(Workspace));
	ws->num = num;
	ws->pinned = 0;

	if (r->monitor != -1) {
		for (m = mons; m && m->num != r->monitor; m = m->next);
		ws->mon = m;
		if (r->pinned > 0 && m && m->num == r->monitor)
			ws->pinned = 1;
	}

	strcpy(ws->name, r->name);

	ws->layout = (r->layout == -1 ? &layouts[0] : &layouts[MIN(r->layout, LENGTH(layouts))]);
	ws->prevlayout = &layouts[1 % LENGTH(layouts)];
	ws->mfact = (r->mfact == -1 ? mfact : r->mfact);
	ws->nmaster = (r->nmaster == -1 ? nmaster : r->nmaster);
	ws->nstack = (r->nstack == -1 ? nstack : r->nstack);
	ws->enablegaps = (r->enablegaps == -1 ? enablegaps : r->enablegaps);

	ws->ltaxis[LAYOUT] = ws->layout->preset.layout;
	ws->ltaxis[MASTER] = ws->layout->preset.masteraxis;
	ws->ltaxis[STACK]  = ws->layout->preset.stack1axis;
	ws->ltaxis[STACK2] = ws->layout->preset.stack2axis;
	ws->icondef = r->icondef; // default icons
	ws->iconvac = r->iconvac; // vacant icons
	ws->iconocc = r->iconocc; // occupied icons
	ws->scheme[NORMAL] = r->norm_scheme;
	ws->scheme[VISIBLE] = r->vis_scheme;
	ws->scheme[SELECTED] = r->sel_scheme;
	ws->scheme[OCCUPIED] = r->occ_scheme;

	getworkspacestate(ws);

	return ws;
}

char *
wsicon(Workspace *ws)
{
	char *icon = enabled(AltWorkspaceIcons)
		? ws->name
		: hasclients(ws)
		? ws->iconocc
		: ws->visible && TEXTW(ws->icondef) == 0
		? ws->iconvac
		: ws->icondef;

	if (icon == NULL)
		icon = ws->name;

	return icon;
}

uint64_t
getwsmask(Monitor *m)
{
	Workspace *ws;
	uint64_t wsmask = 0;

	for (ws = nextvismonws(m, workspaces); ws; ws = nextvismonws(m, ws->next))
		wsmask |= (1L << ws->num);

	return wsmask;
}

uint64_t
getallwsmask(Monitor *m)
{
	Workspace *ws;
	uint64_t wsmask = 0;

	for (ws = nextmonws(m, workspaces); ws; ws = nextmonws(m, ws->next))
		wsmask |= (1L << ws->num);

	return wsmask;
}

void
viewwsmask(Monitor *m, uint64_t wsmask)
{
	Workspace *ws;
	uint64_t currmask = getwsmask(m);

	if (wsmask == currmask)
		wsmask = m->wsmask;
	m->wsmask = currmask;

	for (ws = nextmonws(m, workspaces); ws; ws = nextmonws(m, ws->next))
		ws->visible = wsmask & (1L << ws->num);

	selws = m->selws = nextvismonws(m, workspaces);

	drawws(NULL, m, currmask, 1, 0, 0);
}

int
hasclients(Workspace *ws)
{
	Client *c;

	if (!ws)
		return 0;

	/* Check if the workspace has visible clients on it, intentionally not taking HIDDEN(c)
	 * into account so that workspaces with hidden client windows are still marked as
	 * having clients from a UI point of view */
	for (c = ws->clients; c && ISINVISIBLE(c); c = c->next);
	return c != NULL;
}

int hashidden(Workspace *ws)
{
	Client *c;

	if (!ws)
		return 0;

	for (c = ws->clients; c && (ISINVISIBLE(c) || SKIPTASKBAR(c) || !HIDDEN(c)); c = c->next);
	return c != NULL;
}

int
hasfloating(Workspace *ws)
{
	Client *c;

	if (!ws)
		return 0;

	for (c = ws->clients; c && (ISINVISIBLE(c) || SKIPTASKBAR(c) || HIDDEN(c) || !ISFLOATING(c)); c = c->next);
	return c != NULL;
}

void
adjustwsformonitor(Workspace *ws, Monitor *m)
{
	if (!ws || !m || ws->mon == m)
		return;

	clientsmonresize(ws->clients, ws->mon, m);

	if (enabled(SmartLayoutConvertion))
		layoutmonconvert(ws, ws->mon, m);
}

void
hidews(Workspace *ws)
{
	Workspace *w;
	if (enabled(Debug))
		fprintf(stderr, "hidews called for ws %s\n", ws ? ws->name : "NULL");

	if (!ws)
		return;

	ws->visible = 0;

	/* If the workspace being hidden was the selected workspace, then try to find another
	 * visible workspace on the same monitor that can become the selected workspace. */
	if (ws != selws)
		return;

	/* Find the first available workspace to the right, otherwise on the left */
	w = nextvismonws(ws->mon, ws->next);
	if (!w)
		w = nextvismonws(ws->mon, workspaces);
	if (w == ws)
		ws->mon->selws = NULL;
	else
		selws = ws->mon->selws = w;
}

void
showws(Workspace *ws)
{
	if (enabled(Debug))
		fprintf(stderr, "showws called for ws %s\n", ws ? ws->name : "NULL");

	if (!ws)
		return;

	ws->visible = 1;
	selws = ws->mon->selws = ws;
}

void
hidewsclients(Client *c)
{
	if (!c)
		return;

	/* hide clients bottom up */
	hidewsclients(c->snext);
	hide(c);

	/* auto-hide scratchpads when moving to other workspaces */
	if (enabled(AutoHideScratchpads) && c->win && c->scratchkey != 0 && !ISSTICKY(c)) {
		if (SEMISCRATCHPAD(c) && c->swallowing)
			swapsemiscratchpadclients(c, c->swallowing);
		else
			addflag(c, Invisible);
	}
}

void
showwsclient(Client *c)
{
	if (ISFLOATING(c) && ISVISIBLE(c)) {
		if (NEEDRESIZE(c)) {
			removeflag(c, NeedResize);
			XMoveResizeWindow(dpy, c->win, c->x, c->y, c->w, c->h);
		} else if (c->sfx != -9999 && (!ISFULLSCREEN(c) || ISFAKEFULLSCREEN(c))) {
			restorefloats(c);
		} else
			show(c);
	}
}

void
showwsclients(Client *c)
{
	if (!c)
		return;
	showwsclient(c);
	showwsclients(c->snext);
}

void
movews(const Arg *arg)
{
	Workspace *ws = (Workspace*)arg->v;
	Client *c = selws->sel;
	movetows(c, ws);
}

void
swapws(const Arg *arg)
{
	swapwsclients(selws, (Workspace*)arg->v);
}

void
swapwsbyname(const Arg *arg)
{
	swapwsclients(selws, getwsbyname(arg));
}

void
swapwsclients(Workspace *ws1, Workspace *ws2)
{
	Client *c1, *c2;

	if (!ws1 || !ws2 || ws1 == ws2 || ws1 == stickyws)
		return;

	clientsmonresize(ws1->clients, ws1->mon, ws2->mon);
	clientsmonresize(ws2->clients, ws2->mon, ws1->mon);

	c1 = ws1->clients;
	c2 = ws2->clients;
	ws1->clients = NULL;
	ws2->clients = NULL;

	attachx(c1, AttachBottom, ws2);
	attachx(c2, AttachBottom, ws1);

	clientsfsrestore(c1);
	clientsfsrestore(c2);

	c1 = ws1->stack;
	c2 = ws2->stack;
	ws1->stack = NULL;
	ws2->stack = NULL;

	attachstackx(c1, AttachBottom, ws2);
	attachstackx(c2, AttachBottom, ws1);

	if (ws1->visible) {
		showwsclients(c2);
		arrangews(ws1);
	} else
		hidewsclients(ws1->stack);

	if (ws2->visible) {
		showwsclients(c1);
		arrangews(ws2);
	}
	else
		hidewsclients(ws2->stack);

	drawbars();
}

void
movetows(Client *c, Workspace *ws)
{
	if (!c || !ws || ISSTICKY(c))
		return;

	Client *next;
	Client *hadfocus = NULL;
	Workspace *prevws = NULL;

	for (c = nextmarked(NULL, c); c; c = nextmarked(next, NULL)) {
		next = c->next;
		if (c == selws->sel)
			hadfocus = c;
		clientmonresize(c, c->ws->mon, ws->mon);

		unfocus(c, 1, NULL);
		detach(c);
		detachstack(c);

		if (prevws && prevws != c->ws && prevws != ws && prevws->visible)
			arrange(prevws);

		prevws = c->ws;

		attachx(c, AttachBottom, ws);
		attachstack(c);

		clientfsrestore(c);

		if (!enabled(ViewOnWs) && !ws->visible)
			hide(c);
	}

	if (prevws && prevws->visible && prevws != ws)
		arrange(prevws);

	if (hadfocus && ws->visible)
		focus(hadfocus);
	else
		focus(NULL);

	if (enabled(ViewOnWs) && !ws->visible)
		viewwsonmon(ws, ws->mon, 0);
	else if (ws->visible) {
		arrange(ws);
		if (enabled(ViewOnWs) && hadfocus)
			warp(hadfocus);
	}
}

void
moveallclientstows(Workspace *from, Workspace *to)
{
	Client *clients = from->clients;

	if (!clients || !from || !to || from == to || from == stickyws)
		return;

	clientsmonresize(clients, from->mon, to->mon);

	attachx(from->clients, AttachBottom, to);
	attachstackx(from->stack, AttachBottom, to);

	clientsfsrestore(clients);

	from->clients = NULL;
	from->stack = NULL;

	if (enabled(ViewOnWs) && !to->visible)
		viewwsonmon(to, to->mon, 0);

	if (from->visible)
		arrangews(from);
	else
		hidewsclients(from->stack);

	if (to->visible)
		arrangews(to);
	else
		hidewsclients(to->stack);

	if (from->mon == to->mon)
		drawbar(to->mon);
	else
		drawbars();
}

void
movetowsbyname(const Arg *arg)
{
	movetows(selws->sel, getwsbyname(arg));
}

void
sendtowsbyname(const Arg *arg)
{
	togglefunc(ViewOnWs);
	movetows(selws->sel, getwsbyname(arg));
	togglefunc(ViewOnWs);
}

void
movealltowsbyname(const Arg *arg)
{
	moveallclientstows(selws, getwsbyname(arg));
}

void
moveallfromwsbyname(const Arg *arg)
{
	moveallclientstows(getwsbyname(arg), selws);
}

/* Send client to an adjacent workspace on the current monitor */
void
movewsdir(const Arg *arg)
{
	movetows(selws->sel, dirtows(arg->i));
}

/* View an adjacent workspace on the current monitor */
void
viewwsdir(const Arg *arg)
{
	Workspace *nws = dirtows(arg->i);
	if (!nws)
		return;

	viewwsonmon(nws, nws->mon, 0);
}

void
togglepinnedws(const Arg *arg)
{
	Workspace *ws = selws;
	if (arg->v)
		ws = (Workspace*)arg->v;
	ws->pinned = !ws->pinned;
	drawbar(ws->mon);
}

void
enablews(const Arg *arg)
{
	viewwsonmon((Workspace*)arg->v, NULL, 1);
}

void
enablewsbyname(const Arg *arg)
{
	viewwsonmon(getwsbyname(arg), NULL, 1);
}

void
viewws(const Arg *arg)
{
	viewwsonmon((Workspace*)arg->v, NULL, 0);
}

void
viewallwsonmon(const Arg *arg)
{
	viewwsmask(selmon, getallwsmask(selmon));
}

void
viewalloccwsonmon(const Arg *arg)
{
	Workspace *ws;
	Monitor *m = selmon;
	uint64_t wsmask = 0;
	uint64_t currmask = getwsmask(m);
	int wscount = 0;

	for (ws = nextmonws(m, workspaces); ws; ws = nextmonws(m, ws->next)) {
		if (ws->clients) {
			wsmask |= (1L << ws->num);
			wscount++;
		}
	}

	if (wscount > 1 || wsmask != currmask)
		viewwsmask(m, wsmask);
}

void
viewselws(const Arg *arg)
{
	viewwsonmon(selws, NULL, 0);
}

void
viewwsbyname(const Arg *arg)
{
	viewwsonmon(getwsbyname(arg), NULL, 0);
}

void
viewwsonmon(Workspace *ws, Monitor *m, int enablews)
{
	int do_warp = 0;
	int arrangeall = 0;

	if (!ws)
		return;

	if (m == NULL)
		m = selmon;

	Monitor *omon = NULL;
	Workspace *ows = NULL, *w;

	m->prevwsmask = getwsmask(m);

	if (enabled(WorkspacePreview)) {
		storepreview(ws->mon->selws);
		if (ws->mon != m)
			storepreview(m->selws);
	}

	if (enablews && ws->visible) {
		/* Toggle workspace if it is already shown */
		hidews(ws);
	} else if (ws->pinned) {
		/* The workspace is pinned, show it on the monitor it is assigned to */
		if (selws && selws->mon != ws->mon) {
			do_warp = 1;
			m = ws->mon;
			m->prevwsmask = getwsmask(m);
			monitorchanged = 1;
		}

		if (!ws->visible)
			showws(ws);
		else
			selws = ws->mon->selws = ws;
	} else if (ws->mon == m) {
		/* The workspace is already present on the current monitor, just show it */
		showws(ws);
	} else if (ws->visible) {
		/* The workspace is already visible */
		if (enabled(GreedyMonitor) || !m->selws || m->selws->pinned || enablews) {
			/* The current workspace is pinned, or there are no workspaces on the current
			 * monitor. In this case, move the other workspace to the current monitor and
			 * change to the next available workspace on the other monitor. */
			do_warp = 1;

			adjustwsformonitor(ws, selmon);

			omon = ws->mon;

			/* First check if there are more than one visible workspace on the other monitor,
			 * in which case we just leave the remaining workspaces selected. */
			ows = nextvismonws(ws->mon, workspaces);
			if (ows == ws)
				ows = nextvismonws(ws->mon, ws->next);

			/* Otherwise find the next available workspace on said monitor and enable that */
			if (!ows)
				ows = nextmonws(ws->mon, ws->next);
			if (!ows)
				ows = nextmonws(ws->mon, workspaces);
			if (ows == ws)
				ows = NULL;

			omon->selws = ows;

			ws->mon = m;
			m->selws = ws;
			selws = ws;
			showws(ws);
			clientsfsrestore(ws->clients);

			if (ows)
				ows->visible = 1;
			arrangeall = 1;
		} else {
			/* Swap the selected workspace on this monitor with the visible desired workspace
			 * on the other monitor. */
			ows = m->selws;
			adjustwsformonitor(ows, ws->mon);
			adjustwsformonitor(ws, ows->mon);
			ows->mon = ws->mon;
			ws->mon = m;

			showws(ows);
			showws(ws);
			clientsfsrestore(ows->clients);
			clientsfsrestore(ws->clients);
			arrangeall = 1;
		}
	} else {
		/* Workspace is not visible, just grab it */
		adjustwsformonitor(ws, m);
		ws->mon = m;
		showws(ws);
		clientsfsrestore(ws->clients);
	}

	if (!enablews) {
		for (w = nextvismonws(ws->mon, workspaces); w; w = nextvismonws(ws->mon, w->next))
			if (w != ws)
				hidews(w);
	}

	drawws(ws, m, m->prevwsmask, enablews, arrangeall, do_warp);
}

void
drawws(Workspace *ws, Monitor *m, uint64_t prevwsmask, int enablews, int arrangeall, int do_warp)
{
	int x, y;
	Workspace *mousepointerws, *w;
	Monitor *mon;
	uint64_t hidewsmask = prevwsmask - (prevwsmask & getwsmask(m));

	setworkspaceareas();

	/* Clear the selected workspace for a monitor if there are no visible workspaces,
	 * likewise set the selected workspace for the monitor if present and not already set. */
	for (mon = mons; mon; mon = mon->next) {
		w = nextvismonws(mon, workspaces);
		if (!w)
			mon->selws = NULL;
	}

	/* When enabling new workspaces into view let the focus remain with the one
	 * that has the mouse cursor on it. */
	if (enablews && getrootptr(&x, &y)) {
		mousepointerws = recttows(x, y, 1, 1);
		if (mousepointerws && mousepointerws->mon == m)
			selws = m->selws = mousepointerws;
	}

	/* This reveals floating clients while the arrange handles tiled clients */
	for (w = workspaces; w; w = w->next)
		if (w->visible)
			showwsclients(w->stack);

	if (arrangeall)
		arrange(NULL);
	else
		arrangemon(ws ? ws->mon : m);

	/* Note that we hide workspaces after showing workspaces to avoid flickering when
	 * changing workspace (i.e. seeing the wallpaper for a fraction of a second). */
	for (w = workspaces; w; w = w->next)
		if (w->mon == m && (hidewsmask & (1L << w->num)))
			hidewsclients(w->stack);

	drawbars();
	updatecurrentdesktop();
	focus(NULL);

	if (do_warp && ws && ws->sel)
		warp(ws->sel);
	if (!selws)
		selws = workspaces;
}

Workspace *
getwsbyname(const Arg *arg)
{
	Workspace *ws;
	char *wsname = (char*)arg->v;
	for (ws = workspaces; ws && strcmp(ws->name, wsname) != 0; ws = ws->next);
	return ws;
}

Workspace *
getwsbyindex(int index)
{
	Workspace *ws;
	for (ws = workspaces; ws && ws->num != index; ws = ws->next);
	return ws;
}

Workspace *
nextmonws(Monitor *mon, Workspace *ws)
{
	Workspace *w;
	for (w = ws; w && w->mon != mon; w = w->next);
	return w;
}

Workspace *
nextvismonws(Monitor *mon, Workspace *ws)
{
	Workspace *w;
	for (w = ws; w && !(w->mon == mon && w->visible); w = w->next);
	return w;
}

void
assignworkspacetomonitor(Workspace *ws, Monitor *m)
{
	if (!ws || ws->mon == m)
		return;

	adjustwsformonitor(ws, m);
	if (ws->mon && ws->mon->selws == ws)
		ws->mon->selws = NULL;
	ws->mon = m;

	ws->wx = ws->mon->wx;
	ws->wy = ws->mon->wy;
	ws->wh = ws->mon->wh;
	ws->ww = ws->mon->ww;
}

void
redistributeworkspaces(Monitor *new)
{
	int i;
	const WorkspaceRule *r;
	Monitor *m = mons;
	Workspace *ws;

	for (i = 0, ws = workspaces; ws && i < LENGTH(wsrules); ws = ws->next, i++) {
		r = &wsrules[i];

		if (ws->pinned)
			continue;

		if (r->monitor == new->num) {
			assignworkspacetomonitor(ws, new);
			ws->pinned = r->pinned;
			continue;
		}

		if (r->monitor > -1 && r->monitor < new->num)
			continue;

		assignworkspacetomonitor(ws, m);
		m = (m->next == NULL ? mons : m->next);
	}

	/* Set selected workspaces for monitors, if not already set */
	for (m = mons; m; m = m->next) {
		if (m->selws)
			continue;
		ws = nextvismonws(m, workspaces);
		if (!ws)
			ws = nextmonws(m, workspaces);
		if (ws) {
			m->selws = ws;
			m->selws->visible = 1;
		}
	}
}

void
setworkspaceareas()
{
	Monitor *mon;
	for (mon = mons; mon; mon = mon->next)
		setworkspaceareasformon(mon);
}

void
setworkspaceareasformon(Monitor *mon)
{
	Workspace *ws;
	int rrest, crest, cols, rows, cw, ch, cn, rn, cc, nw, x, y;

	/* get a count of the number of visible workspaces for this monitor */
	for (nw = 0, ws = nextvismonws(mon, workspaces); ws; ws = nextvismonws(mon, ws->next), ++nw);
	if (!nw)
		return;

	/* grid dimensions */
	if (mon->ww > mon->wh) {
		if (mon->ww / nw < mon->wh / 2) {
			rows = 2;
			cols = nw / rows + (nw - rows * (nw / rows));
		} else {
			rows = 1;
			cols = nw;
		}
	} else {
		if (mon->wh / nw < mon->ww / 2) {
			cols = 2;
			rows = nw / cols + (nw - cols * (nw / cols));
		} else {
			cols = 1;
			rows = nw;
		}
	}

	/* window geoms (cell height/width) */
	ch = mon->wh / rows;
	rrest = mon->wh % rows;
	cw = mon->ww / cols;
	crest = mon->ww % cols;
	x = mon->wx;
	y = mon->wy;

	cn = rn = cc = 0; // reset column no, row no, workspace count
	for (ws = nextvismonws(mon, workspaces); ws; ws = nextvismonws(mon, ws->next)) {
		if ((rows * cols) > nw && cc + 1 == nw) {
			rows = 1;
			ch = mon->wh;
			rrest = 0;
		}

		ws->wx = x;
		ws->wy = y + rn * ch + MIN(rn, rrest);
		ws->wh = ch + (rn < rrest ? 1 : 0);
		ws->ww = cw + (cn < crest ? 1 : 0);

		rn++;
		cc++;
		if (rn >= rows) {
			rn = 0;
			x += cw + (cn < crest ? 1 : 0);
			cn++;
		}
	}
}
