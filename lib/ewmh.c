void
persistworkspacestate(Workspace *ws)
{
	Client *c;
	unsigned int i;
	char atom[22] = {0};

	sprintf(atom, "_DUSK_WORKSPACE_%u", ws->num);

	/* Perists workspace information in 32 bits laid out like this:
	 *
	 * 000|1|0|0000|0000|0001|0001|000|000|001|0|1
	 *    | | |    |    |    |    |   |   |   | |-- ws->visible
	 *    | | |    |    |    |    |   |   |   |-- ws->pinned
	 *    | | |    |    |    |    |   |   |-- ws->nmaster
	 *    | | |    |    |    |    |   |-- ws->nstack
	 *    | | |    |    |    |    |-- ws->mon
	 *    | | |    |    |    |-- ws->ltaxis[LAYOUT] (i.e. split)
	 *    | | |    |    |-- ws->ltaxis[MASTER]
	 *    | | |    |-- ws->ltaxis[STACK]
	 *    | | |-- ws->ltaxis[STACK2]
	 *    | |-- mirror layout (indicated by negative ws->ltaxis[LAYOUT])
	 *    |-- ws->enablegaps
	 */
	unsigned long data[] = {
		(ws->visible & 0x1) |
		(ws->pinned & 0x1) << 1 |
		(ws->nmaster & 0x7) << 2 |
		(ws->nstack & 0x7 ) << 5 |
		(ws->mon->num & 0x7) << 8 |
		(abs(ws->ltaxis[LAYOUT]) & 0xF) << 11 |
		(ws->ltaxis[MASTER] & 0xF) << 15 |
		(ws->ltaxis[STACK] & 0xF) << 19 |
		(ws->ltaxis[STACK2] & 0xF) << 23 |
		(ws->ltaxis[LAYOUT] < 0 ? 1 : 0) << 27 |
		(ws->enablegaps & 0x1) << 28
	};

	XChangeProperty(dpy, root, XInternAtom(dpy, atom, False), XA_CARDINAL, 32, PropModeReplace, (unsigned char *)data, 1);

	/* set dusk client atoms */
	for (i = 1, c = ws->clients; c; c = c->next, ++i) {
		c->idx = i;
		setclientflags(c);
		setclientfields(c);
		savewindowfloatposition(c, c->ws->mon);
		if (c->swallowing) {
			c->swallowing->idx = i;
			setclientflags(c->swallowing);
			setclientfields(c->swallowing);
			savewindowfloatposition(c->swallowing, c->swallowing->ws->mon);
		}
	}

	XSync(dpy, False);
}

void
savewindowfloatposition(Client *c, Monitor *m)
{
	char atom[22] = {0};
	if (c->sfx == -9999)
		return;

	sprintf(atom, "_DUSK_FLOATPOS_%u", m->num);
	unsigned long pos[] = { (MAX(c->sfx, 0) & 0xffff) | ((MAX(c->sfy, 0) & 0xffff) << 16) };
	XChangeProperty(dpy, c->win, XInternAtom(dpy, atom, False), XA_CARDINAL, 32, PropModeReplace, (unsigned char *)pos, 1);

	sprintf(atom, "_DUSK_FLOATSIZE_%u", m->num);
	unsigned long size[] = { (c->sfw & 0xffff) | ((c->sfh & 0xffff) << 16) };
	XChangeProperty(dpy, c->win, XInternAtom(dpy, atom, False), XA_CARDINAL, 32, PropModeReplace, (unsigned char *)size, 1);

	XSync(dpy, False);
}

int
restorewindowfloatposition(Client *c, Monitor *m)
{
	char atom[22] = {0};
	Atom key, value;
	int x, y, w, h;

	sprintf(atom, "_DUSK_FLOATPOS_%u", m->num);

	key = XInternAtom(dpy, atom, False);
	if (!key)
		return 0;

	value = getatomprop(c, key, AnyPropertyType);
	if (!value)
		return 0;

	x = value & 0xffff;
	y = value >> 16;

	sprintf(atom, "_DUSK_FLOATSIZE_%u", m->num);

	key = XInternAtom(dpy, atom, False);
	if (!key)
		return 0;

	value = getatomprop(c, key, AnyPropertyType);
	if (!value)
		return 0;

	w = value & 0xffff;
	h = value >> 16;

	if (w <= 0 || h <= 0) {
		fprintf(stderr, "restorewindowfloatposition: bad float values x = %d, y = %d, w = %d, h = %d for client = %s\n", x, y, w, h, c->name);
		return 0;
	}

	c->sfx = x;
	c->sfy = y;
	c->sfw = w;
	c->sfh = h;

	return 1;
}

