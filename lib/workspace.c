static Workspace *stickyws;

void
attachws(Workspace *ws, Workspace *target)
{
	Workspace **wsp;
	for (wsp = &workspaces; *wsp && *wsp != target; wsp = &(*wsp)->next);
	ws->next = target;
	*wsp = ws;
}

void
detachws(Workspace *ws)
{
	Workspace **wsp;
	for (wsp = &workspaces; *wsp && *wsp != ws; wsp = &(*wsp)->next);
	*wsp = ws->next;
	ws->next = NULL;
}

void
attachmon(Monitor *m, Monitor *target)
{
	Monitor **mp;
	for (mp = &mons; *mp && *mp != target; mp = &(*mp)->next);
	m->next = target;
	*mp = m;
}

void
detachmon(Monitor *m)
{
	Monitor **mp;
	for (mp = &mons; *mp && *mp != m; mp = &(*mp)->next);
	*mp = m->next;
	m->next = NULL;
}

void
comboviewwsbyname(const Arg *arg)
{
	viewwsonmon(getwsbyname(arg), NULL, combo);
	combo = 1;
}

void
comboviewwsbyindex(const Arg *arg)
{
	viewwsonmon(getwsbyindex(arg), NULL, combo);
	combo = 1;
}

Workspace *
getnullws(Monitor *m)
{
	if (!m->nullws) {
		m->nullws = m->selws = createnullws(m);
		assignworkspacetomonitor(m->nullws, m);
		attachws(m->nullws, NULL);
		num_workspaces++;
	}

	return m->nullws;
}

Workspace *
createnullws(Monitor *m)
{
	Workspace *ws;

	const WorkspaceRule nullwsrule = {
		.name = "",
		.monitor = m->num,
		.pinned = 1,
		.layout = -1,
		.mfact = -1,
		.nmaster = -1,
		.enablegaps = -1
	};

	ws = createworkspace(stickyws->num + m->num + 1, &nullwsrule);

	return ws;
}

void
teardownnullws(Monitor *m)
{
	Workspace *nullws = m->nullws;

	if (!nullws)
		return;

	/* Migrate clients, if any */
	moveallclientstows(nullws, workspaces, 0);

	if (m->selws == nullws)
		m->selws = NULL;
	m->nullws = NULL;

	detachws(nullws);
	removepreview(nullws);
	free(nullws);

	num_workspaces--;
}

void
createworkspaces(void)
{
	int i;

	for (i = 0; i < LENGTH(wsrules); i++)
		attachws(createworkspace(i, &wsrules[i]), NULL);

	num_workspaces = i;

	stickyws = createstickyworkspace();
	attachws(stickyws, workspaces);
}

Workspace *
createworkspace(int num, const WorkspaceRule *r)
{
	Workspace *ws;
	int num_layouts = LENGTH(layouts) - 1;

	ws = ecalloc(1, sizeof(Workspace));
	ws->num = num;
	ws->pinned = 0;
	ws->wfact = 1.0;
	ws->rule_pinned = r->pinned;
	ws->rule_monitor = r->monitor;

	strlcpy(ws->name, r->name, sizeof ws->name);

	ws->layout = (r->layout == -1 ? &layouts[0] : &layouts[MIN(r->layout, num_layouts)]);
	strlcpy(ws->ltsymbol, ws->layout->symbol, sizeof ws->ltsymbol);
	ws->prevlayout = &layouts[1 % num_layouts];
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

	return ws;
}

Workspace *
createstickyworkspace(void)
{
	int i;
	Workspace *ws;

	/* Find the floating layout for the sticky rule */
	for (i = 0; i < LENGTH(layouts); i++)
		if ((&layouts[i])->arrange == NULL)
			break;

	const WorkspaceRule stickywsrule = { .name = "Sticky", .layout = i };
	ws = createworkspace(num_workspaces, &stickywsrule);
	ws->visible = 1;
	ws->mon = mons;
	ws->wh = sh;
	ws->ww = sw;

	return ws;
}

void
abandonworkspaces(Monitor *mon)
{
	Workspace *ws;

	for (ws = workspaces; ws; ws = ws->next) {
		if (ws->mon != mon)
			continue;

		if (ws == stickyws) {
			ws->mon = mons;
			continue;
		}

		if (workspaces_per_mon && ws->pinned) {
			stash_workspace(ws);
			continue;
		}

		hidews(ws);
		hidewsclients(ws->stack);
		assignworkspacetomonitor(ws, mons);
		ws->pinned = 0;
	}
}

/* Moves all clients to the selected workspace on the primary monitor and moves workspace
 * itself to the dummy monitor for safekeeping. */
