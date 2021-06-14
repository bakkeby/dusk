void
focusdir(const Arg *arg)
{
	Client *s = selws->sel, *f = NULL, *c, *next;

	if (!s)
		return;

	unsigned int score = -1;
	unsigned int client_score;
	int dist;
	int dirweight = 20;
	int isfloating = ISFLOATING(s);

	next = s->next;
	if (!next)
		next = s->ws->clients;
	for (c = next; c != s; c = next) {

		next = c->next;
		if (!next)
			next = s->ws->clients;

		if (!ISVISIBLE(c) || ISFLOATING(c) != isfloating)
			continue;

		switch (arg->i) {
		case 0: // left
			dist = s->x - c->x - c->w;
			client_score =
				dirweight * MIN(abs(dist), abs(dist + s->ws->mon->ww)) +
				abs(s->y - c->y);
			break;
		case 1: // right
			dist = c->x - s->x - s->w;
			client_score =
				dirweight * MIN(abs(dist), abs(dist + s->ws->mon->ww)) +
				abs(c->y - s->y);
			break;
		case 2: // up
			dist = s->y - c->y - c->h;
			client_score =
				dirweight * MIN(abs(dist), abs(dist + s->ws->mon->wh)) +
				abs(s->x - c->x);
			break;
		default:
		case 3: // down
			dist = c->y - s->y - s->h;
			client_score =
				dirweight * MIN(abs(dist), abs(dist + s->ws->mon->wh)) +
				abs(c->x - s->x);
			break;
		}

		if (((arg->i == 0 || arg->i == 2) && client_score <= score) || client_score < score) {
			score = client_score;
			f = c;
		}
	}

	if (f && f != s) {
		focus(f);
		restack(f->ws);
	}
}
