/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xft/Xft.h>

#include "drw.h"
#include "util.h"

#define UTF_INVALID 0xFFFD
#define UTF_SIZ     4

static const unsigned char utfbyte[UTF_SIZ + 1] = {0x80,    0, 0xC0, 0xE0, 0xF0};
static const unsigned char utfmask[UTF_SIZ + 1] = {0xC0, 0x80, 0xE0, 0xF0, 0xF8};
static const long utfmin[UTF_SIZ + 1] = {       0,    0,  0x80,  0x800,  0x10000};
static const long utfmax[UTF_SIZ + 1] = {0x10FFFF, 0x7F, 0x7FF, 0xFFFF, 0x10FFFF};
static const unsigned int alpha_default[] = { 0xffU, 0xd0U, 0xffU };
unsigned int elipsis_width = 0;

static long
utf8decodebyte(const char c, size_t *i)
{
	for (*i = 0; *i < (UTF_SIZ + 1); ++(*i))
		if (((unsigned char)c & utfmask[*i]) == utfbyte[*i])
			return (unsigned char)c & ~utfmask[*i];
	return 0;
}

static size_t
utf8validate(long *u, size_t i)
{
	if (!BETWEEN(*u, utfmin[i], utfmax[i]) || BETWEEN(*u, 0xD800, 0xDFFF))
		*u = UTF_INVALID;
	for (i = 1; *u > utfmax[i]; ++i)
		;
	return i;
}

static size_t
utf8decode(const char *c, long *u, size_t clen)
{
	size_t i, j, len, type;
	long udecoded;

	*u = UTF_INVALID;
	if (!clen)
		return 0;
	udecoded = utf8decodebyte(c[0], &len);
	if (!BETWEEN(len, 1, UTF_SIZ))
		return 1;
	for (i = 1, j = 1; i < clen && j < len; ++i, ++j) {
		udecoded = (udecoded << 6) | utf8decodebyte(c[i], &type);
		if (type)
			return j;
	}
	if (j < len)
		return 0;
	*u = udecoded;
	utf8validate(u, len);

	return len;
}

Drw *
drw_create(Display *dpy, int screen, Window root, unsigned int w, unsigned int h, Visual *visual, unsigned int depth, Colormap cmap)
{
	Drw *drw = ecalloc(1, sizeof(Drw));

	drw->dpy = dpy;
	drw->screen = screen;
	drw->root = root;
	drw->w = w;
	drw->h = h;

	drw->visual = visual;
	drw->depth = depth;
	drw->cmap = cmap;
	drw->drawable = XCreatePixmap(dpy, root, w, h, depth);
	drw->picture = XRenderCreatePicture(dpy, drw->drawable, XRenderFindVisualFormat(dpy, visual), 0, NULL);
	drw->gc = XCreateGC(dpy, drw->drawable, 0, NULL);
	XSetLineAttributes(dpy, drw->gc, 1, LineSolid, CapButt, JoinMiter);

	return drw;
}

void
drw_resize(Drw *drw, unsigned int w, unsigned int h)
{
	if (!drw)
		return;

	drw->w = w;
	drw->h = h;
	if (drw->picture)
		XRenderFreePicture(drw->dpy, drw->picture);
	if (drw->drawable)
		XFreePixmap(drw->dpy, drw->drawable);
	drw->drawable = XCreatePixmap(drw->dpy, drw->root, w, h, drw->depth);
	drw->picture = XRenderCreatePicture(drw->dpy, drw->drawable, XRenderFindVisualFormat(drw->dpy, drw->visual), 0, NULL);
}

void
drw_free(Drw *drw)
{
	XRenderFreePicture(drw->dpy, drw->picture);
	XFreePixmap(drw->dpy, drw->drawable);
	XFreeGC(drw->dpy, drw->gc);
	drw_fontset_free(drw->fonts);
	free(drw);
}

/* This function is an implementation detail. Library users should use
 * drw_fontset_create instead.
 */
