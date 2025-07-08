void
focusdir(const Arg *arg)
{
	Client *s = selws->sel, *f = NULL, *c;

	if (!s)
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

	if (f && f != s) {
		focus(f);
		restack(f->ws);
	}
}

unsigned int
get_direction_score(Client *s, Client *c, int direction)
{
	unsigned int score = 0;
	int dist = 0;
	int wrap = 0;
	int dirweight = 20;

	switch (direction) {
	case LEFT:
		dist = s->x - c->x - c->w;
		wrap = sw;
		score = abs(s->y - c->y);
		break;
	case RIGHT:
		dist = c->x - s->x - s->w;
		wrap = sw;
		score = abs(c->y - s->y);
		break;
	case UP:
		dist = s->y - c->y - c->h;
		wrap = s->ws->mon->wh;
		score = abs(s->x - c->x);
		break;
	case DOWN:
		dist = c->y - s->y - s->h;
		wrap = s->ws->mon->wh;
		score = abs(c->x - s->x);
		break;
	}

	score += dirweight * MIN(abs(dist), abs(dist + wrap));
	return score;
}

Workspace *
get_next_workspace_in_direction(Workspace *ws, int direction)
{
	Workspace *ret = NULL, *w;
	unsigned int best = -1;
	unsigned int dist = 0;
	int xdelta, ydelta;

	for (w = workspaces; w; w = w->next) {
		if (w == ws || !w->visible || w == stickyws)
			continue;

		if (
			(direction == LEFT && w->wx >= ws->wx) ||
			(direction == RIGHT && w->wx <= ws->wx) ||
			(direction == UP && w->wy >= ws->wy) ||
			(direction == DOWN && w->wy <= ws->wy)
		) {
			continue;
		}

		xdelta = (w->wx + w->ww / 2) - (ws->wx + ws->ww / 2);
		ydelta = (w->wy + w->wh / 2) - (ws->wy + ws->wh / 2);
		dist = xdelta * xdelta + ydelta * ydelta;

		if (dist < best) {
			best = dist;
			ret = w;
		}
	}

	return ret;
}
