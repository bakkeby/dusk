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

static const int initshowbar             = 1;   /* 0 means no bar */

static const int bar_height              = 0;   /* 0 means derive from font, >= 1 explicit height */
static const int vertpad                 = borderpx;  /* vertical (outer) padding of bar */
static const int sidepad                 = borderpx;  /* horizontal (outer) padding of bar */

static int floatposgrid_x                = 5;   /* float grid columns */
static int floatposgrid_y                = 5;   /* float grid rows */

static const int horizpadbar             = 2;   /* horizontal (inner) padding for statusbar (increases lrpad) */
static const int vertpadbar              = 0;   /* vertical (inner) padding for statusbar (increases bh, overridden by bar_height) */

static const char slopspawnstyle[]       = "-t 0 -c 0.92,0.85,0.69,0.3 -o"; /* do NOT define -f (format) here */
static const char slopresizestyle[]      = "-t 0 -c 0.92,0.85,0.69,0.3"; /* do NOT define -f (format) here */
static const unsigned int systrayspacing = 2;   /* systray spacing */
static const char *toggle_float_pos      = "50% 50% 80% 80%"; // default floating position when triggering togglefloatpos
static const double defaultopacity       = 0;   /* client default opacity, e.g. 0.75. 0 means don't apply opacity */
static const double moveopacity          = 0;   /* client opacity when being moved, 0 means don't apply opacity */
static const double resizeopacity        = 0;   /* client opacity when being resized, 0 means don't apply opacity */
static const double placeopacity         = 0;   /* client opacity when being placed, 0 means don't apply opacity */

/* Indicators: see lib/bar_indicators.h for options */
static int wsindicatortype               = INDICATOR_BOTTOM_BAR_SLIM;
static int wspinnedindicatortype         = INDICATOR_TOP_RIGHT_PIN;
static int fakefsindicatortype           = INDICATOR_PLUS;
static int floatfakefsindicatortype      = INDICATOR_PLUS_AND_LARGER_SQUARE;
static int floatindicatortype            = INDICATOR_TOP_LEFT_LARGER_SQUARE;
static int tiledindicatortype            = INDICATOR_NONE;

/* Custom indicators using status2d markup, e.g. enabled via INDICATOR_CUSTOM_3 */
static char *custom_2d_indicator_1 = "^c#00A523^^r0,h,w,2^"; // green underline
static char *custom_2d_indicator_2 = "^c#55cdfc^^r3,3,4,4^^c#E72608^^r4,4,2,2^"; // blue rectangle
static char *custom_2d_indicator_3 = "^f-10^^c#E72608^𐄛"; // example using a character as an indicator
static char *custom_2d_indicator_4 = "^c#E26F0B^^r0,h,w,1^^r0,0,1,h^^r0,0,w,1^^rw,0,1,h^"; // orange box
static char *custom_2d_indicator_5 = "^c#CB9700^^r0,h,w,1^^r0,0,w,1^"; // top and bottom lines
static char *custom_2d_indicator_6 = "^c#F0A523^^r6,2,1,-4^^r-6,2,1,-4^"; // orange vertical bars

/* See util.h for options */
static unsigned long functionality = 0
//	|AutoReduceNmaster // automatically reduce the number of master clients if one is closed
//	|SmartGaps // enables no or increased gaps if there is only one visible window
//	|SmartGapsMonocle // enforces no gaps in monocle layout
	|Systray // enables a systray in the bar
	|Swallow // allows terminals to swallow X applications started from the command line
	|SwallowFloating // means swallow floating windows by default
	|CenteredWindowName // center the window titles on the bar
//	|BarActiveGroupBorderColor // use border color of active group, otherwise color for master group is used
	|SpawnCwd // spawn applications in the currently selected client's working directory
	|ColorEmoji // enables color emoji support (removes Xft workaround)
//	|Status2DNoAlpha // option to not use alpha when drawing status2d status
	|BarBorder // draw a border around the bar
	|BarPadding // add vertical and side padding as per vertpad and sidepad variables above
//	|NoBorders // as per the noborder patch, show no border when only one client in tiled mode
//	|Warp // warp cursor to currently focused window
//	|FocusedOnTop // allows focused window to stay on top of other windows
//	|DecorationHints // omit drawing the window border if the applications asks not to
	|FocusOnNetActive //  allow windows demanding attention to receive focus automatically
	|AllowNoModifierButtons // allow some window operations, like move and resize, to work without having to hold down a modifier key
	|CenterSizeHintsClients // center tiled clients subject to size hints within their tiled area
//	|ResizeHints // respect size hints also when windows are tiled
//	|SortScreens // monitors are numbered from left to right
//	|ViewOnWs // follow a window to the workspace it is being moved to
//	|Xresources // add support for changing colours via Xresources
//	|Debug // enables additional debug output
//	|AltWorkspaceIcons // show the workspace name instead of the icons
//	|GreedyMonitor // disables swap of workspaces between monitors
	|SmartLayoutConvertion // automatically adjust layout based on monitor orientation when moving a workspace from one monitor to another
//	|AutoHideScratchpads // automatically hide open scratchpads when moving to another workspace
//	|RioDrawIncludeBorders // indicates whether the area drawn using slop includes the window borders
//	|RioDrawSpawnAsync // spawn the application alongside rather than after drawing area using slop
//	|RestrictFocusstackToMonitor // restrict focusstack to only operate within the monitor, otherwise focus can drift between monitors
;

static const char statussep              = ';'; /* separator between status bars */

static int flexwintitle_masterweight     = 15; // master weight compared to hidden and floating window titles
static int flexwintitle_stackweight      = 4;  // stack weight compared to hidden and floating window titles
static int flexwintitle_hiddenweight     = 0;  // hidden window title weight
static int flexwintitle_floatweight      = 0;  // floating window title weight, set to 0 to not show floating windows
static int flexwintitle_separator        = borderpx; // width of client separator

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
static char wsvisbgcolor[]               = "#5E294B";
static char wsselfgcolor[]               = "#FFF7D4";
static char wsselbgcolor[]               = "#6F3A5C";
static char wsoccfgcolor[]               = "#C6BDBD";
static char wsoccbgcolor[]               = "#180A13";

static char hidnormfgcolor[]             = "#c278b6";
static char hidnormbgcolor[]             = "#222222";
static char hidnormbordercolor[]         = "#440000";

static char hidselfgcolor[]              = "#D288C6";
static char hidselbgcolor[]              = "#111111";
static char hidselbordercolor[]          = "#440000";

static char urgfgcolor[]                 = "#bbbbbb";
static char urgbgcolor[]                 = "#222222";
static char urgbordercolor[]             = "#d10f3f";

static char markedfgcolor[]              = "#615656";
static char markedbgcolor[]              = "#ECB820";
static char markedbordercolor[]          = "#ECB820";

static char scratchnormcolor[]           = "#FFF7D4";
static char scratchnormbgcolor[]         = "#664C67";
static char scratchnormbordercolor[]     = "#77547E";

static char scratchselcolor[]            = "#FFF7D4";
static char scratchselbgcolor[]          = "#77547E";
static char scratchselbordercolor[]      = "#894B9F";

