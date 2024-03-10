static unsigned long systrayorientation = _NET_SYSTEM_TRAY_ORIENTATION_HORZ;

int
size_systray(Bar *bar, BarArg *a)
{
	unsigned int w = 0;
	Client *i;
	if (!systray)
		return 0;

	for (i = systray->icons; i; w += i->w + systrayspacing, i = i->next);
	if (!w)
		XMoveWindow(dpy, systray->win, -systray->h, bar->by);
	systray->bar = bar;

	return w ? w - systrayspacing : 0;
}

int
draw_systray(Bar *bar, BarArg *a)
{
	int bx, by, bw, bh;

	if (!systray)
		return 0;

	if (!bar) {
		XMoveWindow(dpy, systray->win, -systray->h, bar->by);
		return 0;
	}

	XSetWindowAttributes wa;
	XWindowChanges wc;
	Client *i, *next;
	unsigned int w;

	drw_setscheme(drw, scheme[a->scheme]);
	drw_rect(drw, a->x, a->y, a->w, a->h, 1, 1);
	wa.background_pixel = enabled(SystrayNoAlpha) ? scheme[a->scheme][ColBg].pixel : 0;

	if (!systray && !initsystray())
		return 0;

	wc.stack_mode = Above;
	wc.sibling = bar->win;
	XConfigureWindow(dpy, systray->win, CWSibling|CWStackMode, &wc);
	if (enabled(SystrayNoAlpha)) {
		XSetWindowBackground(dpy, systray->win, scheme[a->scheme][ColBg].pixel);
		XClearWindow(dpy, systray->win);
	}

	drw_setscheme(drw, scheme[a->scheme]);
	for (w = 0, i = systray->icons; i; i = next) {
		next = i->next;
		if (ISUNMANAGED(i)) {
			removesystrayicon(i);
			continue;
		}

		XChangeWindowAttributes(dpy, i->win, CWBackPixel, &wa);
		XMapRaised(dpy, i->win);
		i->x = w;
		XMoveResizeWindow(dpy, i->win, i->x, 0, i->w, i->h);
		w += i->w;
		if (i->next)
			w += systrayspacing;
		if (i->ws->mon != bar->mon && bar->mon->selws)
			i->ws = bar->mon->selws;
	}

	bx = bar->bx + a->x + a->lpad;
	by = (w ? bar->by + a->y + (a->h - systray->h) / 2 : -systray->h);
	bw = MAX(w, 1);
	bh = systray->h;

	XMoveResizeWindow(dpy, systray->win, bx, by, bw, bh);
	return w;
}

int
click_systray(Bar *bar, Arg *arg, BarArg *a)
{
	return -1;
}

int
initsystray(void)
{
	XSetWindowAttributes wa;

	if (disabled(Systray))
		return 0;

	if (!(systray = (SystrayWin *)calloc(1, sizeof(SystrayWin))))
		die("fatal: could not malloc() %u bytes\n", sizeof(SystrayWin));

	wa.override_redirect = True;
	wa.event_mask = ButtonPressMask|ExposureMask;
	wa.border_pixel = 0;
	wa.background_pixel = 0;
	systray->h = drw->fonts->h;

	if (!enabled(SystrayNoAlpha)) {
		wa.colormap = cmap;
		systray->win = XCreateWindow(dpy, root, -500, -500, systray->h, systray->h, 0, depth,
			InputOutput, visual,
			CWOverrideRedirect|CWBorderPixel|CWBackPixel|CWColormap|CWEventMask,&wa);
	} else {
		systray->win = XCreateSimpleWindow(dpy, root, -500, -500, systray->h, systray->h, 0, 0, 0);
		XChangeWindowAttributes(dpy, systray->win, CWOverrideRedirect|CWBackPixel|CWBorderPixel|CWEventMask, &wa);
	}

	XSelectInput(dpy, systray->win, SubstructureNotifyMask);
	XChangeProperty(dpy, systray->win, netatom[NetSystemTrayOrientation], XA_CARDINAL, 32,
			PropModeReplace, (unsigned char *)&systrayorientation, 1);

	if (!enabled(SystrayNoAlpha)) {
		XChangeProperty(dpy, systray->win, netatom[NetSystemTrayVisual], XA_VISUALID, 32,
				PropModeReplace, (unsigned char *)&visual->visualid, 1);
	}

	XChangeProperty(dpy, systray->win, netatom[NetWMWindowType], XA_ATOM, 32,
			PropModeReplace, (unsigned char *)&netatom[NetWMWindowTypeDock], 1);
	XMapRaised(dpy, systray->win);
	XSetSelectionOwner(dpy, netatom[NetSystemTray], systray->win, CurrentTime);
	if (XGetSelectionOwner(dpy, netatom[NetSystemTray]) != systray->win) {
		fprintf(stderr, "dusk: unable to obtain system tray.\n");
		free(systray);
		systray = NULL;
		return 0;
	}

	sendevent(root, xatom[Manager], StructureNotifyMask, CurrentTime, netatom[NetSystemTray], systray->win, 0, 0);
	XSync(dpy, False);
	return 1;
}