void
stash_workspace(Workspace *ws)
{
	Workspace *target;

	ws->visible = 0;
	target = mons->selws;
	if (!target || target == stickyws)
		for (target = workspaces; target == stickyws; target = target->next);
	moveallclientstows(ws, target, 0);
	ws->mon = dummymon;
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

	wsmask |= (1L << stickyws->num);

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

void
storewsmask(void)
{
	Monitor *m = selws ? selws->mon : selmon;
	uint64_t wsmask = getwsmask(m);

	if (m->prevwsmask == wsmask && m->wsmask)
		return;

	if (m->prevwsmask)
		m->wsmask = m->prevwsmask;
}

void
togglewsmask(void)
{
	Monitor *m = selws->mon;
	uint64_t wsmask = getwsmask(m);

	if (m->prevwsmask == wsmask && m->wsmask) {
		if (!monitorchanged && getallwsmask(m) & m->wsmask) {
			viewwsmask(m, m->wsmask);
		}
	}

	monitorchanged = 0;
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

int
hashidden(Workspace *ws)
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

	for (c = ws->clients; c && (ISINVISIBLE(c) || SKIPTASKBAR(c) || HIDDEN(c) || ISTILED(c)); c = c->next);
	return c != NULL;
}

int
hasfullscreen(Workspace *ws)
{
	Client *c;

	if (!ws)
		return 0;

	for (c = ws->clients; c && !(ISTRUEFULLSCREEN(c) && !HIDDEN(c) && !ISINVISIBLE(c)); c = c->next);
	return c != NULL;
}

int
ismaximized(Client *c, int x, int y, int w, int h)
{
	Monitor *m = c->ws->mon;

	if (w * h == 0) {
		x = c->x;
		y = c->y;
		w = c->w;
		h = c->h;
	}

	return abs(y - m->wy) <= m->oh &&
	       abs(x - m->wx) <= m->ov &&
	       abs(w - m->ww) <= (m->ov + c->bw) * 2 &&
	       abs(h - m->wh) <= (m->oh + c->bw) * 2;
}

int
noborder(Client *c, int x, int y, int w, int h)
{
	int maximized;

	if (disabled(NoBorders))
		return 0;

	maximized = ismaximized(c, x, y, w, h);

	if (FREEFLOW(c))
		return maximized;

	if (ISTRUEFULLSCREEN(c))
		return 0;

	if (NOBORDER(c))
		return 1;

	/* Special case if client size takes up the entire window area */
	if (maximized)
		return 1;

	if (nexttiled(c->ws->clients) != c || nexttiled(c->next))
		return 0;

	return 1;
}

void
restoreborder(Client *c)
{
	XWindowChanges wc;
	wc.border_width = c->bw;
	wc.width = c->w;
	wc.height = c->h;
	XConfigureWindow(dpy, c->win, CWWidth|CWHeight|CWBorderWidth, &wc);
	configure(c);
}

void
adjustwsformonitor(Workspace *ws, Monitor *m)
{
	if (!ws || !m)
		return;

	ws->wx = m->wx;
	ws->wy = m->wy;
	ws->wh = m->wh;
	ws->ww = m->ww;

	clientsmonresize(ws->clients, ws->mon, m);
	reorientworkspace(ws, m->orientation);
}

void
hidews(Workspace *ws)
{
	if (!ws || ws == stickyws || ws == ws->mon->nullws)
		return;

	ws->visible = 0;
}

void
hidewsotherthan(Workspace *shownws)
{
	Monitor *m = shownws->mon;
	Workspace *ws;

	for (ws = nextvismonws(m, workspaces); ws; ws = nextvismonws(m, ws->next))
		if (ws != shownws)
			hidews(ws);
}

void
showws(Workspace *ws)
{
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
	if (ISVISIBLE(c) && (FREEFLOW(c) || ISTRUEFULLSCREEN(c))) {
		if (WASNOBORDER(c))
			restoreborder(c);
		if (NEEDRESIZE(c)) {
			removeflag(c, NeedResize);
			XMoveResizeWindow(dpy, c->win, c->x, c->y, c->w, c->h);
		} else if (!ISSTICKY(c) && c->sfx != -9999 && !ISTRUEFULLSCREEN(c)) {
			restorefloats(c);
		} else {
			show(c);
		}
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
	movetows(c, ws, enabled(ViewOnWs));
}

int
numtiled(Workspace *ws)
{
	Client *c;
	int n;
	for (n = 0, c = nexttiled(ws->clients); c; c = nexttiled(c->next), n++);
	return n;
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
swapwsbyindex(const Arg *arg)
{
	swapwsclients(selws, getwsbyindex(arg));
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
	} else {
		hidewsclients(ws1->stack);
	}

	if (ws2->visible) {
		showwsclients(c1);
		arrangews(ws2);
	} else {
		hidewsclients(ws2->stack);
	}

	drawbars();
}

void
movetows(Client *c, Workspace *ws, int view_workspace)
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

		if (!view_workspace && !ws->visible)
			hide(c);
	}

	if (prevws && prevws->visible && prevws != ws)
		arrange(prevws);

	if (hadfocus && ws->visible)
		focus(hadfocus);
	else
		focus(NULL);

	if (view_workspace && !ws->visible) {
		viewwsonmon(ws, ws->mon, 0);
		return;
	}

	if (ws->visible) {
		arrange(ws);
		if (view_workspace && hadfocus)
			warp(hadfocus);
		return;
	}

	drawbar(ws->mon);
}

