void
group(const Arg *arg)
{
	Client *c = selws->sel;
	if (!c)
		return;

	int idx = ++grp_idx;

	for (c = nextmarked(NULL, c); c; c = nextmarked(c->next, NULL)) {
		c->group = idx;
		fprintf(stderr, "group: marked client %s with group = %d\n", c->name, idx);
	}
}

void
ungroup(const Arg *arg)
{
	Client *c = selws->sel;
	if (!c)
		return;

	for (c = nextmarked(NULL, c); c; c = nextmarked(c->next, NULL))
		c->group = 0;
}
