static Atom motifatom;

void
updatemotifhints(Client *c)
{
	Atom real;
	int format;
	unsigned char *p = NULL;
	unsigned long nitems, extra;
	unsigned long *motif;
	int width, height;

	if (disabled(DecorationHints) || IGNOREDECORATIONHINTS(c))
		return;

	if (XGetWindowProperty(dpy, c->win, motifatom, 0L, 5L, False, motifatom,
		                   &real, &format, &nitems, &extra, &p) == Success && p != NULL) {
		if (nitems == 0) {
			XFree(p);
			return;
		}

		motif = (unsigned long*)p;
		if (motif[MWM_HINTS_FLAGS_FIELD] & MWM_HINTS_DECORATIONS) {
			width = WIDTH(c);
			height = HEIGHT(c);

			if (motif[MWM_HINTS_DECORATIONS_FIELD] & MWM_DECOR_ALL ||
				motif[MWM_HINTS_DECORATIONS_FIELD] & MWM_DECOR_BORDER ||
				motif[MWM_HINTS_DECORATIONS_FIELD] & MWM_DECOR_TITLE)
				c->bw = c->oldbw = c->ws->mon->borderpx;
			else
				c->bw = c->oldbw = 0;

			resize(c, c->x, c->y, width - (2*c->bw), height - (2*c->bw), 0);
		}
		XFree(p);
	}
}
