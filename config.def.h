/* See LICENSE file for copyright and license details. */

/* appearance */
static const unsigned int borderpx       = 5;   /* border pixel of windows */
static const unsigned int snap           = 32;  /* snap pixel */
static const unsigned int gappih         = 5;   /* horiz inner gap between windows */
static const unsigned int gappiv         = 5;   /* vert inner gap between windows */
static const unsigned int gappoh         = 5;   /* horiz outer gap between windows and screen edge */
static const unsigned int gappov         = 5;   /* vert outer gap between windows and screen edge */
static const unsigned int smartgaps_fact = 0;   /* smartgaps factor when there is only one client; 0 = no gaps, 3 = 3x outer gaps */

static unsigned int attachdefault        = AttachAside; // AttachMaster, AttachAbove, AttachSide, AttachBelow, AttachBottom

static const int showbar                 = 1;   /* 0 means no bar */
static const int topbar                  = 1;   /* 0 means bottom bar */

static const int bar_height              = 0;   /* 0 means derive from font, >= 1 explicit height */
static const int vertpad                 = borderpx;  /* vertical (outer) padding of bar */
static const int sidepad                 = borderpx;  /* horizontal (outer) padding of bar */

static int floatposgrid_x                = 5;   /* float grid columns */
static int floatposgrid_y                = 5;   /* float grid rows */

static const int horizpadbar             = 2;   /* horizontal (inner) padding for statusbar (increases lrpad) */
static const int vertpadbar              = 0;   /* vertical (inner) padding for statusbar (increases bh, overridden by bar_height) */

static const char buttonbar[]            = "⛶";
static const unsigned int systrayspacing = 2;   /* systray spacing */
static char *toggle_float_pos            = "50% 50% 80% 80%"; // default floating position when triggering togglefloatpos
static const double defaultopacity       = 0;   /* client default opacity, e.g. 0.75. 0 means don't apply opacity. */
static const double moveresizeopacity    = 0;   /* client opacity when being moved or resized, 0 means don't apply opacity */

/* Indicators: see lib/bar_indicators.h for options */
static int wsindicatortype               = INDICATOR_BOTTOM_BAR_SLIM;
static int wspinnedindicatortype         = INDICATOR_TOP_RIGHT_PIN;
static int fakefsindicatortype           = INDICATOR_PLUS;
static int floatfakefsindicatortype      = INDICATOR_PLUS_AND_LARGER_SQUARE;
static int floatindicatortype            = INDICATOR_TOP_LEFT_LARGER_SQUARE;
static int tiledindicatortype            = INDICATOR_NONE;

/* See util.h for options */
static unsigned long functionality = 0
//	|SmartGaps // enables no gaps if there is only one visible window
//	|SmartGapsMonocle // enforces no (outer) gaps in monocle layout
	|Systray // enables systray
	|Swallow // allows terminals to swallow X applications started from the command line
	|SwallowFloating // means swallow floating windows by default
	|CenteredWindowName
//	|BarActiveGroupBorderColor // use border color of active group, otherwise color for master group is used
	|SpawnCwd // spawn applications in the currently selected client's working directory
	|ColorEmoji
//	|Status2DNoAlpha // option to not use alpha when drawing status2d status
	|BarBorder // draw a border around the bar
//	|NoBorders // as per the noborder patch, show no border when only one client in tiled mode
//	|Warp // warp patch
//	|FocusedOnTop
//	|DecorationHints // used by setfullscreen, prevents state change
	|FocusOnNetActive
	|AllowNoModifierButtons
	|CenterSizeHintsClients // center tiled clients subject to size hints within their tiled area
//	|ResizeHints // if enabled then dusk will respect size hints in tiled resizals
//	|SortScreens // only applies on startup
//	|ViewOnWs // follow a window to the workspace it is being moved to
//	|Xresources // xrdb patch
//	|AutoSaveFloats // auto save float posistion when using movemouse or resizemouse
//	|Debug
//	|AltWorkspaceIcons // show the workspace name instead of the icons
//	|GreedyMonitor // when viewing a workspace the monitor is greedy and gives nothing in return (i.e. disables swap of workspaces)
	|SmartLayoutConvertion // when moving a workspace from one monitor to another, automatically adjust layout based on monitor orientation (i.e. vertical vs horizontal)
//	|AutoHideScratchpads // automatically hide open scratchpads when moving to another workspace
;

static const char statussep              = ';'; /* separator between status bars */

#define FLEXWINTITLE_MASTERWEIGHT 15   // master weight compared to hidden and floating window titles
#define FLEXWINTITLE_STACKWEIGHT 4     // stack weight compared to hidden and floating window titles
#define FLEXWINTITLE_HIDDENWEIGHT 0    // hidden window title weight
#define FLEXWINTITLE_FLOATWEIGHT 0     // floating window title weight, set to 0 to not show floating windows

static const char *fonts[]               = { "monospace:size=10" };
static const char dmenufont[]            = "monospace:size=10";

static char normfgcolor[]                = "#C6BDBD";
static char normbgcolor[]                = "#180A13";
static char normbordercolor[]            = "#444444";

static char selfgcolor[]                 = "#FFF7D4";
static char selbgcolor[]                 = "#440000";
static char selbordercolor[]             = "#440000";

static char titlenormfgcolor[]           = "#C6BDBD";
static char titlenormbgcolor[]           = "#180A13";
static char titlenormbordercolor[]       = "#440000";

static char titleselfgcolor[]            = "#FFF7D4";
static char titleselbgcolor[]            = "#440000";
static char titleselbordercolor[]        = "#440000";

static char wsnormfgcolor[]              = "#C6BDBD";
static char wsnormbgcolor[]              = "#180A13";
static char wsvisfgcolor[]               = "#FFF7D4";
static char wsvisbgcolor[]               = "#440000";
static char wsselfgcolor[]               = "#FFF7D4";
static char wsselbgcolor[]               = "#550000";

static char hidfgcolor[]                 = "#c278b6";
static char hidbgcolor[]                 = "#222222";
static char hidbordercolor[]             = "#440000";

static char urgfgcolor[]                 = "#bbbbbb";
static char urgbgcolor[]                 = "#222222";
static char urgbordercolor[]             = "#d10f3f";

static char markedfgcolor[]              = "#615656";
static char markedbgcolor[]              = "#ECB820";
static char markedbordercolor[]          = "#ECB820";

static char scratchnormcolor[]           = "#C6BDBD";
static char scratchnormbgcolor[]         = "#180A13";
static char scratchnormbordercolor[]     = "#77547E";

static char scratchselcolor[]            = "#FFF7D4";
static char scratchselbgcolor[]          = "#440000";
static char scratchselbordercolor[]      = "#894B9F";

