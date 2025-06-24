Atom utf8string;

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
	Client *c, *s;
	unsigned int i;

	/* Fill flextile attributes if arrange method is NULL (floating layout) */
	if (!ws->layout->arrange) {
		for (i = 0; i < LTAXIS_LAST; i++)
			ws->ltaxis[i] = 0xF;
	} else if (enabled(SmartLayoutConversion) && ws->orientation != 0) {
		/* If smart layout conversion functionality is enabled then convert vertically
		 * oriented workspaces into horizontal before persisting. This because the orientation
		 * is not persisted and the layouts will be subject conversion when the workspace
		 * is assigned to a vertical monitor. */
		layoutconvert(&((Arg) { .v = ws }));
	}

	/* Perists workspace information in 32 bits laid out like this:
	 *
	 * |1|0|00000|00000|00001|0001|000|000|001|0|1
	 * | | |     |     |     |    |   |   |   | |-- ws->visible
	 * | | |     |     |     |    |   |   |   |-- ws->pinned
	 * | | |     |     |     |    |   |   |-- ws->nmaster
	 * | | |     |     |     |    |   |-- ws->nstack
	 * | | |     |     |     |    |-- ws->mon
	 * | | |     |     |     |-- ws->ltaxis[LAYOUT] (i.e. split)
	 * | | |     |     |-- ws->ltaxis[MASTER]
	 * | | |     |-- ws->ltaxis[STACK]
	 * | | |-- ws->ltaxis[STACK2]
	 * | |-- mirror layout (indicated by negative ws->ltaxis[LAYOUT])
	 * |-- ws->enablegaps
	 */
	uint32_t data[] = {
		(ws->visible & 0x1) |
		(ws->pinned & 0x1) << 1 |
		(ws->nmaster & 0x7) << 2 |
		(ws->nstack & 0x7 ) << 5 |
		(ws->mon->num & 0x7) << 8 |
		(abs(ws->ltaxis[LAYOUT]) & 0xF) << 11 |
		(ws->ltaxis[MASTER] & 0x1F) << 15 |
		(ws->ltaxis[STACK] & 0x1F) << 20 |
		(ws->ltaxis[STACK2] & 0x1F) << 25 |
		(ws->ltaxis[LAYOUT] < 0 ? 1 : 0) << 30 |
		(ws->enablegaps & 0x1) << 31
	};

	XChangeProperty(dpy, root, duskatom[DuskWorkspace], XA_CARDINAL, 32,
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
		setclientalttitle(c);
		setclienticonpath(c);
		savewindowfloatposition(c, c->ws->mon);

		s = c->swallowing;
		while (s) {
			s->idx = i;
			setclientflags(s);
			setclientfields(s);
			setclientlabel(s);
			setclientalttitle(s);
			setclienticonpath(s);
			savewindowfloatposition(s, s->ws->mon);
			s = s->swallowing;
		}
	}

	XSync(dpy, False);
}

void
restoreworkspacestates(void)
{
	Workspace *ws;
	for (ws = workspaces; ws; ws = ws->next)
		restoreworkspacestate(ws);
}

