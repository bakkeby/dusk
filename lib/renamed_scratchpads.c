Client *
cloneclient(Client *c)
{
	Client *clone = ecalloc(1, sizeof(Client));

	strcpy(clone->name, c->name);
	clone->mina = c->mina;
	clone->maxa = c->maxa;
	clone->cfact = c->cfact;
	clone->win = c->win;
	clone->flags = c->flags;
	clone->ws = c->ws;
	clone->pid = c->pid;
	clone->opacity = c->opacity;

	clone->sfx = c->sfx;
	clone->sfy = c->sfy;
	clone->sfh = c->sfh;
	clone->sfw = c->sfw;

	clone->x = c->x;
	clone->y = c->y;
	clone->h = c->h;
	clone->w = c->w;

	clone->oldx = c->oldx;
	clone->oldy = c->oldy;
	clone->oldh = c->oldh;
	clone->oldw = c->oldw;

	clone->basew = c->basew;
	clone->baseh = c->baseh;
	clone->incw = c->incw;
	clone->inch = c->inch;
	clone->maxw = c->maxw;
	clone->maxh = c->maxh;
	clone->minw = c->minw;
	clone->minh = c->minh;

	clone->bw = c->bw;
	clone->oldbw = c->oldbw;

	return clone;
}

void
initsemiscratchpad(Client *c)
{
	if (!SEMISCRATCHPAD(c))
		return;

	removeflag(c, Terminal); // disallow semi scratchpad from swallowing clients
	c->swallowing = cloneclient(c);
	c->swallowing->scratchkey = 0; // the cloned client is normal, not a scratchpad
	updateicon(c->swallowing);
	attachx(c->swallowing, 0, NULL);
	attachstack(c->swallowing);
	swapsemiscratchpadclients(c, c->swallowing); // always start in "normal" mode
}

void
unmanagesemiscratchpad(Client *c)
{
	if (!SEMISCRATCHPAD(c))
		return;

	Client *s;

	if (c->swallowing) {
		s = c->swallowing;
		c->swallowing = NULL;
	} else
		s = semisscratchpadforclient(c);
	if (s) {
		s->swallowing = NULL;
		removeflag(s, SemiScratchpad);
		unmanage(s, 1);
	}
}

void
swapsemiscratchpadclients(Client *o, Client *n)
{
	if (!o->win)
		return;

	n->win = o->win;
	o->win = 0;

	strcpy(n->name, o->name);

	if (ISFLOATING(n)) {
		addflag(n, NeedResize);
		showwsclient(n);
	} else if (ISVISIBLE(n))
		XMoveResizeWindow(dpy, n->win, n->x, n->y, n->w, n->h);
	else
		XMoveResizeWindow(dpy, n->win, WIDTH(n) * -2, n->y, n->w, n->h);
}

Client *
semisscratchpadforclient(Client *s)
{
	Workspace *ws;
	Client *c;

	for (ws = workspaces; ws; ws = ws->next)
		for (c = ws->clients; c; c = c->next)
			if (c->swallowing && c->swallowing == s)
				return c;

	return NULL;
}

void
removescratch(const Arg *arg)
{
	Client *c = selws->sel;
	if (!c)
		return;

	for (c = nextmarked(NULL, c); c; c = nextmarked(c->next, NULL))
		c->scratchkey = 0;
}

void
setscratch(const Arg *arg)
{
	Client *c = selws->sel;
	if (!c)
		return;

	for (c = nextmarked(NULL, c); c; c = nextmarked(c->next, NULL))
		c->scratchkey = ((char**)arg->v)[0][0];
}

