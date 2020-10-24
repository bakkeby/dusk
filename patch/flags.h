// static int hadflag(Client *c, const unsigned int flag);
// static int hasflag(Client *c, const unsigned int flag);
static void addflag(Client *c, const unsigned long flag);
static void setflag(Client *c, const unsigned long flag, const int value);
// static void setflags(Client *c, const unsigned int flags);
static void removeflag(Client *c, const unsigned long flag);

static const unsigned long
	Ruled = 0x1, // indicates whether client was subject to client rules (used internally to determine default behaviour)
	Fixed = 0x2,
	Floating = 0x4,
	Urgent = 0x8,
	NeverFocus = 0x10,
	FullScreen = 0x20,
	FakeFullScreen = 0x40,
	RestoreFakeFullScreen = 0x80,
	Centered = 0x100,
	Permanent = 0x200, // client can't be killed
	Sticky = 0x400, // client shows on all tags
	Hidden = 0x800,
	Terminal = 0x1000,
	NoSwallow = 0x2000,
	Locked = 0x4000, // used by setfullscreen, prevents state change
	Transient = 0x8000, // whether the client has the transient or hint
	OnlyModButtons = 0x10000, // if enabled, allows buttons without modifiers to be used
	NeedResize = 0x20000,
	AttachMaster = 0x40000,
	AttachAbove = 0x80000,
	AttachAside = 0x100000,
	AttachBelow = 0x200000,
	AttachBottom = 0x400000,
	SwitchTag = 0x800000, // automatically moves you to the tag of the newly opened application
	EnableTag = 0x1000000, // enables the tag of the newly opened application in addition to your existing enabled tags
	RevertTag = 0x2000000, // if SwitchTag or EnableTag, closing that window reverts the view back to what it was previously
	IgnoreCfgReq = 0x4000000, // ignore all configure requests coming from the client
	IgnoreCfgReqPos = 0x8000000, // ignore the x,y position details of configure requests coming from the client
	IgnoreCfgReqSize = 0x10000000, // ignore the size details of configure requests coming from the client
	/* Some clients (e.g. alacritty) helpfully send configure requests with a new size or position
	 * when they detect that they have been moved to another monitor. This can cause visual glitches
	 * when moving (or resizing) client windows from one monitor to another. This variable is used
	 * internally to ignore such configure requests while movemouse or resizemouse are being used. */
	MoveResize = 0x20000000, // used internally to indicate that the client is being moved or resized
	AlwaysOnTop = 0x40000000,
	TMP2147483648 = 0x80000000,
	TMP4294967296 = 0x100000000,
	TMP8589934592 = 0x200000000,
	TMP17179869184 = 0x400000000,
	TMP34359738368 = 0x800000000,
	TMP68719476736 = 0x1000000000,
	TMP137438953472 = 0x2000000000,
	TMP274877906944 = 0x4000000000,
	TMP549755813888 = 0x8000000000,
	TMP1099511627776 = 0x10000000000,
	TMP2199023255552 = 0x20000000000,
	TMP4398046511104 = 0x40000000000,
	TMP8796093022208 = 0x80000000000,
	TMP17592186044416 = 0x100000000000,
	TMP35184372088832 = 0x200000000000,
	TMP70368744177664 = 0x400000000000,
	TMP140737488355328 = 0x800000000000,
	TMP281474976710656 = 0x1000000000000,
	TMP562949953421312 = 0x2000000000000,
	TMP1125899906842624 = 0x4000000000000,
	TMP2251799813685248 = 0x8000000000000,
	TMP4503599627370496 = 0x10000000000000,
	TMP9007199254740992 = 0x20000000000000,
	TMP18014398509481984 = 0x40000000000000,
	TMP36028797018963968 = 0x80000000000000,
	TMP72057594037927936 = 0x100000000000000,
	TMP144115188075855872 = 0x200000000000000,
	TMP288230376151711744 = 0x400000000000000,
	TMP576460752303423488 = 0x800000000000000,
	TMP1152921504606846976 = 0x1000000000000000,
	TMP2305843009213693952 = 0x2000000000000000,
	TMP4611686018427387904 = 0x4000000000000000,
	TMP9223372036854775808 = 0x8000000000000000;

#define ALWAYSONTOP(C) (C->flags & AlwaysOnTop)
#define ISFLOATING(C) (C->flags & Floating)
#define ISFIXED(C) (C->flags & Fixed)
#define ISLOCKED(C) (C->flags & Locked)
// #define ISSTEAM(C) (C->flags & Steam)
#define ISSTICKY(C) (C->flags & Sticky)
#define ISCENTERED(C) (C->flags & Centered)
#define ISFULLSCREEN(C) (C->flags & FullScreen)
#define ISFAKEFULLSCREEN(C) (C->flags & FakeFullScreen)
#define ISPERMANENT(C) (C->flags & Permanent)
#define ISTERMINAL(C) (C->flags & Terminal)
#define ISTRANSIENT(C) (C->flags & Transient)
#define ISURGENT(C) (C->flags & Urgent)
#define IGNORECFGREQ(C) (C->flags & IgnoreCfgReq)
#define IGNORECFGREQPOS(C) (C->flags & IgnoreCfgReqPos)
#define IGNORECFGREQSIZE(C) (C->flags & IgnoreCfgReqSize)
#define NEEDRESIZE(C) (C->flags & NeedResize)
#define NEVERFOCUS(C) (C->flags & NeverFocus)
#define NOSWALLOW(C) (C->flags & NoSwallow)
#define ONLYMODBUTTONS(C) (C->flags & OnlyModButtons)
#define RESTOREFAKEFULLSCREEN(C) (C->flags & RestoreFakeFullScreen)
#define RULED(C) (C->flags & Ruled)
#define SWITCHTAG(C) (C->flags & SwitchTag)
#define ENABLETAG(C) (C->flags & EnableTag)
#define REVERTTAG(C) (C->flags & RevertTag)
#define MOVERESIZE(C) (C->flags & MoveResize)

#define WASFLOATING(C) (C->prevflags & Floating)
#define WASFAKEFULLSCREEN(C) (C->prevflags & FakeFullScreen)
#define WASFULLSCREEN(C) (C->prevflags & FullScreen)

#define SETFLOATING(C) (addflag(C, Floating))
#define SETFULLSCREEN(C) (addflag(C, FullScreen))
#define SETTILED(C) (removeflag(C, Floating))

#define LOCK(C) (addflag(C, Locked))
#define UNLOCK(C) (removeflag(C, Locked))

// hmm, switchtag _rules_ has multiple values 1, 2, 3, 4, c->switchtag holds the original tag info