void
setdesktopnames(void)
{
	int i;
	Workspace *ws;
	XTextProperty text;

	char *wslist[num_workspaces];
	for (i = 0, ws = workspaces; ws; ws = ws->next, ++i)
		wslist[i] = wsicon(ws);

	Xutf8TextListToTextProperty(dpy, wslist, num_workspaces, XUTF8StringStyle, &text);
	XSetTextProperty(dpy, root, &text, netatom[NetDesktopNames]);
}

void
setfloatinghint(Client *c)
{
	unsigned int floating[1] = {ISFLOATING(c) ? 1 : 0};
	XChangeProperty(dpy, c->win, clientatom[IsFloating], XA_CARDINAL, 32, PropModeReplace, (unsigned char *)floating, 1);
}

void
setclientflags(Client *c)
{
	unsigned long data1[] = { c->flags & 0xFFFFFFFF };
	unsigned long data2[] = { c->flags >> 32 };
	XChangeProperty(dpy, c->win, clientatom[DuskClientFlags1], XA_CARDINAL, 32, PropModeReplace, (unsigned char *)data1, 1);
	XChangeProperty(dpy, c->win, clientatom[DuskClientFlags2], XA_CARDINAL, 32, PropModeReplace, (unsigned char *)data2, 1);
}

void
setclientfields(Client *c)
{
	unsigned long data[] = { c->ws->num | (c->idx << 6) | (c->scratchkey << 14)};
	XChangeProperty(dpy, c->win, clientatom[DuskClientFields], XA_CARDINAL, 32, PropModeReplace, (unsigned char *)data, 1);
}

void
getclientflags(Client *c)
{
	unsigned long flags1 = getatomprop(c, clientatom[DuskClientFlags1], AnyPropertyType) & 0xFFFFFFFF;
	unsigned long flags2 = getatomprop(c, clientatom[DuskClientFlags2], AnyPropertyType);

	if (flags1 || flags2) {
		c->flags = flags1 | (flags2 << 32);
		removeflag(c, Marked);
		removeflag(c, Centered);
	}
}

void
getclientfields(Client *c)
{
	Workspace *ws;
	Atom fields = getatomprop(c, clientatom[DuskClientFields], AnyPropertyType);
	if (fields) {
		c->scratchkey = (fields >> 14);
		c->idx = (fields & 0x3FC0) >> 6;
		for (ws = workspaces; ws; ws = ws->next)
			if (ws->num == (fields & 0x3F)) {
				c->ws = ws;
				break;
			}
	}
}

void
getworkspacestate(Workspace *ws)
{
	Monitor *m;
	char atom[22] = {0};
	int di, mon;
	unsigned long dl;
	unsigned char *p = NULL;
	Atom da, settings = None;

	sprintf(atom, "_DUSK_WORKSPACE_%u", ws->num);

	Atom wsatom = XInternAtom(dpy, atom, True);
	if (!wsatom)
		return;

	if (XGetWindowProperty(dpy, root, wsatom, 0L, sizeof settings, False, AnyPropertyType,
		&da, &di, &dl, &dl, &p) == Success && p) {
		settings = *(Atom *)p;
		XFree(p);

		/* See bit layout in the persistworkspacestate function */
		mon = (settings >> 8) & 0x7;
		for (m = mons; m && m->num != mon; m = m->next);
		if (m) {
			ws->mon = m;
			ws->visible = settings & 0x1;
			ws->pinned = (settings >> 1) & 0x1;
			ws->nmaster = (settings >> 2) & 0x7;
			ws->nstack = (settings >> 5) & 0x7;
			ws->ltaxis[LAYOUT] = (settings >> 11) & 0xF;
			if (settings & (1 << 27)) // mirror layout
				ws->ltaxis[LAYOUT] *= -1;
			ws->ltaxis[MASTER] = (settings >> 15) & 0xF;
			ws->ltaxis[STACK] = (settings >> 19) & 0xF;
			ws->ltaxis[STACK2] = (settings >> 23) & 0xF;
			ws->enablegaps = (settings >> 28) & 0x1;
			if (ws->visible)
				ws->mon->selws = ws;
		}
	}
}

void
setnumdesktops(void)
{
	long data[] = { num_workspaces };
	XChangeProperty(dpy, root, netatom[NetNumberOfDesktops], XA_CARDINAL, 32, PropModeReplace, (unsigned char *)data, 1);
}

void
setviewport(void)
{
	long data[] = { 0, 0 };
	XChangeProperty(dpy, root, netatom[NetDesktopViewport], XA_CARDINAL, 32, PropModeReplace, (unsigned char *)data, 2);
}

void
updatecurrentdesktop(void)
{
	long data[] = { selws->num };
	XChangeProperty(dpy, root, netatom[NetCurrentDesktop], XA_CARDINAL, 32, PropModeReplace, (unsigned char *)data, 1);
}

void
updateclientdesktop(Client *c)
{
	long data[] = { c->ws->num };
	XChangeProperty(dpy, c->win, netatom[NetWMDesktop], XA_CARDINAL, 32, PropModeReplace, (unsigned char *)data, 1);
}
