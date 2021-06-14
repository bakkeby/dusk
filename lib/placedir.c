void
placedir(const Arg *arg)
{
	Client *s = selws->sel, *f = NULL, *c, *next, *fprior, *sprior;

	if (!s || ISFLOATING(s))
		return;

	unsigned int score = -1;
	unsigned int client_score;
	int dist;
	int dirweight = 20;

	next = s->next;
	if (!next)
		next = s->ws->clients;
	for (c = next; c != s; c = next) {

		next = c->next;
		if (!next)
			next = s->ws->clients;

		if (!ISVISIBLE(c))
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
		for (fprior = f->ws->clients; fprior && fprior->next != f; fprior = fprior->next);
		for (sprior = s->ws->clients; sprior && sprior->next != s; sprior = sprior->next);

		if (s == fprior) {
			next = f->next;
			if (sprior)
				sprior->next = f;
			else
				f->ws->clients = f;
			f->next = s;
			s->next = next;
		} else if (f == sprior) {
			next = s->next;
			if (fprior)
				fprior->next = s;
			else
				s->ws->clients = s;
			s->next = f;
			f->next = next;
		} else { // clients are not adjacent to each other
			next = f->next;
			f->next = s->next;
			s->next = next;
			if (fprior)
				fprior->next = s;
			else
				s->ws->clients = s;
			if (sprior)
				sprior->next = f;
			else
				f->ws->clients = f;
		}

		arrange(f->ws);
	}
}