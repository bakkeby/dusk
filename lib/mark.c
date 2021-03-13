Client *
nextmarked(Client *prev, Client *def)
{
	if (!num_marked || ignore_marked)
		return def;

	Client *c = NULL;
	Workspace *ws;

	for (ws = (prev ? prev->ws : workspaces); ws && !c; ws = ws->next)
		for (c = (prev ? prev : ws->clients); c && !ISMARKED(c); c = c->next, prev = NULL);

	if (c && ISMARKED(c))
		unmarkclient(c);
	return c;
}

void
mark(const Arg *arg)
{
	Client *c = CLIENT;
	if (!c || ISMARKED(c))
		return;
	markclient(c);
	drawbar(c->ws->mon);
}

void
markclient(Client *c)
{
	if (!ISMARKED(c)) {
		addflag(c, Marked);
		++num_marked;
	}
}

void
markall(const Arg *arg)
{
	Client *c;
	for (c = selws->clients; c; c = c->next) {
		if (ISMARKED(c) || ISINVISIBLE(c))
			continue;

		if ((arg->i == 2 && !HIDDEN(c)) || (arg->i != 2 && HIDDEN(c)))
			continue;

		if (arg->i == 1 && !ISFLOATING(c))
			continue;

		markclient(c);
	}
	drawbar(selws->mon);
}

void
markmouse(const Arg *arg)
{
	Client *r = selws->sel;
	Client *prevr = r;
	Workspace *w;
	XEvent ev;
	Time lasttime = 0;
	unsigned long mark = (arg->i == 1 ? Marked : arg->i);

	if (r && mark != ISMARKED(r))
		togglemark(&((Arg) { .v = r }));

	if (XGrabPointer(dpy, root, False, MOUSEMASK, GrabModeAsync, GrabModeAsync,
		None, cursor[CurMove]->cursor, CurrentTime) != GrabSuccess)
		return;

	readclientstackingorder();

	do {
		XMaskEvent(dpy, MOUSEMASK|ExposureMask|SubstructureRedirectMask, &ev);
		switch (ev.type) {
		case ConfigureRequest:
		case Expose:
		case MapRequest:
			handler[ev.type](&ev);
			break;
		case MotionNotify:
			if ((ev.xmotion.time - lasttime) <= (1000 / 60))
				continue;
			lasttime = ev.xmotion.time;

			if ((w = recttows(ev.xmotion.x, ev.xmotion.y, 1, 1)) && w != selws) {
				selws = w;
				selmon = w->mon;
			}

			r = recttoclient(ev.xmotion.x, ev.xmotion.y, 1, 1, 1);
			if (r != prevr && r && mark != ISMARKED(r))
				togglemark(&((Arg) { .v = r }));

			prevr = r;
			break;
		}
	} while (ev.type != ButtonRelease);
	XUngrabPointer(dpy, CurrentTime);
}

void
togglemark(const Arg *arg)
{
	Client *c = CLIENT;
	if (!c)
		return;
	if (ISMARKED(c))
		unmarkclient(c);
	else
		markclient(c);
	drawbar(c->ws->mon);
}

void
unmark(const Arg *arg)
{
	Client *c = CLIENT;
	if (!c || !ISMARKED(c))
		return;
	unmarkclient(c);
	drawbar(c->ws->mon);
}

void
unmarkclient(Client *c)
{
	if (ISMARKED(c)) {
		removeflag(c, Marked);
		--num_marked;
	}
}

void
unmarkall(const Arg *arg)
{
	for (Client *c = nextmarked(NULL, NULL); c; c = nextmarked(c->next, NULL));
	drawbars();
}