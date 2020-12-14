void
killunsel(const Arg *arg)
{
	Client *i = NULL;

	if (!selws->sel)
		return;

	for (i = selws->clients; i; i = i->next) {
		if (ISVISIBLE(i) && i != selws->sel) {
			if (!sendevent(i->win, wmatom[WMDelete], NoEventMask, wmatom[WMDelete], CurrentTime, 0, 0, 0))
			{
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