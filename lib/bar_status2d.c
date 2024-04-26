static char termcol0[]  = "#000000"; /* black   */
static char termcol1[]  = "#ff0000"; /* red     */
static char termcol2[]  = "#33ff00"; /* green   */
static char termcol3[]  = "#ff0099"; /* yellow  */
static char termcol4[]  = "#0066ff"; /* blue    */
static char termcol5[]  = "#cc00ff"; /* magenta */
static char termcol6[]  = "#00ffff"; /* cyan    */
static char termcol7[]  = "#d0d0d0"; /* white   */
static char termcol8[]  = "#808080"; /* black   */
static char termcol9[]  = "#ff0000"; /* red     */
static char termcol10[] = "#33ff00"; /* green   */
static char termcol11[] = "#ff0099"; /* yellow  */
static char termcol12[] = "#0066ff"; /* blue    */
static char termcol13[] = "#cc00ff"; /* magenta */
static char termcol14[] = "#00ffff"; /* cyan    */
static char termcol15[] = "#ffffff"; /* white   */
static char *termcolor[] = {
	termcol0, termcol1, termcol2, termcol3, termcol4, termcol5, termcol6, termcol7,
	termcol8, termcol9, termcol10, termcol11, termcol12, termcol13, termcol14, termcol15,
};
static int statusclicked = -1;

static ImageBuffer imagebuffer[30] = {0};

int
size_status(Bar *bar, BarArg *a)
{
	return (bar->vert ? bh : status2dtextlength(rawstatustext[a->value]));
}

int
click_status(Bar *bar, Arg *arg, BarArg *a)
{
	statusclicked = a->value;
	return ClkStatusText;
}

int
draw_status(Bar *bar, BarArg *a)
{
	return drw_2dtext(drw, a->x, a->y, a->w, a->h, a->lpad, rawstatustext[a->value], 0, 1, a->scheme);
}

