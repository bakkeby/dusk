void
hide(Client *c) {

	Client *n;
	if (!c || HIDDEN(c))
		return;

	Window w = c->win;
	static XWindowAttributes ra, ca;

	// more or less taken directly from blackbox's hide() function
	XGrabServer(dpy);
	XGetWindowAttributes(dpy, root, &ra);
	XGetWindowAttributes(dpy, w, &ca);
	// prevent UnmapNotify events
	XSelectInput(dpy, root, ra.your_event_mask & ~SubstructureNotifyMask);
	XSelectInput(dpy, w, ca.your_event_mask & ~StructureNotifyMask);
	XUnmapWindow(dpy, w);
	setclientstate(c, IconicState);
	XSelectInput(dpy, root, ra.your_event_mask);
	XSelectInput(dpy, w, ca.your_event_mask);
	XUngrabServer(dpy);

	if (ISFLOATING(c) || !c->ws->layout->arrange) {
		for (n = c->snext; n && (!ISVISIBLE(n) || HIDDEN(n)); n = n->snext);
		if (!n)
			for (n = c->ws->stack; n && (!ISVISIBLE(n) || HIDDEN(n)); n = n->snext);
	} else {
		n = nexttiled(c);
		if (!n)
			n = prevvisible(c);
	}
	focus(n);
	arrange(c->ws);
}

void
show(Client *c)
{
	if (!c || !HIDDEN(c))
		return;

	XMapWindow(dpy, c->win);
	setclientstate(c, NormalState);
	arrange(c->ws);
}

void
focuswin(const Arg *arg)
{
	Client *c = (Client*)arg->v;
	if (!c)
		return;

	focus(c);
}

Client *
prevvisible(Client *c)
{
	Client *p, *i;
	for (p = NULL, i = c->ws->clients; c && i != c; i = i->next)
		if (ISVISIBLE(i) && !HIDDEN(i))
			p = i;
	return p;
}

void
showhideclient(const Arg *arg)
{
	Client *c = (Client*)arg->v;
	if (!c)
		c = WS->sel;
	if (!c)
		return;

	force_warp = 1;
	if (HIDDEN(c)) {
		show(c);
		focus(c);
		restack(c->ws);
	} else {
		hide(c);
	}
}