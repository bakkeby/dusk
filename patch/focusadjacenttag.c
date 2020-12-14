/* TODO: SPTAGMASK */

void
tagtoleft(const Arg *arg)
{
	if (selws->sel != NULL
	&& __builtin_popcount(selmon->tagset[selws->seltags] & TAGMASK) == 1
	&& selmon->tagset[selws->seltags] > 1) {
		selws->sel->tags >>= 1;
		focus(NULL);
		arrange(selws);
	}
}

void
tagtoright(const Arg *arg)
{
	if (selws->sel != NULL
	&& __builtin_popcount(selmon->tagset[selws->seltags] & TAGMASK) == 1
	&& selmon->tagset[selws->seltags] & (TAGMASK >> 1)) {
		selws->sel->tags <<= 1;
		focus(NULL);
		arrange(selws);
	}
}

void
viewtoleft(const Arg *arg)
{
	if (__builtin_popcount(selmon->tagset[selws->seltags] & TAGMASK) == 1
	&& selmon->tagset[selws->seltags] > 1) {
		selws->seltags ^= 1; /* toggle sel tagset */
		selmon->tagset[selws->seltags] = selmon->tagset[selws->seltags ^ 1] >> 1;
		focus(NULL);
		arrange(selws);
	}
}

void
viewtoright(const Arg *arg)
{
	if (__builtin_popcount(selmon->tagset[selws->seltags] & TAGMASK) == 1
	&& selmon->tagset[selws->seltags] & (TAGMASK >> 1)) {
		selws->seltags ^= 1; /* toggle sel tagset */
		selmon->tagset[selws->seltags] = selmon->tagset[selws->seltags ^ 1] << 1;
		focus(NULL);
		arrange(selws);
	}
}

void
tagandviewtoleft(const Arg *arg)
{
	if (__builtin_popcount(selmon->tagset[selws->seltags] & TAGMASK) == 1
	&& selmon->tagset[selws->seltags] > 1) {
		selws->sel->tags >>= 1;
		selws->seltags ^= 1; /* toggle sel tagset */
		selmon->tagset[selws->seltags] = selmon->tagset[selws->seltags ^ 1] >> 1;
		focus(selws->sel);
		arrange(selws);
	}
}

void
tagandviewtoright(const Arg *arg)
{
	if (__builtin_popcount(selmon->tagset[selws->seltags] & TAGMASK) == 1
	&& selmon->tagset[selws->seltags] & (TAGMASK >> 1)) {
		selws->sel->tags <<= 1;
		selws->seltags ^= 1; /* toggle sel tagset */
		selmon->tagset[selws->seltags] = selmon->tagset[selws->seltags ^ 1] << 1;
		focus(selws->sel);
		arrange(selws);
	}
}