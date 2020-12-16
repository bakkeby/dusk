void
persistmonitorstate(Monitor *m)
{
	Client *c;
	Workspace *ws = MWS(m);
	unsigned int i;
	char atom[22];

	sprintf(atom, "_DAWN_MONITOR_TAGS_%u", m->num); // TODO workspaces

	unsigned long data[] = { ws->tags };
	XChangeProperty(dpy, root, XInternAtom(dpy, atom, False), XA_CARDINAL, 32, PropModeReplace, (unsigned char *)data, 1);

	/* set dawn client atoms */
	for (ws = m->workspaces; ws; ws = ws->next)
		for (i = 1, c = ws->clients; c; c = c->next, ++i) {
			c->id = i;
			setclientflags(c);
			setclienttags(c);
		}

	XSync(dpy, False);
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
	int i;
	XTextProperty text;
	char *tags[NUMTAGS];
	for (i = 0; i < NUMTAGS; i++)
		tags[i] = tagicon(selmon, i);
	Xutf8TextListToTextProperty(dpy, tags, NUMTAGS, XUTF8StringStyle, &text);
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
	XChangeProperty(dpy, c->win, clientatom[DawnClientFlags], XA_CARDINAL, 32, PropModeReplace, (unsigned char *)data, 1);
}

void
setclienttags(Client *c)
{
	// TODO each workspace has an index (or num), then used to remember tags through restart
	// unsigned long data[] = { c->ws->num | (c->id << 4) | (c->tags << 12)};
	// XChangeProperty(dpy, c->win, clientatom[DawnClientTags], XA_CARDINAL, 32, PropModeReplace, (unsigned char *)data, 1);
}

void
getclientflags(Client *c)
{
	Atom flags = getatomprop(c, clientatom[DawnClientFlags], AnyPropertyType);
	if (flags)
		c->flags |= flags;
}

void
getclienttags(Client *c)
{
	// TODO each workspace has an index (or num), then used to remember tags through restart
	// Monitor *m;
	// Atom clienttags = getatomprop(c, clientatom[DawnClientTags], AnyPropertyType);
	// if (clienttags) {
	// 	c->tags = (clienttags >> 12) & TAGMASK;
	// 	c->id = (clienttags & 0xFF0) >> 4;
	// 	for (m = mons; m; m = m->next)
	// 		if (m->num == (clienttags & 0xF)) {
	// 			c->ws = m;
	// 			break;
	// 		}
	// }
}

void
getmonitorstate(Monitor *m)
{
	char atom[22];
	int di;
	unsigned long dl;
	unsigned char *p = NULL;
	Atom da, tags = None;

	sprintf(atom, "_DAWN_MONITOR_TAGS_%u", m->num); // TODO workspaces

	Atom monitortags = XInternAtom(dpy, atom, True);
	if (!monitortags)
		return;

	if (XGetWindowProperty(dpy, root, monitortags, 0L, sizeof tags, False, AnyPropertyType,
		&da, &di, &dl, &dl, &p) == Success && p) {
		tags = *(Atom *)p;
		XFree(p);
	}

	if (tags)
		m->selws->tags = tags; // tmp workspaces
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