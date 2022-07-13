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
drw_2dtext(Drw *drw, int x, int y, unsigned int w, unsigned int h, unsigned int lpad, const char *text2d, int invert, int drawbg, int defscheme)
{
	if (!w && drawbg)
		return 0;

	int i, tw, dx = x, len, mw = w - 2 * lpad;
	int rx, ry, rw, rh;
	int fillbg = drawbg;
	short isCode = 0;
	char *text = {0};
	char *p = {0};
	Clr oldbg = scheme[defscheme][ColFg];
	Clr oldfg = scheme[defscheme][ColBg];
	len = strlen(text2d) + 1;
	if (!(text = (char*) malloc(sizeof(char)*(len))))
		die("malloc");
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
			if (len <= 0)
				break;

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
	int i, sid = args[0].i;

	if (sid < 0 || sid >= NUM_STATUSES)
		return;

	char const *statustext = args[1].v;

	for (i = 0; i < STATUS_BUFFER - 1 && statustext[i] != '\0'; i++)
		rawstatustext[sid][i] = statustext[i];
	rawstatustext[sid][i] = '\0';

	for (int r = 0; r < LENGTH(barrules); r++) {
		br = &barrules[r];
		if (br->value == sid && br->drawfunc == draw_status)
			drawbarmodule(br, r);
	}
}

int
status2dtextlength(char* text2d)
{
	int i, w, len;
	short isCode = 0;
	char *text = {0};
	char *p = {0};

	len = strlen(text2d) + 1;
	if (!(text = (char*) malloc(sizeof(char)*len)))
		die("malloc");
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
				if (text[++i] == 'f')
					w += atoi(text + ++i);
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
