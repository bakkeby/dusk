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
	fprintf(stderr, "hidewsclients --> %s\n", ws->name);
	Client *c;
	for (c = ws->stack; c; c = c->snext)
		XMoveWindow(dpy, c->win, WIDTH(c) * -2, c->y);
	fprintf(stderr, "hidewsclients <--\n");
}

void
showwsclients(Workspace *ws)
{
	fprintf(stderr, "showwsclients --> %s\n", ws->name);

	// TODO: probably need something in here to handle moving fullscreen windows from one monitor
	// to another
	showhide(ws->stack);
	fprintf(stderr, "showwsclients <--\n");
	// Client *c;
	// for (c = ws->stack; c; c = c->snext) {
	// 	/* show clients top down */
	// 	if (!c->ws->layout->arrange && c->sfx != -9999 && !ISFULLSCREEN(c)) {
	// 		XMoveWindow(dpy, c->win, c->sfx, c->sfy);
	// 		resize(c, c->sfx, c->sfy, c->sfw, c->sfh, 0);
	// 		continue;
	// 	}
	// 	if (NEEDRESIZE(c)) {
	// 		removeflag(c, NeedResize);
	// 		XMoveResizeWindow(dpy, c->win, c->x, c->y, c->w, c->h);
	// 	} else {
	// 		XMoveWindow(dpy, c->win, c->x, c->y);
	// 	}
	// 	if ((!c->ws->layout->arrange || ISFLOATING(c)) && !ISFULLSCREEN(c))
	// 		resize(c, c->x, c->y, c->w, c->h, 0);
	// }
}

void
movews(const Arg *arg)
{
	fprintf(stderr, "movews: -->");
	Workspace *ws = (Workspace*)arg->v;
	Client *c = selws->sel;
	movetows(c, ws);
	fprintf(stderr, "movews: <--");
}

void
movetows(Client *c, Workspace *ws)
{
	if (!c) {
		fprintf(stderr, "movetows: no client received for ws %s\n", ws->name);
		return;
	}
	int changemon = !MOVERESIZE(c) && c->ws->mon != ws->mon;

	if (changemon) {
		clientmonresize(c, c->ws->mon, ws->mon);
	}
	fprintf(stderr, "movetows: --> client %s --> workspace %s\n", c->name, ws->name);
	int hadfocus = (c == WS->sel);
	unfocus(c, 1, NULL);
	detach(c);
	detachstack(c);
	c->next = NULL;

	if (c->ws->visible)
		arrange(c->ws);

	c->ws = ws;
	c->id = 0;

	attachx(c);
	attachstack(c);

	if (ws->visible) {
		arrange(ws);
		if (hadfocus) {
			focus(c);
			restack(c->ws);
		} else {
			focus(NULL);
		}
		if (changemon)
			clientfsrestore(c);
	} else {
		XMoveWindow(dpy, c->win, WIDTH(c) * -2, c->y); // TODO separate function to hide clients?
	}

	drawbar(c->ws->mon);

	fprintf(stderr, "movetows: <--\n");
}
void
movetowsbyname(const Arg *arg)
{
	fprintf(stderr, "movetowsbyname: -->\n");
	Workspace *ws;
	char *wsname = (char*)arg->v;
	for (ws = workspaces; ws && strcmp(ws->name, wsname) != 0; ws = ws->next);
	if (!ws)
		return;

	movetows(selws->sel, ws);
	fprintf(stderr, "movetowsbyname: <--\n");
}

void
togglepinnedws(const Arg *arg)
{
	Workspace *ws = selws;
	if (arg->v)
		ws = (Workspace*)arg->v;
	ws->pinned = !ws->pinned;
	fprintf(stderr, "togglepinnedws: set ws %s pinned to %d\n", ws->name, ws->pinned);
}

void
viewws(const Arg *arg)
{
	fprintf(stderr, "viewws: -->\n");

	Workspace *ws = (Workspace*)arg->v;
	viewwsonmon(ws, NULL);

	fprintf(stderr, "viewws: <--\n");
}

