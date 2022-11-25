void
cyclelayout(const Arg *arg)
{
	Workspace *ws = selws;
	int i;
	int num_layouts = LENGTH(layouts);

	for (i = 0; i < num_layouts && &layouts[i] != ws->layout; i++);
	i += arg->i;

	setlayout(&((Arg) { .v =
		arg->i > 0
		? i < num_layouts
			? &layouts[i]
			: layouts
		: i < 0
			? &layouts[num_layouts - 1]
			: &layouts[i]
	}));
}