static char normTTBbgcolor[]             = "#330000";
static char normLTRbgcolor[]             = "#330033";
static char normMONObgcolor[]            = "#000033";
static char normGRIDbgcolor[]            = "#003300";
static char normGRIDCbgcolor[]           = "#003300";
static char normGRD1bgcolor[]            = "#003300";
static char normGRD2bgcolor[]            = "#003300";
static char normGRDMbgcolor[]            = "#506600";
static char normHGRDbgcolor[]            = "#b96600";
static char normDWDLbgcolor[]            = "#003333";
static char normDWDLCbgcolor[]           = "#003333";
static char normSPRLbgcolor[]            = "#333300";
static char normSPRLCbgcolor[]           = "#333300";
static char normTTMIbgcolor[]            = "#B32727";
static char normTTMICbgcolor[]           = "#B32727";
static char normfloatbgcolor[]           = "#4C314C";
static char actTTBbgcolor[]              = "#440000";
static char actLTRbgcolor[]              = "#440044";
static char actMONObgcolor[]             = "#000044";
static char actGRIDbgcolor[]             = "#004400";
static char actGRIDCbgcolor[]            = "#004400";
static char actGRD1bgcolor[]             = "#004400";
static char actGRD2bgcolor[]             = "#004400";
static char actGRDMbgcolor[]             = "#507711";
static char actHGRDbgcolor[]             = "#b97711";
static char actDWDLbgcolor[]             = "#004444";
static char actDWDLCbgcolor[]            = "#004444";
static char actSPRLbgcolor[]             = "#444400";
static char actSPRLCbgcolor[]            = "#444400";
static char actTTMIbgcolor[]             = "#C81C1C";
static char actTTMICbgcolor[]            = "#C81C1C";
static char actfloatbgcolor[]            = "#4C314C";
static char selTTBbgcolor[]              = "#550000";
static char selLTRbgcolor[]              = "#550055";
static char selMONObgcolor[]             = "#212171";
static char selGRIDbgcolor[]             = "#005500";
static char selGRIDCbgcolor[]            = "#005500";
static char selGRD1bgcolor[]             = "#005500";
static char selGRD2bgcolor[]             = "#005500";
static char selGRDMbgcolor[]             = "#508822";
static char selHGRDbgcolor[]             = "#b98822";
static char selDWDLbgcolor[]             = "#005555";
static char selDWDLCbgcolor[]            = "#005555";
static char selSPRLbgcolor[]             = "#555500";
static char selSPRLCbgcolor[]            = "#555500";
static char selTTMIbgcolor[]             = "#C91717";
static char selTTMICbgcolor[]            = "#C91717";
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
	[SchemeWsOcc]        = { OPAQUE, baralpha, borderalpha },
	[SchemeHidNorm]      = { OPAQUE, baralpha, borderalpha },
	[SchemeHidSel]       = { OPAQUE, baralpha, borderalpha },
	[SchemeUrg]          = { OPAQUE, baralpha, borderalpha },
	[SchemeMarked]       = { OPAQUE, baralpha, borderalpha },
	[SchemeScratchSel]   = { OPAQUE, baralpha, borderalpha },
	[SchemeScratchNorm]  = { OPAQUE, baralpha, borderalpha },
	[SchemeFlexActTTB]   = { OPAQUE, baralpha, borderalpha },
	[SchemeFlexActLTR]   = { OPAQUE, baralpha, borderalpha },
	[SchemeFlexActMONO]  = { OPAQUE, baralpha, borderalpha },
	[SchemeFlexActGRID]  = { OPAQUE, baralpha, borderalpha },
	[SchemeFlexActGRIDC] = { OPAQUE, baralpha, borderalpha },
	[SchemeFlexActGRD1]  = { OPAQUE, baralpha, borderalpha },
	[SchemeFlexActGRD2]  = { OPAQUE, baralpha, borderalpha },
	[SchemeFlexActGRDM]  = { OPAQUE, baralpha, borderalpha },
	[SchemeFlexActHGRD]  = { OPAQUE, baralpha, borderalpha },
	[SchemeFlexActDWDL]  = { OPAQUE, baralpha, borderalpha },
	[SchemeFlexActDWDLC] = { OPAQUE, baralpha, borderalpha },
	[SchemeFlexActSPRL]  = { OPAQUE, baralpha, borderalpha },
	[SchemeFlexActSPRLC] = { OPAQUE, baralpha, borderalpha },
	[SchemeFlexActTTMI]  = { OPAQUE, baralpha, borderalpha },
	[SchemeFlexActTTMIC] = { OPAQUE, baralpha, borderalpha },
	[SchemeFlexActFloat] = { OPAQUE, baralpha, borderalpha },
	[SchemeFlexInaTTB]   = { OPAQUE, baralpha, borderalpha },
	[SchemeFlexInaLTR]   = { OPAQUE, baralpha, borderalpha },
	[SchemeFlexInaMONO]  = { OPAQUE, baralpha, borderalpha },
	[SchemeFlexInaGRID]  = { OPAQUE, baralpha, borderalpha },
	[SchemeFlexInaGRIDC] = { OPAQUE, baralpha, borderalpha },
	[SchemeFlexInaGRD1]  = { OPAQUE, baralpha, borderalpha },
	[SchemeFlexInaGRD2]  = { OPAQUE, baralpha, borderalpha },
	[SchemeFlexInaGRDM]  = { OPAQUE, baralpha, borderalpha },
	[SchemeFlexInaHGRD]  = { OPAQUE, baralpha, borderalpha },
	[SchemeFlexInaDWDL]  = { OPAQUE, baralpha, borderalpha },
	[SchemeFlexInaDWDLC] = { OPAQUE, baralpha, borderalpha },
	[SchemeFlexInaSPRL]  = { OPAQUE, baralpha, borderalpha },
	[SchemeFlexInaSPRLC] = { OPAQUE, baralpha, borderalpha },
	[SchemeFlexInaTTMI]  = { OPAQUE, baralpha, borderalpha },
	[SchemeFlexInaTTMIC] = { OPAQUE, baralpha, borderalpha },
	[SchemeFlexInaFloat] = { OPAQUE, baralpha, borderalpha },
	[SchemeFlexSelTTB]   = { OPAQUE, baralpha, borderalpha },
	[SchemeFlexSelLTR]   = { OPAQUE, baralpha, borderalpha },
	[SchemeFlexSelMONO]  = { OPAQUE, baralpha, borderalpha },
	[SchemeFlexSelGRID]  = { OPAQUE, baralpha, borderalpha },
	[SchemeFlexSelGRIDC] = { OPAQUE, baralpha, borderalpha },
	[SchemeFlexSelGRD1]  = { OPAQUE, baralpha, borderalpha },
	[SchemeFlexSelGRD2]  = { OPAQUE, baralpha, borderalpha },
	[SchemeFlexSelGRDM]  = { OPAQUE, baralpha, borderalpha },
	[SchemeFlexSelHGRD]  = { OPAQUE, baralpha, borderalpha },
	[SchemeFlexSelDWDL]  = { OPAQUE, baralpha, borderalpha },
	[SchemeFlexSelDWDLC] = { OPAQUE, baralpha, borderalpha },
	[SchemeFlexSelSPRL]  = { OPAQUE, baralpha, borderalpha },
	[SchemeFlexSelSPRLC] = { OPAQUE, baralpha, borderalpha },
	[SchemeFlexSelTTMI]  = { OPAQUE, baralpha, borderalpha },
	[SchemeFlexSelTTMIC] = { OPAQUE, baralpha, borderalpha },
	[SchemeFlexSelFloat] = { OPAQUE, baralpha, borderalpha },
};

