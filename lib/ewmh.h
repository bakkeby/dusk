static int atomin(Atom input, Atom *list, int nitems);
static void getclientflags(Client *c);
static void getclientfields(Client *c);
static void getclienticonpath(Client *c);
static void getclientlabel(Client *c);
static void getworkspacestate(Workspace *ws);
static void persistworkspacestate(Workspace *ws);
static void persistpids(void);
static void restorepids(void);
static void savewindowfloatposition(Client *c, Monitor *m);
static int restorewindowfloatposition(Client *c, Monitor *m);
static void setdesktopnames(void);
static void setfloatinghint(Client *c);
static void setfloatinghints(Workspace *ws);
static void setclientflags(Client *c);
static void setclientfields(Client *c);
static void setclienticonpath(Client *c);
static void setclientlabel(Client *c);
static void setnumdesktops(void);
static void setviewport(void);
static void updatecurrentdesktop(void);
static void updateclientdesktop(Client *c);

enum {
	NetActiveWindow,
	NetClientList,
	NetClientListStacking,
	NetCloseWindow,
	NetCurrentDesktop,
	NetDesktopNames,
	NetDesktopViewport,
	NetNumberOfDesktops,
	NetSupported,
	NetSystemTray,
	NetSystemTrayOP,
	NetSystemTrayOrientation,
	NetSystemTrayOrientationHorz,
	NetSystemTrayVisual,
	NetWMAllowedActions,
	NetWMCheck,
	NetWMDemandsAttention,
	NetWMDesktop,
	NetWMFullPlacement,
	NetWMFullscreen,
	NetWMHidden,
	NetWMIcon,
	NetWMName,
	NetWMState,
	NetWMMaximizedVert,
	NetWMMaximizedHorz,
	NetWMSkipTaskbar,
	NetWMStaysOnTop,
	NetWMSticky,
	NetWMWindowOpacity,
	NetWMWindowType,
	NetWMWindowTypeDock,
	NetWMMoveResize,
	NetWMUserTime,
	NetLast
}; /* EWMH atoms */

static char *netatom_names[NetLast] = {
	[NetActiveWindow] = "_NET_ACTIVE_WINDOW",
	[NetClientList] = "_NET_CLIENT_LIST",
	[NetClientListStacking] = "_NET_CLIENT_LIST_STACKING",
	[NetCloseWindow] = "_NET_CLOSE_WINDOW",
	[NetCurrentDesktop] = "_NET_CURRENT_DESKTOP",
	[NetDesktopNames] = "_NET_DESKTOP_NAMES",
	[NetDesktopViewport] = "_NET_DESKTOP_VIEWPORT",
	[NetNumberOfDesktops] = "_NET_NUMBER_OF_DESKTOPS",
	[NetSupported] = "_NET_SUPPORTED",
	[NetSystemTray] = "_NET_SYSTEM_TRAY_S0",
	[NetSystemTrayOP] = "_NET_SYSTEM_TRAY_OPCODE",
	[NetSystemTrayOrientation] = "_NET_SYSTEM_TRAY_ORIENTATION",
	[NetSystemTrayOrientationHorz] = "_NET_SYSTEM_TRAY_ORIENTATION_HORZ",
	[NetSystemTrayVisual] = "_NET_SYSTEM_TRAY_VISUAL",
	[NetWMAllowedActions] = "_NET_WM_ALLOWED_ACTIONS",
	[NetWMCheck] = "_NET_SUPPORTING_WM_CHECK",
	[NetWMDemandsAttention] = "_NET_WM_STATE_DEMANDS_ATTENTION",
	[NetWMDesktop] = "_NET_WM_DESKTOP",
	[NetWMFullPlacement] = "_NET_WM_FULL_PLACEMENT",
	[NetWMFullscreen] = "_NET_WM_STATE_FULLSCREEN",
	[NetWMHidden] = "_NET_WM_STATE_HIDDEN",
	[NetWMIcon] = "_NET_WM_ICON",
	[NetWMName] = "_NET_WM_NAME",
	[NetWMState] = "_NET_WM_STATE",
	[NetWMMaximizedVert] = "_NET_WM_STATE_MAXIMIZED_VERT",
	[NetWMMaximizedHorz] = "_NET_WM_STATE_MAXIMIZED_HORZ",
	[NetWMSkipTaskbar] = "_NET_WM_STATE_SKIP_TASKBAR",
	[NetWMStaysOnTop] = "_NET_WM_STATE_STAYS_ON_TOP",
	[NetWMSticky] = "_NET_WM_STATE_STICKY",
	[NetWMWindowOpacity] = "_NET_WM_WINDOW_OPACITY",
	[NetWMWindowType] = "_NET_WM_WINDOW_TYPE",
	[NetWMWindowTypeDock] = "_NET_WM_WINDOW_TYPE_DOCK",
	[NetWMMoveResize] = "_NET_WM_MOVERESIZE",
	[NetWMUserTime] = "_NET_WM_USER_TIME",
};