void
viewwsbyname(const Arg *arg)
{
	fprintf(stderr, "viewwsbyname: -->\n");
	Workspace *ws;
	char *wsname = (char*)arg->v;
	for (ws = workspaces; ws && strcmp(ws->name, wsname) != 0; ws = ws->next);
	if (!ws)
		return;

	viewwsonmon(ws, NULL);
	focus(ws->sel); // TODO hmmm
	fprintf(stderr, "viewwsbyname: <--\n");
}

void
viewwsonmon(Workspace *ws, Monitor *m)
{
	fprintf(stderr, "viewwsonmon: -->\n");

	if (m == NULL)
		m = selmon;

	Monitor *omon = NULL;
	Workspace *ows = NULL, *hws = NULL;

	if (ws == NULL)
		fprintf(stderr, "viewwsonmon: arg->v is NULL\n");
	else
		fprintf(stderr, "viewwsonmon: arg->v is %s\n", ws->name);

	// for (ws = workspaces; ws != arg->v; ws = ws->next);
	if (!ws || (selws == ws && m == ws->mon)) {
		fprintf(stderr, "viewwsonmon: because %s\n", !ws ? "!ws" : selws == ws ? "selws == ws && m == ws->mon" : "eh");
		arrange(ws);
		return;
	}

	/* Focus on the workspace on the monitor it resides on */

	/*
	 * Scenarios:
	 *   View ws2m0, current ws1m0v
	 *      - ws1m0 --> hidden
	 *      - ws2m0 --> shown
	 * quite difficult to write these out in text
	 */


	// TODO: one bug
	//       1) when I started gedit and switched workspaces gedit (floating) would not move, not sure why


	// focus(NULL);
	if (ws->pinned) {
		fprintf(stderr, "viewwsonmon: ws %s pinned, visible = %d\n", ws->name, ws->visible);
		if (!ws->visible) {
			if (ws->mon->selws && ws->mon->selws->visible)
				hws = ws->mon->selws;
			showws(ws);
		}
		// fprintf(stderr, "viewwsonmon: focusing on client %s\n", ws->sel == NULL ? "NULL" : ws->sel->name);
	} else {
		fprintf(stderr, "viewwsonmon: ws %s not pinned\n", ws->name);
		if (ws->mon != m) {
			fprintf(stderr, "viewwsonmon: ws->mon != m\n");
			if (ws->visible) {
				fprintf(stderr, "viewwsonmon: ws->visible\n");
				if (!m->selws || m->selws->pinned) {
					/* The current workspace is pinned, or there are no workspaces on the current
					 * monitor. In this case, move the other workspace to the current monitor and
					 * change to the next available workspace on the other monitor. */
					fprintf(stderr, "viewwsonmon: m->selws->pinned\n");
					clientsmonresize(ws->clients, ws->mon, selmon);
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
					clientsmonresize(ows->clients, ows->mon, ws->mon);
					clientsmonresize(ws->clients, ws->mon, ows->mon);
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
				clientsmonresize(ws->clients, ws->mon, selmon);
				omon = ws->mon;
				// This is partially the same as the else below, also partially the same as the
				// pinned example, perhaps we need to nail down the different ways of manipulating
				// workspaces, definitely would be good with some refactoring
				if (m->selws && m->selws->visible)
					hws = m->selws;
				ws->mon = m;
				showws(ws);
				clientsfsrestore(ws->clients);
				// drawbar(ws->mon); // TODO not sure this is needed, should be handled by showws --> arrange --> arrangews --> drawbar
			}
		} else {
			fprintf(stderr, "viewwsonmon: ws->mon == m, m->selws = %s and selws = %s, m->selws->visible = %d\n", MWSNAME(m), selws->name, m->selws ? m->selws->visible : 0);

			if (m->selws && m->selws->visible)
				hws = m->selws;
			showws(ws);
		}
		// arrange(m);
	}
	if (hws)
		hidews(hws); // hiding after showing workspace to avoid flickering (seeing the background for a brief second) when changing workspace
	if (omon)
		drawbar(omon);
	updatecurrentdesktop();
	fprintf(stderr, "viewwsonmon: <--\n");
}