void
togglescratch(const Arg *arg)
{
	Client *c, *next, *last = NULL, *found = NULL, *monclients = NULL;
	int x, y;
	Workspace *ws;
	int scratchvisible = 0; // whether the scratchpads are currently visible or not
	int multimonscratch = 0; // whether we have scratchpads that are placed on multiple monitors
	int scratchmon = -1; // the monitor where the scratchpads exist
	int numscratchpads = 0; // count of scratchpads

	/* Looping through monitors and client's twice, the first time to work out whether we need
	   to move clients across from one monitor to another or not */
	for (ws = workspaces; ws; ws = ws->next) {
		for (c = ws->clients; c; c = c->next) {
			if (c->scratchkey != ((char**)arg->v)[0][0])
				continue;
			if (scratchmon != -1 && scratchmon != ws->mon->num)
				multimonscratch = 1;
			if (ISVISIBLE(c))
				++scratchvisible;
			scratchmon = ws->mon->num;
			++numscratchpads;
		}
	}

	/* Now for the real deal. The logic should go like:
	    - hidden scratchpads will be shown
	    - shown scratchpads will be hidden, unless they are being moved to the current monitor
	    - the scratchpads will be moved to the current monitor if they all reside on the same monitor
	    - multiple scratchpads residing on separate monitors will be left in place
	 */
	for (ws = workspaces; ws; ws = ws->next) {
		for (c = ws->stack; c; c = next) {
			next = c->snext;
			if (c->scratchkey != ((char**)arg->v)[0][0])
				continue;

			if (HIDDEN(c))
				reveal(c);

			/* Record the first found scratchpad client for focus purposes, but prioritise the
			   scratchpad on the current monitor if one exists */
			if (!found || (ws == selws && found->ws != selws))
				found = c;

			/* If scratchpad clients reside on another monitor and we are moving them across then
			   as we are looping through monitors we could be moving a client to a monitor that has
			   not been processed yet, hence we could be processing a scratchpad twice. To avoid
			   this we detach them and add them to a temporary list (monclients) which is to be
			   processed later. */
			if (!SCRATCHPADSTAYONMON(c) && !multimonscratch && c->ws != selws) {

				if (SEMISCRATCHPAD(c) && c->swallowing && !c->win)
					swapsemiscratchpadclients(c->swallowing, c);
				detach(c);
				detachstack(c);
				/* Note that we are adding clients at the end of the list, this is to preserve the
				   order of clients as they were on the adjacent monitor (relevant when tiled) */
				if (last)
					last = last->next = c;
				else
					last = monclients = c;
			} else if (scratchvisible == numscratchpads) {
				if (SEMISCRATCHPAD(c) && c->swallowing)
					swapsemiscratchpadclients(c, c->swallowing);
				else
					addflag(c, Invisible);
			} else {
				XSetWindowBorder(dpy, c->win, scheme[SchemeScratchNorm][ColBorder].pixel);
				if (ISFLOATING(c))
					XRaiseWindow(dpy, c->win);
				if (SEMISCRATCHPAD(c) && c->swallowing)
					swapsemiscratchpadclients(c->swallowing, c);
				else
					removeflag(c, Invisible);
			}
		}
	}

	/* Attach moved scratchpad clients on the selected monitor */
	for (c = monclients; c; c = next) {
		next = c->next;
		clientmonresize(c, c->ws->mon, selws->mon);
		c->ws = selws;
		/* Attach scratchpad clients from other monitors at the bottom of the stack */
		if (selws->clients) {
			for (last = selws->clients; last && last->next; last = last->next);
			last->next = c;
		} else
			selws->clients = c;
		c->next = NULL;
		attachstack(c);
		removeflag(c, Invisible);

		if (ISFLOATING(c))
			XRaiseWindow(dpy, c->win);
	}

	if (!found) {
		for (c = stickyws->stack; c; c = next) {
			next = c->snext;
			if (c->scratchkey != ((char**)arg->v)[0][0])
				continue;
			found = c;
			break;
		}
	}

	if (found) {
		c = found;
		arrange_focus_on_monocle = 0;
		if (ISVISIBLE(c)) {
			showwsclient(c);
			if (ISFULLSCREEN(c) && !ISFAKEFULLSCREEN(c))
				clientfsrestore(c);
			focus(c);
		} else {
			/* If the scratchpad toggled away is set to not move between
			 * monitors then move focus back to the monitor where the mouse
			 * cursor is. This is not an ideal solution as one can change
			 * monitors using keybindings in which case the below can lead
			 * to the wrong monitor receiving focus. */
			if (!SEMISCRATCHPAD(c))
				hide(c);
			if (SCRATCHPADSTAYONMON(c) && getrootptr(&x, &y)) {
				selws = recttows(x, y, 1, 1);
				selmon = selws->mon;
			}
			focus(NULL);
		}
		arrange_focus_on_monocle = 1;

		if (multimonscratch || monclients || SEMISCRATCHPAD(c))
			arrange(NULL);
		else
			arrange(c->ws);
		skipfocusevents();
		if (ISFLOATING(c))
			XRaiseWindow(dpy, c->win);
	} else {
		spawn(arg);
	}
}
