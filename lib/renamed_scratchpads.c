void
removescratch(const Arg *arg)
{
	Client *n;
	Client *c = selws->sel;
	if (!c)
		return;

	for (c = nextmarked(NULL, c); c; c = nextmarked(c->next, NULL)) {
		if (SEMISCRATCHPAD(c) && c->linked) {
			n = unmanagesemiscratchpad(c);
			arrangews(n->ws);
			drawbar(n->ws->mon);
		}

		c->scratchkey = 0;
	}
}

void
setscratch(const Arg *arg)
{
	Client *c = selws->sel;
	if (!c)
		return;

	char scratchkey = ((char**)arg->v)[0][0];

	for (c = nextmarked(NULL, c); c; c = nextmarked(c->next, NULL)) {
		if (SEMISCRATCHPAD(c)) {
			initsemiscratchpad(c);
			if (c->linked->scratchkey) {
				c->linked->scratchkey = scratchkey;
				continue;
			}
		}
		c->scratchkey = scratchkey;
	}
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
			if (!SCRATCHPADSTAYONMON(c) && !multimonscratch && c->ws != selws && c->ws != stickyws) {
				if (SEMISCRATCHPAD(c) && c->linked && !c->win)
					swapsemiscratchpadclients(c->linked, c);
				detach(c);
				detachstack(c);
				/* Note that we are adding clients at the end of the list, this is to preserve the
				   order of clients as they were on the adjacent monitor (relevant when tiled) */
				if (last)
					last = last->next = c;
				else
					last = monclients = c;
			} else if (scratchvisible == numscratchpads && (numscratchpads > 1 || c->ws != selws || selws->sel == c)) {
				if (SEMISCRATCHPAD(c) && c->linked)
					swapsemiscratchpadclients(c, c->linked);
				else {
					addflag(c, Invisible);
					hide(c);
				}
			} else {
				XSetWindowBorder(dpy, c->win, scheme[SchemeScratchNorm][ColBorder].pixel);
				if ((c->flags & AttachFlag) && ISINVISIBLE(c) && !ISFLOATING(c)) {
					detach(c);
					attachx(c, 0, c->ws);
				}
				if (SEMISCRATCHPAD(c) && c->linked)
					swapsemiscratchpadclients(c->linked, c);
				else {
					removeflag(c, Invisible);
					showwsclient(c);
					detachstack(c);
					attachstack(c);
				}
			}
		}
	}

	/* Attach moved scratchpad clients on the selected monitor */
	for (c = monclients; c; c = next) {
		next = c->next;
		c->next = NULL;
		clientmonresize(c, c->ws->mon, selws->mon);
		/* Attach scratchpad clients from other monitors at the bottom of the stack */
		attachx(c, c->flags & AttachFlag ? 0 : AttachBottom, selws);
		attachstack(c);
		removeflag(c, Invisible);
		showwsclient(c);
	}

	if (found) {
		c = found;
		arrange_focus_on_monocle = 0;
		if (ISVISIBLE(c)) {
			showwsclient(c);
			if (ISTRUEFULLSCREEN(c))
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
		if (multimonscratch || monclients || SEMISCRATCHPAD(c)) {
			arrange(NULL);
			restack(c->ws);
		} else {
			arrange(c->ws);
		}
		skipfocusevents();
	} else {
		spawn(arg);
	}
}