void
moveallclientstows(Workspace *from, Workspace *to, int view_workspace)
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

	if (view_workspace && !to->visible)
		viewwsonmon(to, to->mon, 0);

	if (to->visible)
		arrangews(to);
	else
		hidewsclients(to->stack);

	drawbar(to->mon);
	if (from->mon != to->mon)
		drawbar(from->mon);
}

void
movetowsbyname(const Arg *arg)
{
	movetows(selws->sel, getwsbyname(arg), 1);
}

void
movetowsbyindex(const Arg *arg)
{
	movetows(selws->sel, getwsbyindex(arg), 1);
}

void
sendtowsbyname(const Arg *arg)
{
	movetows(selws->sel, getwsbyname(arg), 0);
}

void
sendtowsbyindex(const Arg *arg)
{
	movetows(selws->sel, getwsbyindex(arg), 0);
}

void
movealltowsbyname(const Arg *arg)
{
	moveallclientstows(selws, getwsbyname(arg), enabled(ViewOnWs));
}

void
movealltowsbyindex(const Arg *arg)
{
	moveallclientstows(selws, getwsbyindex(arg), enabled(ViewOnWs));
}

void
moveallfromwsbyname(const Arg *arg)
{
	moveallclientstows(getwsbyname(arg), selws, 0);
}

void
moveallfromwsbyindex(const Arg *arg)
{
	moveallclientstows(getwsbyindex(arg), selws, 0);
}

/* Send client to an adjacent workspace on the current monitor */
void
movewsdir(const Arg *arg)
{
	movetows(selws->sel, dirtows(arg->i), enabled(ViewOnWs));
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
	Workspace *ws = (arg->v ? (Workspace*)arg->v : selws);

	if (ws == ws->mon->nullws)
		return;

	ws->pinned = !ws->pinned;
	drawbar(ws->mon);
}

void
togglews(const Arg *arg)
{
	Monitor *m = selmon;

	if (!m->prevwsmask)
		return;

	viewwsmask(m, m->prevwsmask);
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
enablewsbyindex(const Arg *arg)
{
	viewwsonmon(getwsbyindex(arg), NULL, 1);
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
viewwsbyindex(const Arg *arg)
{
	viewwsonmon(getwsbyindex(arg), NULL, 0);
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

	/* Workspaces that are assigned to dummymon are not intended to be viewed; they are temporary
	 * assigned to the the dummy monitor in the context of the workspaces_per_mon functionality and
	 * they are staying there until the surplus monitor is available.
	 *
	 * That said there may be obscure edge cases where a client can get assigned to a workspace on
	 * the dummy monitor or such a workspace being brought into view. If so we allow that workspace
	 * to be brought to the selected monitor.
	 */
	if (ws->mon == dummymon)
		ws->mon = selmon;
	if (m == dummymon)
		m = selmon;

	Monitor *omon = NULL;
	Workspace *ows = NULL;

	if (!combo)
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
			if (!combo)
				m->prevwsmask = getwsmask(m);
			monitorchanged = 1;
			selmon = m;
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
			omon = ws->mon;

			assignworkspacetomonitor(ws, selmon);

			selws = m->selws = ws;
			showws(ws);
			clientsfsrestore(ws->clients);

			ows = selectmonws(omon);
			omon->selws = ows;
			ows->visible = 1;
			arrangeall = 1;
		} else {
			/* Swap the selected workspace on this monitor with the visible desired workspace
			 * on the other monitor. */
			ows = m->selws;
			assignworkspacetomonitor(ows, ws->mon);
			assignworkspacetomonitor(ws, m);

			showws(ows);
			showws(ws);
			clientsfsrestore(ows->clients);
			clientsfsrestore(ws->clients);
			arrangeall = 1;
		}
	} else {
		/* Workspace is not visible, just grab it */
		assignworkspacetomonitor(ws, m);
		showws(ws);
		clientsfsrestore(ws->clients);
	}

	if (!enablews)
		hidewsotherthan(ws);

	if (!combo)
		storewsmask();

	drawws(ws, m, m->prevwsmask, enablews, arrangeall, do_warp);
}

