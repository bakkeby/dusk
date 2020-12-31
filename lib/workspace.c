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
	showwsclients(ws);
	arrange(ws->mon);
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
	fprintf(stderr, "movetows: --> client %s --> workspace %s\n", c->name, ws->name);

	int hadfocus = (c == WS->sel);
	unfocus(c, 1, NULL);
	detach(c);
	detachstack(c);
	c->next = NULL;

	if (c->ws->visible)
		arrange(c->ws->mon);

	c->ws = ws;
	c->id = 0;

	if (!(c->tags & SPTAGMASK)) // TODO SPTAGMASK, scratchpads
		c->tags = ws->tags; /* assign tags of target monitor */

	attachx(c);
	attachstack(c);

	if (ws->visible) {
		arrange(ws->mon);
		if (hadfocus) {
			focus(c);
			restack(c->ws);
		} else {
			focus(NULL);
		}
	} else {
		XMoveWindow(dpy, c->win, WIDTH(c) * -2, c->y); // TODO separate function to hide clients?
	}

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
viewws(const Arg *arg)
{
	fprintf(stderr, "viewws: -->\n");

	Monitor *omon;
	Workspace *ws, *mws;
	ws = (Workspace*)arg->v;

	if (ws == NULL)
		fprintf(stderr, "viewws: arg->v is NULL\n");
	else
		fprintf(stderr, "viewws: arg->v is %s\n", ws->name);

	// for (ws = workspaces; ws != arg->v; ws = ws->next);
	if (!ws || selws == ws) {
		fprintf(stderr, "viewws: because %s\n", !ws ? "!ws" : selws == ws ? "selws == ws" : "eh");
		return;
	}

	/* Focus on the workspace on the monitor it resides on */
	// focus(NULL);
	if (ws->pinned) {
		fprintf(stderr, "viewws: ws %s pinned\n", ws->name);
		if (ws->mon->selws->visible)
			hidews(ws->mon->selws);
		showws(ws);
		arrange(ws->mon);
	} else {
		fprintf(stderr, "viewws: ws %s not pinned\n", ws->name);
		if (ws->mon != selmon) {
			fprintf(stderr, "viewws: ws->mon != selmon\n");
			if (ws->visible) {
				fprintf(stderr, "viewws: ws->visible\n");
				mws = selmon->selws; // TODO can selmon->selws be null?
				mws->mon = ws->mon;
				ws->mon = selmon;
				fprintf(stderr, "views: set %s->mon to %d and %s->mon to %d\n", mws->name, mws->mon->num, ws->name, ws->mon->num);
				selws = ws;
				showws(mws);
				showws(ws);
			} else {
				fprintf(stderr, "viewws: !ws->visible\n");
				omon = ws->mon;
				// This is partially the same as the else below, also partially the same as the
				// pinned example, perhaps we need to nail down the different ways of manipulating
				// workspaces, definitely would be good with some refactoring
				if (selmon->selws->visible)
					hidews(selmon->selws);
				ws->mon = selmon;
				selws = ws;
				showws(ws);
				drawbar(omon);
				drawbar(ws->mon);
			}
		} else {
			fprintf(stderr, "viewws: ws->mon == selmon, selmon->selws = %s and selws = %s, selmon->selws->visible = %d\n", selmon->selws->name, selws->name, selmon->selws->visible);
			if (selmon->selws->visible)
				hidews(selmon->selws);
			selws = ws;
			showws(ws);
		}
		// arrange(selmon);
	}
	updatecurrentdesktop();
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

	viewws(&((Arg) { .v = ws }));
	fprintf(stderr, "viewwsbyname: <--\n");
}