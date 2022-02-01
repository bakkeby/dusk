int
atomin(Atom input, Atom *list, int nitems)
{
	for (int i = 0; list && i < nitems; i++)
		if (input == list[i])
			return 1;
	return 0;
}

void
persistworkspacestate(Workspace *ws)
{
	Client *c;
	unsigned int i;

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

	XChangeProperty(dpy, root, clientatom[DuskWorkspace], XA_CARDINAL, 32,
		ws->num ? PropModeAppend : PropModeReplace, (unsigned char *)data, 1);

	/* set dusk client atoms */
	for (i = 1, c = ws->clients; c; c = c->next, ++i) {
		if (SEMISCRATCHPAD(c) && c->linked) {
			if (c->scratchkey)
				continue;
			if (!c->win)
				swapsemiscratchpadclients(c->linked, c);
			c->scratchkey = c->linked->scratchkey;
		}
		c->idx = i;
		setclientflags(c);
		setclientfields(c);
		setclientlabel(c);
		savewindowfloatposition(c, c->ws->mon);
		if (c->swallowing) {
			c->swallowing->idx = i;
			setclientflags(c->swallowing);
			setclientfields(c->swallowing);
			setclientlabel(c->swallowing);
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
	unsigned long pos[] = { (MAX(c->sfx - m->mx, 0) & 0xffff) | ((MAX(c->sfy - m->my, 0) & 0xffff) << 16) };
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

	c->sfx = m->mx + x;
	c->sfy = m->my + y;
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
	XChangeProperty(dpy, c->win, clientatom[DuskClientFlags], XA_CARDINAL, 32, PropModeReplace, (unsigned char *)data1, 1);
	XChangeProperty(dpy, c->win, clientatom[DuskClientFlags], XA_CARDINAL, 32, PropModeAppend,  (unsigned char *)data2, 1);
}

void
setclientfields(Client *c)
{
	unsigned long data[] = { c->ws->num | (c->idx << 6) | (c->scratchkey << 14)};
	XChangeProperty(dpy, c->win, clientatom[DuskClientFields], XA_CARDINAL, 32, PropModeReplace, (unsigned char *)data, 1);
}

void
setclientlabel(Client *c)
{
	XChangeProperty(dpy, c->win, clientatom[DuskClientLabel], XA_STRING, 8, PropModeReplace, (unsigned char *)c->label, strlen(c->label));
}

void
getclientflags(Client *c)
{
	int di;
	unsigned long dl, nitems, flags1 = 0, flags2 = 0;
	unsigned char *p = NULL;
	Atom da = None;
	Atom *cflags;

	if (XGetWindowProperty(dpy, c->win, clientatom[DuskClientFlags], 0L, 2 * sizeof flags1, False,
			AnyPropertyType, &da, &di, &nitems, &dl, &p) == Success && p) {
		cflags = (Atom *) p;
		if (nitems == 2) {
			flags1 = cflags[0] & 0xFFFFFFFF;
			flags2 = cflags[1] & 0xFFFFFFFF;
		}
		XFree(p);
	}

	/* Temporary code to allow live restart from previous atom properties */
	if (!flags1 && !flags2) {
		Atom flag1atom = XInternAtom(dpy, "_DUSK_CLIENT_FLAGS1", False);
		Atom flag2atom = XInternAtom(dpy, "_DUSK_CLIENT_FLAGS2", False);
		flags1 = getatomprop(c, flag1atom, AnyPropertyType) & 0xFFFFFFFF;
		flags2 = getatomprop(c, flag2atom, AnyPropertyType);
	}
	/* End temporary code */

	if (flags1 || flags2) {
		c->flags = flags1 | (flags2 << 32);
		/* Remove flags that should not survive a restart */
		removeflag(c, Marked|Centered|SwitchWorkspace|EnableWorkspace|RevertWorkspace);
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
getclientlabel(Client *c)
{
	Atom type;
	int format;
	unsigned int i;
	unsigned long after;
	unsigned char *data = 0;
	long unsigned int size = LENGTH(c->label);

	if (XGetWindowProperty(dpy, c->win, clientatom[DuskClientLabel], 0, 1024, 0, XA_STRING,
				&type, &format, &size, &after, &data) == Success) {
		if (data) {
			if (type == XA_STRING) {
				for (i = 0; i < size; ++i)
					c->label[i] = data[i];
			}
			XFree(data);
		}
	}
}

void
getworkspacestate(Workspace *ws)
{
	Monitor *m;
	int di, mon;
	unsigned long dl, nitems;
	unsigned char *p = NULL;
	Atom da, settings = None;

	if (!(XGetWindowProperty(dpy, root, clientatom[DuskWorkspace], ws->num, LENGTH(wsrules) * sizeof dl,
			False, AnyPropertyType, &da, &di, &nitems, &dl, &p) == Success && p)) {
		/* Temporary code to allow live restart from previous atom properties */
		char atom[22] = {0};
		sprintf(atom, "_DUSK_WORKSPACE_%u", ws->num == LENGTH(wsrules) ? 4096 : ws->num);
		Atom wsatom = XInternAtom(dpy, atom, True);
		if (!(XGetWindowProperty(dpy, root, wsatom, 0L, sizeof settings, False, AnyPropertyType,
			&da, &di, &nitems, &dl, &p) == Success && p)) {
			return;
		}
		/* End temporary code */
	}

	if (nitems) {
		settings = *(Atom *)p;

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

	XFree(p);
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
	Monitor *m;
	int nmons = 0;
	for (m = mons; m; m = m->next)
		nmons++;

	long data[nmons * 2];

	m = mons;
	for (int i = 0; i < nmons; i++) {
		data[i*2] = (long)m->mx;
		data[i*2+1] = (long)m->my;
		m = m->next;
	}

	XChangeProperty(dpy, root, netatom[NetDesktopViewport], XA_CARDINAL, 32,
			PropModeReplace, (unsigned char *)data, nmons * 2);
}

void
updatecurrentdesktop(void)
{
	long data[] = { selws ? selws->num : 0 };
	XChangeProperty(dpy, root, netatom[NetCurrentDesktop], XA_CARDINAL, 32, PropModeReplace, (unsigned char *)data, 1);
}

void
updateclientdesktop(Client *c)
{
	long data[] = { c->ws->num };
	XChangeProperty(dpy, c->win, netatom[NetWMDesktop], XA_CARDINAL, 32, PropModeReplace, (unsigned char *)data, 1);
}
