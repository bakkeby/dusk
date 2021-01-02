void
shiftview(const Arg *arg)
{
	Workspace *ws = WS, *nws = NULL, *tws;

	if (arg->i > 0) { // right circular shift
		for (nws = ws->next; nws && !(nws->mon == ws->mon); nws = nws->next);
		if (!nws && ws != workspaces)
			for (tws = workspaces; tws && tws != ws; tws = tws->next)
				if (tws->mon == ws->mon) {
					nws = tws;
					break;
				}
	} else { // left circular shift
		for (tws = workspaces; tws && !(nws && tws == ws); tws = tws->next)
			if (tws->mon == ws->mon)
				nws = tws;
	}

	if (!nws)
		return;

	viewwsonmon(nws, nws->mon);
}