static char *colors[][ColCount] = {
	/*                       fg                bg                  border                  */
	[SchemeNorm]         = { normfgcolor,      normbgcolor,        normbordercolor,        },
	[SchemeSel]          = { selfgcolor,       selbgcolor,         selbordercolor,         },
	[SchemeTitleNorm]    = { titlenormfgcolor, titlenormbgcolor,   selbordercolor,         },
	[SchemeTitleSel]     = { titleselfgcolor,  titleselbgcolor,    selbordercolor,         },
	[SchemeWsNorm]       = { wsnormfgcolor,    wsnormbgcolor,                              },
	[SchemeWsVisible]    = { wsvisfgcolor,     wsvisbgcolor,                               },
	[SchemeWsSel]        = { wsselfgcolor,     wsselbgcolor,                               },
	[SchemeWsOcc]        = { wsoccfgcolor,     wsoccbgcolor,                               },
	[SchemeHidNorm]      = { hidnormfgcolor,   hidnormbgcolor,     hidnormbordercolor,     },
	[SchemeHidSel]       = { hidselfgcolor,    hidselbgcolor,      hidselbordercolor,      },
	[SchemeUrg]          = { urgfgcolor,       urgbgcolor,         urgbordercolor,         },
	[SchemeMarked]       = { markedfgcolor,    markedbgcolor,      markedbordercolor,      },
	[SchemeScratchNorm]  = { scratchnormcolor, scratchnormbgcolor, scratchnormbordercolor, },
	[SchemeScratchSel]   = { scratchselcolor,  scratchselbgcolor,  scratchselbordercolor,  },
	[SchemeFlexActTTB]   = { titleselfgcolor,  actTTBbgcolor,      actTTBbgcolor,          },
	[SchemeFlexActLTR]   = { titleselfgcolor,  actLTRbgcolor,      actLTRbgcolor,          },
	[SchemeFlexActMONO]  = { titleselfgcolor,  actMONObgcolor,     actMONObgcolor,         },
	[SchemeFlexActGRID]  = { titleselfgcolor,  actGRIDbgcolor,     actGRIDbgcolor,         },
	[SchemeFlexActGRIDC] = { titleselfgcolor,  actGRIDCbgcolor,    actGRIDCbgcolor,        },
	[SchemeFlexActGRD1]  = { titleselfgcolor,  actGRD1bgcolor,     actGRD1bgcolor,         },
	[SchemeFlexActGRD2]  = { titleselfgcolor,  actGRD2bgcolor,     actGRD2bgcolor,         },
	[SchemeFlexActGRDM]  = { titleselfgcolor,  actGRDMbgcolor,     actGRDMbgcolor,         },
	[SchemeFlexActHGRD]  = { titleselfgcolor,  actHGRDbgcolor,     actHGRDbgcolor,         },
	[SchemeFlexActDWDL]  = { titleselfgcolor,  actDWDLbgcolor,     actDWDLbgcolor,         },
	[SchemeFlexActDWDLC] = { titleselfgcolor,  actDWDLCbgcolor,    actDWDLCbgcolor,        },
	[SchemeFlexActSPRL]  = { titleselfgcolor,  actSPRLbgcolor,     actSPRLbgcolor,         },
	[SchemeFlexActSPRLC] = { titleselfgcolor,  actSPRLCbgcolor,    actSPRLCbgcolor,        },
	[SchemeFlexActTTMI]  = { titleselfgcolor,  actTTMIbgcolor,     actTTMIbgcolor,         },
	[SchemeFlexActTTMIC] = { titleselfgcolor,  actTTMICbgcolor,    actTTMICbgcolor,        },
	[SchemeFlexActFloat] = { titleselfgcolor,  actfloatbgcolor,    actfloatbgcolor,        },
	[SchemeFlexInaTTB]   = { titlenormfgcolor, normTTBbgcolor,     normTTBbgcolor,         },
	[SchemeFlexInaLTR]   = { titlenormfgcolor, normLTRbgcolor,     normLTRbgcolor,         },
	[SchemeFlexInaMONO]  = { titlenormfgcolor, normMONObgcolor,    normMONObgcolor,        },
	[SchemeFlexInaGRID]  = { titlenormfgcolor, normGRIDbgcolor,    normGRIDbgcolor,        },
	[SchemeFlexInaGRIDC] = { titlenormfgcolor, normGRIDCbgcolor,   normGRIDCbgcolor,       },
	[SchemeFlexInaGRD1]  = { titlenormfgcolor, normGRD1bgcolor,    normGRD1bgcolor,        },
	[SchemeFlexInaGRD2]  = { titlenormfgcolor, normGRD2bgcolor,    normGRD2bgcolor,        },
	[SchemeFlexInaGRDM]  = { titlenormfgcolor, normGRDMbgcolor,    normGRDMbgcolor,        },
	[SchemeFlexInaHGRD]  = { titlenormfgcolor, normHGRDbgcolor,    normHGRDbgcolor,        },
	[SchemeFlexInaDWDL]  = { titlenormfgcolor, normDWDLbgcolor,    normDWDLbgcolor,        },
	[SchemeFlexInaDWDLC] = { titlenormfgcolor, normDWDLCbgcolor,   normDWDLCbgcolor,       },
	[SchemeFlexInaSPRL]  = { titlenormfgcolor, normSPRLbgcolor,    normSPRLbgcolor,        },
	[SchemeFlexInaSPRLC] = { titlenormfgcolor, normSPRLCbgcolor,   normSPRLCbgcolor,       },
	[SchemeFlexInaTTMI]  = { titlenormfgcolor, normTTMIbgcolor,    normTTMIbgcolor,        },
	[SchemeFlexInaTTMIC] = { titlenormfgcolor, normTTMICbgcolor,   normTTMICbgcolor,       },
	[SchemeFlexInaFloat] = { titlenormfgcolor, normfloatbgcolor,   normfloatbgcolor,       },
	[SchemeFlexSelTTB]   = { titleselfgcolor,  selTTBbgcolor,      selTTBbgcolor,          },
	[SchemeFlexSelLTR]   = { titleselfgcolor,  selLTRbgcolor,      selLTRbgcolor,          },
	[SchemeFlexSelMONO]  = { titleselfgcolor,  selMONObgcolor,     selMONObgcolor,         },
	[SchemeFlexSelGRID]  = { titleselfgcolor,  selGRIDCbgcolor,    selGRIDCbgcolor,        },
	[SchemeFlexSelGRIDC] = { titleselfgcolor,  selGRIDbgcolor,     selGRIDbgcolor,         },
	[SchemeFlexSelGRD1]  = { titleselfgcolor,  selGRD1bgcolor,     selGRD1bgcolor,         },
	[SchemeFlexSelGRD2]  = { titleselfgcolor,  selGRD2bgcolor,     selGRD2bgcolor,         },
	[SchemeFlexSelGRDM]  = { titleselfgcolor,  selGRDMbgcolor,     selGRDMbgcolor,         },
	[SchemeFlexSelHGRD]  = { titleselfgcolor,  selHGRDbgcolor,     selHGRDbgcolor,         },
	[SchemeFlexSelDWDL]  = { titleselfgcolor,  selDWDLbgcolor,     selDWDLbgcolor,         },
	[SchemeFlexSelDWDLC] = { titleselfgcolor,  selDWDLCbgcolor,    selDWDLCbgcolor,        },
	[SchemeFlexSelSPRL]  = { titleselfgcolor,  selSPRLbgcolor,     selSPRLbgcolor,         },
	[SchemeFlexSelSPRLC] = { titleselfgcolor,  selSPRLCbgcolor,    selSPRLCbgcolor,        },
	[SchemeFlexSelTTMI]  = { titleselfgcolor,  selTTMIbgcolor,     selTTMIbgcolor,         },
	[SchemeFlexSelTTMIC] = { titleselfgcolor,  selTTMICbgcolor,    selTTMICbgcolor,        },
	[SchemeFlexSelFloat] = { titleselfgcolor,  selfloatbgcolor,    selfloatbgcolor,        },
};

static const char *const autostart[] = {
	"st", NULL,
	NULL /* terminate */
};

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
	RULE(.wintype = WTYPE "DESKTOP", .flags = Unmanaged|Lower)
	RULE(.wintype = WTYPE "DOCK", .flags = Unmanaged|Raise)
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
	RULE(.class = "steam_app_", .flags = SteamGame|IgnoreCfgReqPos|Floating|Centered)
	RULE(.class = "Google-chrome", .role = "GtkFileChooserDialog", .floatpos = "50% 50%", .flags = AlwaysOnTop|Floating)
	RULE(.role = "pop-up", .flags = AlwaysOnTop|Floating|Centered)
	RULE(.role = "browser", .workspace = "8", .flags = AttachBelow|OnlyModButtons|SwitchWorkspace)
	RULE(.class = "Gnome-terminal", .role = "gnome-terminal-preferences", .flags = Centered)
	RULE(.class = "Diffuse", .workspace = "4", .flags = NoSwallow|SwitchWorkspace|RevertWorkspace)
	RULE(.class = "File-roller", .workspace = "9", .flags = Centered|Floating|SwitchWorkspace|RevertWorkspace)
	RULE(.class = "Alacritty", .flags = Terminal)
	RULE(.class = "st-256color", .flags = Terminal|AttachBottom)
	RULE(.class = "XTerm", .flags = Terminal)
	RULE(.class = "Xephyr", .flags = NoSwallow|Floating|Centered)
	RULE(.title = "Event Tester", .flags = NoSwallow)
};

