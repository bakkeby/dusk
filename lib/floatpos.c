void
floatpos(const Arg *arg)
{
	Workspace *ws = selws;
	Client *c = ws->sel;

	if (!c)
		return;

	setfloatpos(c, (char *)arg->v, 1, 1);
	resizeclient(c, c->x, c->y, c->w, c->h);
	savefloats(c);

	raiseclient(c);
	XWarpPointer(dpy, None, c->win, 0, 0, 0, 0, c->w/2, c->h/2);
}

void
setfloatpos(Client *c, const char *floatpos, const int auto_float, const int save_floats)
{
	char xCh, yCh, wCh, hCh;
	int n, x, y, w, h, wx, ww, wy, wh;
	int oh = 0, ov = 0, ih = 0, iv = 0;

	if (!c || !floatpos)
		return;

	switch (sscanf(floatpos, "%d%c %d%c %d%c %d%c", &x, &xCh, &y, &yCh, &w, &wCh, &h, &hCh)) {
		case 4:
			if (xCh == 'w' || xCh == 'W') {
				w = x; wCh = xCh;
				h = y; hCh = yCh;
				x = -1; xCh = 'C';
				y = -1; yCh = 'C';
			} else if (xCh == 'p' || xCh == 'P') {
				w = x; wCh = xCh;
				h = y; hCh = yCh;
				x = 0; xCh = 'G';
				y = 0; yCh = 'G';
			} else if (xCh == 'm' || xCh == 'M') {
				getrootptr(&x, &y);
			} else {
				w = 0; wCh = 0;
				h = 0; hCh = 0;
			}
			break;
		case 8:
			if (xCh == 'm' || xCh == 'M')
				getrootptr(&x, &y);
			break;
		default:
			return;
	}

	if (auto_float && c->ws->layout->arrange && ISTILED(c)) {
		addflag(c, MoveResize);
		togglefloating(&((Arg) { .v = c }));
		removeflag(c, MoveResize);
	}

	if (enabled(SnapToGaps))
		getgaps(c->ws, &oh, &ov, &ih, &iv, &n);

	wx = c->ws->wx + ov;
	wy = c->ws->wy + oh;
	ww = c->ws->ww - 2*ov;
	wh = c->ws->wh - 2*oh;

	addflag(c, IgnoreSizeHints);

	if (FREEFLOW(c)) {
		getfloatpos(x, xCh, w, wCh, wx, ww, c->x, c->w, c->bw, floatposgrid_x, &c->x, &c->w);
		getfloatpos(y, yCh, h, hCh, wy, wh, c->y, c->h, c->bw, floatposgrid_y, &c->y, &c->h);
		if (save_floats) {
			c->sfx = c->x;
			c->sfy = c->y;
			c->sfw = c->w;
			c->sfh = c->h;
		}
	} else {
		getfloatpos(x, xCh, w, wCh, wx, ww, c->x, c->w, c->bw, floatposgrid_x, &c->sfx, &c->sfw);
		getfloatpos(y, yCh, h, hCh, wy, wh, c->y, c->h, c->bw, floatposgrid_y, &c->sfy, &c->sfh);
	}
}

/* p - position, s - size, cp and cs represents current position and size */
void
getfloatpos(int pos, char pCh, int size, char sCh, int min_p, int max_s, int cp, int cs, int cbw, int defgrid, int *out_p, int *out_s)
{
	int abs_p, abs_s, i, delta, rest;

	abs_p = pCh == 'A' || pCh == 'a';
	abs_s = sCh == 'A' || sCh == 'a';

	cs += 2*cbw;

	switch (pCh) {
	case 'A': // absolute position
		cp = pos;
		break;
	case 'a': // absolute relative position
		cp += pos;
		break;
	case 'y':
	case 'x': // client relative position
		cp = MIN(cp + pos, min_p + max_s);
		break;
	case 'Y':
	case 'X': // client position relative to monitor
		cp = min_p + MIN(pos, max_s);
		break;
	case 'S': // fixed client position (sticky)
	case 'C': // fixed client position (center)
	case 'Z': // fixed client right-hand position (position + size)
		if (pos == -1)
			break;
		pos = MAX(MIN(pos, max_s), 0);
		if (pCh == 'Z')
			cs = abs((cp + cs) - (min_p + pos));
		else if (pCh == 'C')
			cs = abs((cp + cs / 2) - (min_p + pos));
		else
			cs = abs(cp - (min_p + pos));
		cp = min_p + pos;
		sCh = 0; // size determined by position, override defined size
		break;
	case 'G': // grid
		if (pos <= 0)
			pos = defgrid; // default configurable
		if (size == 0 || pos < 2 || (sCh != 'p' && sCh != 'P'))
			break;
		delta = (max_s - cs) / (pos - 1);
		rest = max_s - cs - delta * (pos - 1);
		if (sCh == 'P') {
			if (size < 1 || size > pos)
				break;
			cp = min_p + delta * (size - 1);
		} else {
			for (i = 0; i < pos && cp >= min_p + delta * i + (i > pos - rest ? i + rest - pos + 1 : 0); i++);
			cp = min_p + delta * (MAX(MIN(i + size, pos), 1) - 1) + (i > pos - rest ? i + rest - pos + 1 : 0);
		}
		break;
	}

	switch (sCh) {
	case 'A': // absolute size
		cs = size;
		break;
	case 'a': // absolute relative size
		cs = MAX(1, cs + size);
		break;
	case '%': // client size percentage in relation to monitor window area size
		if (size <= 0)
			break;
		size = max_s * MIN(size, 100) / 100;
		/* falls through */
	case 'h':
	case 'w': // size relative to client
		if (sCh == 'w' || sCh == 'h') {
			if (size == 0)
				break;
			size += cs;
		}
		/* falls through */
	case 'H':
	case 'W': // normal size, position takes precedence
		if (pCh == 'S' && cp + size > min_p + max_s)
			size = min_p + max_s - cp;
		else if (size > max_s)
			size = max_s;

		if (pCh == 'C') { // fixed client center, expand or contract client
			delta = size - cs;
			if (delta < 0 || (cp - delta / 2 + size <= min_p + max_s))
				cp -= delta / 2;
			else if (cp - delta / 2 < min_p)
				cp = min_p;
			else if (delta)
				cp = min_p + max_s;
		} else if (pCh == 'Z')
			cp -= size - cs;

		cs = size;
		break;
	}

	if (pCh == '%') // client mid-point position in relation to monitor window area size
		cp = min_p + max_s * MAX(MIN(pos, 100), 0) / 100 - (cs) / 2;
	if (pCh == 'm' || pCh == 'M')
		cp = pos - cs / 2;

	if (!abs_p && cp < min_p)
		cp = min_p;
	if (cp + cs > min_p + max_s && !(abs_p && abs_s)) {
		if (abs_p || cp == min_p)
			cs = min_p + max_s - cp;
		else
			cp = min_p + max_s - cs;
	}

	*out_p = cp;
	*out_s = MAX(cs - 2*cbw, 1);
}
