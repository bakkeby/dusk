void
createpreview(Monitor *m)
{
	if (m->preview)
		return;

	XSetWindowAttributes wa = {
		.override_redirect = True,
		.background_pixel = 0,
		.border_pixel = 0,
		.colormap = cmap,
		.event_mask = ButtonPressMask|ExposureMask
	};
	XClassHint ch = {"preview", "dusk"};

	m->preview = ecalloc(1, sizeof(Preview));
	m->preview->win = XCreateWindow(dpy, root,
		m->bar->next->bx,
		m->bar->next->by + m->bar->bh,
		m->mw / scalepreview,
		m->mh / scalepreview,
		0, depth, InputOutput, visual,
		CWOverrideRedirect|CWBackPixel|CWBorderPixel|CWColormap|CWEventMask, &wa);
	XMapRaised(dpy, m->preview->win);
	XUnmapWindow(dpy, m->preview->win);
	XSetClassHint(dpy, m->preview->win, &ch);
}

void
hidewspreview(Monitor *m)
{
	m->preview->show = 0;
	XUnmapWindow(dpy, m->preview->win);
}

void
showwspreview(Workspace *ws, int x, int y)
{
	Monitor *m = ws->mon;

	if (!m->preview || !enabled(WorkspacePreview))
		return;

	if (!m->preview->show) {
		hidewspreview(m);
		return;
	}

	if (ws->preview) {
		XSetWindowBackgroundPixmap(dpy, m->preview->win, ws->preview);
		XCopyArea(dpy, ws->preview, m->preview->win, drw->gc, 0, 0, m->mw / scalepreview, m->mh / scalepreview, 0, 0);
		XMoveWindow(dpy, m->preview->win, x, y);
		XSync(dpy, False);
		XMapWindow(dpy, m->preview->win);
		XRaiseWindow(dpy, m->preview->win);
	} else
		XUnmapWindow(dpy, m->preview->win);
}

void
storepreview(Workspace *ws)
{
	if (!ws)
		return;

	Imlib_Image image;
	Monitor *m = ws->mon;

	if (ws->preview)
		XFreePixmap(dpy, ws->preview);

	if (!ws->clients)
		return;

	hidewspreview(m);
	XSync(dpy, False);

	image = imlib_create_image(sw, sh);
	imlib_context_set_image(image);
	imlib_context_set_display(dpy);
	imlib_context_set_visual(visual);
	imlib_context_set_drawable(root);
	imlib_copy_drawable_to_image(0, m->mx, m->my, m->mw, m->mh, 0, 0, 1);
	ws->preview = XCreatePixmap(dpy, m->preview->win, m->mw / scalepreview, m->mh / scalepreview, depth);
	imlib_context_set_drawable(ws->preview);
	imlib_render_image_part_on_drawable_at_size(0, 0, m->mw, m->mh, 0, 0, m->mw / scalepreview, m->mh / scalepreview);
	imlib_free_image();
}