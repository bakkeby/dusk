static void addflag(Client *c, const unsigned long flag);
static void setflag(Client *c, const unsigned long flag, const int value);
static void removeflag(Client *c, const unsigned long flag);
static void toggleflag(Client *c, const unsigned long flag);
static void toggleflagex(const Arg *arg);
static const unsigned long getflagbyname(const char *name);

static const unsigned long
	AlwaysOnTop = 0x1, // client window is intended to always display on top (even above floating windows)
	Fixed = 0x2, // used when client has a fixed size where width equals height
	Floating = 0x4, // the client is floating (i.e. not tiled)
	Urgent = 0x8, // indicates that the client urgently needs attention
	NeverFocus = 0x10, // indicates that the client should never receive focus
	FullScreen = 0x20, // indicates that the client is in fullscreen mode
	FakeFullScreen = 0x40, // indicates that the client is in fake fullscreen mode
	RestoreFakeFullScreen = 0x80, // internal flag that indicates that fake fullscreen should be restored when exiting actual fullscreen
	Centered = 0x100, // indicates that the client, if floating, should be centered on the screen on launch
	Permanent = 0x200, // if a client is permanent, then the client can't be killed
	Hidden = 0x400, // indicates that the client is hidden
	Sticky = 0x800, // TODO remove? yajl dumps dependency
	Terminal = 0x1000, // indicates that the client is a terminal, used by swallow
	NoSwallow = 0x2000, // indicates that the client should never be swallowed if launched by a terminal
	Locked = 0x4000, // used by setfullscreen, prevents state change
	Transient = 0x8000, // whether the client has the transient or hint
	OnlyModButtons = 0x10000, // if enabled, allows buttons without modifiers to be used
	FlagPlaceholder0x20000 = 0x20000,
	AttachMaster = 0x40000, // attach the client as the first client in the list
	AttachAbove = 0x80000, // attach the client before the currently selected client
	AttachBelow = 0x100000, // attach the client after the currently selected client
	AttachAside = 0x200000, // attach the client as the first client in the stack area
	AttachBottom = 0x400000, // attach the client at the end of the list
	SwitchWorkspace = 0x800000, // automatically moves you to the workspace of the newly opened application
	EnableWorkspace = 0x1000000, // enables the workspace of the newly opened application in addition to your existing viewed workspaces
	RevertWorkspace = 0x2000000, // if SwitchWorkspace or EnableWorkspace, closing that window reverts the view back to what it was previously
	IgnoreCfgReq = 0x4000000, // ignore all configure requests coming from the client
	IgnoreCfgReqPos = 0x8000000, // ignore the x,y position details of configure requests coming from the client
	IgnoreCfgReqSize = 0x10000000, // ignore the size details of configure requests coming from the client
	IgnorePropTransientFor = 0x20000000, // ignore WM_TRANSIENT_FOR property notifications for buggy client windows (e.g. WebStorm)
	IgnoreSizeHints = 0x40000000, // ignore size hints for clients (floating and tiled), see floatpos
	IgnoreMinimumSizeHints = 0x80000000, // while respecting size hints in general ignore the minimum size restrictions to avoid overlapping windows
	IgnoreDecorationHints = 0x100000000, // ignore decoration hints for client
	FlagPlaceholder0x200000000 = 0x200000000,
	FlagPlaceholder0x400000000 = 0x400000000,
	FlagPlaceholder0x800000000 = 0x800000000,
	RespectSizeHints = 0x1000000000, // respect size hints for this client when ResizeHints is globally disabled
	RioDrawNoMatchPID = 0x2000000000, // do not match PID for this client when spawning via riospawn
	NoBorder = 0x4000000000, // indicates that the client should not be drawn with a border around it
	SteamGame = 0x8000000000,
	FlagPlaceholder0x10000000000 = 0x10000000000,
	FlagPlaceholder0x20000000000 = 0x20000000000,
	FlagPlaceholder0x40000000000 = 0x40000000000,
	FlagPlaceholder0x80000000000 = 0x80000000000,
	FlagPlaceholder0x100000000000 = 0x100000000000,
	FlagPlaceholder0x200000000000 = 0x200000000000,
	FlagPlaceholder0x400000000000 = 0x400000000000,
	FlagPlaceholder0x800000000000 = 0x800000000000,
	FlagPlaceholder0x1000000000000 = 0x1000000000000,
	FlagPlaceholder0x2000000000000 = 0x2000000000000,
	FlagPlaceholder0x4000000000000 = 0x4000000000000,
	FlagPlaceholder0x8000000000000 = 0x8000000000000,
	FlagPlaceholder0x10000000000000 = 0x10000000000000,
	FlagPlaceholder0x20000000000000 = 0x20000000000000,
	FlagPlaceholder0x40000000000000 = 0x40000000000000,
	FlagPlaceholder0x80000000000000 = 0x80000000000000,
	FlagPlaceholder0x100000000000000 = 0x100000000000000,
	/* Below are flags that are intended to only be used internally */
	Invisible = 0x200000000000000, // by default all clients are visible, used by scratchpads to hide clients
	/* Some clients (e.g. alacritty) helpfully send configure requests with a new size or position
	 * when they detect that they have been moved to another monitor. This can cause visual glitches
	 * when moving (or resizing) client windows from one monitor to another. This variable is used
	 * internally to ignore such configure requests while movemouse or resizemouse are being used. */
	MoveResize = 0x400000000000000, // used internally to indicate that the client is being moved or resized
	MovePlace = 0x800000000000000, // used internally to indicate that the client is being moved within stack
	NeedResize = 0x1000000000000000, // internal flag indicating that the client needs to be resized later
	Ruled = 0x2000000000000000, // indicates whether client was subject to client rules (used internally to determine default behaviour)
	Marked = 0x4000000000000000, // indicates that the client has been marked for group action
	FlagPlaceholder0x8000000000000000 = 0x8000000000000000;

