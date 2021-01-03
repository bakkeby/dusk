void
persistworkspacestate(Workspace *ws)
{
	fprintf(stderr, "persistworkspacestate: --> ws %s\n", ws->name);
	Client *c;
	unsigned int i;
	char atom[22];

	sprintf(atom, "_DUSK_WORKSPACE_%u", ws->num);
	fprintf(stderr, "persistworkspacestate: ws = %s, pinned = %d, visible = %d, mon = %d\n", ws->name, ws->pinned, ws->visible, ws->mon->num);
	unsigned long data[] = { ws->visible | ws->pinned << 1 | ws->mon->num << 2 }; // potentially enablegaps, nmaster, nstack, mfact
	fprintf(stderr, "persistworkspacestate: data = %lu\n", *data);
	XChangeProperty(dpy, root, XInternAtom(dpy, atom, False), XA_CARDINAL, 32, PropModeReplace, (unsigned char *)data, 1);

	/* set dusk client atoms */
	for (i = 1, c = ws->clients; c; c = c->next, ++i) {
		c->id = i;
		setclientflags(c);
		setclientfields(c);
	}

	XSync(dpy, False);
	fprintf(stderr, "persistworkspacestate: <--\n");
}

void
setcurrentdesktop(void)
{
	long data[] = { 0 };
	XChangeProperty(dpy, root, netatom[NetCurrentDesktop], XA_CARDINAL, 32, PropModeReplace, (unsigned char *)data, 1);
}

void
setdesktopnames(void)
{
	int i, num_workspaces;
	Workspace *ws;
	XTextProperty text;
	for (num_workspaces = 0, ws = workspaces; ws; ws = ws->next, ++num_workspaces);

	char *wslist[num_workspaces];
	for (i = 0, ws = workspaces; ws; ws = ws->next, ++i) {
		wslist[i] = wsicon(ws);
	}
	Xutf8TextListToTextProperty(dpy, wslist, NUMTAGS, XUTF8StringStyle, &text);
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
	unsigned long data[] = { c->flags };
	XChangeProperty(dpy, c->win, clientatom[DuskClientFlags], XA_CARDINAL, 32, PropModeReplace, (unsigned char *)data, 1);
}

void
setclientfields(Client *c)
{
	unsigned long data[] = { c->ws->num | (c->id << 6) | (c->scratchkey << 14)};
	XChangeProperty(dpy, c->win, clientatom[DuskClientTags], XA_CARDINAL, 32, PropModeReplace, (unsigned char *)data, 1);
}

void
getclientflags(Client *c)
{
	Atom flags = getatomprop(c, clientatom[DuskClientFlags], AnyPropertyType);
	if (flags)
		c->flags = flags;
}

void
getclientfields(Client *c)
{
	fprintf(stderr, "getclientfields: -->\n");
	Workspace *ws;
	Atom fields = getatomprop(c, clientatom[DuskClientTags], AnyPropertyType);
	if (fields) {
		c->scratchkey = (fields >> 14);
		c->id = (fields & 0x3FC0) >> 6;
		for (ws = workspaces; ws; ws = ws->next)
			if (ws->num == (fields & 0x3F)) {
				c->ws = ws;
				break;
			}
	}
	fprintf(stderr, "getclientfields: <--\n");
}

void
getmonitorstate(Monitor *m)
{
	// TODO the only thing that might be worth remembering here are gapps, borderpx and showbar
	//      if these are fine to be "reset" as part of a restart then we can get rid of this func


	// char atom[22];
	// int di;
	// unsigned long dl;
	// unsigned char *p = NULL;
	// Atom da, tags = None;
	// Workspace *ws = MWS(m);

	// sprintf(atom, "_DUSK_MONITOR_TAGS_%u", m->num); // TODO workspaces

	// Atom monitortags = XInternAtom(dpy, atom, True);
	// if (!monitortags)
	// 	return;

	// if (XGetWindowProperty(dpy, root, monitortags, 0L, sizeof tags, False, AnyPropertyType,
	// 	&da, &di, &dl, &dl, &p) == Success && p) {
	// 	tags = *(Atom *)p;
	// 	XFree(p);
	// }

	// if (tags)
	// 	ws->tags = tags; // tmp workspaces
}

void
getworkspacestate(Workspace *ws)
{
	fprintf(stderr, "getworkspacestate: --> %s\n", ws->name);
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
			fprintf(stderr, "getworkspacestate: settings = %ld visible = %ld\n", settings, settings & 1);
			ws->visible = settings & 1;
			ws->pinned = (settings & 0x2) >> 1;

			fprintf(stderr, "getworkspacestate: found monitor %d for workspace %s, visible = %d, pinned = %d (settings = %ld)\n", m->num, ws->name, ws->visible, ws->pinned, settings);
			if (ws->visible)
				ws->mon->selws = ws; // TODO refactor, we so something similar in createworkspaces
		}
	}
	fprintf(stderr, "getworkspacestate: <--\n");
}

void
setnumdesktops(void)
{
	long data[] = { NUMTAGS };
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

	long rawdata[] = { WS->tags }; // TODO workspaces
	int i = 0;
	while (*rawdata >> (i + 1)) {
		i++;
	}
	long data[] = { i };
	XChangeProperty(dpy, root, netatom[NetCurrentDesktop], XA_CARDINAL, 32, PropModeReplace, (unsigned char *)data, 1);
}