static Fnt *
xfont_create(Drw *drw, const char *fontname, FcPattern *fontpattern)
{
	Fnt *font;
	XftFont *xfont = NULL;
	FcPattern *pattern = NULL;

	if (fontname) {
		/* Using the pattern found at font->xfont->pattern does not yield the
		 * same substitution results as using the pattern returned by
		 * FcNameParse; using the latter results in the desired fallback
		 * behaviour whereas the former just results in missing-character
		 * rectangles being drawn, at least with some fonts. */
		if (!(xfont = XftFontOpenName(drw->dpy, drw->screen, fontname))) {
			fprintf(stderr, "error, cannot load font from name: '%s'\n", fontname);
			return NULL;
		}
		if (!(pattern = FcNameParse((FcChar8 *) fontname))) {
			fprintf(stderr, "error, cannot parse font name to pattern: '%s'\n", fontname);
			XftFontClose(drw->dpy, xfont);
			return NULL;
		}
	} else if (fontpattern) {
		if (!(xfont = XftFontOpenPattern(drw->dpy, fontpattern))) {
			fprintf(stderr, "error, cannot load font from pattern.\n");
			return NULL;
		}
	} else {
		die("no font specified.");
	}

	/* Do not allow using color fonts. This is a workaround for a BadLength
	 * error from Xft with color glyphs. Modelled on the Xterm workaround. See
	 * https://bugzilla.redhat.com/show_bug.cgi?id=1498269
	 * https://lists.suckless.org/dev/1701/30932.html
	 * https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=916349
	 * and lots more all over the internet.
	 */
	if (disabled(ColorEmoji)) {
		FcBool iscol;
		if (FcPatternGetBool(xfont->pattern, FC_COLOR, 0, &iscol) == FcResultMatch && iscol) {
			XftFontClose(drw->dpy, xfont);
			return NULL;
		}
	}

	font = ecalloc(1, sizeof(Fnt));
	font->xfont = xfont;
	font->pattern = pattern;
	font->h = xfont->ascent + xfont->descent;
	font->dpy = drw->dpy;

	return font;
}

static void
xfont_free(Fnt *font)
{
	if (!font)
		return;
	if (font->pattern)
		FcPatternDestroy(font->pattern);
	XftFontClose(font->dpy, font->xfont);
	free(font);
}

Fnt*
drw_fontset_create(Drw* drw, const char *fonts[], size_t fontcount)
{
	Fnt *cur, *ret = NULL;
	size_t i;

	if (!drw || !fonts)
		return NULL;

	for (i = 1; i <= fontcount; i++) {
		if ((cur = xfont_create(drw, fonts[fontcount - i], NULL))) {
			cur->next = ret;
			ret = cur;
		}
	}
	return (drw->fonts = ret);
}

void
drw_fontset_free(Fnt *font)
{
	if (font) {
		drw_fontset_free(font->next);
		xfont_free(font);
	}
}

void
drw_clr_create(
	Drw *drw,
	Clr *dest,
	const char *clrname,
	unsigned int alpha
) {
	if (!drw || !dest || !clrname)
		return;

	if (!XftColorAllocName(drw->dpy, drw->visual, drw->cmap, clrname, dest))
		fprintf(stderr, "error, cannot allocate color '%s'\n", clrname);

	dest->pixel = (dest->pixel & 0x00ffffffU) | (alpha << 24);
}

/* Wrapper to create color schemes. The caller has to call free(3) on the
 * returned color scheme when done using it. */
Clr *
drw_scm_create(
	Drw *drw,
	char *clrnames[],
	const unsigned int alphas[],
	size_t clrcount
) {
	int hasalpha = 0;
	size_t i;
	Clr *ret;

	/* need at least two colors for a scheme */
	if (!drw || !clrnames || clrcount < 2 || !(ret = ecalloc(clrcount, sizeof(XftColor))))
		return NULL;

	for (i = 0; i < clrcount; i++)
		hasalpha |= alphas[i];

	for (i = 0; i < clrcount; i++)
		drw_clr_create(drw, &ret[i], clrnames[i],  hasalpha ? alphas[i] : alpha_default[i]);
	return ret;
}

void
drw_setfontset(Drw *drw, Fnt *set)
{
	if (drw)
		drw->fonts = set;
}

void
drw_setscheme(Drw *drw, Clr *scm)
{
	if (drw)
		drw->scheme = scm;
}