/* Bar settings, this defines what bars exists, their position, and what attributes they have.
 *
 *    monitor - the exact monitor number the bar should be created on
 *              (0 - primary, 1 - secondary)
 *    idx     - the bar index, used in relation to bar rules below
 *              (bar indexes can be reused across monitors)
 *    vert    - whether the bar is horizontal (0) or vertical (1), not
 *              all bar modules will have support for being displayed
 *              in a vertical bar
 *    name    - this is just a reference that can be used for logging
 *              purposes
 *
 *    Bar positioning consists of four values, x, y, w and h which,
 *    similarly to floatpos, can have different meaning depending on
 *    the characters used. Absolute positioning (as in cross-monitor)
 *    is not supported, but exact positions relative to the monitor
 *    can be used. Percentage values are recommended for portability.
 *
 *    All values can be a percentage relative to the space available
 *    on the monitor or they can be exact values, here are some example
 *    values:
 *       x
 *                  0% - left aligned (default)
 *                100% - right aligned
 *                 50% - bar is centered on the screen
 *                  0x - exact position relative to the monitor
 *                 -1x - value < 0 means use default
 *       y
 *                  0% - top bar (default)
 *                100% - bottom bar
 *                  0y - exact position relative to the monitor
 *                 -1y - value < 0 means use default
 *       w
 *                100% - bar takes up the full width of the screen (default)
 *                 20% - small bar taking a fifth of the width of the screen
 *                500w - the bar is 500 pixels wide (including border)
 *                 -1w - value <= 0 means use default
 *       h
 *                100% - bar takes up the full height of the screen
 *                 20% - small bar taking a fifth of the height of screen
 *                 30h - the bar is 30 pixels high (including border)
 *                 -1h - value <= 0 means use the default (deduced by font size)
 *
 *    Note that vertical and horizontal side padding are controlled by the
 *    vertpad and sidepad variables towards the top of this configuration file.
 */
static const BarDef bars[] = {
	/* monitor idx  vert   x     y      w     h     name  */
	{  0,      0,   0,    "0%    0%     100% -1h ", "Primary top" },
	{  0,      1,   0,    "0%    100%   100% -1h ", "Primary bottom" },
	{  1,      0,   0,    "0%    0%     100% -1h ", "Side top" },
	{  1,      1,   0,    "0%    100%   100% -1h ", "Side bottom" },
};

/* Bar rules allow you to configure what is shown where on the bar, as well as
 * introducing your own bar modules.
 *
 *    monitor:
 *      -1  show on all monitors
 *       0  show on monitor 0
 *      'A' show on active monitor (i.e. focused / selected) (or just -1 for active?)
 *    bar - bar index, 0 is default, 1 is extrabar
 *    scheme - defines the default scheme for the bar module
 *    alignment - how the module is aligned compared to other modules
 *    widthfunc, drawfunc, clickfunc - providing bar module width, draw and click functions
 *    name - does nothing, intended for visual clue and for logging / debugging
 */