int
drw_2dtext(Drw *drw, int x, int y, unsigned int w, unsigned int h, unsigned int lpad, char *text2d, int invert, int drawbg, int defscheme)
{
	if (!w && drawbg)
		return 0;

	int i, j, tw, dx = x, len, mw = w - 2 * lpad;
	int rx, ry, rw, rh;
	int fillbg = drawbg;
	short isCode = 0;
	char *text = {0};
	char *p = {0};
	Image *image;
	Clr oldbg = scheme[defscheme][ColFg];
	Clr oldfg = scheme[defscheme][ColBg];
	len = strlen(text2d) + 1;
	text = (char*) ecalloc(1, sizeof(char)*(len));
	p = text;

	strcpy(text, text2d);

	if (drawbg) {
		drw_setscheme(drw, scheme[defscheme]);
		drw_rect(drw, x, y, w, h, 1, 1);
	}

	dx += lpad;
	drw_setscheme(drw, scheme[LENGTH(colors)]);
	drw->scheme[ColFg] = scheme[defscheme][ColFg];
	drw->scheme[ColBg] = scheme[defscheme][ColBg];

	/* process status text */
	i = -1;
	while (text[++i]) {
		if (text[i] == '^' && !isCode) {
			isCode = 1;

			text[i] = '\0';
			tw = textw_clamp(text, mw);

			if (tw) {
				drw_text(drw, dx, y, tw, bh, 0, text, invert, fillbg);
				dx += tw;
				mw -= tw;
			}

			/* process code */
			while (text[++i] != '^') {
				if (text[i] == 'c') {
					char buf[8];
					if (i + 7 > len) {
						i += 7;
						break;
					}
					memcpy(buf, (char*)text+i+1, 7);
					buf[7] = '\0';
					drw_clr_create(drw, &drw->scheme[ColFg], buf, enabled(Status2DNoAlpha) ? 0xff : default_alphas[ColFg]);
					i += 7;
				} else if (text[i] == 'b') {
					char buf[8];
					if (i + 7 > len) {
						i += 7;
						break;
					}
					memcpy(buf, (char*)text+i+1, 7);
					buf[7] = '\0';
					drw_clr_create(drw, &drw->scheme[ColBg], buf, enabled(Status2DNoAlpha) ? 0xff : default_alphas[ColBg]);
					i += 7;
				} else if (text[i] == 'C') {
					int c = atoi(text + ++i) % 16;
					drw_clr_create(drw, &drw->scheme[ColFg], termcolor[c], enabled(Status2DNoAlpha) ? 0xff : default_alphas[ColFg]);
				} else if (text[i] == 'B') {
					int c = atoi(text + ++i) % 16;
					drw_clr_create(drw, &drw->scheme[ColBg], termcolor[c], enabled(Status2DNoAlpha) ? 0xff : default_alphas[ColBg]);
				} else if (text[i] == 'S') {
					int s = atoi(text + ++i) % SchemeLast;
					drw->scheme[ColFg] = scheme[s][ColFg];
					drw->scheme[ColBg] = scheme[s][ColBg];
				} else if (text[i] == 'd') {
					drw->scheme[ColFg] = scheme[defscheme][ColFg];
					drw->scheme[ColBg] = scheme[defscheme][ColBg];
				} else if (text[i] == 'w') {
					Clr swp;
					swp = drw->scheme[ColFg];
					drw->scheme[ColFg] = drw->scheme[ColBg];
					drw->scheme[ColBg] = swp;
				} else if (text[i] == 'v') {
					oldfg = drw->scheme[ColFg];
					oldbg = drw->scheme[ColBg];
				} else if (text[i] == '.') {
					fillbg = !fillbg;
				} else if (text[i] == 't') {
					drw->scheme[ColFg] = oldfg;
					drw->scheme[ColBg] = oldbg;
				} else if (text[i] == 'r') {
					if (++i >= len)
						break;
					rx = (strncmp(text + i, "w", 1) == 0 ? w - 1 : atoi(text + i));
					if (rx < 0)
						rx += mw;
					while (i < len && text[++i] != ',');
					if (++i >= len)
						break;
					ry = (strncmp(text + i, "h", 1) == 0 ? h - 1 : atoi(text + i));
					if (ry < 0)
						ry += h;
					while (i < len && text[++i] != ',');
					if (++i >= len)
						break;
					rw = (strncmp(text + i, "w", 1) == 0 ? w : atoi(text + i));
					if (rw < 0)
						rw += mw;
					while (i < len && text[++i] != ',');
					if (++i >= len)
						break;
					rh = (strncmp(text + i, "h", 1) == 0 ? h : atoi(text + i));
					if (rh < 0)
						rh += h;

					if (ry < 0)
						ry = 0;
					if (rx < 0)
						rx = 0;
					if (rw < 0)
						rw = 0;
					if (rh < 0)
						rh = 0;

					drw_rect(drw, dx + rx, y + ry, rw, rh, 1, 0);
				} else if (text[i] == 'i' || text[i] == 'I') {
					/* Linux has a maximum filename length of 255 characters for most filesystems
					 * and a maxixmum path of 4096 characters. For status updates we do not expect
					 * that long path names so we keep this at 255 characters. */
					int maxlen = 256;
					char buf[maxlen];
					int use_cache = 1;

					if (text[i] == 'I') {
						use_cache = 0;
						*(text2d + i) = 'i'; // ensure that the next time this status is used we do it from cache
					}

					for (j = 0, i++; j < maxlen - 1 && i < len && text[i] != '^'; i++, j++)
						buf[j] = text[i];
					buf[j] = '\0';
					i--;

					if ((image = loadimage(buf, use_cache))) {
						drw_pic(drw, dx, y + (h - image->ich) / 2, MIN(image->icw, mw), image->ich, image->icon);
						dx += image->icw;
						mw -= image->icw;
					}
				} else if (text[i] == 'f') {
					if (++i >= len)
						break;
					rx = (strncmp(text + i, "p", 1) == 0 ? 0 : atoi(text + i));
					if (rx < 0)
						rx += mw;
					dx += rx;
					mw -= rx;
				}
			}

			isCode = 0;
			len -= i + 1;
			if (len <= 0) {
				break;
			}

			text = text + i + 1;
			i = -1;
		}
	}
	if (!isCode && len > 0) {
		tw = textw_clamp(text, mw);
		if (tw > 0)
			drw_text(drw, dx, y, tw, bh, 0, text, invert, fillbg);
	}
	free(p);

	return 1;
}

void
setstatus(const Arg args[], int num_args)
{
	const BarRule *br;
	int i, j, sid = args[0].i;

	if (sid < 0 || sid >= NUM_STATUSES)
		return;

	char const *statustext = args[1].v;

	for (j = 0, i = 0; j < STATUS_BUFFER - 1 && statustext[i] != '\0'; j++, i++) {
		if (statustext[i] == '~' && statustext[i + 1] == '/') {
			strlcpy(rawstatustext[sid] + j, env_home, env_homelen + 1);
			j += env_homelen - 1;
		} else {
			rawstatustext[sid][j] = statustext[i];
		}
	}
	rawstatustext[sid][j] = '\0';

	for (int r = 0; r < LENGTH(barrules); r++) {
		br = &barrules[r];
		if (br->value == sid && br->drawfunc == draw_status)
			drawbarmodule(br, r);
	}
}