void
drw_rect(Drw *drw, int x, int y, unsigned int w, unsigned int h, int filled, int invert)
{
	if (!drw || !drw->scheme)
		return;
	XSetForeground(drw->dpy, drw->gc, invert ? drw->scheme[ColBg].pixel : drw->scheme[ColFg].pixel);
	if (filled)
		XFillRectangle(drw->dpy, drw->drawable, drw->gc, x, y, w, h);
	else
		XDrawRectangle(drw->dpy, drw->drawable, drw->gc, x, y, w - 1, h - 1);
}

int
drw_text(Drw *drw, int x, int y, unsigned int w, unsigned int h, unsigned int lpad, const char *text, int invert, Bool ignored, int fillbg)
{
	char buf[1024] = {0};
	int ty, stop = 0, charexists = 0;
	unsigned int ew = 0, elipsis_ew = 0;
	XftDraw *d = NULL;
	Fnt *usedfont, *curfont, *nextfont;
	size_t i, b = 0, elipsis_b = 0;
	int utf8charlen, render = x || y || w || h;
	long utf8codepoint = 0;
	FcCharSet *fccharset;
	FcPattern *fcpattern;
	FcPattern *match;
	XftResult result;
	XGlyphInfo ext;

	if (!drw || (render && !drw->scheme) || !text || !drw->fonts)
		return 0;

	if (!render) {
		w = ~w;
	} else {
		if (fillbg) {
			XSetForeground(drw->dpy, drw->gc, drw->scheme[invert ? ColFg : ColBg].pixel);
			XFillRectangle(drw->dpy, drw->drawable, drw->gc, x, y, w, h);
		}
		d = XftDrawCreate(drw->dpy, drw->drawable, drw->visual, drw->cmap);
		x += lpad;
		w -= lpad;
	}

	usedfont = drw->fonts;
	while (1) {
		elipsis_ew = ew = 0;
		elipsis_b = b = 0;
		nextfont = NULL;

		while (*text) {
			utf8charlen = utf8decode(text, &utf8codepoint, UTF_SIZ);
			for (curfont = drw->fonts; curfont; curfont = curfont->next) {
				charexists = charexists || XftCharExists(drw->dpy, curfont->xfont, utf8codepoint);
				if (charexists) {
					if (curfont == usedfont) {
						XftTextExtentsUtf8(curfont->dpy, curfont->xfont, (XftChar8 *)text, utf8charlen, &ext);
						if (ew + ext.xOff > w || b + utf8charlen > sizeof(buf) - 1) {
							/* Only draw elipsis if we have not recently started another font */
							if (elipsis_b > 3) {
								ew = elipsis_ew;
								b = elipsis_b;
								for (i = 0; i < 3; i++)
									buf[b++] = '.';
							}
							stop = 1;
							break;
						}

						/* Record the last buffer index where the elipsis would still fit */
						if (ew + elipsis_width <= w) {
							elipsis_ew = ew;
							elipsis_b = b;
						}
						for (i = 0; i < utf8charlen; i++)
							buf[b++] = *text++;
						ew += ext.xOff;
					} else {
						nextfont = curfont;
					}
					break;
				}
			}

			if (stop || !charexists || nextfont)
				break;
			charexists = 0;
		}
		buf[b] = '\0';

		if (b) {
			if (render) {
				ty = y + (h - usedfont->h) / 2 + usedfont->xfont->ascent;
				XftDrawStringUtf8(d, &drw->scheme[invert ? ColBg : ColFg],
				                  usedfont->xfont, x, ty, (XftChar8 *)buf, b);
			}
			x += ew;
			w -= ew;
		}

		if (stop || !*text) {
			break;
		} else if (nextfont) {
			charexists = 0;
			usedfont = nextfont;
		} else {
			/* Regardless of whether or not a fallback font is found, the
			 * character must be drawn. */
			charexists = 1;

			fccharset = FcCharSetCreate();
			FcCharSetAddChar(fccharset, utf8codepoint);

			if (!drw->fonts->pattern) {
				/* Refer to the comment in xfont_create for more information. */
				die("the first font in the cache must be loaded from a font string.");
			}

			fcpattern = FcPatternDuplicate(drw->fonts->pattern);
			FcPatternAddCharSet(fcpattern, FC_CHARSET, fccharset);
			FcPatternAddBool(fcpattern, FC_SCALABLE, FcTrue);
			FcPatternAddBool(fcpattern, FC_COLOR, FcFalse);

			FcConfigSubstitute(NULL, fcpattern, FcMatchPattern);
			FcDefaultSubstitute(fcpattern);
			match = XftFontMatch(drw->dpy, drw->screen, fcpattern, &result);

			FcCharSetDestroy(fccharset);
			FcPatternDestroy(fcpattern);

			if (match) {
				usedfont = xfont_create(drw, NULL, match);
				if (usedfont && XftCharExists(drw->dpy, usedfont->xfont, utf8codepoint)) {
					for (curfont = drw->fonts; curfont->next; curfont = curfont->next)
						; /* NOP */
					curfont->next = usedfont;
				} else {
					xfont_free(usedfont);
					usedfont = drw->fonts;
				}
			}
		}
	}
	if (d)
		XftDrawDestroy(d);

	return x + (render ? w : 0);
}

