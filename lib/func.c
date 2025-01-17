#define compareNameToFunc(F) else if (!strcasecmp(name, #F)) return F;
#define mapfunc(N,F) else if (!strcasecmp(name, N)) F(NULL);

void
enable(const Arg *arg)
{
	const char *name = arg->v;
	uint64_t f = getfuncbyname(name);

	if (f) {
		enablefunc(f);
		reload(f);
	}
}

void
disable(const Arg *arg)
{
	const char *name = arg->v;
	uint64_t f = getfuncbyname(name);

	if (f) {
		disablefunc(f);
		reload(f);
	}
}

void
toggle(const Arg *arg)
{
	const char *name = arg->v;
	uint64_t f = getfuncbyname(name);

	if (f) {
		togglefunc(f);
		reload(f);
	}
	mapfunc("bar", togglebar)
	mapfunc("barpadding", togglebarpadding)
	mapfunc("compact", togglecompact)
	mapfunc("fakefullscreen", togglefakefullscreen)
	mapfunc("floating", togglefloating)
	mapfunc("fullscreen", togglefullscreen)
	mapfunc("gaps", togglegaps)
	mapfunc("mark", togglemark)
	mapfunc("nomodbuttons", togglenomodbuttons)
	mapfunc("pinnedws", togglepinnedws)
	mapfunc("sticky", togglesticky)
}

void
reload(const uint64_t functionality)
{
	Workspace *ws;
	Client *c;
	int func_enabled = enabled(functionality);

	/* If the NoBorders functionality was disabled, then loop through and force resize all clients
	 * that previously had the NoBorder flag set in order to restore borders. */
	if (!func_enabled && functionality == NoBorders) {
		for (ws = workspaces; ws; ws = ws->next) {
			for (c = ws->clients; c; c = c->next) {
				if (WASNOBORDER(c)) {
					restoreborder(c);
				}
			}
		}
	}

	arrange(NULL);
	grabkeys();
}

const uint64_t
getfuncbyname(const char *name)
{

	if (!name)
		return 0;
	compareNameToFunc(SmartGaps)
	compareNameToFunc(SmartGapsMonocle)
	compareNameToFunc(Swallow)
	compareNameToFunc(SwallowFloating)
	compareNameToFunc(CenteredWindowName)
	compareNameToFunc(BarActiveGroupBorderColor)
	compareNameToFunc(BarMasterGroupBorderColor)
	compareNameToFunc(SpawnCwd)
	compareNameToFunc(ColorEmoji)
	compareNameToFunc(Status2DNoAlpha)
	compareNameToFunc(Systray)
	compareNameToFunc(BarBorder)
	compareNameToFunc(NoBorders)
	compareNameToFunc(Warp)
	compareNameToFunc(FocusedOnTop)
	compareNameToFunc(DecorationHints)
	compareNameToFunc(FocusOnNetActive)
	compareNameToFunc(AllowNoModifierButtons)
	compareNameToFunc(CenterSizeHintsClients)
	compareNameToFunc(ResizeHints)
	compareNameToFunc(SortScreens)
	compareNameToFunc(ViewOnWs)
	compareNameToFunc(Xresources)
	compareNameToFunc(AltWorkspaceIcons)
	compareNameToFunc(GreedyMonitor)
	compareNameToFunc(SmartLayoutConversion)
	compareNameToFunc(SmartLayoutConvertion)
	compareNameToFunc(AutoHideScratchpads)
	compareNameToFunc(RioDrawIncludeBorders)
	compareNameToFunc(RioDrawSpawnAsync)
	compareNameToFunc(BarPadding)
	compareNameToFunc(RestrictFocusstackToMonitor)
	compareNameToFunc(AutoReduceNmaster)
	compareNameToFunc(WinTitleIcons)
	compareNameToFunc(WorkspacePreview)
	compareNameToFunc(SystrayNoAlpha)
	compareNameToFunc(WorkspaceLabels)
	compareNameToFunc(SnapToWindows)
	compareNameToFunc(SnapToGaps)
	compareNameToFunc(FlexWinBorders)
	compareNameToFunc(FocusOnClick)
	compareNameToFunc(FocusedOnTopTiled)
	compareNameToFunc(BanishMouseCursor)
	compareNameToFunc(FocusFollowMouse)
	compareNameToFunc(BanishMouseCursorToCorner)
	compareNameToFunc(StackerIcons)
	compareNameToFunc(AltWindowTitles)
	compareNameToFunc(FuncPlaceholder70368744177664)
	compareNameToFunc(FuncPlaceholder140737488355328)
	compareNameToFunc(FuncPlaceholder281474976710656)
	compareNameToFunc(FuncPlaceholder562949953421312)
	compareNameToFunc(FuncPlaceholder1125899906842624)
	compareNameToFunc(FuncPlaceholder2251799813685248)
	compareNameToFunc(FuncPlaceholder4503599627370496)
	compareNameToFunc(FuncPlaceholder9007199254740992)
	compareNameToFunc(FuncPlaceholder18014398509481984)
	compareNameToFunc(FuncPlaceholder36028797018963968)
	compareNameToFunc(Debug)
	compareNameToFunc(FuncPlaceholder144115188075855872)
	compareNameToFunc(FuncPlaceholder288230376151711744)
	compareNameToFunc(FuncPlaceholder576460752303423488)
	compareNameToFunc(FuncPlaceholder1152921504606846976)
	compareNameToFunc(FuncPlaceholder2305843009213693952)
	compareNameToFunc(FuncPlaceholder4611686018427387904)
	compareNameToFunc(FuncPlaceholder9223372036854775808)

	return 0;
}

#undef compareNameToFunc
#undef mapfunc
