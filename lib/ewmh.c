void
persistworkspacestate(Workspace *ws)
{
	Client *c;
	unsigned int i;
	char atom[22];

	sprintf(atom, "_DUSK_WORKSPACE_%u", ws->num);
	unsigned long data[] = { ws->visible | ws->pinned << 1 | ws->mon->num << 2 }; // potentially enablegaps, nmaster, nstack, mfact
	XChangeProperty(dpy, root, XInternAtom(dpy, atom, False), XA_CARDINAL, 32, PropModeReplace, (unsigned char *)data, 1);

	/* set dusk client atoms */
	for (i = 1, c = ws->clients; c; c = c->next, ++i) {
		c->idx = i;
		setclientflags(c);
		setclientfields(c);
		if (c->swallowing) {
			c->swallowing->idx = i;
			setclientflags(c->swallowing);
			setclientfields(c->swallowing);
		}
	}

	XSync(dpy, False);
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
	char atom[22];
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

		mon = settings >> 2;
		for (m = mons; m && m->num != mon; m = m->next);
		if (m) {
			ws->mon = m;
			ws->visible = settings & 1;
			ws->pinned = (settings & 0x2) >> 1;

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