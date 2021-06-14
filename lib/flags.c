void
addflag(Client *c, const unsigned long flag)
{
	c->prevflags = (c->prevflags & ~flag) | (c->flags & flag);
	c->flags |= flag;
}

void
setflag(Client *c, const unsigned long flag, const int value)
{
	value ? addflag(c, flag) : removeflag(c, flag);
}

void
removeflag(Client *c, const unsigned long flag)
{
	c->prevflags = (c->prevflags & ~flag) | (c->flags & flag);
	c->flags &= ~flag;
}

void
toggleflagop(Client *c, const unsigned long flag, int op)
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
		c->flags ^= flag;
		break;
	}
}

void
toggleflag(const Arg *arg)
{
	toggleflagop(selws->sel, getflagbyname(arg->v), 2);
	arrangews(selws);
}

const unsigned long
getflagbyname(const char *name)
{
	if (strcmp(name, "AlwaysOnTop") == 0)
		return AlwaysOnTop;
	else if (strcmp(name, "Fixed") == 0)
		return Fixed;
	else if (strcmp(name, "Floating") == 0)
		return Floating;
	else if (strcmp(name, "Urgent") == 0)
		return Urgent;
	else if (strcmp(name, "NeverFocus") == 0)
		return NeverFocus;
	else if (strcmp(name, "FullScreen") == 0)
		return FullScreen;
	else if (strcmp(name, "FakeFullScreen") == 0)
		return FakeFullScreen;
	else if (strcmp(name, "RestoreFakeFullScreen") == 0)
		return RestoreFakeFullScreen;
	else if (strcmp(name, "Centered") == 0)
		return Centered;
	else if (strcmp(name, "Permanent") == 0)
		return Permanent;
	else if (strcmp(name, "Hidden") == 0)
		return Hidden;
	else if (strcmp(name, "Sticky") == 0)
		return Sticky;
	else if (strcmp(name, "Terminal") == 0)
		return Terminal;
	else if (strcmp(name, "NoSwallow") == 0)
		return NoSwallow;
	else if (strcmp(name, "Locked") == 0)
		return Locked;
	else if (strcmp(name, "Transient") == 0)
		return Transient;
	else if (strcmp(name, "OnlyModButtons") == 0)
		return OnlyModButtons;
	else if (strcmp(name, "FlagPlaceholder0x20000") == 0)
		return FlagPlaceholder0x20000;
	else if (strcmp(name, "AttachMaster") == 0)
		return AttachMaster;
	else if (strcmp(name, "AttachAbove") == 0)
		return AttachAbove;
	else if (strcmp(name, "AttachBelow") == 0)
		return AttachBelow;
	else if (strcmp(name, "AttachAside") == 0)
		return AttachAside;
	else if (strcmp(name, "AttachBottom") == 0)
		return AttachBottom;
	else if (strcmp(name, "SwitchWorkspace") == 0)
		return SwitchWorkspace;
	else if (strcmp(name, "EnableWorkspace") == 0)
		return EnableWorkspace;
	else if (strcmp(name, "RevertWorkspace") == 0)
		return RevertWorkspace;
	else if (strcmp(name, "IgnoreCfgReq") == 0)
		return IgnoreCfgReq;
	else if (strcmp(name, "IgnoreCfgReqPos") == 0)
		return IgnoreCfgReqPos;
	else if (strcmp(name, "IgnoreCfgReqSize") == 0)
		return IgnoreCfgReqSize;
	else if (strcmp(name, "IgnorePropTransientFor") == 0)
		return IgnorePropTransientFor;
	else if (strcmp(name, "IgnoreSizeHints") == 0)
		return IgnoreSizeHints;
	else if (strcmp(name, "IgnoreMinimumSizeHints") == 0)
		return IgnoreMinimumSizeHints;
	else if (strcmp(name, "IgnoreDecorationHints") == 0)
		return IgnoreDecorationHints;
	else if (strcmp(name, "NoBorder") == 0)
		return NoBorder;
	else if (strcmp(name, "FlagPlaceholder0x400000000") == 0)
		return FlagPlaceholder0x400000000;
	else if (strcmp(name, "FlagPlaceholder0x800000000") == 0)
		return FlagPlaceholder0x800000000;
	else if (strcmp(name, "RespectSizeHints") == 0)
		return RespectSizeHints;
	else if (strcmp(name, "RioDrawNoMatchPID") == 0)
		return RioDrawNoMatchPID;
	else if (strcmp(name, "FlagPlaceholder0x200000000") == 0)
		return FlagPlaceholder0x200000000;
	else if (strcmp(name, "SteamGame") == 0)
		return SteamGame;
	else if (strcmp(name, "FlagPlaceholder0x10000000000") == 0)
		return FlagPlaceholder0x10000000000;
	else if (strcmp(name, "FlagPlaceholder0x20000000000") == 0)
		return FlagPlaceholder0x20000000000;
	else if (strcmp(name, "FlagPlaceholder0x40000000000") == 0)
		return FlagPlaceholder0x40000000000;
	else if (strcmp(name, "FlagPlaceholder0x80000000000") == 0)
		return FlagPlaceholder0x80000000000;
	else if (strcmp(name, "FlagPlaceholder0x100000000000") == 0)
		return FlagPlaceholder0x100000000000;
	else if (strcmp(name, "FlagPlaceholder0x200000000000") == 0)
		return FlagPlaceholder0x200000000000;
	else if (strcmp(name, "FlagPlaceholder0x400000000000") == 0)
		return FlagPlaceholder0x400000000000;
	else if (strcmp(name, "FlagPlaceholder0x800000000000") == 0)
		return FlagPlaceholder0x800000000000;
	else if (strcmp(name, "FlagPlaceholder0x1000000000000") == 0)
		return FlagPlaceholder0x1000000000000;
	else if (strcmp(name, "FlagPlaceholder0x2000000000000") == 0)
		return FlagPlaceholder0x2000000000000;
	else if (strcmp(name, "FlagPlaceholder0x4000000000000") == 0)
		return FlagPlaceholder0x4000000000000;
	else if (strcmp(name, "FlagPlaceholder0x8000000000000") == 0)
		return FlagPlaceholder0x8000000000000;
	else if (strcmp(name, "FlagPlaceholder0x10000000000000") == 0)
		return FlagPlaceholder0x10000000000000;
	else if (strcmp(name, "FlagPlaceholder0x20000000000000") == 0)
		return FlagPlaceholder0x20000000000000;
	else if (strcmp(name, "FlagPlaceholder0x40000000000000") == 0)
		return FlagPlaceholder0x40000000000000;
	else if (strcmp(name, "FlagPlaceholder0x80000000000000") == 0)
		return FlagPlaceholder0x80000000000000;
	else if (strcmp(name, "FlagPlaceholder0x100000000000000") == 0)
		return FlagPlaceholder0x100000000000000;
	else if (strcmp(name, "Invisible") == 0)
		return Invisible;
	else if (strcmp(name, "MoveResize") == 0)
		return MoveResize;
	else if (strcmp(name, "MovePlace") == 0)
		return MovePlace;
	else if (strcmp(name, "NeedResize") == 0)
		return NeedResize;
	else if (strcmp(name, "Ruled") == 0)
		return Ruled;
	else if (strcmp(name, "Marked") == 0)
		return Marked;
	else if (strcmp(name, "Unmanaged") == 0)
		return Unmanaged;

	return 0;
}
