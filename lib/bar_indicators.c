/* Indicator properties, you can override these in your config.h if you want. */
void
drawindicator(Workspace *ws, Client *c, unsigned int occ, int x, int y, int w, int h, int filled, int invert, int type)
{
	int boxw, boxs, indn = 0, i;
	if (!occ || type == INDICATOR_NONE)
		return;
	boxs = drw->fonts->h / 9;
	boxw = drw->fonts->h / 6 + 2;
	if (filled == -1)
		filled = ws->mon == selmon;

	switch (type) {
	default:
	case INDICATOR_TOP_LEFT_SQUARE:
		drw_rect(drw, x + boxs, y + boxs, boxw, boxw, filled, invert);
		break;
	case INDICATOR_TOP_LEFT_LARGER_SQUARE:
		drw_rect(drw, x + boxs + 2, y + boxs+1, boxw+1, boxw+1, filled, invert);
		break;
	case INDICATOR_TOP_RIGHT_TRIANGLE:
		for (i = boxw; i > 0; --i)
			drw_rect(drw, x + w - i, y + (boxw - i), i, 1, filled, invert);
		break;
	case INDICATOR_TOP_RIGHT_PIN:
		drw_rect(drw, x + w - 2 * boxs, y + boxs, boxs, boxs, 1, invert);
		break;
	case INDICATOR_TOP_CENTERED_DOT:
		drw_rect(drw, x + w/2, y, 1, 2, 0, 0);
		break;
	case INDICATOR_TOP_BAR:
		drw_rect(drw, x + boxw, y, w - ( 2 * boxw ), boxw/2, filled, invert);
		break;
	case INDICATOR_TOP_BAR_SLIM:
		drw_rect(drw, x + boxw, y, w - ( 2 * boxw ) + 1, 1, 0, invert);
		break;
	case INDICATOR_BOTTOM_BAR:
		drw_rect(drw, x + boxw, y + h - boxw/2, w - ( 2 * boxw ), boxw/2, filled, invert);
		break;
	case INDICATOR_BOTTOM_BAR_SLIM:
		drw_rect(drw, x + boxw, y + h - 1, w - ( 2 * boxw ) + 1, 1, 0, invert);
		break;
	case INDICATOR_BOTTOM_BAR_SLIM_DOTS:
		drw_rect(drw, x + boxw, y + h - 2, 1, 2, 0, 0);
		drw_rect(drw, x + w - boxw, y + h - 2, 1, 2, 0, 0);
		break;
	case INDICATOR_BOTTOM_CENTERED_DOT:
		drw_rect(drw, x + w/2, y + h - 1, 2, 1, 0, 0);
		break;
	case INDICATOR_BOX:
		drw_rect(drw, x + boxw, y, w - 2 * boxw, h, 0, invert);
		break;
	case INDICATOR_BOX_WIDER:
		drw_rect(drw, x + boxw/2, y, w - boxw, h, 0, invert);
		break;
	case INDICATOR_BOX_FULL:
		drw_rect(drw, x, y, w, h, 0, invert);
		break;
	case INDICATOR_CLIENT_DOTS:
		for (c = ws->clients; c; c = c->next) {
			if (ISINVISIBLE(c))
				continue;
			drw_rect(drw, x, y + 1 + (indn * 2), ws->sel == c ? 6 : 1, 1, 1, invert);
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
	case INDICATOR_CUSTOM_1:
		drw_2dtext(drw, x, y, w, h, 0, CFG(custom_2d_indicator_1), invert, 0, SchemeNorm);
		break;
	case INDICATOR_CUSTOM_2:
		drw_2dtext(drw, x, y, w, h, 0, CFG(custom_2d_indicator_2), invert, 0, SchemeNorm);
		break;
	case INDICATOR_CUSTOM_3:
		drw_2dtext(drw, x, y, w, h, 0, CFG(custom_2d_indicator_3), invert, 0, SchemeNorm);
		break;
	case INDICATOR_CUSTOM_4:
		drw_2dtext(drw, x, y, w, h, 0, CFG(custom_2d_indicator_4), invert, 0, SchemeNorm);
		break;
	case INDICATOR_CUSTOM_5:
		drw_2dtext(drw, x, y, w, h, 0, CFG(custom_2d_indicator_5), invert, 0, SchemeNorm);
		break;
	case INDICATOR_CUSTOM_6:
		drw_2dtext(drw, x, y, w, h, 0, CFG(custom_2d_indicator_6), invert, 0, SchemeNorm);
		break;
	}
}

void
drawstateindicator(Workspace *ws, Client *c, unsigned int occ, int x, int y, int w, int h, int filled, int invert)
{
	int indicator = 0;

	if (c == selws->sel)
		indicator = IndicatorSelected;
	else if (ISFULLSCREEN(c) && ISFAKEFULLSCREEN(c))
		indicator = ISTILED(c) ? IndicatorFakeFullScreenActive : IndicatorFloatFakeFullScreenActive;
	else if (ISFAKEFULLSCREEN(c))
		indicator = ISTILED(c) ? IndicatorFakeFullScreen : IndicatorFloatFakeFullScreen;
	else if (ISTILED(c))
		indicator = IndicatorTiled;
	else
		indicator = IndicatorFloating;

	drawindicator(ws, c, occ, x, y, w, h, filled, invert, indicators[indicator]);
}
