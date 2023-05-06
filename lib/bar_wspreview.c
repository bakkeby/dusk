void
createpreview(Monitor *m)
{
	if (!m->bar)
		return;

	if (m->preview) {
		XMoveResizeWindow(
			dpy, m->preview->win,
			m->mx,
			m->my + m->bar->bh,
			m->mw * pfact,
			m->mh * pfact
		);
		return;
	}

	XSetWindowAttributes wa = {
		.override_redirect = True,
		.background_pixel = 0,
		.border_pixel = 0,
		.colormap = cmap,
		.event_mask = ButtonPressMask|ExposureMask
	};

	m->preview = ecalloc(1, sizeof(Preview));
	m->preview->win = XCreateWindow(dpy, root,
		m->mx,
		m->my + m->bar->bh,
		m->mw * pfact,
		m->mh * pfact,
		0, depth, InputOutput, visual,
		CWOverrideRedirect|CWBackPixel|CWBorderPixel|CWColormap|CWEventMask,
		&wa
	);

	XClassHint ch = {"preview", "preview"};
	XSetClassHint(dpy, m->preview->win, &ch);
}

void
freepreview(Monitor *m)
{
	if (m->preview) {
		XUnmapWindow(dpy, m->preview->win);
		XDestroyWindow(dpy, m->preview->win);
		free(m->preview);
		m->preview = 0;
	}
}

void
hidepreview(Monitor *m)
{
	m->preview->show = 0;
	XUnmapWindow(dpy, m->preview->win);
}

void
removepreview(Workspace *ws)
{
	if (ws->preview) {
		XFreePixmap(dpy, ws->preview);
		ws->preview = 0;
	}
}

void
showpreview(Workspace *ws, int x, int y)
{
	Monitor *m = ws->mon;

	if (!m->preview || !enabled(WorkspacePreview))
		return;

	if (!m->preview->show) {
		hidepreview(m);
		return;
	}

	if (!ws->preview) {
		XUnmapWindow(dpy, m->preview->win);
		return;
	}

	XSetWindowBackgroundPixmap(dpy, m->preview->win, ws->preview);
	XCopyArea(dpy, ws->preview, m->preview->win, drw->gc,
			0, 0, m->mw * pfact, m->mh * pfact, 0, 0);
	XMoveWindow(dpy, m->preview->win, x, y);
	XSync(dpy, False);
	XMapRaised(dpy, m->preview->win);
}

void
storepreview(Workspace *ws)
{
	if (!ws)
		return;

	Imlib_Image image;
	Monitor *m = ws->mon;

	if (!m->preview)
		createpreview(m);

	removepreview(ws);

	if (!ws->clients)
		return;

	hidepreview(m);
	XFlush(dpy);

	image = imlib_create_image(sw, sh);
	imlib_context_set_image(image);
	imlib_image_set_has_alpha(1);
	imlib_context_set_blend(0);
	imlib_context_set_display(dpy);
	imlib_context_set_visual(visual);
	imlib_context_set_drawable(root);
	imlib_copy_drawable_to_image(0, m->mx, m->my, m->mw, m->mh, 0, 0, 1);
	ws->preview = XCreatePixmap(dpy, m->preview->win,
			m->mw * pfact, m->mh * pfact, depth);
	imlib_context_set_drawable(ws->preview);
	imlib_render_image_part_on_drawable_at_size(
		0, 0, m->mw, m->mh,
		0, 0, m->mw * pfact, m->mh * pfact
	);
	imlib_free_image();
}
