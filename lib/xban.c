void
genericevent(XEvent *e)
{
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
	case XI_RawKeyRelease:
		if (now() - last_button_press > 2000 && !cursor_hidden) {
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
