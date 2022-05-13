void
killunsel(const Arg *arg)
{
	Client *i = NULL;
	Workspace *ws = selws;

	if (!ws->sel)
		return;

	for (i = ws->clients; i; i = i->next) {
		if (ISVISIBLE(i) && i != ws->sel) {
			if (!sendevent(
				i->win,
				wmatom[WMDelete],
				NoEventMask,
				wmatom[WMDelete],
				CurrentTime,
				0, 0, 0
			)) {
				XGrabServer(dpy);
				XSetErrorHandler(xerrordummy);
				XSetCloseDownMode(dpy, DestroyAll);
				XKillClient(dpy, i->win);
				XSync(dpy, False);
				XSetErrorHandler(xerror);
				XUngrabServer(dpy);
			}
		}
	}
}
