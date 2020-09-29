// static int hadflag(Client *c, const unsigned int flag);
// static int hasflag(Client *c, const unsigned int flag);
static void addflag(Client *c, const unsigned int flag);
static void setflag(Client *c, const unsigned int flag, const int value);
// static void setflags(Client *c, const unsigned int flags);
static void removeflag(Client *c, const unsigned int flag);

enum {
	Fixed = 1 << 0,
	Floating = 1 << 1,
	Urgent = 1 << 2,
	NeverFocus = 1 << 3,
	FullScreen = 1 << 4,
	FakeFullScreen = 1 << 5,
	Centered = 1 << 6,
	Permanent = 1 << 7, // client can't be killed
	Sticky = 1 << 8, // client shows on all tags
	Steam = 1 << 9,
	Terminal = 1 << 10,
	NoSwallow = 1 << 11,
	Locked = 1 << 12, // used by setfullscreen, prevents state change
	Transient = 1 << 13, // whether the client has the transient or hint
	OnlyModButtons = 1 << 14, // if enabled, allows buttons without modifiers to be used
} flags; /* flags */
// 100000001110
//          \\\- floating
//           \\- urgent
//            \- neverfocus

#define ISFLOATING(C) (C->flags & Floating)
#define ISFIXED(C) (C->flags & Fixed)
#define ISLOCKED(C) (C->flags & Locked)
#define ISSTEAM(C) (C->flags & Steam)
#define ISSTICKY(C) (C->flags & Sticky)
#define ISCENTERED(C) (C->flags & Centered)
#define ISFULLSCREEN(C) (C->flags & FullScreen)
#define ISPERMANENT(C) (C->flags & Permanent)
#define ISTERMINAL(C) (C->flags & Terminal)
#define ISTRANSIENT(C) (C->flags & Transient)
#define ISURGENT(C) (C->flags & Urgent)
#define NEVERFOCUS(C) (C->flags & NeverFocus)
#define NOSWALLOW(C) (C->flags & NoSwallow)
#define ONLYMODBUTTONS(C) (C->flags & OnlyModButtons)

#define WASFLOATING(C) (C->prevflags & Floating)

#define SETFLOATING(C) (addflag(C, Floating))
#define SETFULLSCREEN(C) (addflag(C, FullScreen))
#define SETTILED(C) (removeflag(C, Floating))

#define LOCK(C) (addflag(C, Locked))
#define UNLOCK(C) (removeflag(C, Locked))

// hmm, switchtag _rules_ has multiple values 1, 2, 3, 4, c->switchtag holds the original tag info
// and noswallow can be negative to indicate that flag was not explicitly set? review this
// fakefullscreen can be 0, 1, 2, 3 where 2 is the same as wasfullscreen, 3 being a special case

