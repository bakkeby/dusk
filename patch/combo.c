static int combo = 0;

void
keyrelease(XEvent *e)
{
	combo = 0;
}

void
combotag(const Arg *arg)
{
	if (selws->sel && arg->ui & TAGMASK) {
		if (selws->sel->reverttags)
			selws->sel->reverttags = 0;
		if (combo) {
			selws->sel->tags |= arg->ui & TAGMASK;
		} else {
			combo = 1;
			selws->sel->tags = arg->ui & TAGMASK;
		}
		focus(NULL);
		arrange(selws);
	}
}

void
comboview(const Arg *arg)
{
	unsigned newtags = arg->ui & TAGMASK;
	if (combo) {
		selws->tagset[selws->seltags] |= newtags;
	} else {
		selws->seltags ^= 1;	/*toggle tagset*/
		combo = 1;
		if (newtags)
			view(&((Arg) { .ui = newtags }));
	}
	focus(NULL);
	arrange(selws);
}