void
restoreworkspacestate(Workspace *ws)
{
	const Layout *layout;
	int i, di, mon, num_ws = 0;
	unsigned long dl, nitems;
	unsigned char *p = NULL;
	Atom da, settings = None;

	if (XGetWindowProperty(dpy, root, netatom[NetNumberOfDesktops], 0L, sizeof da,
			False, AnyPropertyType, &da, &di, &nitems, &dl, &p) == Success && p) {
		num_ws = *(Atom *)p;
		XFree(p);
	}

	if (ws->num > num_ws)
		return;

	if (!(XGetWindowProperty(dpy, root, duskatom[DuskWorkspace], ws->num, num_ws * sizeof dl,
			False, AnyPropertyType, &da, &di, &nitems, &dl, &p) == Success && p)) {
		return;
	}

	/* If the root window has the _DUSK_WORKSPACES property, which is confirmed by the above if
	 * statement, then we do not want to trigger autostart of applications. This is only to happen
	 * during the initial startup and not as part of restarts. The autostart_startup variable is
	 * defined in lib/autostart.c */
	autostart_startup = 0;

	if (nitems) {
		settings = *(Atom *)p;

		/* See bit layout in the persistworkspacestate function */
		mon = (settings >> 8) & 0x7;
		ws->rule_monitor = mon;
		ws->visible = settings & 0x1;
		ws->rule_pinned = (settings >> 1) & 0x1;
		ws->nmaster = (settings >> 2) & 0x7;
		ws->nstack = (settings >> 5) & 0x7;
		ws->ltaxis[LAYOUT] = (settings >> 11) & 0xF;
		if (settings & (1 << 30)) // mirror layout
			ws->ltaxis[LAYOUT] *= -1;
		ws->ltaxis[MASTER] = WRAP((settings >> 15) & 0x1F, 0, AXIS_LAST - 1);
		ws->ltaxis[STACK]  = WRAP((settings >> 20) & 0x1F, 0, AXIS_LAST - 1);
		ws->ltaxis[STACK2] = WRAP((settings >> 25) & 0x1F, 0, AXIS_LAST - 1);
		ws->enablegaps = (settings >> 31) & 0x1;

		/* Restore layout if we have an exact match, floating layout interpreted as 0x7fff800 */
		for (i = 0; i < LENGTH(layouts); i++) {
			layout = &layouts[i];
			if ((layout->arrange == flextile
				&& ws->ltaxis[LAYOUT] == layout->preset.layout
				&& ws->ltaxis[MASTER] == layout->preset.masteraxis
				&& ws->ltaxis[STACK]  == layout->preset.stack1axis
				&& ws->ltaxis[STACK2] == layout->preset.stack2axis)
				|| ((settings & 0x7fff800) == 0x7fff800
				&& layout->arrange == NULL)
			) {
				ws->layout = layout;
				strlcpy(ws->ltsymbol, ws->layout->symbol, sizeof ws->ltsymbol);
				break;
			}
		}
	}

	XFree(p);
}

void
persistpids(void)
{
	unsigned int i, count = 0;

	for (i = 0; i < LENGTH(autostart_pids); i++) {
		if (autostart_pids[i] == 0)
			break;

		if (autostart_pids[i] == -1)
			continue;

		/* Append the PID to the root window property */
		long data[] = { autostart_pids[i] };
		XChangeProperty(dpy, root, duskatom[DuskAutostartPIDs], XA_CARDINAL, 32,
			count ? PropModeAppend : PropModeReplace, (unsigned char *)data, 1);

		count++;
	}

	/* Record the count of PID properties. */
	long data[] = { count };
	XChangeProperty(dpy, root, duskatom[DuskAutostartCount], XA_CARDINAL, 32,
			PropModeReplace, (unsigned char *)data, 1);

	XSync(dpy, False);
}

void
restorepids(void)
{
	int di, count = 0;
	unsigned int i;
	unsigned long dl, nitems;
	unsigned char *p = NULL;
	Atom da, *pids;

	/* Get the count of PIDs (if any). */
	if (XGetWindowProperty(dpy, root, duskatom[DuskAutostartCount], 0L, sizeof da,
			False, AnyPropertyType, &da, &di, &nitems, &dl, &p) == Success && p) {
		count = *(Atom *)p;
		XFree(p);
	}

	if (!count)
		return;

	if (XGetWindowProperty(dpy, root, duskatom[DuskAutostartPIDs], 0L, count * sizeof da,
			False, AnyPropertyType, &da, &di, &nitems, &dl, &p) == Success && p) {
		pids = (Atom *)p;
		for (i = 0; i < nitems; i++) {
			autostart_addpid(pids[i]);
		}
		XFree(p);
	}

	XDeleteProperty(dpy, root, duskatom[DuskAutostartCount]);
	XDeleteProperty(dpy, root, duskatom[DuskAutostartPIDs]);
}

