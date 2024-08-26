
/* Indicator types */
enum {
	IndicatorWs = 0, // occupied alias for backwards support, may be removed in the future
	IndicatorWsOcc = 0,
	IndicatorWsSel,
	IndicatorWsVis,
	IndicatorWsNorm,
	IndicatorPinnedWs,
	IndicatorFakeFullScreen,
	IndicatorFakeFullScreenActive,
	IndicatorFloatFakeFullScreen,
	IndicatorFloatFakeFullScreenActive,
	IndicatorTiled,
	IndicatorFloating,
	IndicatorLast,
};

/* Indicator options */
enum {
	INDICATOR_NONE,
	INDICATOR_TOP_LEFT_SQUARE,
	INDICATOR_TOP_LEFT_LARGER_SQUARE,
	INDICATOR_TOP_RIGHT_TRIANGLE,
	INDICATOR_TOP_RIGHT_PIN,
	INDICATOR_TOP_BAR,
	INDICATOR_TOP_BAR_SLIM,
	INDICATOR_TOP_CENTERED_DOT,
	INDICATOR_BOTTOM_BAR,
	INDICATOR_BOTTOM_BAR_SLIM,
	INDICATOR_BOTTOM_BAR_SLIM_DOTS,
	INDICATOR_BOTTOM_CENTERED_DOT,
	INDICATOR_BOX,
	INDICATOR_BOX_WIDER,
	INDICATOR_BOX_FULL,
	INDICATOR_CLIENT_DOTS,
	INDICATOR_PLUS,
	INDICATOR_PLUS_AND_SQUARE,
	INDICATOR_PLUS_AND_LARGER_SQUARE,
	INDICATOR_CUSTOM_1,
	INDICATOR_CUSTOM_2,
	INDICATOR_CUSTOM_3,
	INDICATOR_CUSTOM_4,
	INDICATOR_CUSTOM_5,
	INDICATOR_CUSTOM_6,
};

static void drawindicator(Workspace *ws, Client *c, unsigned int occ, int x, int y, int w, int h, int filled, int invert, int type);
static void drawstateindicator(Workspace *ws, Client *c, unsigned int occ, int x, int y, int w, int h, int filled, int invert);