static char normTTBbgcolor[]             = "#330000";
static char normLTRbgcolor[]             = "#330033";
static char normMONObgcolor[]            = "#000033";
static char normGRIDbgcolor[]            = "#003300";
static char normGRD1bgcolor[]            = "#003300";
static char normGRD2bgcolor[]            = "#003300";
static char normGRDMbgcolor[]            = "#506600";
static char normHGRDbgcolor[]            = "#b96600";
static char normDWDLbgcolor[]            = "#003333";
static char normSPRLbgcolor[]            = "#333300";
static char normfloatbgcolor[]           = "#4C314C";
static char actTTBbgcolor[]              = "#440000";
static char actLTRbgcolor[]              = "#440044";
static char actMONObgcolor[]             = "#000044";
static char actGRIDbgcolor[]             = "#004400";
static char actGRD1bgcolor[]             = "#004400";
static char actGRD2bgcolor[]             = "#004400";
static char actGRDMbgcolor[]             = "#507711";
static char actHGRDbgcolor[]             = "#b97711";
static char actDWDLbgcolor[]             = "#004444";
static char actSPRLbgcolor[]             = "#444400";
static char actfloatbgcolor[]            = "#4C314C";
static char selTTBbgcolor[]              = "#550000";
static char selLTRbgcolor[]              = "#550055";
static char selMONObgcolor[]             = "#212171";
static char selGRIDbgcolor[]             = "#005500";
static char selGRD1bgcolor[]             = "#005500";
static char selGRD2bgcolor[]             = "#005500";
static char selGRDMbgcolor[]             = "#508822";
static char selHGRDbgcolor[]             = "#b98822";
static char selDWDLbgcolor[]             = "#005555";
static char selSPRLbgcolor[]             = "#555500";
static char selfloatbgcolor[]            = "#5C415C";

static const unsigned int baralpha = 0xc0;
static const unsigned int borderalpha = OPAQUE;
static const unsigned int alphas[][3] = {
	/*                       fg      bg        border     */
	[SchemeNorm]         = { OPAQUE, baralpha, borderalpha },
	[SchemeSel]          = { OPAQUE, baralpha, borderalpha },
	[SchemeTitleNorm]    = { OPAQUE, baralpha, borderalpha },
	[SchemeTitleSel]     = { OPAQUE, baralpha, borderalpha },
	[SchemeWsNorm]       = { OPAQUE, baralpha, borderalpha },
	[SchemeWsVisible]    = { OPAQUE, baralpha, borderalpha },
	[SchemeWsSel]        = { OPAQUE, baralpha, borderalpha },
	[SchemeHid]          = { OPAQUE, baralpha, borderalpha },
	[SchemeUrg]          = { OPAQUE, baralpha, borderalpha },
	[SchemeMarked]       = { OPAQUE, baralpha, borderalpha },
	[SchemeScratchSel]   = { OPAQUE, baralpha, borderalpha },
	[SchemeScratchNorm]  = { OPAQUE, baralpha, borderalpha },
	[SchemeFlexActTTB]   = { OPAQUE, baralpha, borderalpha },
	[SchemeFlexActLTR]   = { OPAQUE, baralpha, borderalpha },
	[SchemeFlexActMONO]  = { OPAQUE, baralpha, borderalpha },
	[SchemeFlexActGRID]  = { OPAQUE, baralpha, borderalpha },
	[SchemeFlexActGRD1]  = { OPAQUE, baralpha, borderalpha },
	[SchemeFlexActGRD2]  = { OPAQUE, baralpha, borderalpha },
	[SchemeFlexActGRDM]  = { OPAQUE, baralpha, borderalpha },
	[SchemeFlexActHGRD]  = { OPAQUE, baralpha, borderalpha },
	[SchemeFlexActDWDL]  = { OPAQUE, baralpha, borderalpha },
	[SchemeFlexActSPRL]  = { OPAQUE, baralpha, borderalpha },
	[SchemeFlexActFloat] = { OPAQUE, baralpha, borderalpha },
	[SchemeFlexInaTTB]   = { OPAQUE, baralpha, borderalpha },
	[SchemeFlexInaLTR]   = { OPAQUE, baralpha, borderalpha },
	[SchemeFlexInaMONO]  = { OPAQUE, baralpha, borderalpha },
	[SchemeFlexInaGRID]  = { OPAQUE, baralpha, borderalpha },
	[SchemeFlexInaGRD1]  = { OPAQUE, baralpha, borderalpha },
	[SchemeFlexInaGRD2]  = { OPAQUE, baralpha, borderalpha },
	[SchemeFlexInaGRDM]  = { OPAQUE, baralpha, borderalpha },
	[SchemeFlexInaHGRD]  = { OPAQUE, baralpha, borderalpha },
	[SchemeFlexInaDWDL]  = { OPAQUE, baralpha, borderalpha },
	[SchemeFlexInaSPRL]  = { OPAQUE, baralpha, borderalpha },
	[SchemeFlexInaFloat] = { OPAQUE, baralpha, borderalpha },
	[SchemeFlexSelTTB]   = { OPAQUE, baralpha, borderalpha },
	[SchemeFlexSelLTR]   = { OPAQUE, baralpha, borderalpha },
	[SchemeFlexSelMONO]  = { OPAQUE, baralpha, borderalpha },
	[SchemeFlexSelGRID]  = { OPAQUE, baralpha, borderalpha },
	[SchemeFlexSelGRD1]  = { OPAQUE, baralpha, borderalpha },
	[SchemeFlexSelGRD2]  = { OPAQUE, baralpha, borderalpha },
	[SchemeFlexSelGRDM]  = { OPAQUE, baralpha, borderalpha },
	[SchemeFlexSelHGRD]  = { OPAQUE, baralpha, borderalpha },
	[SchemeFlexSelDWDL]  = { OPAQUE, baralpha, borderalpha },
	[SchemeFlexSelSPRL]  = { OPAQUE, baralpha, borderalpha },
	[SchemeFlexSelFloat] = { OPAQUE, baralpha, borderalpha },
};