void
drawws(Workspace *ws, Monitor *m, uint64_t prevwsmask, int enablews, int arrangeall, int do_warp)
{
	int x, y;
	Workspace *mousepointerws, *w;
	uint64_t hidewsmask = prevwsmask - (prevwsmask & getwsmask(m));

	setworkspaceareas();

	/* Ensure that there is a selected workspace for every monitor */
	reviewworkspaces(0);

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
		selws = m->selws;
}

Workspace *
getwsbyname(const Arg *arg)
{
	Workspace *ws;
	char *wsname = (char*)arg->v;
	for (ws = workspaces; ws && strcmp(ws->name, wsname) != 0; ws = ws->next);
	if (workspaces_per_mon && ws && ws->mon == dummymon)
		return NULL;
	return ws;
}

Workspace *
getwsbyindex(const Arg *arg)
{
	Workspace *ws;
	int index = arg->i;
	int i = 0;
	for (ws = workspaces; ws; ws = ws->next) {
		if (ws->mon != selmon || ws == stickyws)
			continue;
		if (++i == index)
			return ws;
	}
	return NULL;
}

Workspace *
getwsbynum(int num)
{
	Workspace *ws;
	for (ws = workspaces; ws && ws->num != num; ws = ws->next);
	return ws;
}

Workspace *
nextmonws(Monitor *mon, Workspace *ws)
{
	Workspace *w;
	for (w = ws; w && (w->mon != mon || w == stickyws); w = w->next);
	return w;
}

Workspace *
nextoccmonws(Monitor *mon, Workspace *ws)
{
	Workspace *w;
	for (w = ws; w && (w->mon != mon || w == stickyws || w->stack == NULL); w = w->next);
	return w;
}

Workspace *
nextvismonws(Monitor *mon, Workspace *ws)
{
	Workspace *w;
	for (w = ws; w && !(w->mon == mon && w->visible && w != stickyws); w = w->next);
	return w;
}

Workspace *
selectmonws(Monitor *m)
{
	Workspace *ws;

	/* Prioritise the first visible workspace on the given monitor */
	ws = nextvismonws(m, workspaces);

	/* Fall back to the first occcupied workspace on the given monitor */
	if (!ws)
		ws = nextoccmonws(m, workspaces);

	/* Fall back to the first workspace on the given monitor */
	if (!ws)
		ws = nextmonws(m, workspaces);

	/* Fall back to creating a "null" workspace */
	if (!ws)
		ws = getnullws(m);

	return ws;
}

void
assignworkspacetomonitor(Workspace *ws, Monitor *m)
{
	Monitor *wsmon;

	if (!ws || !m || ws->mon == m)
		return;

	wsmon = ws->mon;

	adjustwsformonitor(ws, m);
	ws->mon = m;
	if (wsmon && wsmon->selws == ws) {
		wsmon->selws = selectmonws(wsmon);
	}

	if (m->nullws && ws != m->nullws) {
		if (m->selws == m->nullws)
			m->selws = ws;
		moveallclientstows(m->nullws, ws, 0);
		teardownnullws(m);
	}
}

/* Handles (re-)distribution of workspaces across all available monitors. */
void
distributeworkspaces(void)
{
	Monitor *m = mons, *mr = NULL;
	Workspace *ws;

	for (ws = workspaces; ws; ws = ws->next) {
		if (ws == stickyws)
			continue;

		/* If the workspace rule specified a designated monitor, and that monitor exists, then
		 * this will have precedence. A restart may override the specified rule values. */
		for (mr = mons; mr && mr->num != ws->rule_monitor; mr = mr->next);
		if (mr) {
			assignworkspacetomonitor(ws, mr);
			ws->pinned = ws->rule_pinned;
			continue;
		}

		/* Otherwise distribute workspaces evenly. */
		ws->pinned = 0;
		assignworkspacetomonitor(ws, m);
		m = (m->next == NULL ? mons : m->next);
	}
}

void
reorientworkspaces(void)
{
	Workspace *ws;

	for (ws = workspaces; ws; ws = ws->next) {
		adjustwsformonitor(ws, ws->mon);
	}
}

