void
togglefakefullscreen(const Arg *arg)
{
	Workspace *ws = selws;
	Client *c = ws->sel;
	if (!c)
		return;

	if (ISTRUEFULLSCREEN(c)) { // exit fullscreen --> fake fullscreen
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
