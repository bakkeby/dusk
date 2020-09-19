static int hadflag(Client *c, const int flag);
static int hasflag(Client *c, const int flag);
static void addflag(Client *c, const int flag);
static void setflag(Client *c, const int flag, const int value);
static void setflags(Client *c, const long flags);
static void removeflag(Client *c, const int flag);

enum {
	Floating,
	FullScreen,
	FakeFullScreen,
	IsCentered,
	IsPermanent, // client can't be killed
	IsSticky,
	IsSteam,
	IsTerminal,
	NoSwallow,
} flags; /* flags */

#define ISSTEAM(C) (hasflag(C, IsSteam))
#define ISCENTERED(C) (hasflag(C, IsCentered))
#define ISPERMANENT(C) (hasflag(C, IsPermanent))
