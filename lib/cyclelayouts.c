void
cyclelayout(const Arg *arg)
{
	Workspace *ws = selws;
	Layout *l;
	for (l = (Layout *)layouts; l != ws->layout; l++);
	setlayout(&((Arg) { .v =
		arg->i > 0
		? l->symbol && (l + 1)->symbol
			? l + 1
			: layouts
		: l != layouts && (l - 1)->symbol
			? l - 1
			: &layouts[LENGTH(layouts) - 1]
	}));
}
