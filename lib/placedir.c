void
placedir(const Arg *arg)
{
	Client *s = selws->sel, *f = NULL, *c;
	Workspace *dirws = NULL;

	if (!s || ISFLOATING(s))
		return;

	unsigned int best = -1;
	unsigned int score;
	int direction = arg->i;
	int istiled = ISTILED(s);

	for (c = nextwsclient(s); c != s; c = nextwsclient(c)) {

		/* Skip clients that are e.g. behind others in a deck or monocle layout */
		if (!c->shown)
			continue;

		if (!ISVISIBLE(c) || (!ISTILED(c) != !istiled))
			continue;

		score = get_direction_score(s, c, direction);

		if (((direction == LEFT || direction == UP) && score <= best) || score < best) {
			best = score;
			f = c;
		}
	}

	/* If we do not have a candidate client, or that client was found because we wrapped around,
	 * then first check if there is an empty workspace in the given direction. If so then it will
	 * be more intuitive to move the client to that workspace instead. */
	if (
		(!f || f == s || f->ws != s->ws) ||
		(direction == LEFT && f->x >= s->x) ||
		(direction == RIGHT && f->x <= s->x) ||
		(direction == UP && f->y >= s->y) ||
		(direction == DOWN && f->y <= s->y)
	) {
		dirws = get_next_workspace_in_direction(s->ws, direction);
		if (dirws && dirws->clients == NULL) {
			movetows(s, dirws, 1);
			return;
		}
	}

	if (f && f != s) {
		swap(f, s);
		arrange(f->ws);
		if (f->ws != s->ws) {
			arrange(s->ws);
		}
		focus(s);
	}
}