static char *colors[][ColCount] = {
	/*                       fg                bg                  border                  */
	[SchemeNorm]         = { normfgcolor,      normbgcolor,        normbordercolor,        },
	[SchemeSel]          = { selfgcolor,       selbgcolor,         selbordercolor,         },
	[SchemeTitleNorm]    = { titlenormfgcolor, titlenormbgcolor,                           },
	[SchemeTitleSel]     = { titleselfgcolor,  titleselbgcolor,                            },
	[SchemeWsNorm]       = { wsnormfgcolor,    wsnormbgcolor,                              },
	[SchemeWsVisible]    = { wsvisfgcolor,     wsvisbgcolor,                               },
	[SchemeWsSel]        = { wsselfgcolor,     wsselbgcolor,                               },
	[SchemeHid]          = { hidfgcolor,       hidbgcolor,         hidbordercolor,         },
	[SchemeUrg]          = { urgfgcolor,       urgbgcolor,         urgbordercolor,         },
	[SchemeMarked]       = { markedfgcolor,    markedbgcolor,      markedbordercolor,      },
	[SchemeScratchNorm]  = { scratchnormcolor, scratchnormbgcolor, scratchnormbordercolor, },
	[SchemeScratchSel]   = { scratchselcolor,  scratchselbgcolor,  scratchselbordercolor,  },
	[SchemeFlexActTTB]   = { titleselfgcolor,  actTTBbgcolor,      actTTBbgcolor,          },
	[SchemeFlexActLTR]   = { titleselfgcolor,  actLTRbgcolor,      actLTRbgcolor,          },
	[SchemeFlexActMONO]  = { titleselfgcolor,  actMONObgcolor,     actMONObgcolor,         },
	[SchemeFlexActGRID]  = { titleselfgcolor,  actGRIDbgcolor,     actGRIDbgcolor,         },
	[SchemeFlexActGRD1]  = { titleselfgcolor,  actGRD1bgcolor,     actGRD1bgcolor,         },
	[SchemeFlexActGRD2]  = { titleselfgcolor,  actGRD2bgcolor,     actGRD2bgcolor,         },
	[SchemeFlexActGRDM]  = { titleselfgcolor,  actGRDMbgcolor,     actGRDMbgcolor,         },
	[SchemeFlexActHGRD]  = { titleselfgcolor,  actHGRDbgcolor,     actHGRDbgcolor,         },
	[SchemeFlexActDWDL]  = { titleselfgcolor,  actDWDLbgcolor,     actDWDLbgcolor,         },
	[SchemeFlexActSPRL]  = { titleselfgcolor,  actSPRLbgcolor,     actSPRLbgcolor,         },
	[SchemeFlexActFloat] = { titleselfgcolor,  actfloatbgcolor,    actfloatbgcolor,        },
	[SchemeFlexInaTTB]   = { titlenormfgcolor, normTTBbgcolor,     normTTBbgcolor,         },
	[SchemeFlexInaLTR]   = { titlenormfgcolor, normLTRbgcolor,     normLTRbgcolor,         },
	[SchemeFlexInaMONO]  = { titlenormfgcolor, normMONObgcolor,    normMONObgcolor,        },
	[SchemeFlexInaGRID]  = { titlenormfgcolor, normGRIDbgcolor,    normGRIDbgcolor,        },
	[SchemeFlexInaGRD1]  = { titlenormfgcolor, normGRD1bgcolor,    normGRD1bgcolor,        },
	[SchemeFlexInaGRD2]  = { titlenormfgcolor, normGRD2bgcolor,    normGRD2bgcolor,        },
	[SchemeFlexInaGRDM]  = { titlenormfgcolor, normGRDMbgcolor,    normGRDMbgcolor,        },
	[SchemeFlexInaHGRD]  = { titlenormfgcolor, normHGRDbgcolor,    normHGRDbgcolor,        },
	[SchemeFlexInaDWDL]  = { titlenormfgcolor, normDWDLbgcolor,    normDWDLbgcolor,        },
	[SchemeFlexInaSPRL]  = { titlenormfgcolor, normSPRLbgcolor,    normSPRLbgcolor,        },
	[SchemeFlexInaFloat] = { titlenormfgcolor, normfloatbgcolor,   normfloatbgcolor,       },
	[SchemeFlexSelTTB]   = { titleselfgcolor,  selTTBbgcolor,      selTTBbgcolor,          },
	[SchemeFlexSelLTR]   = { titleselfgcolor,  selLTRbgcolor,      selLTRbgcolor,          },
	[SchemeFlexSelMONO]  = { titleselfgcolor,  selMONObgcolor,     selMONObgcolor,         },
	[SchemeFlexSelGRID]  = { titleselfgcolor,  selGRIDbgcolor,     selGRIDbgcolor,         },
	[SchemeFlexSelGRD1]  = { titleselfgcolor,  selGRD1bgcolor,     selGRD1bgcolor,         },
	[SchemeFlexSelGRD2]  = { titleselfgcolor,  selGRD2bgcolor,     selGRD2bgcolor,         },
	[SchemeFlexSelGRDM]  = { titleselfgcolor,  selGRDMbgcolor,     selGRDMbgcolor,         },
	[SchemeFlexSelHGRD]  = { titleselfgcolor,  selHGRDbgcolor,     selHGRDbgcolor,         },
	[SchemeFlexSelDWDL]  = { titleselfgcolor,  selDWDLbgcolor,     selDWDLbgcolor,         },
	[SchemeFlexSelSPRL]  = { titleselfgcolor,  selSPRLbgcolor,     selSPRLbgcolor,         },
	[SchemeFlexSelFloat] = { titleselfgcolor,  selfloatbgcolor,    selfloatbgcolor,        },
};

static const char *const autostart[] = {
	"st", NULL,
	NULL /* terminate */
};

static const char *spcmd1[] = {"w", "st", "-n", "spterm (w)", "-g", "120x34", NULL };
static const char *spcmd2[] = {"e", "st", "-n", "spterm (e)", "-g", "120x34", NULL };
static const char *spcmd3[] = {"r", "st", "-n", "spfm (r)", "-g", "144x41", "-e", "ranger", NULL };

/* There are two options when it comes to per-client rules:
 *  - a traditional struct table or
 *  - using the RULE macro
 *
 * A traditional struct table looks like this:
 *    // class      role      instance  title  wintype  opacity   flags   floatpos   scratchkey   workspace
 *    { "Gimp",     NULL,     NULL,     NULL,  NULL,    0,        0,      NULL,      NULL,        "4"        },
 *    { "Firefox",  NULL,     NULL,     NULL,  NULL,    0,        0,      NULL,      NULL,        "9"        },
 *
 * The RULE macro has the default values set for each field allowing you to only
 * specify the values that are relevant for your rule, e.g.
 *
 *    RULE(.class = "Gimp", .workspace = "5")
 *    RULE(.class = "Firefox", .workspace = "9")
 *
 * Refer to the Rule struct definition for the list of available fields.
 */
static const Rule clientrules[] = {
	/* xprop(1):
	 *	WM_CLASS(STRING) = instance, class
	 *	WM_NAME(STRING) = title
	 *	WM_WINDOW_ROLE(STRING) = role
	 *	_NET_WM_WINDOW_TYPE(ATOM) = wintype
	 */
	RULE(.wintype = WTYPE "DIALOG", .flags = AlwaysOnTop|Centered|Floating)
	RULE(.wintype = WTYPE "UTILITY", .flags = AlwaysOnTop|Centered|Floating)
	RULE(.wintype = WTYPE "TOOLBAR", .flags = AlwaysOnTop|Centered|Floating)
	RULE(.wintype = WTYPE "SPLASH", .flags = AlwaysOnTop|Centered|Floating)
	RULE(.instance = "spterm (w)", .scratchkey = 'w', .flags = Floating)
	RULE(.instance = "spterm (e)", .scratchkey = 'e', .flags = Floating)
	RULE(.instance = "spfm (r)", .scratchkey = 'r', .flags = Floating)
	RULE(.class = "Gimp", .workspace = "5", .flags = Floating|SwitchWorkspace)
	RULE(.class = "firefox", .workspace = "8", .flags = AttachMaster|SwitchWorkspace)
	RULE(.class = "Steam", .flags = IgnoreCfgReqPos|Floating|Centered)
	RULE(.class = "steam_app_", .flags = IgnoreCfgReqPos|Floating|Centered)
	RULE(.class = "Google-chrome", .role = "GtkFileChooserDialog", .floatpos = "50% 50%", .flags = AlwaysOnTop|Floating)
	RULE(.role = "pop-up", .flags = AlwaysOnTop|Floating|Centered)
	RULE(.role = "browser", .workspace = "8", .flags = AttachBelow|OnlyModButtons|SwitchWorkspace)
	RULE(.class = "Gnome-terminal", .role = "gnome-terminal-preferences", .flags = Centered)
	RULE(.class = "Diffuse", .workspace = "4", .flags = NoSwallow|SwitchWorkspace|RevertWorkspace)
	RULE(.class = "File-roller", .workspace = "9", .flags = Centered|Floating|SwitchWorkspace|RevertWorkspace)
	RULE(.class = "Alacritty", .flags = Terminal|IgnoreCfgReqPos|IgnoreCfgReqSize)
	RULE(.class = "st-256color", .flags = Terminal|AttachBottom)
	RULE(.class = "XTerm", .flags = Terminal)
	RULE(.class = "Xephyr", .flags = NoSwallow|Floating|Centered)
	RULE(.title = "Event Tester", .flags = NoSwallow)
};

