void
cyclelayout(const Arg *arg)
{
	int i;
	int num_layouts = LENGTH(layouts);

	for (i = 0; i < num_layouts && &layouts[i] != selws->layout; i++);
	i += arg->i;
	setlayout(&((Arg) { .i = (i % num_layouts + num_layouts) % num_layouts })); // modulo
}
