void
togglefullscreen(const Arg *arg)
{
	Workspace *ws = WS;
	Client *c = ws->sel;
	if (!c)
		return;

	if (ISFAKEFULLSCREEN(c)) { // fake fullscreen --> fullscreen
		removeflag(c, FakeFullScreen);
		addflag(c, RestoreFakeFullScreen);
		setfullscreen(c, 1, 0);
	} else
		setfullscreen(c, !ISFULLSCREEN(c), 0);
}
