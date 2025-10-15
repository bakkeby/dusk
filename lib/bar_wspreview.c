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

Pixmap
create_scaled_preview_xrender(Display *dpy, Window root, Visual *visual, int depth,
                              int mx, int my, int mw, int mh, float pfact)
{
	int dw = (int)(mw * pfact);
	int dh = (int)(mh * pfact);

	XWindowAttributes attrs;
	XGetWindowAttributes(dpy, root, &attrs);

	XRenderPictFormat *srcfmt = XRenderFindVisualFormat(dpy, attrs.visual);
	XRenderPictFormat *dstfmt = XRenderFindVisualFormat(dpy, visual);
	if (!srcfmt || !dstfmt)
		return 0;

	Pixmap pixmap = XCreatePixmap(dpy, root, dw, dh, depth);
	Picture src = XRenderCreatePicture(dpy, root, srcfmt, 0, NULL);
	Picture dst = XRenderCreatePicture(dpy, pixmap, dstfmt, 0, NULL);

	/* Options: FilterFast, FilterGood, FilterBest */
	XRenderSetPictureFilter(dpy, src, FilterGood, NULL, 0);

	/* Apply inverse scaling transform (destination → source) */
	XTransform transform = {{
		{ XDoubleToFixed((double)1.0 / pfact), XDoubleToFixed(0), XDoubleToFixed(0) },
		{ XDoubleToFixed(0), XDoubleToFixed((double)1.0 / pfact), XDoubleToFixed(0) },
		{ XDoubleToFixed(0), XDoubleToFixed(0), XDoubleToFixed(1) }
	}};
	XRenderSetPictureTransform(dpy, src, &transform);

	/* Directly composite the source region into the destination, scaling it. */
	XRenderComposite(dpy, PictOpSrc,
	                 src, None, dst,
	                 mx * pfact, my * pfact,  // source x, y (after transformation)
	                 0, 0,                    // mask x, y
	                 0, 0,                    // dest x, y
	                 dw, dh);                 // dest width, height (after transformation)

	XRenderFreePicture(dpy, src);
	XRenderFreePicture(dpy, dst);

	return pixmap;
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
	XClearWindow(dpy, m->preview->win);
	XMoveWindow(dpy, m->preview->win, x, y);
	XMapRaised(dpy, m->preview->win);
}

void
storepreview(Workspace *ws)
{
	if (!ws)
		return;

	Monitor *m = ws->mon;
	int preview_shown;

	if (!m->preview)
		createpreview(m);

	preview_shown = m->preview->show;

	removepreview(ws);

	if (!ws->clients)
		return;

	hidepreview(m);
	XFlush(dpy);
	XSync(dpy, False);

	/* When hiding the preview window we unmap it and the unmapping is handled asynchronously by
	 * the X server. This means that the preview window may still be visible on the screen when
	 * we are grabbing the next screenshot.
	 *
	 * There is nothing that we can do to implicitly sync up to the X server in this situation.
	 * Even if we were to subscribe to StructureNotifyMask events for the preview window the
	 * unmap notification comes through before the window has been graphically removed.
	 *
	 * So we are left with simply waiting for this to happen. The below adds an artificial delay
	 * to give the preview window time to disappear. We only need to wait if we did have a preview
	 * window shown.
	 *
	 * Feel free to play with higher or lower values. ~20 ms seems like a reasonable trade-off to
	 * have clean previews for someone who relies on this feature.
	 */
	if (preview_shown) {
		/* Force server roundtrip — In principle this ensures all pending drawing and unmap events
		 * are processed, but this does not necessarily mean that the compositor has dealt with the
		 * rendering yet, hence we still have an arbitray sleep. */
		XWindowAttributes wa;
		XGetWindowAttributes(dpy, m->preview->win, &wa);
		usleep(15000);
	}

	ws->preview = create_scaled_preview_xrender(dpy, root, visual, depth, m->mx, m->my, m->mw, m->mh, pfact);
}
