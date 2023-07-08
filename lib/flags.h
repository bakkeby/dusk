static void addflag(Client *c, const uint64_t flag);
static void setflag(Client *c, const uint64_t flag, const int value);
static void removeflag(Client *c, const uint64_t flag);
static void toggleflag(Client *c, const uint64_t flag);
static void toggleclientflag(const Arg *arg);
static void toggleflagop(Client *c, const uint64_t flag, int op);
static const uint64_t getflagbyname(const char *name);

static const uint64_t
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
	Sticky = 0x800, // indicates that the client is sticky (always shown)
	Terminal = 0x1000, // indicates that the client is a terminal, used by swallow
	NoSwallow = 0x2000, // indicates that the client should never swallow another window
	Locked = 0x4000, // used by setfullscreen, prevents state change
	Transient = 0x8000, // whether the client has the transient for hint
	OnlyModButtons = 0x10000, // restricts button keybindings to those that involve modifiers
	Disallowed = 0x20000, // window is disallowed and will be forcibly killed
	/* 0x40000, 0x80000, 0x100000 - three bits for attach modes as they are mutually exclusive, still room for two more up to 7 */
	AttachMaster = 1 << 18, // attach at the top of the master area
	AttachAbove = 2 << 18, // attach the client before the currently selected client
	AttachBelow = 3 << 18, // attach the client after the currently selected client
	AttachAside = 4 << 18, // attach the client as the first client in the stack area
	AttachBottom = 5 << 18, // attach the client at the end of the list
	AttachFlag = 7 << 18, // used internally to test for the attach flags above
	FlagPlaceholder0x200000 = 0x200000,
	FlagPlaceholder0x400000 = 0x400000,
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
	SemiScratchpad = 0x800000000, // a window that switches between being a normal window and a scratchpad window
	RespectSizeHints = 0x1000000000, // respect size hints for this client when ResizeHints is globally disabled
	RioDrawNoMatchPID = 0x2000000000, // do not match PID for this client when spawning via riospawn
	NoBorder = 0x4000000000, // indicates that the client should not be drawn with a border around it
	SteamGame = 0x8000000000,
	NoFocusOnNetActive = 0x10000000000, // do not allow focus on net active for this client
	ScratchpadStayOnMon = 0x20000000000, // prevents the scratchpad from being moved to the active monitor when toggled
	Lower = 0x40000000000, // place this window below all other windows, used when window is unmanaged
	Raise = 0x80000000000, // place this window above all other windows, used when window is unmanaged
	SkipTaskbar = 0x100000000000, // do not include the window title for this client on the bar
	ReapplyRules = 0x200000000000, // allow the client to get rules re-applied once when the window title changes
	CfgReqPosRelativeToMonitor = 0x400000000000, // makes configure requests relative to the client's monitor
	SwallowRetainSize = 0x800000000000, // allows for a client to retain its height and width when swallowed or unswallowed
	NoWarp = 0x1000000000000, // disallow cursor to warp to this client
	FlagPlaceholder0x2000000000000 = 0x2000000000000,
	FlagPlaceholder0x4000000000000 = 0x4000000000000,
	FlagPlaceholder0x8000000000000 = 0x8000000000000,
	FlagPlaceholder0x10000000000000 = 0x10000000000000,
	FlagPlaceholder0x20000000000000 = 0x20000000000000,
	FlagPlaceholder0x40000000000000 = 0x40000000000000,
	/* Below are flags that are intended to only be used internally */
	RefreshSizeHints = 0x80000000000000, // used internally to indicate that size hints for the window should be (re-)loaded
	/* Debug = 0x100000000000000,  // same name and value as debug functionality, see util.h */
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
	Unmanaged = 0x8000000000000000; // indicates that the client is not to be managed by the window manager

