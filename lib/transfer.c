void
transfer(const Arg *arg)
{
	Workspace *ws = selws;
	Client *c, *mtail = ws->clients, *stail = NULL, *insertafter;
	int transfertostack = 0, i;

	for (i = 0, c = ws->clients; c; c = c->next) {
		if (!ISVISIBLE(c) || ISFLOATING(c))
			continue;
		if (ws->sel == c) {
			transfertostack = i < ws->nmaster && ws->nmaster != 0;
		}
		if (i < ws->nmaster) {
			mtail = c;
		}
		stail = c;
		i++;
	}
	if (ISFLOATING(ws->sel) || i == 0) {
		return;
	} else if (transfertostack) {
		ws->nmaster = MIN(i, ws->nmaster) - 1;
		insertafter = stail;
	} else {
		ws->nmaster = ws->nmaster + 1;
		insertafter = mtail;
	}
	if (insertafter != ws->sel) {
		detach(ws->sel);
		if (ws->nmaster == 1 && !transfertostack) {
		 attach(ws->sel); // Head prepend case
		} else {
			ws->sel->next = insertafter->next;
			insertafter->next = ws->sel;
		}
	}
	arrange(ws);
}
