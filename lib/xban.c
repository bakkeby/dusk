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
				ban_to_corner(selws->sel);
			}
		}
	}
	cursor_hidden = 1;
}

void
ban_to_corner(Client *c)
{
	int i, x, y;

	if (!c) {
		XWarpPointer(dpy, None, root, 0, 0, 0, 0, selmon->mx + selmon->mw, selmon->my);
		return;
	}

	/* This will attempt to move the mouse cursor to one of the corners of the client window.
	 * The order of preference is: NE, NW, SE, SW
	 * If the corner is overlapped by a floating window, then another corner will be selected.
	 * If all four corners are overlapped by other windows then the mouse cursor is not moved.
	 */

	int corners[4][2] = {
		{ c->x + c->w - 1, c->y },
		{ c->x, c->y },
		{ c->x + c->w - 1, c->y + c->h - 1 },
		{ c->x, c->y + c->h - 1 },
	};

	readclientstackingorder();

	for (i = 0; i < 4; i++) {
		x = corners[i][0];
		y = corners[i][1];
		if (c == recttoclient(x, y, 1, 1, 1)) {
			XWarpPointer(dpy, None, root, 0, 0, 0, 0, x, y);
			break;
		}
	}
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
