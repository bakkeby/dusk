void
setcfact(const Arg *arg)
{
	float f;
	Client *c;
	Workspace *ws = WS;

	c = ws->sel;

	if (!arg || !c || !ws->layout->arrange)
		return;
	if (!arg->f)
		f = 1.0;
	else if (arg->f > 4.0) // set fact absolutely
		f = arg->f - 4.0;
	else
		f = arg->f + c->cfact;
	if (f < 0.25)
		f = 0.25;
	else if (f > 4.0)
		f = 4.0;
	c->cfact = f;
	arrangemon(ws->mon);
}