void
drw_arrow(Drw *drw, int x, int y, unsigned int w, unsigned int h, int style, Clr prev, Clr next, Clr separator)
{
	if (!drw || !drw->scheme)
		return;

	int direction = 1;
	int hh = 0, w1 = 0, w2 = 0;

	switch (style) {
	case PwrlRightArrow:
		hh = h / 2;
		w1 = w;
		break;
	case PwrlLeftArrow:
		hh = h / 2;
		x += w;
		w = -w;
		w1 = w;
		direction = 0;
		break;
	case PwrlForwardSlash:
		hh = 0;
		w1 = w;
		break;
	case PwrlBackslash:
		w2 = w;
		hh = h;
		break;
	case PwrlSolid:
		break;
	case PwrlSolidRev:
		direction = 0;
		break;
	}

	if (direction) {
		Clr tmp = next;
		next = prev;
		prev = tmp;
	}

	XPoint points[] = {
		{ x      , y      },
		{ x + w1 , y + hh },
		{ x + w2 , y + h  },
	};

	XPoint bg[] = {
		{ x     , y     },
		{ x + w , y     },
		{ x + w , y + h },
		{ x     , y + h },
	};

	XSetForeground(drw->dpy, drw->gc, prev.pixel);
	XFillPolygon(drw->dpy, drw->drawable, drw->gc, bg, 4, Convex, CoordModeOrigin);
	XSetForeground(drw->dpy, drw->gc, next.pixel);
	XFillPolygon(drw->dpy, drw->drawable, drw->gc, points, 3, Nonconvex, CoordModeOrigin);
}

void
drw_map(Drw *drw, Window win, int x, int y, unsigned int w, unsigned int h)
{
	if (!drw)
		return;

	XCopyArea(drw->dpy, drw->drawable, win, drw->gc, x, y, w, h, x, y);
	XFlush(drw->dpy);
}

unsigned int
drw_fontset_getwidth(Drw *drw, const char *text, Bool markup)
{
	if (!drw || !drw->fonts || !text)
		return 0;
	return drw_text(drw, 0, 0, 0, 0, 0, text, 0, markup, 0);
}

void
drw_font_getexts(Fnt *font, const char *text, unsigned int len, unsigned int *w, unsigned int *h)
{
	XGlyphInfo ext;

	if (!font || !text)
		return;

	XftTextExtentsUtf8(font->dpy, font->xfont, (XftChar8 *)text, len, &ext);
	if (w)
		*w = ext.xOff;
	if (h)
		*h = font->h;
}

Cur *
drw_cur_create(Drw *drw, int shape)
{
	Cur *cur;

	if (!drw || !(cur = ecalloc(1, sizeof(Cur))))
		return NULL;

	cur->cursor = XCreateFontCursor(drw->dpy, shape);

	return cur;
}

void
drw_cur_free(Drw *drw, Cur *cursor)
{
	if (!cursor)
		return;

	XFreeCursor(drw->dpy, cursor->cursor);
	free(cursor);
}
