static int combo = 0;

void
keyrelease(XEvent *e)
{
	combo = 0;
}

void
combotag(const Arg *arg)
{
	Workspace *ws = WS;
	if (ws->sel && arg->ui & TAGMASK) {
		if (ws->sel->reverttags)
			ws->sel->reverttags = 0;
		if (combo) {
			ws->sel->tags |= arg->ui & TAGMASK;
		} else {
			combo = 1;
			ws->sel->tags = arg->ui & TAGMASK;
		}
		focus(NULL);
		arrange(ws);
	}
}

void
comboview(const Arg *arg)
{
	Workspace *ws = WS;
	unsigned newtags = arg->ui & TAGMASK;
	if (combo) {
		ws->tags |= newtags;
	} else {
		ws->prevtags = ws->tags;
		combo = 1;
		if (newtags)
			view(&((Arg) { .ui = newtags }));
	}
	focus(NULL);
	arrange(ws);
}