void
addsystrayicon(XClientMessageEvent *cme)
{
	Client *c;
	XSetWindowAttributes swa;
	XWindowAttributes wa;

	if (!systray)
		return;

	if (cme->data.l[1] != SYSTEM_TRAY_REQUEST_DOCK)
		return;

	if (!(c = (Client *)calloc(1, sizeof(Client))))
		die("fatal: could not malloc() %u bytes\n", sizeof(Client));

	if (!(c->win = cme->data.l[2])) {
		free(c);
		return;
	}

	c->ws = selws;
	c->next = systray->icons;
	systray->icons = c;
	XGetWindowAttributes(dpy, c->win, &wa);
	c->x = c->oldx = 0;
	c->y = c->oldy = 0;
	c->w = c->oldw = wa.width;
	c->h = c->oldh = wa.height;
	c->oldbw = wa.border_width;
	c->bw = 0;
	SETFLOATING(c);
	updatesizehints(c);
	updatetitle(c);
	updatesystrayicongeom(c, wa.width, wa.height);
	XAddToSaveSet(dpy, c->win);
	XSelectInput(dpy, c->win, StructureNotifyMask | PropertyChangeMask | ResizeRedirectMask);
	XClassHint ch = {"systray", "systray"};
	if (enabled(Debug))
		fprintf(stderr, "clientmessage: received systray request dock for window %ld (%s)\n", c->win, c->name);
	XSetClassHint(dpy, c->win, &ch);
	XReparentWindow(dpy, c->win, systray->win, 0, 0);
	/* use parents background color */
	swa.background_pixel = scheme[SchemeNorm][ColBg].pixel;
	XChangeWindowAttributes(dpy, c->win, CWBackPixel, &swa);
	sendevent(c->win, netatom[Xembed], StructureNotifyMask, CurrentTime, XEMBED_EMBEDDED_NOTIFY, 0 , systray->win, XEMBED_EMBEDDED_VERSION);
	XSync(dpy, False);
	setclientstate(c, NormalState);
}

void
removesystrayicon(Client *i)
{
	Client **ii;

	if (!i)
		return;
	for (ii = &systray->icons; *ii && *ii != i; ii = &(*ii)->next);
	if (ii)
		*ii = i->next;
	XReparentWindow(dpy, i->win, root, 0, 0);
	free(i);
}

void
resizerequest(XEvent *e)
{
	XResizeRequestEvent *ev = &e->xresizerequest;
	Client *i;

	if ((i = wintosystrayicon(ev->window))) {
		updatesystrayicongeom(i, ev->width, ev->height);
		drawbarwin(systray->bar);
	}
}

void
updatesystrayicongeom(Client *i, int w, int h)
{
	if (!systray)
		return;

	int bar_height = systray->h;
	if (i) {
		i->h = bar_height;
		if (w == h)
			i->w = bar_height;
		else if (h == bar_height)
			i->w = w;
		else
			i->w = (int) ((float)bar_height * ((float)w / (float)h));
		applysizehints(i, &(i->x), &(i->y), &(i->w), &(i->h), False);
		/* force icons into the systray dimensions if they don't want to */
		if (i->h > bar_height) {
			if (i->w == i->h)
				i->w = bar_height;
			else
				i->w = (int) ((float)bar_height * ((float)i->w / (float)i->h));
			i->h = bar_height;
		}
		if (i->w > 2 * bar_height)
			i->w = bar_height;
	}
}

void
updatesystrayiconstate(Client *i, XPropertyEvent *ev)
{
	long flags;
	int code = 0;

	if (enabled(Debug) || DEBUGGING(i))
		fprintf(stderr, "updatesystrayiconstate: ev->atom = %ld (%s), xatom[XembedInfo] = %ld\n", ev->atom, XGetAtomName(dpy, ev->atom), xatom[XembedInfo]);

	if (!systray || !i || ev->atom != xatom[XembedInfo] ||
			!(flags = getatomprop(i, xatom[XembedInfo], xatom[XembedInfo])))
		return;

	if (flags & XEMBED_MAPPED && !i->ws) {
		i->ws = selws;
		code = XEMBED_WINDOW_ACTIVATE;
		XMapRaised(dpy, i->win);
		setclientstate(i, NormalState);
	}
	else if (!(flags & XEMBED_MAPPED) && i->ws) {
		i->ws = NULL;
		code = XEMBED_WINDOW_DEACTIVATE;
		XUnmapWindow(dpy, i->win);
		setclientstate(i, WithdrawnState);
	}
	else
		return;

	sendevent(i->win, xatom[Xembed], StructureNotifyMask, CurrentTime, code, 0,
			systray->win, XEMBED_EMBEDDED_VERSION);
}

Client *
wintosystrayicon(Window w)
{
	if (!systray)
		return NULL;
	Client *i = NULL;
	if (!w)
		return i;
	for (i = systray->icons; i && i->win != w; i = i->next);
	return i;
}
