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

#define compareNameToFlag(F) else if (!strcasecmp(name, #F)) return F;

const uint64_t
getflagbyname(const char *name)
{
	if (!name)
		return 0;
	compareNameToFlag(AlwaysOnTop)
	compareNameToFlag(Fixed)
	compareNameToFlag(Floating)
	compareNameToFlag(Urgent)
	compareNameToFlag(NeverFocus)
	compareNameToFlag(FullScreen)
	compareNameToFlag(FakeFullScreen)
	compareNameToFlag(RestoreFakeFullScreen)
	compareNameToFlag(Centered)
	compareNameToFlag(Permanent)
	compareNameToFlag(Hidden)
	compareNameToFlag(Sticky)
	compareNameToFlag(Terminal)
	compareNameToFlag(NoSwallow)
	compareNameToFlag(Locked)
	compareNameToFlag(Transient)
	compareNameToFlag(OnlyModButtons)
	compareNameToFlag(Disallowed)
	compareNameToFlag(AttachMaster)
	compareNameToFlag(AttachAbove)
	compareNameToFlag(AttachBelow)
	compareNameToFlag(AttachAside)
	compareNameToFlag(AttachBottom)
	compareNameToFlag(SwitchWorkspace)
	compareNameToFlag(EnableWorkspace)
	compareNameToFlag(RevertWorkspace)
	compareNameToFlag(IgnoreCfgReq)
	compareNameToFlag(IgnoreCfgReqPos)
	compareNameToFlag(IgnoreCfgReqSize)
	compareNameToFlag(IgnorePropTransientFor)
	compareNameToFlag(IgnoreSizeHints)
	compareNameToFlag(IgnoreMinimumSizeHints)
	compareNameToFlag(IgnoreDecorationHints)
	compareNameToFlag(NoBorder)
	compareNameToFlag(FlagPlaceholder0x400000000)
	compareNameToFlag(SemiScratchpad)
	compareNameToFlag(RespectSizeHints)
	compareNameToFlag(RioDrawNoMatchPID)
	compareNameToFlag(FlagPlaceholder0x200000000)
	compareNameToFlag(SteamGame)
	compareNameToFlag(NoFocusOnNetActive)
	compareNameToFlag(ScratchpadStayOnMon)
	compareNameToFlag(Lower)
	compareNameToFlag(Raise)
	compareNameToFlag(SkipTaskbar)
	compareNameToFlag(ReapplyRules)
	compareNameToFlag(CfgReqPosRelativeToMonitor)
	compareNameToFlag(SwallowRetainSize)
	compareNameToFlag(NoWarp)
	compareNameToFlag(SwallowNoInheritFullScreen)
	compareNameToFlag(FlagPlaceholder0x4000000000000)
	compareNameToFlag(FlagPlaceholder0x8000000000000)
	compareNameToFlag(FlagPlaceholder0x10000000000000)
	compareNameToFlag(FlagPlaceholder0x20000000000000)
	compareNameToFlag(Swallowed)
	compareNameToFlag(RefreshSizeHints)
	compareNameToFlag(Debug)
	compareNameToFlag(Invisible)
	compareNameToFlag(MoveResize)
	compareNameToFlag(MovePlace)
	compareNameToFlag(NeedResize)
	compareNameToFlag(Ruled)
	compareNameToFlag(Marked)
	compareNameToFlag(Unmanaged)

	if (enabled(Debug))
		fprintf(stderr, "getflagbyname for name '%s' not found\n", name);
	return 0;
}

#undef compareNameToFlag
