// static int hadflag(Client *c, const unsigned int flag);
// static int hasflag(Client *c, const unsigned int flag);
// static void addflag(Client *c, const unsigned int flag);
// static void setflag(Client *c, const unsigned int flag, const int value);
// static void setflags(Client *c, const unsigned int flags);
// static void removeflag(Client *c, const unsigned int flag);

enum {
	Floating = 1 << 0,
	FullScreen = 1 << 1,
	FakeFullScreen = 1 << 2,
	Centered = 1 << 3,
	Permanent = 1 << 4, // client can't be killed
	Sticky = 1 << 5, // client shows on all tags
	Steam = 1 << 6,
	Terminal = 1 << 7,
	NoSwallow = 1 << 8,
} flags; /* flags */

// #define ISSTEAM(C) (hasflag(C, Steam))
// #define ISCENTERED(C) (hasflag(C, Centered))
// #define ISPERMANENT(C) (hasflag(C, Permanent))

#define ISFLOATING(C) (c->flags & Floating)
#define ISSTEAM(C) (C->flags & Steam)
#define ISCENTERED(C) (C->flags & Centered)
#define ISPERMANENT(C) (C->flags & Permanent)
#define ISTERMINAL(C) (C->flags & Terminal)
#define NOSWALLOW(C) (C->flags & NoSwallow)

// consider the client states
//    int isfixed, isfloating, isurgent, neverfocus, oldstate, isfullscreen;

// hmm, switchtag _rules_ has multiple values 1, 2, 3, 4, c->switchtag holds the original tag info
// and noswallow can be negative to indicate that flag was not explicitly set? review this
// fakefullscreen can be 0, 1, 2, 3 where 2 is the same as wasfullscreen, 3 being a special case

