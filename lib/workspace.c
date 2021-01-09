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
	fprintf(stderr, "hidews --> %s\n", ws->name);
	ws->visible = 0;
	hidewsclients(ws);
	fprintf(stderr, "hidews <--\n");
}

void
showws(Workspace *ws)
{
	fprintf(stderr, "showws --> %s\n", ws->name);
	ws->visible = 1;
	ws->mon->selws = ws;
	selws = ws;
	showwsclients(ws);
	arrange(ws);
	fprintf(stderr, "showws <--\n");
}

void
hidewsclients(Workspace *ws)
{
	Client *c;
	for (c = ws->stack; c; c = c->snext)
		XMoveWindow(dpy, c->win, WIDTH(c) * -2, c->y);
}

void
showwsclients(Workspace *ws)
{
	showhide(ws->stack);
}

void
movews(const Arg *arg)  // TODO movews - bad name perhaps?
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
		viewwsonmon(ws, ws->mon);
	else if (ws->visible) {
		arrange(ws);
		if (enabled(ViewOnWs) && hadfocus)
			warp(c);
	} else
		XMoveWindow(dpy, c->win, WIDTH(c) * -2, c->y); // TODO separate function to hide clients?
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
		viewwsonmon(to, to->mon);
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

	viewwsonmon(nws, nws->mon);
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
viewws(const Arg *arg)
{
	viewwsonmon((Workspace*)arg->v, NULL);
}

void
viewwsbyname(const Arg *arg)
{
	viewwsonmon(getwsbyname(arg), NULL);
}

void
viewwsonmon(Workspace *ws, Monitor *m)
{
	fprintf(stderr, "viewwsonmon: -->\n");
	int do_warp = 0;

	if (m == NULL)
		m = selmon;

	Monitor *omon = NULL;
	Workspace *ows = NULL, *hws = NULL;

	if (!ws || (selws == ws && m == ws->mon)) {
		fprintf(stderr, "viewwsonmon: because %s\n", !ws ? "!ws" : selws == ws ? "selws == ws && m == ws->mon" : "eh");
		arrange(ws);
		return;
	}

	if (ws->pinned) {
		fprintf(stderr, "viewwsonmon: ws %s pinned, visible = %d\n", ws->name, ws->visible);
		if (selws->mon != ws->mon)
			do_warp = 1;
		if (!ws->visible) {
			if (ws->mon->selws && ws->mon->selws->visible)
				hws = ws->mon->selws;
			showws(ws);
		}
	} else {
		fprintf(stderr, "viewwsonmon: ws %s not pinned\n", ws->name);
		if (ws->mon != m) {
			fprintf(stderr, "viewwsonmon: ws->mon != m\n");
			if (ws->visible) {
				fprintf(stderr, "viewwsonmon: ws->visible\n");
				if (enabled(GreedyMonitor) || !m->selws || m->selws->pinned) {
					fprintf(stderr, "viewwsonmon: m->selws->pinned\n");
					/* The current workspace is pinned, or there are no workspaces on the current
					 * monitor. In this case, move the other workspace to the current monitor and
					 * change to the next available workspace on the other monitor. */
					do_warp = 1;

					adjustwsformonitor(ws, selmon);

					if (m->selws)
						hws = m->selws;
					omon = ws->mon;

					/* Find the next available workspace on said monitor */
					for (ows = ws->next; ows && ows->mon != ws->mon; ows = ows->next);
					if (!ows)
						for (ows = workspaces; ows && ows != ws && ows->mon != ws->mon; ows = ows->next);
					if (ows == ws)
						ows = NULL;

					fprintf(stderr, "viewwsonmon: mws is %s\n", ows ? ows->name : "NULL");
					omon->selws = ows;
					ws->mon = m;
					m->selws = ws;
					selws = ws;
					showws(ws);
					clientsfsrestore(ws->clients);
					if (ows)
						showws(ows);
					// drawbar(omon);
				} else {
					fprintf(stderr, "viewwsonmon: !m->selws->pinned\n");
					ows = m->selws;
					adjustwsformonitor(ows, ws->mon);
					adjustwsformonitor(ws, ows->mon);
					ows->mon = ws->mon;
					ws->mon = m;
					fprintf(stderr, "views: set %s->mon to %d and %s->mon to %d\n", ows->name, ows->mon->num, ws->name, ws->mon->num);

					showws(ows);
					showws(ws);
					clientsfsrestore(ows->clients);
					clientsfsrestore(ws->clients);
				}
			} else {
				fprintf(stderr, "viewwsonmon: !ws->visible\n");
				adjustwsformonitor(ws, m);
				omon = ws->mon;
				// This is partially the same as the else below, also partially the same as the
				// pinned example, perhaps we need to nail down the different ways of manipulating
				// workspaces, definitely would be good with some refactoring
				if (m->selws && m->selws->visible)
					hws = m->selws;
				ws->mon = m;
				showws(ws);
				clientsfsrestore(ws->clients);
			}

		} else {
			fprintf(stderr, "viewwsonmon: ws->mon == m, m->selws = %s and selws = %s, m->selws->visible = %d\n", MWSNAME(m), selws->name, m->selws ? m->selws->visible : 0);

			if (m->selws && ws != m->selws && m->selws->visible)
				hws = m->selws;
			showws(ws);
		}
		// arrange(m);
	}
	if (hws)
		hidews(hws); // hiding after showing workspace to avoid flickering (seeing the background for a brief second) when changing workspace
	if (omon) {
		/* if all workspaces have been moved over then clear selws for the other monitor */
		for (ws = workspaces; ws && ws->mon != omon; ws = ws->next);
		if (!ws)
			omon->selws = NULL;
		drawbar(omon);
	}
	updatecurrentdesktop();
	focus(NULL);
	fprintf(stderr, "viewwsonmon: <--\n");

	if (do_warp && ws->sel) {
		fprintf(stderr, "viewwsonmon: warping to client %s\n", ws->sel ? ws->sel->name : "NULL");
		warp(ws->sel);
	}
}

Workspace *
getwsbyname(const Arg *arg)
{
	Workspace *ws;
	char *wsname = (char*)arg->v;
	for (ws = workspaces; ws && strcmp(ws->name, wsname) != 0; ws = ws->next);
	return ws;
}