/* Bar rules allow you to configure what is shown where on the bar, as well as
 * introducing your own bar modules.
 *
 *    monitor:
 *      -1  show on all monitors
 *       0  show on monitor 0
 *      'A' show on active monitor (i.e. focused / selected) (or just -1 for active?)
 *    bar - bar index, 0 is default, 1 is extrabar
 *    alignment - how the module is aligned compared to other modules
 *    widthfunc, drawfunc, clickfunc - providing bar module width, draw and click functions
 *    name - does nothing, intended for visual clue and for logging / debugging
 */
static const BarRule barrules[] = {
	/* monitor  bar    alignment               widthfunc                 drawfunc                 clickfunc                 name */
	{ -1,       0,     BAR_ALIGN_LEFT,         width_stbutton,           draw_stbutton,           click_stbutton,           "statusbutton" },
	{ -1,       0,     BAR_ALIGN_LEFT,         width_workspaces,         draw_workspaces,         click_workspaces,         "workspaces" },
	{ 'A',      0,     BAR_ALIGN_RIGHT,        width_systray,            draw_systray,            click_systray,            "systray" },
	{ -1,       0,     BAR_ALIGN_LEFT,         width_ltsymbol,           draw_ltsymbol,           click_ltsymbol,           "layout" },
	{ 'A',      0,     BAR_ALIGN_RIGHT,        width_status2d,           draw_status2d,           click_statuscmd,          "status2d" },
	{ -1,       0,     BAR_ALIGN_NONE,         width_flexwintitle,       draw_flexwintitle,       click_flexwintitle,       "flexwintitle" },
	{ 'A',      1,     BAR_ALIGN_CENTER,       width_status2d_es,        draw_status2d_es,        click_statuscmd_es,       "status2d_es" },
	{ -1,       1,     BAR_ALIGN_RIGHT_RIGHT,  width_wintitle_hidden,    draw_wintitle_hidden,    click_wintitle_hidden,    "wintitle_hidden" },
	{ -1,       1,     BAR_ALIGN_LEFT,         width_wintitle_floating,  draw_wintitle_floating,  click_wintitle_floating,  "wintitle_floating" },
};

/* Workspace rules define what workspaces are available and their properties.
 *
 *    name     - the name of the workspace, this is a reference used for keybindings - see WSKEYS
 *    monitor  - the monitor number the workspace starts on by default, -1 means assign freely
 *    pinned   - whether the workspace is pinned on the assigned monitor
 *    layout   - the layout index the workspace should start with, refer to the layouts array
 *    mfact    - factor of master area size, -1 means use global config
 *    nmaster  - number of clients in master area, -1 means use global config
 *    nstack   - number of clients in primary stack area, -1 means use global config
 *    gaps     - whether gaps are enabled for the workspace, -1 means use global config
 *
 *    icons:
 *       def   - the default icon shown for the workspace, if empty string then the workspace is
 *               hidden by default, if NULL then the workspace name is used for the icon
 *       vac   - the vacant icon shows if the workspace is selected, the default icon is an empty
 *               string (hidden by default) and the workspace has no clients
 *       occ   - the occupied icon shows if the workspace has clients
 *
 */
static const WorkspaceRule wsrules[] = {
	/*                                                                     ------ icons ------
	   name,  monitor,  pinned,  layout,  mfact,  nmaster,  nstack,  gaps, def,   vac,   occ,  */
	{  "1",   -1,       0,       0,       -1,    -1,       -1,      -1,    "𝟣",   "",   "𝟭", },
	{  "2",   -1,       0,       9,       .80,   -1,       -1,      -1,    "𝟤",   "",   "𝟮", },
	{  "3",   -1,       0,       0,       -1,    -1,       -1,      -1,    "𝟥",   "",   "𝟯", },
	{  "4",   -1,       0,       0,       -1,    -1,       -1,      -1,    "𝟦",   "",   "𝟰", },
	{  "5",   -1,       0,       0,       -1,    -1,       -1,      -1,    "𝟧",   "",   "𝟱", },
	{  "6",   -1,       0,       0,       -1,    -1,       -1,      -1,    "𝟨",   "",   "𝟲", },
	{  "7",   -1,       0,       10,      .75,   -1,       -1,      -1,    "𝟩",   "",   "𝟳", },
	{  "8",   -1,       0,       1,       -1,    -1,       -1,      -1,    "𝟪",   "",   "𝟴", },
	{  "9",   -1,       0,       0,       -1,    -1,       -1,      -1,    "𝟫",   "",   "𝟵", },
};

static const float mfact     = 0.50; /* factor of master area size [0.05..0.95] */
static const int nmaster     = 1;    /* number of clients in master area */
static const int nstack      = 0;    /* number of clients in primary stack area */
static const int enablegaps  = 1;    /* whether gaps are enabled by default or not */

/* layout(s) */
static const Layout layouts[] = {
	/* symbol     arrange function, { nmaster, nstack, layout, master axis, stack axis, secondary stack axis, symbol func } */
	{ "[]=",      flextile,         { -1, -1, SPLIT_VERTICAL, TOP_TO_BOTTOM, TOP_TO_BOTTOM, 0, NULL } }, // default tile layout
	{ "|||",      flextile,         { -1, -1, NO_SPLIT, LEFT_TO_RIGHT, LEFT_TO_RIGHT, 0, NULL } }, // columns
	{ "===",      flextile,         { -1, -1, NO_SPLIT, TOP_TO_BOTTOM, TOP_TO_BOTTOM, 0, NULL } }, // rows
	{ "[M]",      flextile,         { -1, -1, NO_SPLIT, MONOCLE, MONOCLE, 0, NULL } }, // monocle
	{ "||=",      flextile,         { -1, -1, SPLIT_VERTICAL, LEFT_TO_RIGHT, TOP_TO_BOTTOM, 0, NULL } }, // columns (col) layout
	{ ">M>",      flextile,         { -1, -1, FLOATING_MASTER, LEFT_TO_RIGHT, LEFT_TO_RIGHT, 0, NULL } }, // floating master
	{ "[D]",      flextile,         { -1, -1, SPLIT_VERTICAL, TOP_TO_BOTTOM, MONOCLE, 0, NULL } }, // deck
	{ "TTT",      flextile,         { -1, -1, SPLIT_HORIZONTAL, LEFT_TO_RIGHT, LEFT_TO_RIGHT, 0, NULL } }, // bstack
	{ "===",      flextile,         { -1, -1, SPLIT_HORIZONTAL, LEFT_TO_RIGHT, TOP_TO_BOTTOM, 0, NULL } }, // bstackhoriz
	{ "==#",      flextile,         { -1, -1, SPLIT_HORIZONTAL, TOP_TO_BOTTOM, GAPPLESSGRID, 0, NULL } }, // bstackgrid
	{ "|M|",      flextile,         { -1, -1, SPLIT_CENTERED_VERTICAL, LEFT_TO_RIGHT, TOP_TO_BOTTOM, TOP_TO_BOTTOM, NULL } }, // centeredmaster
	{ "-M-",      flextile,         { -1, -1, SPLIT_CENTERED_HORIZONTAL, TOP_TO_BOTTOM, LEFT_TO_RIGHT, LEFT_TO_RIGHT, NULL } }, // centeredmaster horiz
	{ ":::",      flextile,         { -1, -1, NO_SPLIT, GAPPLESSGRID, GAPPLESSGRID, 0, NULL } }, // gappless grid
	{ "[\\]",     flextile,         { -1, -1, NO_SPLIT, DWINDLE, DWINDLE, 0, NULL } }, // fibonacci dwindle
	{ "(@)",      flextile,         { -1, -1, NO_SPLIT, SPIRAL, SPIRAL, 0, NULL } }, // fibonacci spiral
 	{ "><>",      NULL,             {0} },    /* no layout function means floating behavior */
	{ NULL,       NULL,             {0} },    /* end of layouts marker for cyclelayouts */
};