int
status2dtextlength(char* text2d)
{
	int i, j, w, len;
	short isCode = 0;
	char *text = {0};
	char *p = {0};
	Image *image;

	len = strlen(text2d) + 1;
	text = (char*) ecalloc(1, sizeof(char)*len);
	p = text;

	strcpy(text, text2d);

	/* compute width of the status text */
	w = 0;
	i = -1;
	while (text[++i]) {
		if (text[i] == '^') {
			if (!isCode) {
				isCode = 1;
				text[i] = '\0';
				w += TEXTW(text);
				text[i] = '^';
				++i;
				if (text[i] == 'f') {
					w += atoi(text + ++i);
				} else if (text[i] == 'i' || text[i] == 'I') {
					int maxlen = 256;
					char buf[maxlen];
					int use_cache = 1;

					if (text[i] == 'I') {
						use_cache = 0;
						*(text2d + i) = 'i'; // ensure that the next time this status is used we do it from cache
					}

					for (j = 0, i++; j < maxlen - 1 && i < len && text[i] != '^'; i++, j++)
						buf[j] = text[i];
					buf[j] = '\0';
					i--;

					if ((image = loadimage(buf, use_cache))) {
						w += image->icw;
					}
				}
			} else {
				isCode = 0;
				text = text + i + 1;
				i = -1;
			}
		}
	}
	if (!isCode)
		w += TEXTW(text);
	free(p);
	return w;
}

void
statusclick(const Arg *arg)
{
	spawncmd(&((Arg) { .v = statusclickcmd }), arg->i, 1);
}

Image *
loadimage(char *path, int use_cache)
{
	int i;
	int least = -1;
	time_t leasttime = INT_MAX - 1;
	Image *image;

	/* First see if we can find the image path in our list of buffered images */
	for (i = 0; i < LENGTH(imagebuffer); i++) {
		if (imagebuffer[i].atime < leasttime) {
			least = i;
			leasttime = imagebuffer[i].atime;
		}

		if (imagebuffer[i].image.icon == None)
			continue;

		if (!strcmp(imagebuffer[i].image.iconpath, path)) {
			if (use_cache) {
				imagebuffer[i].atime = time(NULL);
				return &imagebuffer[i].image;
			}

			least = i;
			break;
		}
	}

	/* If we did not find our image in the buffer then we need to try to load it from file.
	 * We may be overwriting the oldest image in the buffer, so free that first if present. */
	image = &imagebuffer[least].image;

	if (image->icon) {
		XRenderFreePicture(dpy, image->icon);
		image->icon = None;
		image->icw = 0;
		image->ich = 0;
		image->iconpath[0] = '\0';
		imagebuffer[least].atime = 0;
	}

	if (!loadimagefromfile(image, path))
		return NULL;

	imagebuffer[least].atime = time(NULL);
	return image;
}

int
loadimagefromfile(Image *image, char *path)
{
	Imlib_Image im;
	int w, h, s, ich, icw;

	struct stat stbuf;
	s = stat(path, &stbuf);
	if (s == -1 || S_ISDIR(s) || strlen(path) <= 2)
		return 0; /* no readable file */

	strlcpy(image->iconpath, path, sizeof image->iconpath);
	im = imlib_load_image_immediately_without_cache(path);
	if (!im) {
		return 0; /* corrupt or otherwise not loadable file */
	}

	imlib_context_set_image(im);
	imlib_image_set_has_alpha(1);
	icw = w = imlib_image_get_width();
	ich = h = imlib_image_get_height();

	if (h >= bh) {
		icw = w * ((float)(bh) / (float)h);
		ich = bh;
	}

	image->icon = drw_picture_create_resized_image(drw, im, w, h, icw, ich);

	imlib_context_set_image(im);
	imlib_free_image_and_decache();

	image->icw = icw;
	image->ich = ich;

	return image->icon;
}

void
cleanup2dimagebuffer(void)
{
	int i;

	for (i = 0; i < LENGTH(imagebuffer); i++) {
		if (imagebuffer[i].image.icon != None) {
			XRenderFreePicture(dpy, imagebuffer[i].image.icon);
		}
	}
}
