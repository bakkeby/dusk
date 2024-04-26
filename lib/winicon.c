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
	Picture pic;

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
	if (h >= w) {
		ich = iconsize;
		icw = MAX(w * iconsize / h, 1);
	} else {
		icw = iconsize;
		ich = MAX(h * iconsize / w, 1);
	}
	*picw = icw; *pich = ich;

	uint32_t i, *bstp32 = (uint32_t *)bstp;
	for (sz = w * h, i = 0; i < sz; ++i)
		bstp32[i] = prealpha(bstp[i]);
	pic = drw_picture_create_resized_data(drw, (char *)bstp, w, h, icw, ich);

	XFree(p);

	return pic;
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

	c->icon = drw_picture_create_resized_image(drw, image, w, h, icw, ich);
	c->icw = icw;
	c->ich = ich;

	imlib_context_set_image(image);
	imlib_free_image();

	return 1;
}