#define Shift ShiftMask
#define Ctrl ControlMask
#define Alt Mod1Mask
#define AltGr Mod3Mask
#define Super Mod4Mask
#define ShiftGr Mod5Mask

/* key definitions */
#define MODKEY Super

#define SCRATCHKEYS(KEY,CMD) \
	{ MODKEY,                      KEY,      togglescratch,     {.v = CMD } }, \
	{ MODKEY|Ctrl,                 KEY,      setscratch,        {.v = CMD } }, \
	{ MODKEY|Ctrl|Shift,           KEY,      removescratch,     {.v = CMD } }, \

#define WSKEYS(KEY,NAME) \
	{ MODKEY,                      KEY,      viewwsbyname,      {.v = NAME} }, \
	{ MODKEY|Shift,                KEY,      movetowsbyname,    {.v = NAME} }, \
	{ MODKEY|Ctrl|Shift,           KEY,      movealltowsbyname, {.v = NAME} }, \
	{ MODKEY|Ctrl,                 KEY,      swapwsbyname,      {.v = NAME} }, \
	{ MODKEY|Alt,                  KEY,      enablewsbyname,    {.v = NAME} }, \

#define STACKKEYS(MOD,ACTION) \
	{ MOD, XK_j, ACTION, {.i = INC(+1) } }, \
	{ MOD, XK_k, ACTION, {.i = INC(-1) } }, \
	{ MOD, XK_s, ACTION, {.i = PREVSEL } }, \
	{ MOD, XK_w, ACTION, {.i = 0 } }, \
	{ MOD, XK_e, ACTION, {.i = 1 } }, \
	{ MOD, XK_a, ACTION, {.i = 2 } }, \
	{ MOD, XK_z, ACTION, {.i = -1 } },

/* commands */
static const char *dmenucmd[] = {
	"dmenu_run",
	"-fn", dmenufont,
	"-nb", normbgcolor,
	"-nf", normfgcolor,
	"-sb", selbgcolor,
	"-sf", selfgcolor,
	topbar ? NULL : "-b",
	NULL
};
static const char *termcmd[]  = { "st", NULL };

