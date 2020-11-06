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
setdawnclientflags(Client *c)
{
	unsigned long data[] = { c->flags };
	XChangeProperty(dpy, c->win, clientatom[DawnClientFlags], XA_CARDINAL, 32, PropModeReplace, (unsigned char *)data, 1);
}

void
setdawnmonitortags(Client *c)
{
	unsigned long data[] = { c->mon->index | (c->id << 4) | (c->tags << 12)};
	XChangeProperty(dpy, c->win, clientatom[DawnMonitorTags], XA_CARDINAL, 32, PropModeReplace, (unsigned char *)data, 1);
}

void
getdawnclientflags(Client *c)
{
	Atom flags = getatomprop(c, clientatom[DawnClientFlags]);
	if (flags)
		c->flags |= flags;
}

void
getdawnmonitortags(Client *c)
{
	Monitor *m;
	Atom monitortags = getatomprop(c, clientatom[DawnMonitorTags]);
	if (monitortags) {
		c->tags = (monitortags >> 12);
		c->id = (monitortags & 0xFF0) >> 4;
		for (m = mons; m; m = m->next)
			if (m->index == (monitortags & 0xF)) {
				c->mon = m;
				break;
			}
	}
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
	long rawdata[] = { selmon->tagset[selmon->seltags] };
	int i = 0;
	while (*rawdata >> (i + 1)) {
		i++;
	}
	long data[] = { i };
	XChangeProperty(dpy, root, netatom[NetCurrentDesktop], XA_CARDINAL, 32, PropModeReplace, (unsigned char *)data, 1);
}