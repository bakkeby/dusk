static uint32_t prealpha(uint32_t p) {
	uint8_t a = p >> 24u;
	uint32_t rb = (a * (p & 0xFF00FFu)) >> 8u;
	uint32_t g = (a * (p & 0x00FF00u)) >> 8u;
	return (rb & 0xFF00FFu) | (g & 0x00FF00u) | (a << 24u);
}

Picture
geticonprop(Window win, unsigned int *picw, unsigned int *pich)
{
	int format;
	unsigned long n, extra, *p = NULL;
	Atom real;

	if (XGetWindowProperty(dpy, win, netatom[NetWMIcon], 0L, LONG_MAX, False, AnyPropertyType,
						   &real, &format, &n, &extra, (unsigned char **)&p) != Success)
		return None;

	if (n == 0 || format != 32) {
		XFree(p);
		return None;
	}

	unsigned long *bstp = NULL;
	uint32_t w, h, sz;
	{
		unsigned long *i; const unsigned long *end = p + n;
		uint32_t bstd = UINT32_MAX, d, m;
		for (i = p; i < end - 1; i += sz) {
			if ((w = *i++) >= 16384 || (h = *i++) >= 16384) {
				XFree(p);
				return None;
			}
			if ((sz = w * h) > end - i)
				break;
			if ((m = w > h ? w : h) >= iconsize && (d = m - iconsize) < bstd) {
				bstd = d;
				bstp = i;
			}
		}
		if (!bstp) {
			for (i = p; i < end - 1; i += sz) {
				if ((w = *i++) >= 16384 || (h = *i++) >= 16384) {
					XFree(p);
					return None;
				}
				if ((sz = w * h) > end - i)
					break;
				if ((d = iconsize - (w > h ? w : h)) < bstd) {
					bstd = d;
					bstp = i;
				}
			}
		}
		if (!bstp) {
			XFree(p);
			return None;
		}
	}

	if ((w = *(bstp - 2)) == 0 || (h = *(bstp - 1)) == 0) {
		XFree(p);
		return None;
	}

	uint32_t icw, ich;
	if (w <= h) {
		ich = iconsize; icw = w * iconsize / h;
		if (icw == 0)
			icw = 1;
	}
	else {
		icw = iconsize; ich = h * iconsize / w;
		if (ich == 0)
			ich = 1;
	}
	*picw = icw; *pich = ich;

	uint32_t i, *bstp32 = (uint32_t *)bstp;
	for (sz = w * h, i = 0; i < sz; ++i)
		bstp32[i] = prealpha(bstp[i]);

	Picture ret = drw_picture_create_resized(drw, (char *)bstp, w, h, icw, ich);
	XFree(p);

	return ret;
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
	if (strlen(c->iconpath) && load_icon_from_png_image(c, c->iconpath))
		return;

	c->icon = geticonprop(c->win, &c->icw, &c->ich);
}

int
load_icon_from_png_image(Client *c, const char *iconpath)
{
	unsigned int* data;
	Imlib_Image image;
	int w, h, s, ich, icw;

	struct stat stbuf;
	s = stat(iconpath, &stbuf);

	if (s == -1 || S_ISDIR(s) || strlen(iconpath) <= 2)
		return 0; /* no readable file */

	freeicon(c);
	strlcpy(c->iconpath, iconpath, sizeof c->iconpath);
	image = imlib_load_image(iconpath);
	if (!image)
		return 0; /* corrupt or otherwise not loadable file */
	imlib_context_set_image(image);
	imlib_image_set_has_alpha(1);
	icw = w = imlib_image_get_width();
	ich = h = imlib_image_get_height();
	if (h >= bh) {
		icw = w * ((float)(bh - 2) / (float)h);
		ich = bh - 2;
	}

	data = imlib_image_get_data_for_reading_only();
	imlib_free_image();

	c->icon = drw_picture_create_resized(drw, (char*)data, w, h, icw, ich);
	c->icw = icw;
	c->ich = ich;
	return 1;
}

Picture
drw_picture_create_resized(Drw *drw, char *src, unsigned int srcw, unsigned int srch, unsigned int dstw, unsigned int dsth)
{
	Pixmap pm;
	Picture pic;
	GC gc;

	if (srcw <= (dstw << 1u) && srch <= (dsth << 1u)) {
		XImage img = {
			srcw, srch, 0, ZPixmap, src,
			ImageByteOrder(drw->dpy), BitmapUnit(drw->dpy), BitmapBitOrder(drw->dpy), 32,
			32, 0, 32,
			0, 0, 0
		};
		XInitImage(&img);
		pm = XCreatePixmap(drw->dpy, drw->root, srcw, srch, 32);
		gc = XCreateGC(drw->dpy, pm, 0, NULL);
		XPutImage(drw->dpy, pm, gc, &img, 0, 0, 0, 0, srcw, srch);
		XFreeGC(drw->dpy, gc);

		pic = XRenderCreatePicture(drw->dpy, pm, XRenderFindStandardFormat(drw->dpy, PictStandardARGB32), 0, NULL);
		XFreePixmap(drw->dpy, pm);

		XRenderSetPictureFilter(drw->dpy, pic, FilterBilinear, NULL, 0);
		XTransform xf;
		xf.matrix[0][0] = (srcw << 16u) / dstw; xf.matrix[0][1] = 0; xf.matrix[0][2] = 0;
		xf.matrix[1][0] = 0; xf.matrix[1][1] = (srch << 16u) / dsth; xf.matrix[1][2] = 0;
		xf.matrix[2][0] = 0; xf.matrix[2][1] = 0; xf.matrix[2][2] = 65536;
		XRenderSetPictureTransform(drw->dpy, pic, &xf);
	} else {
		Imlib_Image origin = imlib_create_image_using_data(srcw, srch, (DATA32 *)src);
		if (!origin)
			return None;
		imlib_context_set_image(origin);
		imlib_image_set_has_alpha(1);
		Imlib_Image scaled = imlib_create_cropped_scaled_image(0, 0, srcw, srch, dstw, dsth);
		imlib_free_image_and_decache();
		if (!scaled)
			return None;
		imlib_context_set_image(scaled);
		imlib_image_set_has_alpha(1);
		XImage img = {
		    dstw, dsth, 0, ZPixmap, (char *)imlib_image_get_data_for_reading_only(),
		    ImageByteOrder(drw->dpy), BitmapUnit(drw->dpy), BitmapBitOrder(drw->dpy), 32,
		    32, 0, 32,
		    0, 0, 0
		};
		XInitImage(&img);

		pm = XCreatePixmap(drw->dpy, drw->root, dstw, dsth, 32);
		gc = XCreateGC(drw->dpy, pm, 0, NULL);
		XPutImage(drw->dpy, pm, gc, &img, 0, 0, 0, 0, dstw, dsth);
		XFreeGC(drw->dpy, gc);

		pic = XRenderCreatePicture(drw->dpy, pm, XRenderFindStandardFormat(drw->dpy, PictStandardARGB32), 0, NULL);
		imlib_free_image_and_decache();
		XFreePixmap(drw->dpy, pm);
	}

	return pic;
}

void
drw_pic(Drw *drw, int x, int y, unsigned int w, unsigned int h, Picture pic)
{
	if (!drw)
		return;
	XRenderComposite(drw->dpy, PictOpOver, pic, None, drw->picture, 0, 0, 0, 0, x, y, w, h);
}