static Key keys[] = {
	/* modifier                     key              function                argument */
	{ MODKEY,                       XK_d,            spawn,                  {.v = dmenucmd } }, // spawn dmenu for launching other programs
	{ MODKEY,                       XK_Return,       spawn,                  {.v = termcmd } }, // spawn a terminal
	{ MODKEY,                       XK_b,            togglebar,              {0} }, // toggles the display of the bar(s) on the current monitor

	{ MODKEY,                       XK_j,            focusstack,             {.i = +1 } }, // focus on the next client in the stack
	{ MODKEY,                       XK_k,            focusstack,             {.i = -1 } }, // focus on the previous client in the stack
	{ MODKEY|Alt|Shift,             XK_e,            focusstack,             {.i = +2 } }, // allows focusing on hidden clients
	{ MODKEY|Alt|Shift,             XK_r,            focusstack,             {.i = -2 } }, // allows focusing on hidden clients
	{ MODKEY,                       XK_Left,         focusdir,               {.i = 0 } }, // focus on the client left of the currently focused client
	{ MODKEY,                       XK_Right,        focusdir,               {.i = 1 } }, // focus on the client right of the currently focused client
	{ MODKEY,                       XK_Up,           focusdir,               {.i = 2 } }, // focus on the client above the currently focused client
	{ MODKEY,                       XK_Down,         focusdir,               {.i = 3 } }, // focus on the client below the currently focused client

	{ MODKEY|Ctrl,                  XK_j,            pushdown,               {0} }, // move the selected client down the stack
	{ MODKEY|Ctrl,                  XK_k,            pushup,                 {0} }, // move the selected client up the stack
	{ MODKEY,                       XK_i,            incnmaster,             {.i = +1 } }, // increase the number of clients in the master area
	{ MODKEY,                       XK_u,            incnmaster,             {.i = -1 } }, // decrease the number of clients in the master area
	{ MODKEY|Ctrl,                  XK_i,            incnstack,              {.i = +1 } }, // increase the number of clients in the primary (first) stack area
	{ MODKEY|Ctrl,                  XK_u,            incnstack,              {.i = -1 } }, // increase the number of clients in the primary (first) stack area
	{ MODKEY,                       XK_h,            setmfact,               {.f = -0.05} }, // decrease the size of the master area compared to the stack area(s)
	{ MODKEY,                       XK_l,            setmfact,               {.f = +0.05} }, // increase the size of the master area compared to the stack area(s)
	{ MODKEY|Shift,                 XK_h,            setcfact,               {.f = +0.25} }, // increase size respective to other windows within the same area
	{ MODKEY|Shift,                 XK_l,            setcfact,               {.f = -0.25} }, // decrease client size respective to other windows within the same area
	{ MODKEY|Shift,                 XK_o,            setcfact,               {0} },

	{ MODKEY,                       XK_backslash,    togglepinnedws,         {0} }, // toggle pinning of currently selected workspace on the current monitor
	{ MODKEY,                       XK_z,            showhideclient,         {0} }, // hide the currently selected client (or show if hidden)
	{ MODKEY,                       XK_q,            killclient,             {0} }, // close the currently focused window
	{ MODKEY|Shift,                 XK_q,            quit,                   {1} }, // restart dusk
	{ MODKEY|Ctrl|Alt,              XK_q,            quit,                   {0} }, // exit dusk

	{ MODKEY,                       XK_a,            markall,                {0} }, // marks all clients on the selected workspace
	{ MODKEY|Ctrl,                  XK_a,            markall,                {1} }, // marks all floating clients on the selected workspace
	{ MODKEY|Shift,                 XK_a,            unmarkall,              {0} }, // unmarks all clients
	{ MODKEY,                       XK_m,            togglemark,             {0} }, // marks or unmarks the selected client for group action
	{ MODKEY|Alt,                   XK_m,            zoom,                   {0} }, // moves the currently focused window to/from the master area (for tiled layouts)

	{ MODKEY,                       XK_bracketleft,  rotatelayoutaxis,       {.i = -1 } }, // cycle through the available layout splits (horizontal, vertical, centered, no split, etc.)
	{ MODKEY,                       XK_bracketright, rotatelayoutaxis,       {.i = +1 } }, // cycle through the available layout splits (horizontal, vertical, centered, no split, etc.)
	{ MODKEY|Alt,                   XK_bracketleft,  rotatelayoutaxis,       {.i = -2 } }, // cycle through the available tiling arrangements for the master area
	{ MODKEY|Alt,                   XK_bracketright, rotatelayoutaxis,       {.i = +2 } }, // cycle through the available tiling arrangements for the master area
	{ MODKEY|Shift,                 XK_bracketleft,  rotatelayoutaxis,       {.i = -3 } }, // cycle through the available tiling arrangements for the primary (first) stack area
	{ MODKEY|Shift,                 XK_bracketright, rotatelayoutaxis,       {.i = +3 } }, // cycle through the available tiling arrangements for the primary (first) stack area
	{ MODKEY|Ctrl,                  XK_bracketleft,  rotatelayoutaxis,       {.i = -4 } }, // cycle through the available tiling arrangements for the secondary stack area
	{ MODKEY|Ctrl,                  XK_bracketright, rotatelayoutaxis,       {.i = +4 } }, // cycle through the available tiling arrangements for the secondary stack area
	{ MODKEY|Ctrl,                  XK_m,            mirrorlayout,           {0} }, // flip the master and stack areas
	{ MODKEY,                       XK_space,        setlayout,              {0} }, // toggles between current and previous layout

	{ MODKEY|Shift,                 XK_g,            savefloats,             {0} }, // pins a client's floating position and size
	{ MODKEY,                       XK_g,            togglefloating,         {0} }, // toggles between tiled and floating arrangement for the currently focused client
	{ MODKEY,                       XK_f,            togglefullscreen,       {0} }, // toggles fullscreen for the currently selected client
	{ MODKEY|Shift,                 XK_f,            togglefakefullscreen,   {0} }, // toggles "fake" fullscreen for the selected window
	{ Ctrl|Alt,                     XK_Tab,          togglenomodbuttons,     {0} }, // disables / enables keybindings that are not accompanied by any modifier buttons for a client
	{ MODKEY|ShiftMask,             XK_plus,         changeopacity,          {.f = +0.05 } }, // increase the client opacity (for compositors that support _NET_WM_OPACITY)
	{ MODKEY|ShiftMask,             XK_minus,        changeopacity,          {.f = -0.05 } }, // decrease the client opacity (for compositors that support _NET_WM_OPACITY)

	{ MODKEY|Shift,                 XK_comma,        focusmon,               {.i = -1 } }, // focus on the previous monitor, if any
	{ MODKEY|Shift,                 XK_period,       focusmon,               {.i = +1 } }, // focus on the next monitor, if any
	{ MODKEY|Alt,                   XK_comma,        clienttomon,            {.i = -1 } }, // sends the current client to an adjacent monitor
	{ MODKEY|Alt,                   XK_period,       clienttomon,            {.i = +1 } }, // sends the current client to an adjacent monitor
	{ MODKEY|Alt|Shift,             XK_comma,        clientstomon,           {.i = +1 } }, // sends all clients to an adjacent monitor
	{ MODKEY|Alt|Shift,             XK_period,       clientstomon,           {.i = -1 } }, // sends all clients to an adjacent monitor
	{ MODKEY|Ctrl,                  XK_0,            viewallwsonmon,         {0} },        // view all workspaces on the current monitor
	{ MODKEY,                       XK_0,            viewalloccwsonmon,      {0} },        // view all workspaces on the current monitor that has clients
	{ MODKEY,                       XK_o,            viewselws,              {0} },        // view the selected workspace (only relevant when viewing multiple workspaces)
	{ MODKEY|Ctrl,                  XK_comma,        viewwsdir,              {.i = -1 } }, // view the workspace on the immediate left of current workspace (on the current monitor)
	{ MODKEY|Ctrl,                  XK_period,       viewwsdir,              {.i = +1 } }, // view the workspace on the immediate right of current workspace (on the current monitor)
	{ MODKEY,                       XK_comma,        viewwsdir,              {.i = -2 } }, // view the next workspace left of current workspace that has clients (on the current monitor)
	{ MODKEY,                       XK_period,       viewwsdir,              {.i = +2 } }, // view the next workspace right of current workspace that has clients (on the current monitor)
	{ MODKEY|Ctrl|Alt,              XK_comma,        movewsdir,              {.i = -1 } }, // move client to workspace on the immediate left of current workspace (on the current monitor)
	{ MODKEY|Ctrl|Alt,              XK_period,       movewsdir,              {.i = +1 } }, // move client to workspace on the immediate right of current workspace (on the current monitor)


	STACKKEYS(AltGr|Ctrl,                            stackfocus)                           // focus on the nth client in the stack, see the STACKKEYS macro for keybindings
	STACKKEYS(AltGr|Ctrl|Shift,                      stackpush)                            // move the currently focused client to the nth place in the stack

	SCRATCHKEYS(                    XK_w,                                    spcmd1)
	SCRATCHKEYS(                    XK_e,                                    spcmd2)
	SCRATCHKEYS(                    XK_r,                                    spcmd3)

	WSKEYS(                         XK_1,                                    "1")
	WSKEYS(                         XK_2,                                    "2")
	WSKEYS(                         XK_3,                                    "3")
	WSKEYS(                         XK_4,                                    "4")
	WSKEYS(                         XK_5,                                    "5")
	WSKEYS(                         XK_6,                                    "6")
	WSKEYS(                         XK_7,                                    "7")
	WSKEYS(                         XK_8,                                    "8")
	WSKEYS(                         XK_9,                                    "9")

	/* Unassigned key bindings (available externally via the duskc command) */
//	{ MODKEY,                       XK_,             incrgaps,               {.i = +1 } }, // increase all gaps (outer, inner, horizontal and vertical)
//	{ MODKEY,                       XK_,             incrgaps,               {.i = -1 } }, // decrease all gaps (outer, inner, horizontal and vertical)
//	{ MODKEY,                       XK_,             incrigaps,              {.i = +1 } }, // increase inner gaps (horizontal and vertical)
//	{ MODKEY,                       XK_,             incrigaps,              {.i = -1 } }, // decrease inner gaps (horizontal and vertical)
//	{ MODKEY,                       XK_,             incrogaps,              {.i = +1 } }, // increase outer gaps (horizontal and vertical)
//	{ MODKEY,                       XK_,             incrogaps,              {.i = -1 } }, // decrease outer gaps (horizontal and vertical)
//	{ MODKEY,                       XK_,             incrihgaps,             {.i = +1 } }, // increase inner horizontal gaps
//	{ MODKEY,                       XK_,             incrihgaps,             {.i = -1 } }, // decrease inner horizontal gaps
//	{ MODKEY,                       XK_,             incrivgaps,             {.i = +1 } }, // increase inner vertical gaps
//	{ MODKEY,                       XK_,             incrivgaps,             {.i = -1 } }, // decrease inner vertical gaps
//	{ MODKEY,                       XK_,             incrohgaps,             {.i = +1 } }, // increase outer horizontal gaps
//	{ MODKEY,                       XK_,             incrohgaps,             {.i = -1 } }, // decrease outer horizontal gaps
//	{ MODKEY,                       XK_,             incrovgaps,             {.i = +1 } }, // increase outer vertical gaps
//	{ MODKEY,                       XK_,             incrovgaps,             {.i = -1 } }, // decrease outer vertical gaps
//	{ MODKEY,                       XK_,             mark,                   {0} }, // marks the currently selected client
//	{ MODKEY,                       XK_,             unmark,                 {0} }, // unmarks the currently selected client
//	{ MODKEY,                       XK_,             togglegaps,             {0} }, // enables and disables the rendering of gaps in tiled layouts
//	{ MODKEY,                       XK_,             defaultgaps,            {0} }, // revert gaps to the default settings
//	{ MODKEY,                       XK_,             cyclelayout,            {.i = -1 } }, // cycle through the available layouts
//	{ MODKEY,                       XK_,             cyclelayout,            {.i = +1 } }, // cycle through the available layouts (in reverse)
//	{ MODKEY,                       XK_,             viewwsdir,              {.i = -1 } }, // move to the workspace on the immediate left of the current workspace on the current monitor (wraps around)
//	{ MODKEY,                       XK_,             viewwsdir,              {.i = +1 } }, // move to the workspace on the immediate right of the current workspace on the current monitor (wraps around)
//	{ MODKEY,                       XK_,             focusmaster,            {0} }, // change focus to the first client in the stack (master)
//	{ MODKEY,                       XK_,             transfer,               {0} }, // move a client between the master and stack area automatically adjusting nmaster
//	{ MODKEY,                       XK_,             transferall,            {0} }, // swaps all clients in the stack area with all clients in the master area
//	{ MODKEY,                       XK_,             togglesticky,           {0} }, // makes a client show on all workspaces)
//	{ MODKEY,                       XK_,             focusurgent,            {0} }, // focus on the client marked as urgent
//	{ MODKEY,                       XK_,             inplacerotate,          {.i = +1} }, // rotate clients within the respective area (master, primary stack, secondary stack) clockwise
//	{ MODKEY,                       XK_,             inplacerotate,          {.i = -1} }, // rotate clients within the respective area (master, primary stack, secondary stack) counter-clockwise
//	{ MODKEY,                       XK_,             rotatestack,            {.i = +1 } }, // rotate all clients (clockwise)
//	{ MODKEY,                       XK_,             rotatestack,            {.i = -1 } }, // rotate all clients (counter-clockwise)
//	{ MODKEY,                       XK_,             unfloatvisible,         {0} }, // makes all floating clients on the currently selected workspace tiled
//	{ MODKEY,                       XK_,             switchcol,              {0} }, // changes focus between the master and the primary stack area
//	{ MODKEY,                       XK_,             setlayout,              {.v = &layouts[0]} }, // sets a specific layout, see the layouts array for indices
//	{ MODKEY,                       XK_,             xrdb,                   {0 } }, // reloads colors from XResources
};