static const BarRule barrules[] = {
	/* monitor  bar    scheme   lpad rpad value  alignment               widthfunc                 drawfunc                 clickfunc                 name */
	{ -1,       0,     0,       0,   5,   0,     BAR_ALIGN_LEFT,         size_workspaces,          draw_workspaces,         click_workspaces,         "workspaces" },
	{ 'A',      0,     0,       5,   5,   0,     BAR_ALIGN_RIGHT,        size_systray,             draw_systray,            click_systray,            "systray" },
	{ -1,       0,     0,       0,   0,   0,     BAR_ALIGN_LEFT,         size_ltsymbol,            draw_ltsymbol,           click_ltsymbol,           "layout" },
	{  0,       0,     0,       10,  0,   0,     BAR_ALIGN_RIGHT,        size_status,              draw_status,             click_status,             "status0" },
	{  0,       0,     0,       10,  0,   1,     BAR_ALIGN_RIGHT,        size_status,              draw_status,             click_status,             "status1" },
	{  0,       0,     0,       10,  0,   2,     BAR_ALIGN_RIGHT,        size_status,              draw_status,             click_status,             "status2" },
	{  0,       0,     0,       10,  0,   3,     BAR_ALIGN_RIGHT,        size_status,              draw_status,             click_status,             "status3" },
	{  0,       0,     0,       10,  0,   4,     BAR_ALIGN_RIGHT,        size_status,              draw_status,             click_status,             "status4" },
	{  0,       0,     0,       10,  0,   5,     BAR_ALIGN_RIGHT,        size_status,              draw_status,             click_status,             "status5" },
	{  0,       0,     0,       10,  0,   6,     BAR_ALIGN_RIGHT,        size_status,              draw_status,             click_status,             "status6" },
	{  0,       0,     0,       10,  0,   7,     BAR_ALIGN_RIGHT,        size_status,              draw_status,             click_status,             "status7" },
	{  0,       0,     0,       10,  0,   8,     BAR_ALIGN_RIGHT,        size_status,              draw_status,             click_status,             "status8" },
	{ -1,       0,     0,       5,   0,   0,     BAR_ALIGN_NONE,         size_flexwintitle,        draw_flexwintitle,       click_flexwintitle,       "flexwintitle" },
	{ 'A',      1,     0,       10,  10,  9,     BAR_ALIGN_RIGHT,        size_status,              draw_status,             click_status,             "status9" },
	{ -1,       1,     0,       0,   0,   0,     BAR_ALIGN_RIGHT_RIGHT,  size_wintitle_hidden,     draw_wintitle_hidden,    click_wintitle_hidden,    "wintitle_hidden" },
	{ -1,       1,     0,       0,   0,   0,     BAR_ALIGN_LEFT,         size_wintitle_floating,   draw_wintitle_floating,  click_wintitle_floating,  "wintitle_floating" },
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
	   name,  monitor,  pinned,  layout,  mfact,  nmaster,  nstack,  gaps, def,   vac,  occ,  */
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
	{ "==#",      flextile,         { -1, -1, SPLIT_HORIZONTAL, TOP_TO_BOTTOM, GAPPLESSGRID_CFACTS, 0, NULL } }, // bstackgrid
	{ "|M|",      flextile,         { -1, -1, SPLIT_CENTERED_VERTICAL, LEFT_TO_RIGHT, TOP_TO_BOTTOM, TOP_TO_BOTTOM, NULL } }, // centeredmaster
	{ "-M-",      flextile,         { -1, -1, SPLIT_CENTERED_HORIZONTAL, TOP_TO_BOTTOM, LEFT_TO_RIGHT, LEFT_TO_RIGHT, NULL } }, // centeredmaster horiz
	{ ":::",      flextile,         { -1, -1, NO_SPLIT, GAPPLESSGRID_CFACTS, GAPPLESSGRID_CFACTS, 0, NULL } }, // gappless grid
	{ "[\\]",     flextile,         { -1, -1, NO_SPLIT, DWINDLE_CFACTS, DWINDLE_CFACTS, 0, NULL } }, // fibonacci dwindle
	{ "(@)",      flextile,         { -1, -1, NO_SPLIT, SPIRAL_CFACTS, SPIRAL_CFACTS, 0, NULL } }, // fibonacci spiral
	{ "[T]",      flextile,         { -1, -1, SPLIT_VERTICAL, LEFT_TO_RIGHT, TATAMI_CFACTS, 0, NULL } }, // tatami mats
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
	{ KeyPress,   MODKEY,                      KEY,      togglescratch,     {.v = CMD } }, \
	{ KeyPress,   MODKEY|Ctrl,                 KEY,      setscratch,        {.v = CMD } }, \
	{ KeyPress,   MODKEY|Ctrl|Shift,           KEY,      removescratch,     {.v = CMD } }, \

#define WSKEYS(KEY,NAME) \
	{ KeyPress,   MODKEY,                      KEY,      viewwsbyname,      {.v = NAME} }, \
	{ KeyPress,   MODKEY|Shift,                KEY,      movetowsbyname,    {.v = NAME} }, \
	{ KeyPress,   MODKEY|Ctrl|Shift,           KEY,      movealltowsbyname, {.v = NAME} }, \
	{ KeyPress,   MODKEY|Ctrl,                 KEY,      swapwsbyname,      {.v = NAME} }, \
	{ KeyPress,   MODKEY|Alt,                  KEY,      enablewsbyname,    {.v = NAME} }, \

#define STACKKEYS(MOD,ACTION) \
	{ KeyPress,   MOD, XK_j, ACTION, {.i = INC(+1) } }, \
	{ KeyPress,   MOD, XK_k, ACTION, {.i = INC(-1) } }, \
	{ KeyPress,   MOD, XK_s, ACTION, {.i = PREVSEL } }, \
	{ KeyPress,   MOD, XK_w, ACTION, {.i = 0 } }, \
	{ KeyPress,   MOD, XK_e, ACTION, {.i = 1 } }, \
	{ KeyPress,   MOD, XK_a, ACTION, {.i = 2 } }, \
	{ KeyPress,   MOD, XK_z, ACTION, {.i = -1 } },

/* Scratch/Spawn commands:        NULL (scratchkey), command, argument, argument, ..., NULL */
static const char *termcmd[]  = { NULL, "st", NULL };
static const char *dmenucmd[] = {
	NULL,
	"dmenu_run",
	"-fn", dmenufont,
	"-nb", normbgcolor,
	"-nf", normfgcolor,
	"-sb", selbgcolor,
	"-sf", selfgcolor,
	NULL
};
static const char *spcmd1[] = {"w", "st", "-n", "spterm (w)", "-g", "120x34", NULL };
static const char *spcmd2[] = {"e", "st", "-n", "spterm (e)", "-g", "120x34", NULL };
static const char *spcmd3[] = {"r", "st", "-n", "spfm (r)", "-g", "144x41", "-e", "ranger", NULL };
static const char *statusclickcmd[] = { NULL, "/path/to/statusclick", NULL };

static Key keys[] = {
	/* type       modifier                      key              function                argument */
	{ KeyPress,   MODKEY,                       XK_d,            spawn,                  {.v = dmenucmd } }, // spawn dmenu for launching other programs
	{ KeyPress,   MODKEY,                       XK_Return,       spawn,                  {.v = termcmd } }, // spawn a terminal
	{ KeyPress,   MODKEY|Shift,                 XK_Return,       riospawn,               {.v = termcmd } }, // draw/spawn a terminal
	{ KeyPress,   MODKEY,                       XK_b,            togglebar,              {0} }, // toggles the display of the bar(s) on the current monitor

	{ KeyPress,   MODKEY,                       XK_j,            focusstack,             {.i = +1 } }, // focus on the next client in the stack
	{ KeyPress,   MODKEY,                       XK_k,            focusstack,             {.i = -1 } }, // focus on the previous client in the stack
	{ KeyPress,   MODKEY|Alt|Shift,             XK_j,            focusstack,             {.i = +2 } }, // allows focusing on hidden clients
	{ KeyPress,   MODKEY|Alt|Shift,             XK_k,            focusstack,             {.i = -2 } }, // allows focusing on hidden clients
	{ KeyPress,   MODKEY,                       XK_Left,         focusdir,               {.i = 0 } }, // focus on the client left of the currently focused client
	{ KeyPress,   MODKEY,                       XK_Right,        focusdir,               {.i = 1 } }, // focus on the client right of the currently focused client
	{ KeyPress,   MODKEY,                       XK_Up,           focusdir,               {.i = 2 } }, // focus on the client above the currently focused client
	{ KeyPress,   MODKEY,                       XK_Down,         focusdir,               {.i = 3 } }, // focus on the client below the currently focused client
	{ KeyPress,   MODKEY|ControlMask,           XK_Left,         placedir,               {.i = 0 } }, // swap places with the client window on the immediate left of the current client
	{ KeyPress,   MODKEY|ControlMask,           XK_Right,        placedir,               {.i = 1 } }, // swap places with the client window on the immediate right of the current client
	{ KeyPress,   MODKEY|ControlMask,           XK_Up,           placedir,               {.i = 2 } }, // swap places with the client window on the immediate up of the current client
	{ KeyPress,   MODKEY|ControlMask,           XK_Down,         placedir,               {.i = 3 } }, // swap places with the client window on the immediate down of the current client

	{ KeyPress,   MODKEY|Ctrl,                  XK_j,            pushdown,               {0} }, // move the selected client down the stack
	{ KeyPress,   MODKEY|Ctrl,                  XK_k,            pushup,                 {0} }, // move the selected client up the stack
	{ KeyPress,   MODKEY,                       XK_i,            incnmaster,             {.i = +1 } }, // increase the number of clients in the master area
	{ KeyPress,   MODKEY,                       XK_u,            incnmaster,             {.i = -1 } }, // decrease the number of clients in the master area
	{ KeyPress,   MODKEY|Ctrl,                  XK_i,            incnstack,              {.i = +1 } }, // increase the number of clients in the primary (first) stack area
	{ KeyPress,   MODKEY|Ctrl,                  XK_u,            incnstack,              {.i = -1 } }, // increase the number of clients in the primary (first) stack area
	{ KeyPress,   MODKEY,                       XK_h,            setmfact,               {.f = -0.05} }, // decrease the size of the master area compared to the stack area(s)
	{ KeyPress,   MODKEY,                       XK_l,            setmfact,               {.f = +0.05} }, // increase the size of the master area compared to the stack area(s)
	{ KeyPress,   MODKEY|Shift,                 XK_h,            setcfact,               {.f = +0.25} }, // increase size respective to other windows within the same area
	{ KeyPress,   MODKEY|Shift,                 XK_l,            setcfact,               {.f = -0.25} }, // decrease client size respective to other windows within the same area
	{ KeyPress,   MODKEY|Shift,                 XK_o,            setcfact,               {0} },

	{ KeyPress,   MODKEY,                       XK_backslash,    togglepinnedws,         {0} }, // toggle pinning of currently selected workspace on the current monitor
	{ KeyPress,   MODKEY,                       XK_z,            showhideclient,         {0} }, // hide the currently selected client (or show if hidden)
	{ KeyPress,   MODKEY,                       XK_q,            killclient,             {0} }, // close the currently focused window
	{ KeyPress,   MODKEY|Shift,                 XK_q,            restart,                {0} }, // restart dusk
	{ KeyPress,   MODKEY|Ctrl|Alt,              XK_q,            quit,                   {0} }, // exit dusk

	{ KeyPress,   MODKEY,                       XK_a,            markall,                {0} }, // marks all clients on the selected workspace
	{ KeyPress,   MODKEY|Ctrl,                  XK_a,            markall,                {1} }, // marks all floating clients on the selected workspace
	{ KeyPress,   MODKEY|Alt,                   XK_a,            markall,                {2} }, // marks all hidden clients on the selected workspace
	{ KeyPress,   MODKEY|Shift,                 XK_a,            unmarkall,              {0} }, // unmarks all clients
	{ KeyPress,   MODKEY,                       XK_m,            togglemark,             {0} }, // marks or unmarks the selected client for group action
	{ KeyPress,   MODKEY|Alt,                   XK_m,            zoom,                   {0} }, // moves the currently focused window to/from the master area (for tiled layouts)

	{ KeyPress,   MODKEY,                       XK_bracketleft,  rotatelayoutaxis,       {.i = -1 } }, // cycle through the available layout splits (horizontal, vertical, centered, no split, etc.)
	{ KeyPress,   MODKEY,                       XK_bracketright, rotatelayoutaxis,       {.i = +1 } }, // cycle through the available layout splits (horizontal, vertical, centered, no split, etc.)
	{ KeyPress,   MODKEY|Alt,                   XK_bracketleft,  rotatelayoutaxis,       {.i = -2 } }, // cycle through the available tiling arrangements for the master area
	{ KeyPress,   MODKEY|Alt,                   XK_bracketright, rotatelayoutaxis,       {.i = +2 } }, // cycle through the available tiling arrangements for the master area
	{ KeyPress,   MODKEY|Shift,                 XK_bracketleft,  rotatelayoutaxis,       {.i = -3 } }, // cycle through the available tiling arrangements for the primary (first) stack area
	{ KeyPress,   MODKEY|Shift,                 XK_bracketright, rotatelayoutaxis,       {.i = +3 } }, // cycle through the available tiling arrangements for the primary (first) stack area
	{ KeyPress,   MODKEY|Ctrl,                  XK_bracketleft,  rotatelayoutaxis,       {.i = -4 } }, // cycle through the available tiling arrangements for the secondary stack area
	{ KeyPress,   MODKEY|Ctrl,                  XK_bracketright, rotatelayoutaxis,       {.i = +4 } }, // cycle through the available tiling arrangements for the secondary stack area
	{ KeyPress,   MODKEY|Ctrl,                  XK_m,            mirrorlayout,           {0} }, // flip the master and stack areas
	{ KeyPress,   MODKEY|Ctrl|Shift,            XK_m,            layoutconvert,          {0} }, // flip between horizontal and vertical layout
	{ KeyPress,   MODKEY,                       XK_space,        setlayout,              {0} }, // toggles between current and previous layout

	{ KeyPress,   MODKEY|Ctrl,                  XK_g,            floatpos,               {.v = "50% 50% 80% 80%" } }, // center client and take up 80% of the screen
	{ KeyPress,   MODKEY,                       XK_g,            togglefloating,         {0} }, // toggles between tiled and floating arrangement for the currently focused client
	{ KeyPress,   MODKEY,                       XK_f,            togglefullscreen,       {0} }, // toggles fullscreen for the currently selected client
	{ KeyPress,   MODKEY|Shift,                 XK_f,            togglefakefullscreen,   {0} }, // toggles "fake" fullscreen for the selected window
	{ KeyPress,   Ctrl|Alt,                     XK_Tab,          togglenomodbuttons,     {0} }, // disables / enables keybindings that are not accompanied by any modifier buttons for a client
	{ KeyPress,   MODKEY|ShiftMask,             XK_plus,         changeopacity,          {.f = +0.05 } }, // increase the client opacity (for compositors that support _NET_WM_OPACITY)
	{ KeyPress,   MODKEY|ShiftMask,             XK_minus,        changeopacity,          {.f = -0.05 } }, // decrease the client opacity (for compositors that support _NET_WM_OPACITY)

	{ KeyPress,   MODKEY|Shift,                 XK_comma,        focusmon,               {.i = -1 } }, // focus on the previous monitor, if any
	{ KeyPress,   MODKEY|Shift,                 XK_period,       focusmon,               {.i = +1 } }, // focus on the next monitor, if any
	{ KeyPress,   MODKEY|Alt,                   XK_comma,        clienttomon,            {.i = -1 } }, // sends the current client to an adjacent monitor
	{ KeyPress,   MODKEY|Alt,                   XK_period,       clienttomon,            {.i = +1 } }, // sends the current client to an adjacent monitor
	{ KeyPress,   MODKEY|Alt|Shift,             XK_comma,        clientstomon,           {.i = +1 } }, // sends all clients to an adjacent monitor
	{ KeyPress,   MODKEY|Alt|Shift,             XK_period,       clientstomon,           {.i = -1 } }, // sends all clients to an adjacent monitor
	{ KeyPress,   MODKEY|Ctrl,                  XK_0,            viewallwsonmon,         {0} },        // view all workspaces on the current monitor
	{ KeyPress,   MODKEY,                       XK_0,            viewalloccwsonmon,      {0} },        // view all workspaces on the current monitor that has clients
	{ KeyPress,   MODKEY,                       XK_o,            viewselws,              {0} },        // view the selected workspace (only relevant when viewing multiple workspaces)
	{ KeyPress,   MODKEY|Ctrl,                  XK_comma,        viewwsdir,              {.i = -1 } }, // view the workspace on the immediate left of current workspace (on the current monitor)
	{ KeyPress,   MODKEY|Ctrl,                  XK_period,       viewwsdir,              {.i = +1 } }, // view the workspace on the immediate right of current workspace (on the current monitor)
	{ KeyPress,   MODKEY,                       XK_comma,        viewwsdir,              {.i = -2 } }, // view the next workspace left of current workspace that has clients (on the current monitor)
	{ KeyPress,   MODKEY,                       XK_period,       viewwsdir,              {.i = +2 } }, // view the next workspace right of current workspace that has clients (on the current monitor)
	{ KeyPress,   MODKEY|Shift,                 XK_Tab,          viewwsdir,              {.i = -2 } }, // view the next workspace left of current workspace that has clients (on the current monitor)
	{ KeyPress,   MODKEY,                       XK_Tab,          viewwsdir,              {.i = +2 } }, // view the next workspace right of current workspace that has clients (on the current monitor)
	{ KeyPress,   MODKEY|Ctrl|Alt,              XK_comma,        movewsdir,              {.i = -1 } }, // move client to workspace on the immediate left of current workspace (on the current monitor)
	{ KeyPress,   MODKEY|Ctrl|Alt,              XK_period,       movewsdir,              {.i = +1 } }, // move client to workspace on the immediate right of current workspace (on the current monitor)


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
//	{ KeyPress,   MODKEY,                       XK_Control_R,    showbar,                {0} },
//	{ KeyRelease, MODKEY|AltGr,                 XK_Control_R,    hidebar,                {0} },
//	{ KeyPress,   MODKEY,                       XK_,             incrgaps,               {.i = +1 } }, // increase all gaps (outer, inner, horizontal and vertical)
//	{ KeyPress,   MODKEY,                       XK_,             incrgaps,               {.i = -1 } }, // decrease all gaps (outer, inner, horizontal and vertical)
//	{ KeyPress,   MODKEY,                       XK_,             incrigaps,              {.i = +1 } }, // increase inner gaps (horizontal and vertical)
//	{ KeyPress,   MODKEY,                       XK_,             incrigaps,              {.i = -1 } }, // decrease inner gaps (horizontal and vertical)
//	{ KeyPress,   MODKEY,                       XK_,             incrogaps,              {.i = +1 } }, // increase outer gaps (horizontal and vertical)
//	{ KeyPress,   MODKEY,                       XK_,             incrogaps,              {.i = -1 } }, // decrease outer gaps (horizontal and vertical)
//	{ KeyPress,   MODKEY,                       XK_,             incrihgaps,             {.i = +1 } }, // increase inner horizontal gaps
//	{ KeyPress,   MODKEY,                       XK_,             incrihgaps,             {.i = -1 } }, // decrease inner horizontal gaps
//	{ KeyPress,   MODKEY,                       XK_,             incrivgaps,             {.i = +1 } }, // increase inner vertical gaps
//	{ KeyPress,   MODKEY,                       XK_,             incrivgaps,             {.i = -1 } }, // decrease inner vertical gaps
//	{ KeyPress,   MODKEY,                       XK_,             incrohgaps,             {.i = +1 } }, // increase outer horizontal gaps
//	{ KeyPress,   MODKEY,                       XK_,             incrohgaps,             {.i = -1 } }, // decrease outer horizontal gaps
//	{ KeyPress,   MODKEY,                       XK_,             incrovgaps,             {.i = +1 } }, // increase outer vertical gaps
//	{ KeyPress,   MODKEY,                       XK_,             incrovgaps,             {.i = -1 } }, // decrease outer vertical gaps
//	{ KeyPress,   MODKEY,                       XK_,             mark,                   {0} }, // marks the currently selected client
//	{ KeyPress,   MODKEY,                       XK_,             unmark,                 {0} }, // unmarks the currently selected client
//	{ KeyPress,   MODKEY,                       XK_,             togglegaps,             {0} }, // enables and disables the rendering of gaps in tiled layouts
//	{ KeyPress,   MODKEY,                       XK_,             defaultgaps,            {0} }, // revert gaps to the default settings
//	{ KeyPress,   MODKEY,                       XK_,             cyclelayout,            {.i = -1 } }, // cycle through the available layouts
//	{ KeyPress,   MODKEY,                       XK_,             cyclelayout,            {.i = +1 } }, // cycle through the available layouts (in reverse)
//	{ KeyPress,   MODKEY,                       XK_,             viewwsdir,              {.i = -1 } }, // move to the workspace on the immediate left of the current workspace on the current monitor (wraps around)
//	{ KeyPress,   MODKEY,                       XK_,             viewwsdir,              {.i = +1 } }, // move to the workspace on the immediate right of the current workspace on the current monitor (wraps around)
//	{ KeyPress,   MODKEY,                       XK_,             focusmaster,            {0} }, // change focus to the first client in the stack (master)
//	{ KeyPress,   MODKEY,                       XK_,             transfer,               {0} }, // move a client between the master and stack area automatically adjusting nmaster
//	{ KeyPress,   MODKEY,                       XK_,             transferall,            {0} }, // swaps all clients in the stack area with all clients in the master area
//	{ KeyPress,   MODKEY,                       XK_,             togglesticky,           {0} }, // makes a client show on all workspaces)
//	{ KeyPress,   MODKEY,                       XK_,             focusurgent,            {0} }, // focus on the client marked as urgent
//	{ KeyPress,   MODKEY,                       XK_,             inplacerotate,          {.i = +1} }, // rotate clients within the respective area (master, primary stack, secondary stack) clockwise
//	{ KeyPress,   MODKEY,                       XK_,             inplacerotate,          {.i = -1} }, // rotate clients within the respective area (master, primary stack, secondary stack) counter-clockwise
//	{ KeyPress,   MODKEY,                       XK_,             rotatestack,            {.i = +1 } }, // rotate all clients (clockwise)
//	{ KeyPress,   MODKEY,                       XK_,             rotatestack,            {.i = -1 } }, // rotate all clients (counter-clockwise)
//	{ KeyPress,   MODKEY,                       XK_,             riodraw,                {0} }, // use slop to resize the currently selected client
//	{ KeyPress,   MODKEY,                       XK_,             unfloatvisible,         {0} }, // makes all floating clients on the currently selected workspace tiled
//	{ KeyPress,   MODKEY,                       XK_,             switchcol,              {0} }, // changes focus between the master and the primary stack area
//	{ KeyPress,   MODKEY,                       XK_,             setlayout,              {.v = &layouts[0]} }, // sets a specific layout, see the layouts array for indices
//	{ KeyPress,   MODKEY,                       XK_,             xrdb,                   {0 } }, // reloads colors from XResources
};

/* button definitions */
/* click can be ClkButton, ClkWorkspaceBar, ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static Button buttons[] = {
	/* click                     event mask               button          function          argument */
	{ ClkLtSymbol,               0,                       Button1,        setlayout,        {0} }, // toggles between current and previous layout
	{ ClkLtSymbol,               0,                       Button4,        cyclelayout,      {.i = +1 } }, // cycle through the available layouts
	{ ClkLtSymbol,               0,                       Button5,        cyclelayout,      {.i = -1 } }, // cycle through the available layouts (in reverse)
	{ ClkWinTitle,               0,                       Button1,        focuswin,         {0} }, // focus on the given client
	{ ClkWinTitle,               0,                       Button3,        showhideclient,   {0} }, // hide the currently selected client (or show if hidden)
	{ ClkWinTitle,               0,                       Button2,        zoom,             {0} }, // moves the currently focused window to/from the master area (for tiled layouts)
	{ ClkStatusText,             0,                       Button1,        statusclick,      {.i = 1 } }, // sends mouse button presses to statusclick script when clicking on status modules
	{ ClkStatusText,             0,                       Button2,        statusclick,      {.i = 2 } },
	{ ClkStatusText,             0,                       Button3,        statusclick,      {.i = 3 } },
	{ ClkStatusText,             0,                       Button4,        statusclick,      {.i = 4 } },
	{ ClkStatusText,             0,                       Button5,        statusclick,      {.i = 5 } },
	{ ClkStatusText,             0,                       Button6,        statusclick,      {.i = 6 } },
	{ ClkStatusText,             0,                       Button7,        statusclick,      {.i = 7 } },
	{ ClkStatusText,             0,                       Button8,        statusclick,      {.i = 8 } },
	{ ClkStatusText,             0,                       Button9,        statusclick,      {.i = 9 } },
	{ ClkStatusText,             Shift,                   Button1,        statusclick,      {.i = 10 } },
	{ ClkStatusText,             Shift,                   Button2,        statusclick,      {.i = 11 } },
	{ ClkStatusText,             Shift,                   Button3,        statusclick,      {.i = 12 } },
	{ ClkClientWin,              MODKEY,                  Button8,        markmouse,        {1} }, // marks clients under the mouse cursor for group action
	{ ClkClientWin,              MODKEY|Shift,            Button8,        markmouse,        {0} }, // unmarks clients under the mouse cursor for group action
	{ ClkClientWin,              MODKEY,                  Button9,        markmouse,        {2} }, // toggles marking of clients under the mouse cursor for group action
	{ ClkClientWin,              MODKEY,                  Button1,        moveorplace,      {1} }, // moves a client window into a floating or tiled position depending on floating state
	{ ClkClientWin,              MODKEY|Shift,            Button1,        movemouse,        {0} }, // moves a floating window, if the window is tiled then it will snap out to become floating
	{ ClkClientWin,              MODKEY|Alt,              Button2,        togglefloating,   {0} }, // toggles between tiled and floating arrangement for given client
	{ ClkClientWin,              MODKEY,                  Button3,        resizeorcfacts,   {0} }, // change the size of a floating client window or adjust cfacts (dragcfact) when tiled
	{ ClkClientWin,              MODKEY|Shift,            Button3,        resizemouse,      {0} }, // change the size of a floating client window
	{ ClkClientWin,              0,                       Button8,        movemouse,        {0} }, // move a client window using extra mouse buttons (previous)
	{ ClkClientWin,              0,                       Button9,        resizemouse,      {0} }, // resize a client window using extra mouse buttons (next)
	{ ClkClientWin,              MODKEY,                  Button2,        zoom,             {0} }, // moves the currently focused window to/from the master area (for tiled layouts)
	{ ClkClientWin,              MODKEY|Ctrl,             Button1,        dragmfact,        {0} }, // dynamically change the size of the master area compared to the stack area(s)
	{ ClkRootWin,                MODKEY|Ctrl,             Button1,        dragmfact,        {0} }, // dynamically change the size of the master area compared to the stack area(s)
	{ ClkClientWin,              MODKEY,                  Button4,        inplacerotate,    {.i = +1 } }, // rotate clients within the respective area (master, primary stack, secondary stack) clockwise
	{ ClkClientWin,              MODKEY,                  Button5,        inplacerotate,    {.i = -1 } }, // rotate clients within the respective area (master, primary stack, secondary stack) counter-clockwise
	{ ClkClientWin,              MODKEY|Shift,            Button4,        rotatestack,      {.i = +1 } }, // rotate all clients (clockwise)
	{ ClkClientWin,              MODKEY|Shift,            Button5,        rotatestack,      {.i = -1 } }, // rotate all clients (counter-clockwise)
	{ ClkWorkspaceBar,           0,                       Button1,        viewws,           {0} }, // view the workspace by clicking on workspace icon
	{ ClkWorkspaceBar,           MODKEY,                  Button1,        movews,           {0} }, // sends (moves) the currently focused client to given workspace
	{ ClkWorkspaceBar,           MODKEY|Shift|Ctrl,       Button1,        swapws,           {0} }, // swaps all clients on current workspace with that of the given workspace
	{ ClkWorkspaceBar,           0,                       Button3,        enablews,         {0} }, // enables the workspace in addition to other workspaces
	{ ClkWorkspaceBar,           0,                       Button4,        viewwsdir,        {.i = +2 } }, // view the next workspace right of current workspace that has clients (on the current monitor)
	{ ClkWorkspaceBar,           0,                       Button5,        viewwsdir,        {.i = -2 } }, // view the next workspace left of current workspace that has clients (on the current monitor)
	{ ClkWorkspaceBar,           MODKEY,                  Button2,        togglepinnedws,   {0} }, // toggles the pinning of a workspace to the current monitor
};

