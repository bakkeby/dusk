void
changeopacity(const Arg *arg)
{
	Client *c = selws->sel;
	if (!c)
		return;

	for (c = nextmarked(NULL, c); c; c = nextmarked(c->next, NULL)) {
		c->opacity += (c->opacity == 0 ? 1.0 + arg->f : arg->f);

		if (c->opacity > 1.0)
			c->opacity = 1.0;

		if (c->opacity < 0)
			c->opacity = 0;

		opacity(c, c->opacity);
	}
}

void
opacity(Client *c, double opacity)
{
	if (opacity > 0 && opacity <= 1) {
		unsigned long real_opacity[] = { opacity * 0xffffffff };
		XChangeProperty(dpy, c->win, netatom[NetWMWindowOpacity], XA_CARDINAL,
				32, PropModeReplace, (unsigned char *)real_opacity,	1);
	} else
		XDeleteProperty(dpy, c->win, netatom[NetWMWindowOpacity]);
}

void
getclientopacity(Client *c)
{
	signed long atom = getatomprop(c, netatom[NetWMWindowOpacity], AnyPropertyType);
	if (atom < 0)
		atom = atom + 0xffffffff;

	if (atom == 0)
		c->opacity = defaultopacity;
	else
		c->opacity = (double)(atom) / 0xffffffff;
}