/* button definitions */
/* click can be ClkButton, ClkWorkspaceBar, ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static Button buttons[] = {
	/* click                     event mask               button          function          argument */
	{ ClkButton,                 0,                       Button1,        spawn,            {.v = termcmd } }, // spawns a terminal
	{ ClkLtSymbol,               0,                       Button1,        setlayout,        {0} }, // toggles between current and previous layout
	{ ClkLtSymbol,               0,                       Button4,        cyclelayout,      {.i = +1 } }, // cycle through the available layouts
	{ ClkLtSymbol,               0,                       Button5,        cyclelayout,      {.i = -1 } }, // cycle through the available layouts (in reverse)
	{ ClkWinTitle,               0,                       Button1,        focuswin,         {0} }, // focus on the given client
	{ ClkWinTitle,               0,                       Button3,        showhideclient,   {0} }, // hide the currently selected client (or show if hidden)
	{ ClkWinTitle,               0,                       Button2,        zoom,             {0} }, // moves the currently focused window to/from the master area (for tiled layouts)
	{ ClkStatusText,             0,                       Button1,        sigdwmblocks,     {.i = 1 } }, // sends mouse button presses to dwmblocks when clicking on the status
	{ ClkStatusText,             0,                       Button2,        sigdwmblocks,     {.i = 2 } },
	{ ClkStatusText,             0,                       Button3,        sigdwmblocks,     {.i = 3 } },
	{ ClkStatusText,             0,                       Button4,        sigdwmblocks,     {.i = 4 } },
	{ ClkStatusText,             0,                       Button5,        sigdwmblocks,     {.i = 5 } },
	{ ClkStatusText,             0,                       Button6,        sigdwmblocks,     {.i = 6 } },
	{ ClkStatusText,             0,                       Button7,        sigdwmblocks,     {.i = 7 } },
	{ ClkStatusText,             0,                       Button8,        sigdwmblocks,     {.i = 8 } },
	{ ClkStatusText,             0,                       Button9,        sigdwmblocks,     {.i = 9 } },
	{ ClkStatusText,             Shift,                   Button1,        sigdwmblocks,     {.i = 10 } },
	{ ClkStatusText,             Shift,                   Button2,        sigdwmblocks,     {.i = 11 } },
	{ ClkStatusText,             Shift,                   Button3,        sigdwmblocks,     {.i = 12 } },
	{ ClkClientWin,              MODKEY,                  Button8,        markmouse,        {1} }, // marks clients under the mouse cursor for group action
	{ ClkClientWin,              MODKEY|Shift,            Button8,        markmouse,        {0} }, // unmarks clients under the mouse cursor for group action
	{ ClkClientWin,              MODKEY,                  Button9,        markmouse,        {2} }, // toggles marking of clients under the mouse cursor for group action
	{ ClkClientWin,              MODKEY,                  Button1,        movemouse,        {0} }, // moves a client window into a floating position
	{ ClkClientWin,              MODKEY|Ctrl,             Button1,        placemouse,       {1} }, // moves a client window between tiled positions (0 = relative to mouse cursor, 1 = relative to window center, 2 = mouse cursor warps to window center)
	{ ClkClientWin,              MODKEY|Alt,              Button2,        togglefloating,   {0} }, // toggles between tiled and floating arrangement for given client
	{ ClkClientWin,              MODKEY,                  Button3,        resizemouse,      {0} }, // change the size of a floating client window
	{ ClkClientWin,              0,                       Button8,        movemouse,        {0} }, // move a client window using extra mouse buttons (previous)
	{ ClkClientWin,              0,                       Button9,        resizemouse,      {0} }, // resize a client window using extra mouse buttons (next)
	{ ClkClientWin,              MODKEY,                  Button2,        zoom,             {0} }, // moves the currently focused window to/from the master area (for tiled layouts)
	{ ClkClientWin,              MODKEY|Shift,            Button3,        dragcfact,        {0} }, // dynamically change a client's size respective to other windows within the same area
	{ ClkClientWin,              MODKEY|Shift,            Button1,        dragmfact,        {0} }, // dynamically change the size of the master area compared to the stack area(s)
	{ ClkRootWin,                MODKEY|Shift,            Button1,        dragmfact,        {0} }, // dynamically change the size of the master area compared to the stack area(s)
	{ ClkClientWin,              MODKEY,                  Button4,        inplacerotate,    {.i = +1 } }, // rotate clients within the respective area (master, primary stack, secondary stack) clockwise
	{ ClkClientWin,              MODKEY,                  Button5,        inplacerotate,    {.i = -1 } }, // rotate clients within the respective area (master, primary stack, secondary stack) counter-clockwise
	{ ClkClientWin,              MODKEY|Shift,            Button4,        rotatestack,      {.i = +1 } }, // rotate all clients (clockwise)
	{ ClkClientWin,              MODKEY|Shift,            Button5,        rotatestack,      {.i = -1 } }, // rotate all clients (counter-clockwise)
	{ ClkWorkspaceBar,           0,                       Button1,        viewws,           {0} }, // view the workspace by clicking on workspace icon
	{ ClkWorkspaceBar,           MODKEY,                  Button1,        movews,           {0} }, // sends (moves) the currently focused client to given workspace
	{ ClkWorkspaceBar,           MODKEY|Shift|Ctrl,       Button1,        swapws,           {0} }, // swaps all clients on current workspace with that of the given workspace
	{ ClkWorkspaceBar,           MODKEY|Shift,            Button1,        enablews,         {0} }, // enables the workspace in addition to other workspaces
	{ ClkWorkspaceBar,           MODKEY,                  Button2,        togglepinnedws,   {0} }, // toggles the pinning of a workspace to the current monitor
};

