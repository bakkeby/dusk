char *
wsicon(Workspace *ws)
{
	char *icon = enabled(AltWorkspaceIcons)
		? ws->name
		: ws->clients
		? ws->iconocc
		: ws->visible && TEXTW(ws->icondef) <= lrpad
		? ws->iconvac
		: ws->icondef;

	if (icon == NULL)
		icon = ws->name;

	return icon;
}

void
adjustwsformonitor(Workspace *ws, Monitor *m)
{
	if (ws->mon == m)
		return;

	clientsmonresize(ws->clients, ws->mon, m);

	if (enabled(SmartLayoutConvertion))
		layoutmonconvert(ws, ws->mon, m);
}

void
hidews(Workspace *ws)
{
	ws->visible = 0;
	hidewsclients(ws);
}

void
showws(Workspace *ws)
{
	if (!ws)
		return;
	ws->visible = 1;
	ws->mon->selws = ws;
	selws = ws;
	showwsclients(ws);
}

void
hidewsclients(Workspace *ws)
{
	Client *c;
	for (c = ws->stack; c; c = c->snext) {
		XMoveWindow(dpy, c->win, WIDTH(c) * -2, c->y);
		if (enabled(AutoHideScratchpads) && c->scratchkey != 0) {
			/* auto-hide scratchpads when moving to other workspaces */
			addflag(c, Invisible);
		}
	}
}

void
showwsclients(Workspace *ws)
{
	showhide(ws->stack);
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

	if (ws1 == ws2)
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

	arrange(NULL);
}

void
movetows(Client *c, Workspace *ws)
{
	if (!c)
		return;

	int hadfocus = (c == selws->sel);

	clientmonresize(c, c->ws->mon, ws->mon);

	unfocus(c, 1, NULL);
	detach(c);
	detachstack(c);

	if (c->ws->visible)
		arrange(c->ws);

	attachx(c, AttachBottom, ws);
	attachstack(c);

	clientsfsrestore(c);

	if (hadfocus && ws->visible)
		focus(c);
	else
		focus(NULL);

	if (enabled(ViewOnWs) && !ws->visible)
		viewwsonmon(ws, ws->mon, 0);
	else if (ws->visible) {
		arrange(ws);
		if (enabled(ViewOnWs) && hadfocus)
			warp(c);
	} else
		XMoveWindow(dpy, c->win, WIDTH(c) * -2, c->y);
}

void
moveallclientstows(Workspace *from, Workspace *to)
{
	Client *clients = from->clients;

	if (!clients || from == to)
		return;

	clientsmonresize(clients, from->mon, to->mon);

	attachx(from->clients, AttachBottom, to);
	attachstackx(from->stack, AttachBottom, to);

	clientsfsrestore(clients);

	from->clients = NULL;
	from->stack = NULL;

	if (enabled(ViewOnWs) && !to->visible)
		viewwsonmon(to, to->mon, 0);
}

void
movetowsbyname(const Arg *arg)
{
	Workspace *ws = getwsbyname(arg);
	if (!ws)
		return;

	movetows(selws->sel, ws);
}

void
movealltowsbyname(const Arg *arg)
{
	Workspace *ws = getwsbyname(arg);
	if (!ws)
		return;

	moveallclientstows(selws, ws);
}

/* Send client to an adjacent workspace on the current monitor */
void
movewsdir(const Arg *arg)
{
	Workspace *nws = dirtows(arg->i);

	if (!nws)
		return;

	movetows(selws->sel, nws);
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
viewwsbyname(const Arg *arg)
{
	viewwsonmon(getwsbyname(arg), NULL, 0);
}

void
viewwsonmon(Workspace *ws, Monitor *m, int enablews)
{
	int do_warp = 0;

	if (m == NULL)
		m = selmon;

	Monitor *mon, *omon = NULL;
	Workspace *w, *ows = NULL;

	if (enablews && ws->visible && ws->mon == m) {
		/* Toggle workspace if it is already shown */
		hidews(ws);
	} else if (ws->pinned && !enablews) {
		/* The workspace is pinned, show it on the monitor it is assigned to */
		if (selws->mon != ws->mon)
			do_warp = 1;
		if (!ws->visible)
			showws(ws);
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

			/* Find the next available workspace on said monitor */
			for (ows = ws->next; ows && ows->mon != ws->mon; ows = ows->next);
			if (!ows)
				for (ows = workspaces; ows && ows != ws && ows->mon != ws->mon; ows = ows->next);
			if (ows == ws)
				ows = NULL;

			omon->selws = ows;

			ws->mon = m;
			m->selws = ws;
			selws = ws;
			showws(ws);
			clientsfsrestore(ws->clients);
			if (ows)
				showws(ows);
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
		}
	} else {
		/* Workspace is not visible, just grab it */
		adjustwsformonitor(ws, m);
		ws->mon = m;
		showws(ws);
		clientsfsrestore(ws->clients);
	}

	if (!enablews) {
		/* Note that we hide workspaces after showing workspaces to avoid flickering when
		 * changing workspace (i.e. seeing the wallpaper for a fraction of a second). */
		for (w = workspaces; w; w = w->next)
			if (w != ws && w->mon == ws->mon && ws->visible)
				hidews(w);
	}

	/* Clear the selected workspace for a monitor if there are no visible workspaces */
	for (mon = mons; mon; mon = mon->next) {
		for (w = workspaces; w && !(w->mon == mon && w->visible); w = w->next);
		if (!w)
			mon->selws = NULL;
	}

	setworkspaceareas();
	arrange(NULL);
	drawbars();
	updatecurrentdesktop();
	focus(NULL);

	if (do_warp && ws->sel)
		warp(ws->sel);
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
nextmonws(Monitor *mon, Workspace *ws)
{
	Workspace *w;
	for (w = ws; w && !(w->mon == mon && w->visible); w = w->next);
	return w;
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
	for (nw = 0, ws = nextmonws(mon, workspaces); ws; ws = nextmonws(mon, ws->next), ++nw);
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
	rrest = mon->wh - ch * rows;
	cw = mon->ww / cols;
	crest = mon->wh - cw * cols;
	x = mon->wx;
	y = mon->wy;

	cn = rn = cc = 0; // reset column no, row no, workspace count
	for (ws = nextmonws(mon, workspaces); ws; ws = nextmonws(mon, ws->next)) {
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