void
reorientworkspace(Workspace *ws, int orientation)
{
	if (ws->orientation == orientation)
		return;

	if (enabled(SmartLayoutConversion) && ws->layout->arrange == flextile)
		layoutconvert(&((Arg) { .v = ws }));
	ws->orientation = orientation;
}

void
reviewworkspaces(int hide_others)
{
	Monitor *m;

	/* Make sure that at least one workspace is visible on each monitor. */
	for (m = mons; m; m = m->next) {
		if (!m->selws || m->selws->mon != m) {
			m->selws = selectmonws(m);
			m->selws->visible = 1;
		}

		/* Hide the rest, if any */
		if (hide_others) {
			hidewsotherthan(m->selws);
		}
	}
}

void
setwfact(const Arg *arg)
{
	float f;
	Workspace *ws = selws;

	if (!ws)
		return;

	if (!arg->f)
		f = 1.0;
	else if (arg->f < 4.0)
		f = arg->f + ws->wfact;
	else // 4.0 or greater - set fact absolutely
		f = arg->f - 4.0;

	/* Restrict to minimum and maximum fact values */
	if (f < 0.25)
		f = 0.25;
	else if (f > 4.0)
		f = 4.0;

	ws->wfact = f;

	setworkspaceareasformon(ws->mon);
	arrangemon(ws->mon);
}

void
setworkspaceareas(void)
{
	Monitor *mon;
	for (mon = mons; mon; mon = mon->next)
		setworkspaceareasformon(mon);
}

void
setworkspaceareasformon(Monitor *m)
{
	Workspace *ws;
	int i, crest, colw, cols, rows, cx, cy, cw, ch, cn, rn, wc, nw;
	float wfacts_total = 0;

	/* get a count of the number of visible workspaces for this monitor */
	for (nw = 0, ws = nextvismonws(m, workspaces); ws; ws = nextvismonws(m, ws->next), ++nw);
	if (!nw)
		return;

	/* grid dimensions */
	if (m->ww > m->wh) {
		if (m->ww / nw < m->wh / 2) {
			rows = 2;
			cols = nw / rows + (nw - rows * (nw / rows));
		} else {
			rows = 1;
			cols = nw;
		}
	} else {
		if (m->wh / nw < m->ww / 2) {
			cols = 2;
			rows = nw / cols + (nw - cols * (nw / cols));
		} else {
			cols = 1;
			rows = nw;
		}
	}

	crest = colw = m->ww;

	float wfacts[cols];
	int rrests[cols];
	for (i = 0; i < cols; i++) {
		wfacts[i] = 0;
		rrests[i] = 0;
	}

	/* Sum wfacts for columns */
	cn = rn = 0; /* reset column no, row no */
	for (ws = nextvismonws(m, workspaces); ws; ws = nextvismonws(m, ws->next)) {
		wfacts[cn] += ws->wfact;
		wfacts_total += ws->wfact;
		rn++;
		if (rn >= rows) {
			rn = 0;
			cn++;
		}
	}

	/* Work out wfact remainders */
	cn = rn = 0; /* reset column no, row no */
	for (ws = nextvismonws(m, workspaces); ws; ws = nextvismonws(m, ws->next)) {
		rrests[cn] += m->wh * (ws->wfact / wfacts[cn]);
		rn++;
		if (rn >= rows) {
			rn = 0;
			cn++;
		}
	}

	/* Now wfacts and rrests contain a sum of height and width used. This goes through each
	 * column and make sure that we only store the remainder. */
	for (i = 0; i < cols; i++) {
		crest -= (int)(colw * (wfacts[i] / wfacts_total));
		rrests[i] = m->wh - rrests[i];
	}

	cx = m->wx;
	cy = m->wy;

	cn = rn = wc = 0; /* reset column no, row no, workspace count */
	for (ws = nextvismonws(m, workspaces); ws; ws = nextvismonws(m, ws->next)) {
		if ((rows * cols) > nw && wc + 1 == nw) {
			rows = 1;
			ch = m->wh;
		}

		cw = (int)(colw * (wfacts[cn] / wfacts_total)) + (cn < crest ? 1 : 0);
		ch = m->wh * ((double)ws->wfact / (double)wfacts[cn]) + (rn < rrests[cn] ? 1 : 0);

		ws->ww = cw;
		ws->wh = ch;
		ws->wx = cx;
		ws->wy = cy;
		rn++;
		wc++;
		cy += ch;
		if (rn >= rows) {
			rn = 0;
			cx += cw + (cn < crest ? 1 : 0);
			cn++;
			cy = m->wy;
		}
	}
}
