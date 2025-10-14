/* geticonprop: read, scale, and return an XRender Picture */
Picture
geticonprop(Window win, int iconsize, unsigned int *icw, unsigned int *ich)
{
	Atom net_wm_icon = XInternAtom(dpy, "_NET_WM_ICON", False);
	Atom actual_type;
	int actual_format;
	unsigned long nitems, bytes_after;
	unsigned char *data = NULL;
	Picture pict = None;

	if (XGetWindowProperty(dpy, win, net_wm_icon, 0, LONG_MAX, False, AnyPropertyType,
			&actual_type, &actual_format, &nitems, &bytes_after, &data) != Success || !data) {
		return None;
	}

	unsigned long *p = (unsigned long *)data;
	unsigned long *end = p + nitems;

	/* Pick icon closest to desired size */
	unsigned long *best = NULL;
	unsigned long best_w = 0, best_h = 0;
	unsigned long best_diff = ~0UL;

	while (p + 2 < end) {
		unsigned long w0 = *p++;
		unsigned long h0 = *p++;
		if (w0 == 0 || h0 == 0 || p + w0*h0 > end) break;

		unsigned long diff = (w0 > (unsigned long)iconsize ? w0 - iconsize : iconsize - w0)
		                   + (h0 > (unsigned long)iconsize ? h0 - iconsize : iconsize - h0);

		if (diff < best_diff) {
			best_diff = diff;
			best_w = w0;
			best_h = h0;
			best = p;
		}
		p += w0 * h0;
	}

	if (!best) {
		XFree(data);
		return None;
	}

	/* Copy into fixed 32-bit array */
	uint32_t *src = malloc(best_w * best_h * sizeof(uint32_t));
	for (size_t i = 0; i < best_w * best_h; i++)
		src[i] = (uint32_t)best[i];
	XFree(data);

	/* Calculate scaled size */
	unsigned int dstw, dsth;
	if (best_w > best_h) {
		dstw = iconsize;
		dsth = best_h * iconsize / best_w;
	} else {
		dsth = iconsize;
		dstw = best_w * iconsize / best_h;
	}

	pict = drw_picture_scale_from_argb32(drw, src, best_w, best_h, dstw, dsth);
	free(src);

	*icw = dstw;
	*ich = dsth;
	return pict;
}

void
freeicon(Client *c)
{
	if (c->icon) {
		XRenderFreePicture(dpy, c->icon);
		c->icon = None;
	}
}

void
updateicon(Client *c)
{
	freeicon(c);
	if (c->iconpath && strlen(c->iconpath) && load_icon_from_file(c, c->iconpath))
		return;

	c->icon = geticonprop(c->win, iconsize, &c->icw, &c->ich);
}

int
load_icon_from_file(Client *c, const char *path)
{
	/* Empty image struct passed to load_image_from_file to hold the returned payload. */
	Image image = {
		.icon = None,
		.icw = 0,
		.ich = 0,
		.iconpath = NULL
	};

	if (load_image_from_file(&image, path)) {
		freeicon(c);
		free(c->iconpath);
		c->icw = image.icw;
		c->ich = image.ich;
		c->icon = image.icon;
		c->iconpath = image.iconpath;
	}

	return 0;
}

int
load_image_from_file(Image *img, const char *path)
{
	char *iconpath = subst_home_directory(path);
	Picture icon;
	unsigned int icw, ich, w, h;

	struct stat stbuf;
	if (stat(iconpath, &stbuf) == -1 || S_ISDIR(stbuf.st_mode) || strlen(iconpath) <= 2) {
		fprintf(stderr, "load_image_from_file: not a file: %s\n", iconpath);
		free(iconpath);
		return 0; /* not a file or empty */
	}

#if HAVE_IMLIB

	Imlib_Image image = imlib_load_image(iconpath);
	if (!image) {
		free(iconpath);
		return 0; /* corrupt or otherwise not loadable file */
	}

	imlib_context_set_image(image);
	imlib_image_set_has_alpha(1);

	icw = w = imlib_image_get_width();
	ich = h = imlib_image_get_height();

	if (h >= bh) {
		icw = w * ((float)(bh - 2) / (float)h);
		ich = bh - 2;
	}

	uint32_t *img_data = (uint32_t *)imlib_image_get_data_for_reading_only();

	/* Apply scaling if necessary */
	if (w != icw || h != ich) {
		icon = drw_picture_scale_from_argb32(drw, img_data, w, h, icw, ich);
	} else {
		icon = drw_picture_create_from_argb32(drw, img_data, w, h);
	}

	imlib_context_set_image(image);
	imlib_free_image();

#else  /* No IMLIB, fall back to loading farbfeld image */

	FILE *fp = fopen(iconpath, "rb");
	if (!fp) {
		fprintf(stderr, "load_image_from_file: could not open file: %s\n", iconpath);
		return 0;
	}

	char header[16];
	if (fread(header, 1, sizeof(header), fp) != sizeof(header) || memcmp(header, "farbfeld", 8) != 0) {
		fprintf(stderr, "load_image_from_file: not a farbfeld image: %s\n", iconpath);
		fclose(fp);
		return 0;
	}

	icw = w = (header[8] << 24) | (header[9] << 16) | (header[10] << 8) | header[11];
	ich = h = (header[12] << 24) | (header[13] << 16) | (header[14] << 8) | header[15];

	if (h >= bh) {
		icw = w * ((float)(bh - 2) / (float)h);
		ich = bh - 2;
	}

	size_t npixels = (size_t)w * h;
	uint32_t *pixels = malloc(npixels * sizeof(uint32_t));
	if (!pixels) {
		fprintf(stderr, "load_image_from_file: failed to allocate memory for %s\n", iconpath);
		fclose(fp);
		return 0;
	}

	for (size_t i = 0; i < npixels; i++) {
		unsigned char rgba16[8];
		if (fread(rgba16, 1, 8, fp) != 8) {
			free(pixels);
			fclose(fp);
			fprintf(stderr, "load_image_from_file: incomplete file: %s\n", iconpath);
			return 0;
		}
		uint16_t r16 = (rgba16[0] << 8) | rgba16[1];
		uint16_t g16 = (rgba16[2] << 8) | rgba16[3];
		uint16_t b16 = (rgba16[4] << 8) | rgba16[5];
		uint16_t a16 = (rgba16[6] << 8) | rgba16[7];

		uint8_t r8 = r16 >> 8;
		uint8_t g8 = g16 >> 8;
		uint8_t b8 = b16 >> 8;
		uint8_t a8 = a16 >> 8;

		pixels[i] = (a8 << 24) | (r8 << 16) | (g8 << 8) | b8;
	}

	fclose(fp);

	/* Apply scaling if necessary */
	if (w != icw || h != ich) {
		icon = drw_picture_scale_from_argb32(drw, pixels, w, h, icw, ich);
	} else {
		icon = drw_picture_create_from_argb32(drw, pixels, w, h);
	}

	free(pixels);

#endif // HAVE_IMLIB

	img->icon = icon;
	img->icw = icw;
	img->ich = ich;
	freestrdup(&img->iconpath, iconpath);
	free(iconpath);
	return 1;
}