#define ALWAYSONTOP(C) (C->flags & AlwaysOnTop)
#define HIDDEN(C) (C->flags & Hidden)
#define ISFLOATING(C) (C->flags & Floating)
#define ISFIXED(C) (C->flags & Fixed)
#define ISLOCKED(C) (C->flags & Locked)
#define ISSTICKY(C) (C->flags & Sticky)
#define ISCENTERED(C) (C->flags & Centered)
#define ISFULLSCREEN(C) (C->flags & FullScreen)
#define ISFAKEFULLSCREEN(C) (C->flags & FakeFullScreen)
#define ISPERMANENT(C) (C->flags & Permanent)
#define ISTERMINAL(C) (C->flags & Terminal)
#define ISTRANSIENT(C) (C->flags & Transient)
#define ISURGENT(C) (C->flags & Urgent)
#define ISMARKED(C) (C->flags & Marked)
#define ISVISIBLE(C) (C->ws->visible && !(C->flags & (Invisible|Hidden)))
#define ISINVISIBLE(C) (C->flags & Invisible)
#define IGNORECFGREQ(C) (C->flags & IgnoreCfgReq)
#define IGNORECFGREQPOS(C) (C->flags & IgnoreCfgReqPos)
#define IGNORECFGREQSIZE(C) (C->flags & IgnoreCfgReqSize)
#define IGNOREDECORATIONHINTS(C) (C->flags & IgnoreDecorationHints)
#define IGNOREPROPTRANSIENTFOR(C) (C->flags & IgnorePropTransientFor)
#define IGNORESIZEHINTS(C) (C->flags & IgnoreSizeHints)
#define IGNOREMINIMUMSIZEHINTS(C) (C->flags & IgnoreMinimumSizeHints)
#define NEEDRESIZE(C) (C->flags & NeedResize)
#define NEVERFOCUS(C) (C->flags & NeverFocus)
#define NOBORDER(C) (C->flags & NoBorder)
#define NOSWALLOW(C) (C->flags & NoSwallow)
#define ONLYMODBUTTONS(C) (C->flags & OnlyModButtons)
#define RESPECTSIZEHINTS(C) (C->flags & RespectSizeHints)
#define RESTOREFAKEFULLSCREEN(C) (C->flags & RestoreFakeFullScreen)
#define RIODRAWNOMATCHPID(C) (C->flags & RioDrawNoMatchPID)
#define RULED(C) (C->flags & Ruled)
#define STEAMGAME(C) (C->flags & SteamGame)
#define SWITCHWORKSPACE(C) (C->flags & SwitchWorkspace)
#define ENABLEWORKSPACE(C) (C->flags & EnableWorkspace)
#define REVERTWORKSPACE(C) (C->flags & RevertWorkspace)
#define MOVERESIZE(C) (C->flags & MoveResize)
#define MOVEPLACE(C) (C->flags & MovePlace)

#define WASFLOATING(C) (C->prevflags & Floating)
#define WASFAKEFULLSCREEN(C) (C->prevflags & FakeFullScreen)
#define WASFULLSCREEN(C) (C->prevflags & FullScreen)

#define SETFLOATING(C) (addflag(C, Floating))
#define SETFULLSCREEN(C) (addflag(C, FullScreen))
#define SETTILED(C) (removeflag(C, Floating))

#define LOCK(C) (addflag(C, Locked))
#define UNLOCK(C) (removeflag(C, Locked))