static const char *ipcsockpath = "/tmp/dusk.sock";
static IPCCommand ipccommands[] = {
	IPCCOMMANDS( customlayout, 8, ARG_TYPE_SINT, ARG_TYPE_STR, ARG_TYPE_SINT, ARG_TYPE_SINT, ARG_TYPE_SINT, ARG_TYPE_SINT, ARG_TYPE_SINT, ARG_TYPE_SINT ),
	IPCCOMMAND( changeopacity, ARG_TYPE_FLOAT ),
	IPCCOMMAND( clienttomon, ARG_TYPE_SINT ),
	IPCCOMMAND( clientstomon, ARG_TYPE_SINT ),
	IPCCOMMAND( cyclelayout, ARG_TYPE_SINT ),
	IPCCOMMAND( defaultgaps, ARG_TYPE_NONE ),
	IPCCOMMAND( enable, ARG_TYPE_STR ),
	IPCCOMMAND( disable, ARG_TYPE_STR ),
	IPCCOMMAND( floatpos, ARG_TYPE_STR ),
	IPCCOMMAND( focusdir, ARG_TYPE_SINT ),
	IPCCOMMAND( focusmaster, ARG_TYPE_NONE ),
	IPCCOMMAND( focusmon, ARG_TYPE_SINT ),
	IPCCOMMAND( focusstack, ARG_TYPE_SINT ),
	IPCCOMMAND( focusurgent, ARG_TYPE_NONE ),
	IPCCOMMAND( hidebar, ARG_TYPE_NONE ),
	IPCCOMMAND( incrgaps, ARG_TYPE_SINT ),
	IPCCOMMAND( incrigaps, ARG_TYPE_SINT ),
	IPCCOMMAND( incrogaps, ARG_TYPE_SINT ),
	IPCCOMMAND( incrihgaps, ARG_TYPE_SINT ),
	IPCCOMMAND( incrivgaps, ARG_TYPE_SINT ),
	IPCCOMMAND( incrohgaps, ARG_TYPE_SINT ),
	IPCCOMMAND( incrovgaps, ARG_TYPE_SINT ),
	IPCCOMMAND( incnmaster, ARG_TYPE_SINT ),
	IPCCOMMAND( incnstack, ARG_TYPE_SINT ),
	IPCCOMMAND( inplacerotate, ARG_TYPE_SINT ),
	IPCCOMMAND( killclient, ARG_TYPE_NONE ),
	IPCCOMMAND( killunsel, ARG_TYPE_NONE ),
	IPCCOMMAND( layoutconvert, ARG_TYPE_NONE ),
	IPCCOMMAND( mark, ARG_TYPE_PTR ),
	IPCCOMMAND( markall, ARG_TYPE_SINT ), // 0 = mark all, 1 = mark floating, 2 = mark hidden
	IPCCOMMAND( mirrorlayout, ARG_TYPE_NONE ),
	IPCCOMMAND( rotatelayoutaxis, ARG_TYPE_SINT ),
	IPCCOMMAND( rotatestack, ARG_TYPE_SINT ),
	IPCCOMMAND( placedir, ARG_TYPE_SINT ),
	IPCCOMMAND( pushdown, ARG_TYPE_NONE ),
	IPCCOMMAND( pushup, ARG_TYPE_NONE ),
	IPCCOMMAND( quit, ARG_TYPE_NONE ),
	IPCCOMMAND( removescratch, ARG_TYPE_SINT ),
	IPCCOMMAND( restart, ARG_TYPE_NONE ),
	IPCCOMMAND( rioresize, ARG_TYPE_NONE ),
	IPCCOMMAND( setattachdefault, ARG_TYPE_STR),
	IPCCOMMAND( setborderpx, ARG_TYPE_SINT ),
	IPCCOMMAND( setlayoutaxisex, ARG_TYPE_SINT ),
	IPCCOMMAND( setlayoutex, ARG_TYPE_SINT ),
	IPCCOMMAND( setlayoutsafe, ARG_TYPE_PTR ),
	IPCCOMMAND( setcfact, ARG_TYPE_FLOAT ),
	IPCCOMMAND( setmfact, ARG_TYPE_FLOAT ),
	IPCCOMMAND( setgapsex, ARG_TYPE_SINT ),
	IPCCOMMAND( setscratch, ARG_TYPE_SINT ),
	IPCCOMMANDS( setstatus, 2, ARG_TYPE_UINT, ARG_TYPE_STR ),
	IPCCOMMAND( showbar, ARG_TYPE_NONE ),
	IPCCOMMAND( showhideclient, ARG_TYPE_NONE ),
	IPCCOMMAND( stackpush, ARG_TYPE_SINT ),
	IPCCOMMAND( stackfocus, ARG_TYPE_SINT ),
	IPCCOMMAND( switchcol, ARG_TYPE_NONE ),
	IPCCOMMAND( toggle, ARG_TYPE_STR ), // toggle functionality on and off
	IPCCOMMAND( togglebar, ARG_TYPE_NONE ),
	IPCCOMMAND( togglebarpadding, ARG_TYPE_NONE ),
	IPCCOMMAND( togglefakefullscreen, ARG_TYPE_NONE ),
	IPCCOMMAND( toggleflag, ARG_TYPE_STR ),
	IPCCOMMAND( togglefloating, ARG_TYPE_NONE ),
	IPCCOMMAND( togglefullscreen, ARG_TYPE_NONE ),
	IPCCOMMAND( togglegaps, ARG_TYPE_NONE ),
	IPCCOMMAND( togglemark, ARG_TYPE_PTR ),
	IPCCOMMAND( togglenomodbuttons, ARG_TYPE_NONE ),
	IPCCOMMAND( togglescratch, ARG_TYPE_UINT ),
	IPCCOMMAND( togglesticky, ARG_TYPE_NONE ),
	IPCCOMMAND( transfer, ARG_TYPE_NONE ),
	IPCCOMMAND( transferall, ARG_TYPE_NONE ),
	IPCCOMMAND( unfloatvisible, ARG_TYPE_NONE ),
	IPCCOMMAND( unmark, ARG_TYPE_PTR ),
	IPCCOMMAND( unmarkall, ARG_TYPE_NONE ),
	IPCCOMMAND( viewallwsonmon, ARG_TYPE_NONE ),
	IPCCOMMAND( viewalloccwsonmon, ARG_TYPE_NONE ),
	IPCCOMMAND( viewselws, ARG_TYPE_NONE ),
	IPCCOMMAND( viewwsdir, ARG_TYPE_SINT ),
	IPCCOMMAND( xrdb, ARG_TYPE_NONE ), // reload xrdb / Xresources
	IPCCOMMAND( zoom, ARG_TYPE_NONE ),
};
