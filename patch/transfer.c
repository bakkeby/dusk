void
transfer(const Arg *arg)
{
	Client *c, *mtail = selws->clients, *stail = NULL, *insertafter;
	int transfertostack = 0, i, nmasterclients;

	for (i = 0, c = selws->clients; c; c = c->next) {
		if (!ISVISIBLE(c) || ISFLOATING(c)) continue;
		if (selws->sel == c) { transfertostack = i < selmon->nmaster && selmon->nmaster != 0; }
		if (i < selmon->nmaster) { nmasterclients++; mtail = c; }
		stail = c;
		i++;
	}
	if (ISFLOATING(selws->sel) || i == 0) {
		return;
	} else if (transfertostack) {
		selmon->nmaster = MIN(i, selmon->nmaster) - 1;
		insertafter = stail;
	} else {
		selmon->nmaster = selmon->nmaster + 1;
		insertafter = mtail;
	}
	if (insertafter != selws->sel) {
		detach(selws->sel);
		if (selmon->nmaster == 1 && !transfertostack) {
		 attach(selws->sel); // Head prepend case
		} else {
			selws->sel->next = insertafter->next;
			insertafter->next = selws->sel;
		}
	}
	arrange(selws);
}