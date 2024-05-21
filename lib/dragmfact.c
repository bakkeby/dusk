void
dragmfact(const Arg *arg)
{
	unsigned int n;
	int py, px; // pointer coordinates
	int ax, ay, aw, ah; // area position, width and height
	int oh, ov, ih, iv;
	int center = 0, horizontal = 0, mirror = 0, fixed = 0; // layout configuration
	double fact;
	Monitor *m;
	Workspace *ws = selws;
	XEvent ev;
	Time lasttime = 0;

	getgaps(ws, &oh, &ov, &ih, &iv, &n);

	m = ws->mon;

	ax = m->wx;
	ay = m->wy;
	ah = m->wh;
	aw = m->ww;

	if (!n)
		return;

	if (ws->layout->arrange == &flextile) {
		int layout = ws->ltaxis[LAYOUT];
		if (layout < 0) {
			mirror = 1;
			layout *= -1;
		}
		if (layout > FLOATING_MASTER) {
			layout -= FLOATING_MASTER;
			fixed = 1;
		}

		if (layout == SPLIT_HORIZONTAL || layout == SPLIT_HORIZONTAL_DUAL_STACK)
			horizontal = 1;
		else if (layout == SPLIT_CENTERED_VERTICAL && (fixed || n - ws->nmaster > 1))
			center = 1;
		else if (layout == FLOATING_MASTER) {
			center = 1;
			if (aw < ah)
				horizontal = 1;
		}
		else if (layout == SPLIT_CENTERED_HORIZONTAL) {
			if (fixed || n - ws->nmaster > 1)
				center = 1;
			horizontal = 1;
		}
	}

	/* do not allow mfact to be modified under certain conditions */
	if (!ws->layout->arrange                           // floating layout
		|| (!fixed && ws->nmaster && n <= ws->nmaster) // no master
		|| (ws->layout->arrange == &flextile && ws->ltaxis[LAYOUT] == NO_SPLIT)
	) {
		return;
	}

	ay += oh;
	ax += ov;
	aw -= 2*ov;
	ah -= 2*oh;

	if (center) {
		if (horizontal) {
			px = ax + aw / 2;
			py = ay + ah / 2 + (ah - 2*ih) * (ws->mfact / 2.0) + ih / 2;
		} else { // vertical split
			px = ax + aw / 2 + (aw - 2*iv) * ws->mfact / 2.0 + iv / 2;
			py = ay + ah / 2;
		}
	} else if (horizontal) {
		px = ax + aw / 2;
		if (mirror)
			py = ay + (ah - ih) * (1.0 - ws->mfact) + ih / 2;
		else
			py = ay + ((ah - ih) * ws->mfact) + ih / 2;
	} else { // vertical split
		if (mirror)
			px = ax + (aw - iv) * (1.0 - ws->mfact) + iv / 2;
		else
			px = ax + ((aw - iv) * ws->mfact) + iv / 2;
		py = ay + ah / 2;
	}

	if (XGrabPointer(dpy, root, False, MOUSEMASK, GrabModeAsync, GrabModeAsync,
		None, cursor[horizontal ? CurResizeVertArrow : CurResizeHorzArrow]->cursor, CurrentTime) != GrabSuccess)
		return;
	ignore_warp = 1;
	XWarpPointer(dpy, None, root, 0, 0, 0, 0, px, py);

	do {
		XMaskEvent(dpy, MOUSEMASK|ExposureMask|SubstructureRedirectMask, &ev);
		switch (ev.type) {
		case ConfigureRequest:
		case Expose:
		case MapRequest:
			handler[ev.type](&ev);
			break;
		case MotionNotify:
			if ((ev.xmotion.time - lasttime) <= (1000 / 40))
				continue;
			if (lasttime != 0) {
				px = ev.xmotion.x;
				py = ev.xmotion.y;
			}
			lasttime = ev.xmotion.time;

			if (center)
				if (horizontal)
					if (py - ay > ah / 2)
						fact = (double) 1.0 - (ay + ah - py - ih / 2) * 2 / (double) (ah - 2*ih);
					else
						fact = (double) 1.0 - (py - ay - ih / 2) * 2 / (double) (ah - 2*ih);
				else
					if (px - ax > aw / 2)
						fact = (double) 1.0 - (ax + aw - px - iv / 2) * 2 / (double) (aw - 2*iv);
					else
						fact = (double) 1.0 - (px - ax - iv / 2) * 2 / (double) (aw - 2*iv);
			else
				if (horizontal)
					fact = (double) (py - ay - ih / 2) / (double) (ah - ih);
				else
					fact = (double) (px - ax - iv / 2) / (double) (aw - iv);

			if (!center && mirror)
				fact = 1.0 - fact;

			setmfact(&((Arg) { .f = 1.0 + fact }));
			px = ev.xmotion.x;
			py = ev.xmotion.y;
			break;
		}
	} while (ev.type != ButtonRelease);

	ignore_warp = 0;
	XUngrabPointer(dpy, CurrentTime);
	skipfocusevents();
}