#define ALWAYSONTOP(C) (C && C->flags & AlwaysOnTop)
#define CFGREQPOSRELATIVETOMONITOR(C) (C && C->flags & CfgReqPosRelativeToMonitor)
#define DEBUGGING(C) (C && C->flags & Debug)
#define DISALLOWED(C) (C && C->flags & Disallowed)
#define HIDDEN(C) (C && ((C->flags & Hidden) || (getstate(C->win) == IconicState)))
#define ISFIXED(C) (C && C->flags & Fixed)
#define ISFLOATING(C) (C && C->flags & (Floating|Sticky))
#define ISLOCKED(C) (C && C->flags & Locked)
#define ISSTICKY(C) (C && C->flags & Sticky)
#define ISCENTERED(C) (C && C->flags & Centered)
#define ISFULLSCREEN(C) (C && C->flags & FullScreen)
#define ISFAKEFULLSCREEN(C) (C && C->flags & FakeFullScreen)
#define ISPERMANENT(C) (C && C->flags & Permanent)
#define ISTERMINAL(C) (C && C->flags & Terminal)
#define ISTRANSIENT(C) (C && C->flags & Transient)
#define ISURGENT(C) (C && C->flags & Urgent)
#define ISMARKED(C) (C && C->flags & Marked)
#define ISSCRATCHPAD(C) (C && C->scratchkey != 0)
#define ISUNMANAGED(C) (C && C->flags & Unmanaged)
#define ISVISIBLE(C) (C && C->ws->visible && C->win && !(C->flags & (Invisible|Hidden)))
#define ISINVISIBLE(C) (C && (!C->win || C->flags & Invisible))
#define IGNORECFGREQ(C) (C && C->flags & IgnoreCfgReq)
#define IGNORECFGREQPOS(C) (C && C->flags & IgnoreCfgReqPos)
#define IGNORECFGREQSIZE(C) (C && C->flags & IgnoreCfgReqSize)
#define IGNOREDECORATIONHINTS(C) (C && C->flags & IgnoreDecorationHints)
#define IGNOREPROPTRANSIENTFOR(C) (C && C->flags & IgnorePropTransientFor)
#define IGNORESIZEHINTS(C) (C && C->flags & IgnoreSizeHints)
#define IGNOREMINIMUMSIZEHINTS(C) (C && C->flags & IgnoreMinimumSizeHints)
#define REFRESHSIZEHINTS(C) (C && C->flags & RefreshSizeHints)
#define NEEDRESIZE(C) (C && C->flags & NeedResize)
#define NEVERFOCUS(C) (C && C->flags & NeverFocus)
#define NOBORDER(C) (C && C->flags & NoBorder)
#define NOFOCUSONNETACTIVE(C) (C && C->flags & NoFocusOnNetActive)
#define NOSWALLOW(C) (C && C->flags & NoSwallow)
#define NOWARP(C) (C && C->flags & NoWarp)
#define ONLYMODBUTTONS(C) (C && C->flags & OnlyModButtons)
#define REAPPLYRULES(C) (C && C->flags & ReapplyRules)
#define RESPECTSIZEHINTS(C) (C && C->flags & RespectSizeHints)
#define RESTOREFAKEFULLSCREEN(C) (C && C->flags & RestoreFakeFullScreen)
#define RIODRAWNOMATCHPID(C) (C && C->flags & RioDrawNoMatchPID)
#define RULED(C) (C && C->flags & Ruled)
#define SCRATCHPADSTAYONMON(C) (C && C->flags & ScratchpadStayOnMon)
#define SEMISCRATCHPAD(C) (C && C->flags & SemiScratchpad)
#define SKIPTASKBAR(C) (C && C->flags & SkipTaskbar)
#define STEAMGAME(C) (C && C->flags & SteamGame)
#define SWALLOWRETAINSIZE(C) (C && C->flags & SwallowRetainSize)
#define SWITCHWORKSPACE(C) (C && C->flags & SwitchWorkspace)
#define ENABLEWORKSPACE(C) (C && C->flags & EnableWorkspace)
#define REVERTWORKSPACE(C) (C && C->flags & RevertWorkspace)
#define MOVERESIZE(C) (C && C->flags & MoveResize)
#define MOVEPLACE(C) (C && C->flags & MovePlace)
#define LOWER(C) (C && C->flags & Lower)
#define RAISE(C) (C && C->flags & Raise)
#define TILED(C) (C && C->win && !(C->flags & (Invisible|Hidden|Floating|Sticky)))

#define WASFLOATING(C) (C && C->prevflags & Floating)
#define WASFAKEFULLSCREEN(C) (C && C->prevflags & FakeFullScreen)
#define WASFULLSCREEN(C) (C && C->prevflags & FullScreen)

#define SETFLOATING(C) (addflag(C, Floating))
#define SETFULLSCREEN(C) (addflag(C, FullScreen))
#define SETTILED(C) (removeflag(C, Floating))

#define LOCK(C) (addflag(C, Locked))
#define UNLOCK(C) (removeflag(C, Locked))
