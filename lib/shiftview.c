void
shiftview(const Arg *arg)
{
	Workspace *ws = WS;
	Arg shifted;
	unsigned int seltags = ws->tags & ~SPTAGMASK;

	if (arg->i > 0) // left circular shift
		shifted.ui = (seltags << arg->i)
		   | (seltags >> (NUMTAGS - arg->i));
	else // right circular shift
		shifted.ui = seltags >> -arg->i
		   | seltags << (NUMTAGS + arg->i);

	view(&shifted);
}