enum {
	NetWMActionMove,
	NetWMActionResize,
	NetWMActionMinimize,
	NetWMActionShade,
	NetWMActionStick,
	NetWMActionMaximizeHorz,
	NetWMActionMaximizeVert,
	NetWMActionFullscreen,
	NetWMActionChangeDesktop,
	NetWMActionClose,
	NetWMActionAbove,
	NetWMActionBelow,
	NetWMActionLast
}; /* _NET_WM_ALLOWED_ACTIONS */

static char *allowed_names[NetWMActionLast] = {
	[NetWMActionMove] = "_NET_WM_ACTION_MOVE",
	[NetWMActionResize] = "_NET_WM_ACTION_RESIZE",
	[NetWMActionMinimize] = "_NET_WM_ACTION_MINIMIZE",
	[NetWMActionShade] = "_NET_WM_ACTION_SHADE",
	[NetWMActionStick] = "_NET_WM_ACTION_STICK",
	[NetWMActionMaximizeHorz] = "_NET_WM_ACTION_MAXIMIZE_HORZ",
	[NetWMActionMaximizeVert] = "_NET_WM_ACTION_MAXIMIZE_VERT",
	[NetWMActionFullscreen] = "_NET_WM_ACTION_FULLSCREEN",
	[NetWMActionChangeDesktop] = "_NET_WM_ACTION_CHANGE_DESKTOP",
	[NetWMActionClose] = "_NET_WM_ACTION_CLOSE",
	[NetWMActionAbove] = "_NET_WM_ACTION_ABOVE",
	[NetWMActionBelow] = "_NET_WM_ACTION_BELOW",
};

enum {
	WMChangeState,
	WMClass,
	WMDelete,
	WMProtocols,
	WMState,
	WMTakeFocus,
	WMWindowRole,
	WMLast
}; /* default atoms */

static char *wmatom_names[WMLast] = {
	[WMChangeState] = "WM_CHANGE_STATE",
	[WMClass] = "WM_CLASS",
	[WMDelete] = "WM_DELETE_WINDOW",
	[WMProtocols] = "WM_PROTOCOLS",
	[WMState] = "WM_STATE",
	[WMTakeFocus] = "WM_TAKE_FOCUS",
	[WMWindowRole] = "WM_WINDOW_ROLE",
};

enum {
	IsFloating,
	DuskAutostartCount,
	DuskAutostartPIDs,
	DuskClientFlags,
	DuskClientFields,
	DuskClientLabel,
	DuskClientIconPath,
	DuskWorkspace,
	SteamGameID,
	DuskLast
}; /* dusk atoms */

static char *dusk_names[DuskLast] = {
	[IsFloating] = "_IS_FLOATING",
	[DuskAutostartCount] = "_DUSK_AUTOSTART_COUNT",
	[DuskAutostartPIDs] = "_DUSK_AUTOSTART_PIDS",
	[DuskClientFlags] = "_DUSK_CLIENT_FLAGS",
	[DuskClientFields] = "_DUSK_CLIENT_FIELDS",
	[DuskClientLabel] = "_DUSK_CLIENT_LABEL",
	[DuskClientIconPath] = "_DUSK_CLIENT_ICON_PATH",
	[DuskWorkspace] = "_DUSK_WORKSPACE",
	[SteamGameID] = "STEAM_GAME",
};

enum {
	Manager,
	Xembed,
	XembedInfo,
	XLast
}; /* Xembed atoms */

static char *xembed_names[XLast] = {
	[Manager] = "MANAGER",
	[Xembed] = "_XEMBED",
	[XembedInfo] = "_XEMBED_INFO",
};

/* https://specifications.freedesktop.org/wm-spec/latest/ar01s05.html - Application Window Properties */
