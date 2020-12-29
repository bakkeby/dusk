void
tagnextmonex(const Arg *arg)
{
	tagnextmon(&((Arg) { .ui = 1 << arg->ui }));
}

void
tagprevmonex(const Arg *arg)
{
	tagprevmon(&((Arg) { .ui = 1 << arg->ui }));
}

void
tagnextmon(const Arg *arg)
{
	tagothermon(arg, 1);
}

void
tagprevmon(const Arg *arg)
{
	tagothermon(arg, -1);
}

void
tagothermon(const Arg *arg, int dir)
{
	Workspace *ws = WS;
	Client *sel;
	Monitor *newmon;

	if (!ws->sel || !mons->next)
		return;
	sel = ws->sel;
	newmon = dirtomon(dir);
	sendmon(sel, newmon);
	if (arg->ui & TAGMASK) {
		sel->tags = arg->ui & TAGMASK;
		focus(NULL);
		arrange(newmon);
	}
}
