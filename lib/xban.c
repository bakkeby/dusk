XKeyEvent
createKeyEvent(
	Display *display,
	Window win,
	Window winRoot,
	int press,
	int keycode,
	int modifiers
) {
	XKeyEvent event;

	event.display     = display;
	event.window      = win;
	event.root        = winRoot;
	event.subwindow   = None;
	event.time        = CurrentTime;
	event.same_screen = True;
	event.keycode     = keycode;
	event.state       = modifiers;
	event.send_event  = 1;
	event.serial      = 0;

	if (press)
		event.type = KeyPress;
	else
		event.type = KeyRelease;

	return event;
}

void
genericevent(XEvent *e)
{
	Client *c;
	Workspace *ws;
	int press;
	int keycode;
	unsigned int modifiers;
	XKeyEvent event;
	XIRawEvent *rawevent;
	XGenericEventCookie *cookie;
	KeySym keysym;

	if (e->xcookie.extension != xi_opcode)
		return;

	if (!XGetEventData(dpy, &e->xcookie))
		return;

	switch (e->xcookie.evtype) {
	case XI_RawMotion:
		if (cursor_hidden)
			show_cursor(NULL);
		break;
	case XI_RawTouchBegin:
	case XI_RawTouchEnd:
	case XI_RawTouchUpdate:
		if (!cursor_hidden)
			hide_cursor(NULL);
		break;
	case XI_RawButtonPress:
		last_button_press = now();
		if (cursor_hidden)
			show_cursor(NULL);
		break;
	case XI_RawKeyRelease:
	case XI_RawKeyPress:

		cookie = &e->xcookie;
		rawevent = cookie->data;
		keycode = rawevent->detail;
		keysym = XkbKeycodeToKeysym(dpy, keycode, 0, 0);
		press = (e->xcookie.evtype == XI_RawKeyPress);

		if (IsModifierKey(keysym)) {
			modifiers = XkbKeysymToModifiers(dpy, keysym);
			if (press) {
				modmask |= modifiers;
			} else {
				modmask &= ~modifiers;
			}
			break;
		}

		if (ISMARKED(selws->sel) && num_marked > 1) {
			for (ws = workspaces; ws; ws = ws->next) {
				for (c = ws->clients; c; c = c->next) {
					if (!ISMARKED(c))
						continue;
					if (c == selws->sel)
						continue;
					event = createKeyEvent(dpy, c->win, root, press, keycode, modmask);
					XSendEvent(dpy, c->win, False, press ? KeyPressMask : KeyReleaseMask, (XEvent *) &event);
				}
			}

			XFlush(dpy);
		}

		if (e->xcookie.evtype == XI_RawKeyRelease && now() - last_button_press > 2000 && !cursor_hidden) {
			hide_cursor(NULL);
		}
		break;
	}

	XFreeEventData(dpy, &e->xcookie);
}

void
hide_cursor(const Arg *arg)
{
	if (cursor_hidden)
		return;
	if (enabled(BanishMouseCursor)) {
		XFixesHideCursor(dpy, root);
		if (getrootptr(&mouse_x, &mouse_y)) {
			if (enabled(BanishMouseCursorToCorner)) {
				XWarpPointer(dpy, None, root, 0, 0, 0, 0, selmon->mx + selmon->mw, selmon->my);
			}
		}
	}
	cursor_hidden = 1;
}

void
show_cursor(const Arg *arg)
{
	if (!cursor_hidden)
		return;
	if (enabled(BanishMouseCursor)) {
		XWarpPointer(dpy, None, root, 0, 0, 0, 0, mouse_x, mouse_y);
		XFixesShowCursor(dpy, root);
		XSync(dpy, False);
	}
	cursor_hidden = 0;
}

void
toggle_cursor(const Arg *arg)
{
	if (cursor_hidden)
		show_cursor(arg);
	else
		hide_cursor(arg);
}
