void
shiftview(const Arg *arg)
{
	Arg shifted;
	unsigned int seltagset = selmon->tagset[selmon->seltags] & ~SPTAGMASK;

	if (arg->i > 0) // left circular shift
		shifted.ui = (seltagset << arg->i)
		   | (seltagset >> (NUMTAGS - arg->i));
	else // right circular shift
		shifted.ui = seltagset >> -arg->i
		   | seltagset << (NUMTAGS + arg->i);

	view(&shifted);
}
