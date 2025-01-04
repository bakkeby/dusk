static int cross_typing_enabled = 0;

void
enable_cross_typing(const Arg *arg)
{
	if (cross_typing_enabled)
		return;

	XMapWindow(dpy, wmcheckwin);
	XSelectInput(dpy, wmcheckwin, KeyPressMask | KeyReleaseMask);
	XSetInputFocus(dpy, wmcheckwin, RevertToPointerRoot, CurrentTime);

	cross_typing_enabled = 1;
}

void
disable_cross_typing(const Arg *arg)
{
	if (!cross_typing_enabled)
		return;

	XUnmapWindow(dpy, wmcheckwin);
	cross_typing_enabled = 0;
}

/* The general idea here is to give input focus to a window that the window manager controls and
 * to subscribe to key presses and releases.
 *
 * Any keys that are received are forwarded to all marked clients using XSendEvent. The event
 * serial is set to 0 otherwise the X server may ignore the event knowing that it has sent the
 * event before.
 *
 * XSendEvent will set e->send_event to True.
 *
 * Some programs opt to ignore key press events where this flag is set to True for security
 * reasons - i.e. they won't accept fake keypresses sent by potentially malicious programs.
 * They do this rightfully so. Such programs will behave as no input is being received when using
 * this feature.
 *
 * Other programs that accept key press events also when send_event is set to True will behave
 * as if you were typing in the program itself. This means that modifiers like Ctrl, Shift and Alt
 * will work as intended. As will key repeat (holding a button down).
 *
 * If the user uses the Super modifier in combination with any other key then these will not be
 * forwarded to marked clients. These are instead processed and interpreted as functional
 * keybindings within the window manager.
 *
 * If the focus shifts then the spell breaks and we are back to typing in individual windows.
 */
void
forward_key_press_event(XKeyEvent *e)
{
	Workspace *ws;
	Client *c;

	for (ws = workspaces; ws; ws = ws->next) {
		for (c = ws->clients; c; c = c->next) {
			if (!ISMARKED(c))
				continue;

			e->serial = 0;
			XSendEvent(dpy, c->win, False, e->type == KeyPress ? KeyPressMask : KeyReleaseMask, (XEvent *) e);
		}

		XFlush(dpy);
	}
}
