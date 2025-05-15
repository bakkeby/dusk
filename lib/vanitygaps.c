static void
setgaps(int oh, int ov, int ih, int iv)
{
	if (oh < 0) oh = 0;
	if (ov < 0) ov = 0;
	if (ih < 0) ih = 0;
	if (iv < 0) iv = 0;

	selmon->gappoh = oh;
	selmon->gappov = ov;
	selmon->gappih = ih;
	selmon->gappiv = iv;
	arrangemon(selmon);
}

/* External function that takes one integer and splits it
 * into four gap values:
 *    - outer horizontal (oh)
 *    - outer vertical (ov)
 *    - inner horizontal (ih)
 *    - inner vertical (iv)
 *
 * Each value is represented as one byte with the uppermost
 * bit of each byte indicating whether or not to keep the
 * current value.
 *
 * Example:
 *
 *   10000000   10000000   00001111   00001111
 *   |          |          |          |
 *   + keep oh  + keep ov  + ih 15px  + iv 15px
 *
 * This gives an int of:
 *   10000000100000000000111100001111 = 2155876111
 *
 * Thus this command should set inner gaps to 15:
 *   xsetroot -name "fsignal:setgaps i 2155876111"
 */
static void
setgapsex(const Arg *arg)
{
	Workspace *ws = selws;
	int oh = selmon->gappoh;
	int ov = selmon->gappov;
	int ih = selmon->gappih;
	int iv = selmon->gappiv;

	if (!(arg->i & (1 << 31)))
		oh = (arg->i & 0x7f000000) >> 24;
	if (!(arg->i & (1 << 23)))
		ov = (arg->i & 0x7f0000) >> 16;
	if (!(arg->i & (1 << 15)))
		ih = (arg->i & 0x7f00) >> 8;
	if (!(arg->i & (1 << 7)))
		iv = (arg->i & 0x7f);

	/* Auto enable gaps if disabled */
	if (!ws->enablegaps)
		ws->enablegaps = 1;

	setgaps(oh, ov, ih, iv);
}

static void
togglegaps(const Arg *arg)
{
	Workspace *ws = selws;
	ws->enablegaps = !ws->enablegaps;
	arrange(NULL);
}

static void
defaultgaps(const Arg *arg)
{
	setgaps(gappoh, gappov, gappih, gappiv);
}

static void
incrgaps(const Arg *arg)
{
	setgaps(
		selmon->gappoh + arg->i,
		selmon->gappov + arg->i,
		selmon->gappih + arg->i,
		selmon->gappiv + arg->i
	);
}

static void
incrigaps(const Arg *arg)
{
	setgaps(
		selmon->gappoh,
		selmon->gappov,
		selmon->gappih + arg->i,
		selmon->gappiv + arg->i
	);
}

static void
incrogaps(const Arg *arg)
{
	setgaps(
		selmon->gappoh + arg->i,
		selmon->gappov + arg->i,
		selmon->gappih,
		selmon->gappiv
	);
}

static void
incrohgaps(const Arg *arg)
{
	setgaps(
		selmon->gappoh + arg->i,
		selmon->gappov,
		selmon->gappih,
		selmon->gappiv
	);
}

static void
incrovgaps(const Arg *arg)
{
	setgaps(
		selmon->gappoh,
		selmon->gappov + arg->i,
		selmon->gappih,
		selmon->gappiv
	);
}

static void
incrihgaps(const Arg *arg)
{
	setgaps(
		selmon->gappoh,
		selmon->gappov,
		selmon->gappih + arg->i,
		selmon->gappiv
	);
}

static void
incrivgaps(const Arg *arg)
{
	setgaps(
		selmon->gappoh,
		selmon->gappov,
		selmon->gappih,
		selmon->gappiv + arg->i
	);
}

static void
getgaps(Workspace *ws, int *oh, int *ov, int *ih, int *iv, int *nc)
{
	int oe, ie, n = numtiled(ws);
	oe = ie = ws->enablegaps;

	if (enabled(SmartGaps) && n == 1) {
		oe = smartgaps_fact; // outer gaps disabled or multiplied when only one client
	}

	*oh = ws->mon->gappoh*oe; // outer horizontal gap
	*ov = ws->mon->gappov*oe; // outer vertical gap
	*ih = ws->mon->gappih*ie; // inner horizontal gap
	*iv = ws->mon->gappiv*ie; // inner vertical gap
	*nc = n; // number of clients
}
