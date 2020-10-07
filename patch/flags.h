// static int hadflag(Client *c, const unsigned int flag);
// static int hasflag(Client *c, const unsigned int flag);
static void addflag(Client *c, const unsigned int flag);
static void setflag(Client *c, const unsigned int flag, const int value);
// static void setflags(Client *c, const unsigned int flags);
static void removeflag(Client *c, const unsigned int flag);

enum {
	Ruled = 1 << 0, // indicates whether client was subject to client rules (used internally to determine default behaviour)
	Fixed = 1 << 1,
	Floating = 1 << 2,
	Urgent = 1 << 3,
	NeverFocus = 1 << 4,
	FullScreen = 1 << 5,
	FakeFullScreen = 1 << 6,
	RestoreFakeFullScreen = 1 << 7,
	Centered = 1 << 8,
	Permanent = 1 << 9, // client can't be killed
	Sticky = 1 << 10, // client shows on all tags
	// Steam = 1 << 11,
	Terminal = 1 << 12,
	NoSwallow = 1 << 13,
	Locked = 1 << 14, // used by setfullscreen, prevents state change
	Transient = 1 << 15, // whether the client has the transient or hint
	OnlyModButtons = 1 << 16, // if enabled, allows buttons without modifiers to be used
	NeedResize = 1 << 17,
	AttachMaster = 1 << 18,
	AttachAbove = 1 << 19,
	AttachAside = 1 << 20,
	AttachBelow = 1 << 21,
	AttachBottom = 1 << 22,
	SwitchTag = 1 << 23, // automatically moves you to the tag of the newly opened application
	EnableTag = 1 << 24, // enables the tag of the newly opened application in addition to your existing enabled tags
	RevertTag = 1 << 25, // if SwitchTag or EnableTag, closing that window reverts the view back to what it was previously
	IgnoreCfgReq = 1 << 26, // ignore all configure requests coming from the client
	IgnoreCfgReqPos = 1 << 27, // ignore the x,y position details of configure requests coming from the client
	IgnoreCfgReqSize = 1 << 28, // ignore the size details of configure requests coming from the client
	/* Some clients (e.g. alacritty) helpfully send configure requests with a new size or position
	 * when they detect that they have been moved to another monitor. This can cause visual glitches
	 * when moving (or resizing) client windows from one monitor to another. This variable is used
	 * internally to ignore such configure requests while movemouse or resizemouse are being used. */
	MoveResize = 1 << 29, // used internally to indicate that the client is being moved or resized
	AlwaysOnTop = 1 << 30,
} flags; /* flags */

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

#define SETFLOATING(C) (addflag(C, Floating))
#define SETFULLSCREEN(C) (addflag(C, FullScreen))
#define SETTILED(C) (removeflag(C, Floating))

#define LOCK(C) (addflag(C, Locked))
#define UNLOCK(C) (removeflag(C, Locked))

// hmm, switchtag _rules_ has multiple values 1, 2, 3, 4, c->switchtag holds the original tag info