static const char *ipcsockpath = "/tmp/dusk.sock";
static IPCCommand ipccommands[] = {
	IPCCOMMAND( changeopacity, 1, {ARG_TYPE_FLOAT} ),
	IPCCOMMAND( clienttomon, 1, {ARG_TYPE_UINT} ),
	IPCCOMMAND( clientstomon, 1, {ARG_TYPE_SINT} ),
	IPCCOMMAND( cyclelayout, 1, {ARG_TYPE_SINT} ),
	IPCCOMMAND( defaultgaps, 1, {ARG_TYPE_NONE} ),
	IPCCOMMAND( enable, 1, {ARG_TYPE_UINT} ),
	IPCCOMMAND( disable, 1, {ARG_TYPE_UINT} ),
	IPCCOMMAND( floatpos, 1, {ARG_TYPE_STR} ),
	IPCCOMMAND( focusdir, 1, {ARG_TYPE_SINT} ),
	IPCCOMMAND( focusmaster, 1, {ARG_TYPE_NONE} ),
	IPCCOMMAND( focusmon, 1, {ARG_TYPE_SINT} ),
	IPCCOMMAND( focusstack, 1, {ARG_TYPE_SINT} ),
	IPCCOMMAND( focusurgent, 1, {ARG_TYPE_NONE} ),
	IPCCOMMAND( grabkeys, 1, {ARG_TYPE_NONE} ),
	IPCCOMMAND( incrgaps, 1, {ARG_TYPE_SINT} ),
	IPCCOMMAND( incrigaps, 1, {ARG_TYPE_SINT} ),
	IPCCOMMAND( incrogaps, 1, {ARG_TYPE_SINT} ),
	IPCCOMMAND( incrihgaps, 1, {ARG_TYPE_SINT} ),
	IPCCOMMAND( incrivgaps, 1, {ARG_TYPE_SINT} ),
	IPCCOMMAND( incrohgaps, 1, {ARG_TYPE_SINT} ),
	IPCCOMMAND( incrovgaps, 1, {ARG_TYPE_SINT} ),
	IPCCOMMAND( incnmaster, 1, {ARG_TYPE_SINT} ),
	IPCCOMMAND( incnstack, 1, {ARG_TYPE_SINT} ),
	IPCCOMMAND( inplacerotate, 1, {ARG_TYPE_SINT} ),
	IPCCOMMAND( killclient, 1, {ARG_TYPE_SINT} ),
	IPCCOMMAND( killunsel, 1, {ARG_TYPE_NONE} ),
	IPCCOMMAND( mark, 1, {ARG_TYPE_PTR} ),
	IPCCOMMAND( markall, 1, {ARG_TYPE_SINT} ), // 0 = mark all, 1 = mark all floating
	IPCCOMMAND( mirrorlayout, 1, {ARG_TYPE_NONE} ),
	IPCCOMMAND( rotatelayoutaxis, 1, {ARG_TYPE_SINT} ),
	IPCCOMMAND( rotatestack, 1, {ARG_TYPE_SINT} ),
	IPCCOMMAND( pushdown, 1, {ARG_TYPE_NONE} ),
	IPCCOMMAND( pushup, 1, {ARG_TYPE_NONE} ),
	IPCCOMMAND( quit, 1, {ARG_TYPE_SINT} ), // 0 = quit, 1 = restart
	IPCCOMMAND( removescratch, 1, {ARG_TYPE_SINT} ),
	IPCCOMMAND( setborderpx, 1, {ARG_TYPE_SINT} ),
	IPCCOMMAND( setlayoutaxisex, 1, {ARG_TYPE_SINT} ),
	IPCCOMMAND( setlayoutex, 1, {ARG_TYPE_SINT} ),
	IPCCOMMAND( setlayoutsafe, 1, {ARG_TYPE_PTR} ),
	IPCCOMMAND( setcfact, 1, {ARG_TYPE_FLOAT} ),
	IPCCOMMAND( setmfact, 1, {ARG_TYPE_FLOAT} ),
	IPCCOMMAND( setgapsex, 1, {ARG_TYPE_SINT} ),
	IPCCOMMAND( setscratch, 1, {ARG_TYPE_SINT} ),
	IPCCOMMAND( setstatus, 1, {ARG_TYPE_STR} ),
	IPCCOMMAND( stackpush, 1, {ARG_TYPE_SINT} ),
	IPCCOMMAND( stackfocus, 1, {ARG_TYPE_SINT} ),
	IPCCOMMAND( switchcol, 1, {ARG_TYPE_NONE} ),
	IPCCOMMAND( toggle, 1, {ARG_TYPE_UINT} ), // toggle functionality on and off
	IPCCOMMAND( togglebar, 1, {ARG_TYPE_NONE} ),
	IPCCOMMAND( togglefakefullscreen, 1, {ARG_TYPE_NONE} ),
	IPCCOMMAND( togglefloating, 1, {ARG_TYPE_NONE} ),
	IPCCOMMAND( togglefullscreen, 1, {ARG_TYPE_NONE} ),
	IPCCOMMAND( togglegaps, 1, {ARG_TYPE_NONE} ),
	IPCCOMMAND( togglemark, 1, {ARG_TYPE_PTR} ),
	IPCCOMMAND( togglenomodbuttons, 1, {ARG_TYPE_NONE} ),
	IPCCOMMAND( togglescratch, 1, {ARG_TYPE_UINT} ),
	IPCCOMMAND( togglesticky, 1, {ARG_TYPE_NONE} ),
	IPCCOMMAND( transfer, 1, {ARG_TYPE_NONE} ),
	IPCCOMMAND( transferall, 1, {ARG_TYPE_NONE} ),
	IPCCOMMAND( unfloatvisible, 1, {ARG_TYPE_NONE} ),
	IPCCOMMAND( unmark, 1, {ARG_TYPE_PTR} ),
	IPCCOMMAND( unmarkall, 1, {ARG_TYPE_NONE} ),
	IPCCOMMAND( viewwsdir, 1, {ARG_TYPE_SINT} ),
	IPCCOMMAND( xrdb, 1, {ARG_TYPE_NONE} ), // reload xrdb / Xresources
	IPCCOMMAND( zoom, 1, {ARG_TYPE_NONE} ),
};
