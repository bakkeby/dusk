/* TODO: SPTAGMASK */

void
tagtoleft(const Arg *arg)
{
	Workspace *ws = WS;
	if (ws->sel != NULL
	&& __builtin_popcount(ws->tags & TAGMASK) == 1
	&& ws->tags > 1) {
		ws->sel->tags >>= 1;
		focus(NULL);
		arrange(ws->mon);
	}
}

void
tagtoright(const Arg *arg)
{
	Workspace *ws = WS;
	if (ws->sel != NULL
	&& __builtin_popcount(ws->tags & TAGMASK) == 1
	&& ws->tags & (TAGMASK >> 1)) {
		ws->sel->tags <<= 1;
		focus(NULL);
		arrange(ws->mon);
	}
}

void
viewtoleft(const Arg *arg)
{
	Workspace *ws = WS;
	if (__builtin_popcount(ws->tags & TAGMASK) == 1
	&& ws->tags > 1) {
		ws->prevtags = ws->tags;
		ws->tags >>= 1;
		focus(NULL);
		arrange(ws->mon);
	}
}

void
viewtoright(const Arg *arg)
{
	Workspace *ws = WS;
	if (__builtin_popcount(ws->tags & TAGMASK) == 1
	&& ws->tags & (TAGMASK >> 1)) {
		ws->prevtags = ws->tags;
		ws->tags <<= 1;
		focus(NULL);
		arrange(ws->mon);
	}
}

void
tagandviewtoleft(const Arg *arg)
{
	Workspace *ws = WS;
	if (__builtin_popcount(ws->tags & TAGMASK) == 1
	&& ws->tags > 1) {
		ws->sel->tags >>= 1;
		ws->prevtags = ws->tags;
		ws->tags >>= 1;
		focus(ws->sel);
		arrange(ws->mon);
	}
}

void
tagandviewtoright(const Arg *arg)
{
	Workspace *ws = WS;
	if (__builtin_popcount(ws->tags & TAGMASK) == 1
	&& ws->tags & (TAGMASK >> 1)) {
		ws->sel->tags <<= 1;
		ws->prevtags = ws->tags;
		ws->tags <<= 1;
		focus(ws->sel);
		arrange(ws->mon);
	}
}