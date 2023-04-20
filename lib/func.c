#define mapfunc(N,F) else if (!strcasecmp(name, N)) F(NULL);
#define map(N,F) else if (!strcasecmp(name, N)) return F;

void
enable(const Arg *arg)
{
	const char *name = arg->v;
	uint64_t f = getfuncbyname(name);

	if (f) {
		enablefunc(getfuncbyname(arg->v));
		reload();
	}
}

void
disable(const Arg *arg)
{
	const char *name = arg->v;
	uint64_t f = getfuncbyname(name);

	if (f) {
		disablefunc(f);
		reload();
	}
}

void
toggle(const Arg *arg)
{
	const char *name = arg->v;
	uint64_t f = getfuncbyname(name);

	if (f) {
		togglefunc(f);
		reload();
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
reload()
{
	arrange(NULL);
	grabkeys();
}

const uint64_t
getfuncbyname(const char *name)
{

	if (!name)
		return 0;
	map("SmartGaps", SmartGaps)
	map("SmartGapsMonocle", SmartGapsMonocle)
	map("Swallow", Swallow)
	map("SwallowFloating", SwallowFloating)
	map("CenteredWindowName", CenteredWindowName)
	map("BarActiveGroupBorderColor", BarActiveGroupBorderColor)
	map("BarMasterGroupBorderColor", BarMasterGroupBorderColor)
	map("SpawnCwd", SpawnCwd)
	map("ColorEmoji", ColorEmoji)
	map("Status2DNoAlpha", Status2DNoAlpha)
	map("Systray", Systray)
	map("BarBorder", BarBorder)
	map("NoBorders", NoBorders)
	map("Warp", Warp)
	map("FocusedOnTop", FocusedOnTop)
	map("DecorationHints", DecorationHints)
	map("FocusOnNetActive", FocusOnNetActive)
	map("AllowNoModifierButtons", AllowNoModifierButtons)
	map("CenterSizeHintsClients", CenterSizeHintsClients)
	map("ResizeHints", ResizeHints)
	map("SortScreens", SortScreens)
	map("ViewOnWs", ViewOnWs)
	map("Xresources", Xresources)
	map("FuncPlaceholder0x800000", FuncPlaceholder0x800000)
	map("AltWorkspaceIcons", AltWorkspaceIcons)
	map("GreedyMonitor", GreedyMonitor)
	map("SmartLayoutConvertion", SmartLayoutConvertion)
	map("AutoHideScratchpads", AutoHideScratchpads)
	map("RioDrawIncludeBorders", RioDrawIncludeBorders)
	map("RioDrawSpawnAsync", RioDrawSpawnAsync)
	map("BarPadding", BarPadding)
	map("RestrictFocusstackToMonitor", RestrictFocusstackToMonitor)
	map("AutoReduceNmaster", AutoReduceNmaster)
	map("WinTitleIcons", WinTitleIcons)
	map("WorkspacePreview", WorkspacePreview)
	map("SystrayNoAlpha", SystrayNoAlpha)
	map("WorkspaceLabels", WorkspaceLabels)
	map("SnapToWindows", SnapToWindows)
	map("FlexWinBorders", FlexWinBorders)
	map("FocusOnClick", FocusOnClick)
	map("FuncPlaceholder1099511627776", FuncPlaceholder1099511627776)
	map("FuncPlaceholder2199023255552", FuncPlaceholder2199023255552)
	map("FuncPlaceholder4398046511104", FuncPlaceholder4398046511104)
	map("FuncPlaceholder8796093022208", FuncPlaceholder8796093022208)
	map("FuncPlaceholder17592186044416", FuncPlaceholder17592186044416)
	map("FuncPlaceholder35184372088832", FuncPlaceholder35184372088832)
	map("FuncPlaceholder70368744177664", FuncPlaceholder70368744177664)
	map("FuncPlaceholder140737488355328", FuncPlaceholder140737488355328)
	map("FuncPlaceholder281474976710656", FuncPlaceholder281474976710656)
	map("FuncPlaceholder562949953421312", FuncPlaceholder562949953421312)
	map("FuncPlaceholder1125899906842624", FuncPlaceholder1125899906842624)
	map("FuncPlaceholder2251799813685248", FuncPlaceholder2251799813685248)
	map("FuncPlaceholder4503599627370496", FuncPlaceholder4503599627370496)
	map("FuncPlaceholder9007199254740992", FuncPlaceholder9007199254740992)
	map("FuncPlaceholder18014398509481984", FuncPlaceholder18014398509481984)
	map("FuncPlaceholder36028797018963968", FuncPlaceholder36028797018963968)
	map("Debug", Debug)
	map("FuncPlaceholder144115188075855872", FuncPlaceholder144115188075855872)
	map("FuncPlaceholder288230376151711744", FuncPlaceholder288230376151711744)
	map("FuncPlaceholder576460752303423488", FuncPlaceholder576460752303423488)
	map("FuncPlaceholder1152921504606846976", FuncPlaceholder1152921504606846976)
	map("FuncPlaceholder2305843009213693952", FuncPlaceholder2305843009213693952)
	map("FuncPlaceholder4611686018427387904", FuncPlaceholder4611686018427387904)
	map("FuncPlaceholder9223372036854775808", FuncPlaceholder9223372036854775808)

	return 0;
}

#undef map
#undef mapfunc
