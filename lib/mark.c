static int num_marked = 0;

Client *
nextmarked(Client *prev, Client *def)
{
	fprintf(stderr, "nextmarked: received previous client %s\n", prev ? prev->name : "NULL");
	if (!num_marked) {
		fprintf(stderr, "nextmarked: returning default client %s because num_marked = %d\n", def ? def->name : "NULL", num_marked);
		return def;
	}

	Client *c = NULL;
	Workspace *ws;

	for (ws = (prev ? prev->ws : workspaces); ws && !c; ws = ws->next)
		for (c = (prev ? prev : ws->clients); c && !ISMARKED(c); c = c->next);

	if (c && ISMARKED(c))
		unmark(&((Arg) { .v = c }));
	fprintf(stderr, "nextmarked: returning client %s, num_marked = %d\n", c ? c->name : "NULL", num_marked);
	return c;
}

void
mark(const Arg *arg)
{
	Client *c = CLIENT;
	if (!c)
		return;
	addflag(c, Marked);
	++num_marked;
	drawbar(c->ws->mon);
	fprintf(stderr, "mark: marked client %s, num_marked = %d\n", c->name, num_marked);
}

void
markmouse(const Arg *arg)
{
	Client *r = NULL;
	Workspace *w;
	XEvent ev;
	Time lasttime = 0;

	if (XGrabPointer(dpy, root, False, MOUSEMASK, GrabModeAsync, GrabModeAsync,
		None, cursor[CurMove]->cursor, CurrentTime) != GrabSuccess)
		return;

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

			r = recttoclient(ev.xmotion.x, ev.xmotion.y, 1, 1, 0);

			if (r && !ISMARKED(r))
				mark(&((Arg) { .v = r }));

			break;
		}
	} while (ev.type != ButtonRelease);
	XUngrabPointer(dpy, CurrentTime);
}

void
unmark(const Arg *arg)
{
	Client *c = CLIENT;
	if (!c)
		return;
	removeflag(c, Marked);
	--num_marked;
	drawbar(c->ws->mon);
	fprintf(stderr, "unmark: unmarked client %s, num_marked = %d\n", c->name, num_marked);
}

void
togglemark(const Arg *arg)
{
	if (ISMARKED(CLIENT))
		unmark(arg);
	else
		mark(arg);
}

void
unmarkall(const Arg *arg)
{
	for (Client *c = nextmarked(NULL, NULL); c; c = nextmarked(c->next, NULL));
}