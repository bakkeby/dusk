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
	Steam = 1 << 11,
	Terminal = 1 << 12,
	NoSwallow = 1 << 13,
	Locked = 1 << 14, // used by setfullscreen, prevents state change
	Transient = 1 << 15, // whether the client has the transient or hint
	OnlyModButtons = 1 << 16, // if enabled, allows buttons without modifiers to be used
	NeedResize = 1 << 17,
} flags; /* flags */

#define ISFLOATING(C) (C->flags & Floating)
#define ISFIXED(C) (C->flags & Fixed)
#define ISLOCKED(C) (C->flags & Locked)
#define ISSTEAM(C) (C->flags & Steam)
#define ISSTICKY(C) (C->flags & Sticky)
#define ISCENTERED(C) (C->flags & Centered)
#define ISFULLSCREEN(C) (C->flags & FullScreen)
#define ISFAKEFULLSCREEN(C) (C->flags & FakeFullScreen)
#define ISPERMANENT(C) (C->flags & Permanent)
#define ISTERMINAL(C) (C->flags & Terminal)
#define ISTRANSIENT(C) (C->flags & Transient)
#define ISURGENT(C) (C->flags & Urgent)
#define NEEDRESIZE(C) (C->flags & NeedResize)
#define NEVERFOCUS(C) (C->flags & NeverFocus)
#define NOSWALLOW(C) (C->flags & NoSwallow)
#define ONLYMODBUTTONS(C) (C->flags & OnlyModButtons)
#define RESTOREFAKEFULLSCREEN(C) (C->flags & RestoreFakeFullScreen)
#define RULED(C) (C->flags & Ruled)

#define WASFLOATING(C) (C->prevflags & Floating)
#define WASFAKEFULLSCREEN(C) (C->prevflags & FakeFullScreen)

#define SETFLOATING(C) (addflag(C, Floating))
#define SETFULLSCREEN(C) (addflag(C, FullScreen))
#define SETTILED(C) (removeflag(C, Floating))

#define LOCK(C) (addflag(C, Locked))
#define UNLOCK(C) (removeflag(C, Locked))

// hmm, switchtag _rules_ has multiple values 1, 2, 3, 4, c->switchtag holds the original tag info

