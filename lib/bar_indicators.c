/* Indicator properties, you can override these in your config.h if you want. */
void
drawindicator(Monitor *m, Client *c, unsigned int occ, int x, int y, int w, int h, int filled, int invert, int type)
{
	Workspace *ws = MWS(m);
	int boxw, boxs, indn = 0;
	if (!occ || type == INDICATOR_NONE)
		return;

	boxs = drw->fonts->h / 9;
	boxw = drw->fonts->h / 6 + 2;
	if (filled == -1)
		filled = m == selmon;

	switch (type) {
	default:
	case INDICATOR_TOP_LEFT_SQUARE:
		drw_rect(drw, x + boxs, y + boxs, boxw, boxw, filled, invert);
		break;
	case INDICATOR_TOP_LEFT_LARGER_SQUARE:
		drw_rect(drw, x + boxs + 2, y + boxs+1, boxw+1, boxw+1, filled, invert);
		break;
	case INDICATOR_TOP_BAR:
		drw_rect(drw, x + boxw, y, w - ( 2 * boxw + 1), boxw/2, filled, invert);
		break;
	case INDICATOR_TOP_BAR_SLIM:
		drw_rect(drw, x + boxw, y, w - ( 2 * boxw + 1), 1, 0, invert);
		break;
	case INDICATOR_BOTTOM_BAR:
		drw_rect(drw, x + boxw, y + h - boxw/2, w - ( 2 * boxw + 1), boxw/2, filled, invert);
		break;
	case INDICATOR_BOTTOM_BAR_SLIM:
		drw_rect(drw, x + boxw, y + h - 1, w - ( 2 * boxw + 1), 1, 0, invert);
		break;
	case INDICATOR_BOX:
		drw_rect(drw, x + boxw, y, w - 2 * boxw, h, 0, invert);
		break;
	case INDICATOR_BOX_WIDER:
		drw_rect(drw, x + boxw/2, y, w - boxw, h, 0, invert);
		break;
	case INDICATOR_BOX_FULL:
		drw_rect(drw, x, y, w - 2, h, 0, invert);
		break;
	case INDICATOR_CLIENT_DOTS:
		for (c = ws->clients; c; c = c->next) {
			drw_rect(drw, x, 1 + (indn * 2), ws->sel == c ? 6 : 1, 1, 1, invert);
			indn++;
			if (h <= 1 + (indn * 2)) {
				indn = 0;
				x += 2;
			}
		}
		break;
	case INDICATOR_PLUS_AND_LARGER_SQUARE:
		boxs += 2;
		boxw += 2;
		/* falls through */
	case INDICATOR_PLUS_AND_SQUARE:
		drw_rect(drw, x + boxs, y + boxs, boxw % 2 ? boxw : boxw + 1, boxw % 2 ? boxw : boxw + 1, filled, invert);
		/* falls through */
	case INDICATOR_PLUS:
		if (!(boxw % 2))
			boxw += 1;
		drw_rect(drw, x + boxs + boxw / 2, y + boxs, 1, boxw, filled, invert); // |
		drw_rect(drw, x + boxs, y + boxs + boxw / 2, boxw + 1, 1, filled, invert); // ‒
		break;
	}
}

void
drawstateindicator(Monitor *m, Client *c, unsigned int occ, int x, int y, int w, int h, int filled, int invert)
{
	if (ISFAKEFULLSCREEN(c) && ISFLOATING(c))
		drawindicator(m, c, occ, x, y, w, h, filled, invert, floatfakefsindicatortype);
	else if (ISFAKEFULLSCREEN(c))
		drawindicator(m, c, occ, x, y, w, h, filled, invert, fakefsindicatortype);
	else if (ISFLOATING(c))
		drawindicator(m, c, occ, x, y, w, h, filled, invert, floatindicatortype);
	else
		drawindicator(m, c, occ, x, y, w, h, filled, invert, tiledindicatortype);
}