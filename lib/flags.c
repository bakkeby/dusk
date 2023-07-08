void
addflag(Client *c, const uint64_t flag)
{
	c->prevflags = (c->prevflags & ~flag) | (c->flags & flag);
	c->flags |= flag;
}

void
setflag(Client *c, const uint64_t flag, const int value)
{
	value ? addflag(c, flag) : removeflag(c, flag);
}

void
removeflag(Client *c, const uint64_t flag)
{
	c->prevflags = (c->prevflags & ~flag) | (c->flags & flag);
	c->flags &= ~flag;
}

void
toggleflag(Client *c, const uint64_t flag)
{
	c->flags ^= flag;
}

void
toggleflagop(Client *c, const uint64_t flag, int op)
{
	switch (op) {
	default:
	case 0: /* _NET_WM_STATE_REMOVE */
		removeflag(c, flag);
		break;
	case 1: /* _NET_WM_STATE_ADD */
		addflag(c, flag);
		break;
	case 2: /* _NET_WM_STATE_TOGGLE */
		toggleflag(c, flag);
		break;
	}
}

void
toggleclientflag(const Arg *arg)
{
	if (!selws->sel)
		return;

	toggleflagop(selws->sel, getflagbyname(arg->v), 2);
	arrangews(selws);
}

#define map(N,F) else if (!strcmp(name, N)) return F;

const uint64_t
getflagbyname(const char *name)
{
	if (!name)
		return 0;
	map("AlwaysOnTop", AlwaysOnTop)
	map("Fixed", Fixed)
	map("Floating", Floating)
	map("Urgent", Urgent)
	map("NeverFocus", NeverFocus)
	map("FullScreen", FullScreen)
	map("FakeFullScreen", FakeFullScreen)
	map("RestoreFakeFullScreen", RestoreFakeFullScreen)
	map("Centered", Centered)
	map("Permanent", Permanent)
	map("Hidden", Hidden)
	map("Sticky", Sticky)
	map("Terminal", Terminal)
	map("NoSwallow", NoSwallow)
	map("Locked", Locked)
	map("Transient", Transient)
	map("OnlyModButtons", OnlyModButtons)
	map("Disallowed", Disallowed)
	map("AttachMaster", AttachMaster)
	map("AttachAbove", AttachAbove)
	map("AttachBelow", AttachBelow)
	map("AttachAside", AttachAside)
	map("AttachBottom", AttachBottom)
	map("SwitchWorkspace", SwitchWorkspace)
	map("EnableWorkspace", EnableWorkspace)
	map("RevertWorkspace", RevertWorkspace)
	map("IgnoreCfgReq", IgnoreCfgReq)
	map("IgnoreCfgReqPos", IgnoreCfgReqPos)
	map("IgnoreCfgReqSize", IgnoreCfgReqSize)
	map("IgnorePropTransientFor", IgnorePropTransientFor)
	map("IgnoreSizeHints", IgnoreSizeHints)
	map("IgnoreMinimumSizeHints", IgnoreMinimumSizeHints)
	map("IgnoreDecorationHints", IgnoreDecorationHints)
	map("NoBorder", NoBorder)
	map("FlagPlaceholder0x400000000", FlagPlaceholder0x400000000)
	map("SemiScratchpad", SemiScratchpad)
	map("RespectSizeHints", RespectSizeHints)
	map("RioDrawNoMatchPID", RioDrawNoMatchPID)
	map("FlagPlaceholder0x200000000", FlagPlaceholder0x200000000)
	map("SteamGame", SteamGame)
	map("NoFocusOnNetActive", NoFocusOnNetActive)
	map("ScratchpadStayOnMon", ScratchpadStayOnMon)
	map("Lower", Lower)
	map("Raise", Raise)
	map("SkipTaskbar", SkipTaskbar)
	map("ReapplyRules", ReapplyRules)
	map("CfgReqPosRelativeToMonitor", CfgReqPosRelativeToMonitor)
	map("SwallowRetainSize", SwallowRetainSize)
	map("NoWarp", NoWarp)
	map("FlagPlaceholder0x2000000000000", FlagPlaceholder0x2000000000000)
	map("FlagPlaceholder0x4000000000000", FlagPlaceholder0x4000000000000)
	map("FlagPlaceholder0x8000000000000", FlagPlaceholder0x8000000000000)
	map("FlagPlaceholder0x10000000000000", FlagPlaceholder0x10000000000000)
	map("FlagPlaceholder0x20000000000000", FlagPlaceholder0x20000000000000)
	map("FlagPlaceholder0x40000000000000", FlagPlaceholder0x40000000000000)
	map("RefreshSizeHints", RefreshSizeHints)
	map("Debug", Debug)
	map("Invisible", Invisible)
	map("MoveResize", MoveResize)
	map("MovePlace", MovePlace)
	map("NeedResize", NeedResize)
	map("Ruled", Ruled)
	map("Marked", Marked)
	map("Unmanaged", Unmanaged)

	if (enabled(Debug))
		fprintf(stderr, "getflagbyname for name '%s' not found\n", name);
	return 0;
}

#undef map