void
savewindowfloatposition(Client *c, Monitor *m)
{
	char atom[22] = {0};
	if (c->sfx == -9999)
		return;

	snprintf(atom, sizeof atom, "_DUSK_FLOATPOS_%u", m->num);
	uint32_t pos[] = { (MAX(c->sfx - m->mx, 0) & 0xffff) | ((MAX(c->sfy - m->my, 0) & 0xffff) << 16) };
	XChangeProperty(dpy, c->win, XInternAtom(dpy, atom, False), XA_CARDINAL, 32, PropModeReplace, (unsigned char *)pos, 1);

	snprintf(atom, sizeof atom, "_DUSK_FLOATSIZE_%u", m->num);
	uint32_t size[] = { (c->sfw & 0xffff) | ((c->sfh & 0xffff) << 16) };
	XChangeProperty(dpy, c->win, XInternAtom(dpy, atom, False), XA_CARDINAL, 32, PropModeReplace, (unsigned char *)size, 1);

	XSync(dpy, False);
}

int
restorewindowfloatposition(Client *c, Monitor *m)
{
	char atom[22] = {0};
	Atom key, value;
	int x, y, w, h;

	snprintf(atom, sizeof atom, "_DUSK_FLOATPOS_%u", m->num);

	key = XInternAtom(dpy, atom, False);
	if (!key)
		return 0;

	value = getatomprop(c, key, AnyPropertyType);
	if (!value)
		return 0;

	x = value & 0xffff;
	y = value >> 16;

	snprintf(atom, sizeof atom, "_DUSK_FLOATSIZE_%u", m->num);

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

/* Sets WM_STATE, which is a basic window manager hint part of the older ICCCM specification */
void
setclientstate(Client *c, long state)
{
	long data[] = { state, None };

	XChangeProperty(dpy, c->win, wmatom[WMState], wmatom[WMState], 32,
		PropModeReplace, (unsigned char *)data, 2);
}

/* Sets _NET_WM_STATE, which is an extended window manager hint part of the EWMH specification */
void
setclientnetstate(Client *c, int state)
{
	if (!state) {
		/* Clear property if we have no state */
		XChangeProperty(dpy, c->win, netatom[NetWMState], XA_ATOM, 32,
			PropModeReplace, (unsigned char*)0, 0);
		return;
	}

	XChangeProperty(dpy, c->win, netatom[NetWMState], XA_ATOM, 32,
		PropModeReplace, (unsigned char*)&netatom[state], 1);
}

void
setdesktopnames(void)
{
	int i;
	Workspace *ws;
	XTextProperty text;

	char *wslist[num_workspaces];
	for (i = 0, ws = workspaces; ws && i < num_workspaces; ws = ws->next) {
		if (ws == stickyws)
			continue;
		wslist[i] = wsicon(ws);
		++i;
	}

	Xutf8TextListToTextProperty(dpy, wslist, num_workspaces, XUTF8StringStyle, &text);
	XSetTextProperty(dpy, root, &text, netatom[NetDesktopNames]);
	XFree(text.value);
}

void
setfloatinghint(Client *c)
{
	unsigned int floating[1] = {FREEFLOW(c) ? 1 : 0};
	XChangeProperty(dpy, c->win, duskatom[IsFloating], XA_CARDINAL, 32, PropModeReplace, (unsigned char *)floating, 1);
}

void
setfloatinghints(Workspace *ws)
{
	Client *c;
	for (c = ws->clients; c; c = c->next)
		setfloatinghint(c);
}

void
setclientflags(Client *c)
{
	uint32_t data1[] = { c->flags & 0xFFFFFFFF };
	uint32_t data2[] = { c->flags >> 32 };
	XChangeProperty(dpy, c->win, duskatom[DuskClientFlags], XA_CARDINAL, 32, PropModeReplace, (unsigned char *)data1, 1);
	XChangeProperty(dpy, c->win, duskatom[DuskClientFlags], XA_CARDINAL, 32, PropModeAppend,  (unsigned char *)data2, 1);
}

void
setclientfields(Client *c)
{
	uint32_t data[] = { c->ws->num | (c->idx << 6) | (c->scratchkey << 14)};
	XChangeProperty(dpy, c->win, duskatom[DuskClientFields], XA_CARDINAL, 32, PropModeReplace, (unsigned char *)data, 1);
}

void
setclienticonpath(Client *c)
{
	if (!strlen(c->iconpath))
		return;

	XChangeProperty(dpy, c->win, duskatom[DuskClientIconPath], XA_STRING, 8, PropModeReplace, (unsigned char *)c->iconpath, strlen(c->iconpath));
}

void
setclientlabel(Client *c)
{
	XChangeProperty(dpy, c->win, duskatom[DuskClientLabel], XA_STRING, 8, PropModeReplace, (unsigned char *)c->label, strlen(c->label));
}

void
setclientalttitle(Client *c)
{
	XChangeProperty(dpy, c->win, duskatom[DuskClientAltName], utf8string, 8, PropModeReplace, (unsigned char *)c->altname, strlen(c->altname));
}

void
getclientflags(Client *c)
{
	int di;
	unsigned long dl, nitems;
	uint64_t flags1 = 0, flags2 = 0;
	unsigned char *p = NULL;
	Atom da = None;
	Atom *cflags;

	if (XGetWindowProperty(dpy, c->win, duskatom[DuskClientFlags], 0L, 2 * sizeof flags1, False,
			AnyPropertyType, &da, &di, &nitems, &dl, &p) == Success && p) {
		cflags = (Atom *)p;
		if (nitems == 2) {
			flags1 = cflags[0] & 0xFFFFFFFF;
			flags2 = cflags[1] & 0xFFFFFFFF;
		}
		XFree(p);
	}

	if (flags1 || flags2) {
		c->flags = flags1 | (flags2 << 32);
		/* Remove flags that should not survive a restart */
		removeflag(c, Marked|Centered|SwitchWorkspace|EnableWorkspace|RevertWorkspace|Locked);
	}
}

void
getclientfields(Client *c)
{
	Workspace *ws;
	Atom fields = getatomprop(c, duskatom[DuskClientFields], AnyPropertyType);
	if (fields) {
		c->scratchkey = (fields >> 14);
		c->idx = (fields & 0x3FC0) >> 6;
		for (ws = workspaces; ws; ws = ws->next) {
			if (ws->num == (fields & 0x3F)) {
				c->ws = ws;
				break;
			}
		}
	}
}

void
getclienticonpath(Client *c)
{
	Atom type;
	int format;
	unsigned int i;
	unsigned long after;
	unsigned char *data = 0;
	long unsigned int size = LENGTH(c->iconpath);

	if (XGetWindowProperty(dpy, c->win, duskatom[DuskClientIconPath], 0, 1024, 0, XA_STRING,
				&type, &format, &size, &after, &data) == Success) {
		if (data) {
			if (type == XA_STRING) {
				for (i = 0; i < size; ++i)
					c->iconpath[i] = data[i];
			}
			XFree(data);
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

	if (XGetWindowProperty(dpy, c->win, duskatom[DuskClientLabel], 0, 1024, 0, XA_STRING,
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
getclientalttitle(Client *c)
{
	Atom type;
	int format;
	unsigned int i;
	unsigned long after;
	unsigned char *data = 0;
	long unsigned int size = LENGTH(c->altname);

	if (XGetWindowProperty(dpy, c->win, duskatom[DuskClientAltName], 0, 1024, 0, utf8string,
				&type, &format, &size, &after, &data) == Success) {
		if (data) {
			if (type == utf8string) {
				for (i = 0; i < size; ++i)
					c->altname[i] = data[i];
			}
			XFree(data);
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
