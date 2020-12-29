void
togglefakefullscreen(const Arg *arg)
{
	Workspace *ws = WS;
	Client *c = ws->sel;
	if (!c)
		return;

	if (!ISFAKEFULLSCREEN(c) && ISFULLSCREEN(c)) { // exit fullscreen --> fake fullscreen
		addflag(c, RestoreFakeFullScreen);
		setfullscreen(c, 0, 0);
	} else if (ISFAKEFULLSCREEN(c)) {
		setfullscreen(c, 0, 0);
		removeflag(c, FakeFullScreen);
	} else {
		addflag(c, FakeFullScreen);
		removeflag(c, RestoreFakeFullScreen);
		setfullscreen(c, 1, 0);
	}
}
