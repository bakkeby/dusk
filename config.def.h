/* See LICENSE file for copyright and license details. */

/* appearance */
static const unsigned int borderpx       = 5;   /* border pixel of windows */
static const unsigned int snap           = 32;  /* snap pixel */
static const unsigned int gappih         = 5;   /* horiz inner gap between windows */
static const unsigned int gappiv         = 5;   /* vert inner gap between windows */
static const unsigned int gappoh         = 5;   /* horiz outer gap between windows and screen edge */
static const unsigned int gappov         = 5;   /* vert outer gap between windows and screen edge */
static const unsigned int smartgaps_fact = 0;   /* smartgaps factor when there is only one client; 0 = no gaps, 3 = 3x outer gaps */

static unsigned int attachdefault        = AttachAside; // 0 (default), AttachAbove, AttachSide, AttachBelow, AttachBottom

static const int showbar                 = 1;   /* 0 means no bar */
static const int topbar                  = 1;   /* 0 means bottom bar */

static const int bar_height              = 0;   /* 0 means derive from font, >= 1 explicit height */
static const int vertpad                 = borderpx;  /* vertical (outer) padding of bar */
static const int sidepad                 = borderpx;  /* horizontal (outer) padding of bar */

static const int taggridrows             = 2; /* number of rows to render the tag grid */

static int floatposgrid_x                = 5;  /* float grid columns */
static int floatposgrid_y                = 5;  /* float grid rows */

static const int horizpadbar             = 2;   /* horizontal (inner) padding for statusbar (increases lrpad) */
static const int vertpadbar              = 0;   /* vertical (inner) padding for statusbar (increases bh, overridden by bar_height) */

static const char buttonbar[]            = "⛶";
static const unsigned int systrayspacing = 2;   /* systray spacing */
static char *toggle_float_pos            = "50% 50% 80% 80%"; // default floating position when triggering togglefloatpos

/* Indicators: see patch/bar_indicators.h for options */
static int tagindicatortype              = INDICATOR_BOTTOM_BAR_SLIM;
static int fakefsindicatortype           = INDICATOR_PLUS;
static int floatfakefsindicatortype      = INDICATOR_PLUS_AND_LARGER_SQUARE;
static int floatindicatortype            = INDICATOR_TOP_LEFT_LARGER_SQUARE;
static int tiledindicatortype            = INDICATOR_NONE;

/* See util.h for options */
static unsigned long functionality = 0
	|Systray
	|SwallowFloating
	|CenteredWindowName
	|SpawnCwd
	|BarBorder
	|Warp
	|FocusOnNetActive
	|AllowNoModifierButtons
	|TagIntoStack
	|ColorEmoji
//	|FocusedOnTop
//	|BarActiveGroupBorderColor
//	|DecorationHints
//	|NoBorder
//	|PerTagBar
//	|SortScreens
//	|ViewOnTag
//  |Xresources
;

static const char statussep              = ';'; /* separator between status bars */

#define FLEXWINTITLE_MASTERWEIGHT 15   // master weight compared to hidden and floating window titles
#define FLEXWINTITLE_STACKWEIGHT 4     // stack weight compared to hidden and floating window titles
#define FLEXWINTITLE_HIDDENWEIGHT 0    // hidden window title weight
#define FLEXWINTITLE_FLOATWEIGHT 0     // floating window title weight, set to 0 to not show floating windows

static const char *fonts[]               = { "monospace:size=10" };
static const char dmenufont[]            = "monospace:size=10";

static char c000000[]                    = "#000000"; // placeholder value

static char normfgcolor[]                = "#C6BDBD";
static char normbgcolor[]                = "#180A13";
static char normbordercolor[]            = "#444444";
static char normfloatcolor[]             = "#787590";

static char selfgcolor[]                 = "#FFF7D4";
static char selbgcolor[]                 = "#330000";
static char selbordercolor[]             = "#330000";
static char selfloatcolor[]              = "#8a902c";

static char titlenormfgcolor[]           = "#C6BDBD";
static char titlenormbgcolor[]           = "#180A13";
static char titlenormbordercolor[]       = "#330000";
static char titlenormfloatcolor[]        = "#787590";

static char titleselfgcolor[]            = "#FFF7D4";
static char titleselbgcolor[]            = "#330000";
static char titleselbordercolor[]        = "#330000";
static char titleselfloatcolor[]         = "#8a902c";

static char tagsnormfgcolor[]            = "#C6BDBD";
static char tagsnormbgcolor[]            = "#180A13";
static char tagsnormbordercolor[]        = "#444444";
static char tagsnormfloatcolor[]         = "#787590";

static char tagsselfgcolor[]             = "#FFF7D4";
static char tagsselbgcolor[]             = "#330000";
static char tagsselbordercolor[]         = "#330000";
static char tagsselfloatcolor[]          = "#8a902c";

static char hidfgcolor[]                 = "#c278b6";
static char hidbgcolor[]                 = "#222222";
static char hidbordercolor[]             = "#330000";
static char hidfloatcolor[]              = "#f76e0c";

static char urgfgcolor[]                 = "#bbbbbb";
static char urgbgcolor[]                 = "#222222";
static char urgbordercolor[]             = "#d10f3f";
static char urgfloatcolor[]              = "#db8fd9";

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
	[SchemeTagsNorm]     = { OPAQUE, baralpha, borderalpha },
	[SchemeTagsSel]      = { OPAQUE, baralpha, borderalpha },
	[SchemeHid]          = { OPAQUE, baralpha, borderalpha },
	[SchemeUrg]          = { OPAQUE, baralpha, borderalpha },
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
	/*                       fg                bg                border                float */
	[SchemeNorm]         = { normfgcolor,      normbgcolor,      normbordercolor,      normfloatcolor },
	[SchemeSel]          = { selfgcolor,       selbgcolor,       selbordercolor,       selfloatcolor },
	[SchemeTitleNorm]    = { titlenormfgcolor, titlenormbgcolor, titlenormbordercolor, titlenormfloatcolor },
	[SchemeTitleSel]     = { titleselfgcolor,  titleselbgcolor,  titleselbordercolor,  titleselfloatcolor },
	[SchemeTagsNorm]     = { tagsnormfgcolor,  tagsnormbgcolor,  tagsnormbordercolor,  tagsnormfloatcolor },
	[SchemeTagsSel]      = { tagsselfgcolor,   tagsselbgcolor,   tagsselbordercolor,   tagsselfloatcolor },
	[SchemeHid]          = { hidfgcolor,       hidbgcolor,       hidbordercolor,       hidfloatcolor },
	[SchemeUrg]          = { urgfgcolor,       urgbgcolor,       urgbordercolor,       urgfloatcolor },
	[SchemeFlexActTTB]   = { titleselfgcolor,  actTTBbgcolor,    actTTBbgcolor,        c000000 },
	[SchemeFlexActLTR]   = { titleselfgcolor,  actLTRbgcolor,    actLTRbgcolor,        c000000 },
	[SchemeFlexActMONO]  = { titleselfgcolor,  actMONObgcolor,   actMONObgcolor,       c000000 },
	[SchemeFlexActGRID]  = { titleselfgcolor,  actGRIDbgcolor,   actGRIDbgcolor,       c000000 },
	[SchemeFlexActGRD1]  = { titleselfgcolor,  actGRD1bgcolor,   actGRD1bgcolor,       c000000 },
	[SchemeFlexActGRD2]  = { titleselfgcolor,  actGRD2bgcolor,   actGRD2bgcolor,       c000000 },
	[SchemeFlexActGRDM]  = { titleselfgcolor,  actGRDMbgcolor,   actGRDMbgcolor,       c000000 },
	[SchemeFlexActHGRD]  = { titleselfgcolor,  actHGRDbgcolor,   actHGRDbgcolor,       c000000 },
	[SchemeFlexActDWDL]  = { titleselfgcolor,  actDWDLbgcolor,   actDWDLbgcolor,       c000000 },
	[SchemeFlexActSPRL]  = { titleselfgcolor,  actSPRLbgcolor,   actSPRLbgcolor,       c000000 },
	[SchemeFlexActFloat] = { titleselfgcolor,  actfloatbgcolor,  actfloatbgcolor,      c000000 },
	[SchemeFlexInaTTB]   = { titlenormfgcolor, normTTBbgcolor,   normTTBbgcolor,       c000000 },
	[SchemeFlexInaLTR]   = { titlenormfgcolor, normLTRbgcolor,   normLTRbgcolor,       c000000 },
	[SchemeFlexInaMONO]  = { titlenormfgcolor, normMONObgcolor,  normMONObgcolor,      c000000 },
	[SchemeFlexInaGRID]  = { titlenormfgcolor, normGRIDbgcolor,  normGRIDbgcolor,      c000000 },
	[SchemeFlexInaGRD1]  = { titlenormfgcolor, normGRD1bgcolor,  normGRD1bgcolor,      c000000 },
	[SchemeFlexInaGRD2]  = { titlenormfgcolor, normGRD2bgcolor,  normGRD2bgcolor,      c000000 },
	[SchemeFlexInaGRDM]  = { titlenormfgcolor, normGRDMbgcolor,  normGRDMbgcolor,      c000000 },
	[SchemeFlexInaHGRD]  = { titlenormfgcolor, normHGRDbgcolor,  normHGRDbgcolor,      c000000 },
	[SchemeFlexInaDWDL]  = { titlenormfgcolor, normDWDLbgcolor,  normDWDLbgcolor,      c000000 },
	[SchemeFlexInaSPRL]  = { titlenormfgcolor, normSPRLbgcolor,  normSPRLbgcolor,      c000000 },
	[SchemeFlexInaFloat] = { titlenormfgcolor, normfloatbgcolor, normfloatbgcolor,     c000000 },
	[SchemeFlexSelTTB]   = { titleselfgcolor,  selTTBbgcolor,    selTTBbgcolor,        c000000 },
	[SchemeFlexSelLTR]   = { titleselfgcolor,  selLTRbgcolor,    selLTRbgcolor,        c000000 },
	[SchemeFlexSelMONO]  = { titleselfgcolor,  selMONObgcolor,   selMONObgcolor,       c000000 },
	[SchemeFlexSelGRID]  = { titleselfgcolor,  selGRIDbgcolor,   selGRIDbgcolor,       c000000 },
	[SchemeFlexSelGRD1]  = { titleselfgcolor,  selGRD1bgcolor,   selGRD1bgcolor,       c000000 },
	[SchemeFlexSelGRD2]  = { titleselfgcolor,  selGRD2bgcolor,   selGRD2bgcolor,       c000000 },
	[SchemeFlexSelGRDM]  = { titleselfgcolor,  selGRDMbgcolor,   selGRDMbgcolor,       c000000 },
	[SchemeFlexSelHGRD]  = { titleselfgcolor,  selHGRDbgcolor,   selHGRDbgcolor,       c000000 },
	[SchemeFlexSelDWDL]  = { titleselfgcolor,  selDWDLbgcolor,   selDWDLbgcolor,       c000000 },
	[SchemeFlexSelSPRL]  = { titleselfgcolor,  selSPRLbgcolor,   selSPRLbgcolor,       c000000 },
	[SchemeFlexSelFloat] = { titleselfgcolor,  selfloatbgcolor,  selfloatbgcolor,      c000000 },
};

static const char *const autostart[] = {
	"st", NULL,
	NULL /* terminate */
};


const char *spcmd1[] = {"st", "-n", "spterm", "-g", "120x34", NULL };
const char *spcmd2[] = {"st", "-n", "spfm", "-g", "144x41", "-e", "ranger", NULL };
const char *spcmd3[] = {"keepassxc", NULL };
static Sp scratchpads[] = {
   /* name          cmd  */
   {"spterm",      spcmd1},
   {"spranger",    spcmd2},
   {"keepassxc",   spcmd3},
};

/* There are two options when it comes to per-client rules:
 *  - a typical struct table or
 *  - using the RULE macro
 *
 * A traditional struct table looks like this:
 *    // class      instance  title  wintype  tags mask  monitor
 *    { "Gimp",     NULL,     NULL,  NULL,    1 << 4,    -1 },
 *    { "Firefox",  NULL,     NULL,  NULL,    1 << 7,    -1 },
 *
 * The RULE macro has the default values set for each field allowing you to only
 * specify the values that are relevant for your rule, e.g.
 *
 *    RULE(.class = "Gimp", .tags = 1 << 4)
 *    RULE(.class = "Firefox", .tags = 1 << 7)
 *
 * Refer to the Rule struct definition for the list of available fields depending on
 * the patches you enable.
 */
static const Rule rules[] = {
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
	RULE(.instance = "spterm", .tags = SPTAG(0), .flags = Floating)
	RULE(.instance = "spfm", .tags = SPTAG(1), .flags = Floating)
	RULE(.instance = "keepassxc", .tags = SPTAG(2))
	RULE(.title = "TermScratchpad (r)", .floatpos = "50% 50% 80% 80%", .tags = SPTAG(2), .flags = AlwaysOnTop|Floating)
	RULE(.title = "TermScratchpad (t)", .floatpos = "50% 50% 80% 80%", .tags = SPTAG(3), .flags = AlwaysOnTop|Floating)
	RULE(.class = "Gimp", .tags = 1 << 4, .flags = Floating|SwitchTag)
	RULE(.class = "firefox", .tags = 1 << 7, .flags = AttachMaster|SwitchTag)
	RULE(.class = "Steam", .flags = IgnoreCfgReqPos|Floating|Centered)
	RULE(.class = "steam_app_", .flags = IgnoreCfgReqPos|Floating|Centered)
	RULE(.class = "Google-chrome", .role = "GtkFileChooserDialog", .floatpos = "50% 50%", .flags = AlwaysOnTop|Floating)
	RULE(.role = "pop-up", .flags = AlwaysOnTop|Floating|Centered)
	RULE(.role = "browser", .tags = 1 << 7, .monitor = 0, .flags = AttachMaster|OnlyModButtons|SwitchTag)
	RULE(.class = "Gnome-terminal", .role = "gnome-terminal-preferences", .flags = Centered)
	RULE(.class = "Diffuse", .tags = 1 << 3, .flags = NoSwallow|SwitchTag|RevertTag)
	RULE(.class = "File-roller", .tags = 1 << 8, .flags = Centered|Floating|SwitchTag|RevertTag)
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
	{ -1,       0,     BAR_ALIGN_LEFT,         width_tags,               draw_tags,               click_tags,               "tags" },
	{ -1,       0,     BAR_ALIGN_LEFT,         width_taggrid,            draw_taggrid,            click_taggrid,            "taggrid" },
	{  0,       0,     BAR_ALIGN_RIGHT,        width_systray,            draw_systray,            click_systray,            "systray" },
	{ -1,       0,     BAR_ALIGN_LEFT,         width_ltsymbol,           draw_ltsymbol,           click_ltsymbol,           "layout" },
	{ 'A',      0,     BAR_ALIGN_RIGHT,        width_status2d,           draw_status2d,           click_statuscmd,          "status2d" },
	{ -1,       0,     BAR_ALIGN_NONE,         width_flexwintitle,       draw_flexwintitle,       click_flexwintitle,       "flexwintitle" },
	{ 'A',      1,     BAR_ALIGN_CENTER,       width_status2d_es,        draw_status2d_es,        click_statuscmd_es,       "status2d_es" },
	{ -1,       1,     BAR_ALIGN_RIGHT_RIGHT,  width_wintitle_hidden,    draw_wintitle_hidden,    click_wintitle_hidden,    "wintitle_hidden" },
	{ -1,       1,     BAR_ALIGN_LEFT,         width_wintitle_floating,  draw_wintitle_floating,  click_wintitle_floating,  "wintitle_floating" },
};

static const MonitorRule monrules[] = {
	/* monitor  tag   layout  mfact  nmaster  showbar  topbar */
	{  1,       -1,   2,      -1,    -1,      -1,      -1     }, // use a different layout for the second monitor
	{  -1,      -1,   0,      -1,    -1,      -1,      -1     }, // default
};

/* Tags
 * In a traditional dwm the number of tags in use can be changed simply by changing the number
 * of strings in the tags array. This build does things a bit different which has some added
 * benefits. If you need to change the number of tags here then change the NUMTAGS macro in dwm.c.
 *
 * Examples:
 *
 *  1) static char *tagicons[][NUMTAGS*2] = {
 *         [DEFAULT_TAGS] = { "1", "2", "3", "4", "5", "6", "7", "8", "9", "A", "B", "C", "D", "E", "F", "G", "H", "I" },
 *     }
 *
 *  2) static char *tagicons[][1] = {
 *         [DEFAULT_TAGS] = { "•" },
 *     }
 *
 * The first example would result in the tags on the first monitor to be 1 through 9, while the
 * tags for the second monitor would be named A through I. A third monitor would start again at
 * 1 through 9 while the tags on a fourth monitor would also be named A through I. Note the tags
 * count of NUMTAGS*2 in the array initialiser which defines how many tag text / icon exists in
 * the array. This can be changed to *3 to add separate icons for a third monitor.
 *
 * For the second example each tag would be represented as a bullet point. Both cases work the
 * same from a technical standpoint - the icon index is derived from the tag index and the monitor
 * index. If the icon index is is greater than the number of tag icons then it will wrap around
 * until it an icon matches. Similarly if there are two tag icons then it would alternate between
 * them. This works seamlessly with alternative tags and alttagsdecoration patches.
 */
static char *tagicons[][NUMTAGS*2] = {
	[DEFAULT_TAGS]        = { " ₁", " ₂", " ₃", " ₄", " ₅", " ₆", " ₇", " ₈", " ₉", " ₁", " ₂", " ₃", " ₄", " ₅", " ₆", " ₇", " ₈", " ₉" },
	[ALTERNATIVE_TAGS]    = { "A₁", "B₂", "C₃", "D₄", "E₅", "F₆", "G₇", "H₈", "I₉", "1₁", "2₂", "3₃", "4₄", "5₅", "6₆", "7₇", "8₈", "9₉" },
	[ALT_TAGS_DECORATION] = { "◉₁", "☢₂", "❖₃", "⚉₄", "♻₅", "⌬₆", "♹₇", "✇₈", "☉₉", "☋₁", "ℓ₂", "҂₃", "∆₄", "√₅", "∏₆", "Ξ₇", "Ω₈", "ƒ₉" },
};



/* layout(s) */
static const float mfact     = 0.50; /* factor of master area size [0.05..0.95] */
static const int nmaster     = 1;    /* number of clients in master area */
static const int nstack      = 0;    /* number of clients in primary stack area */
static const int resizehints = 0;    /* 1 means respect size hints in tiled resizals */

static const Layout layouts[] = {
	/* symbol     arrange function, { nmaster, nstack, layout, master axis, stack axis, secondary stack axis, symbol func } */
	{ "[]=",      flextile,         { -1, -1, SPLIT_VERTICAL, TOP_TO_BOTTOM, TOP_TO_BOTTOM, 0, NULL } }, // default tile layout
	{ "|||",      flextile,         { -1, -1, NO_SPLIT, LEFT_TO_RIGHT, LEFT_TO_RIGHT, 0, NULL } }, // columns
	{ "===",      flextile,         { -1, -1, NO_SPLIT, TOP_TO_BOTTOM, TOP_TO_BOTTOM, 0, NULL } }, // rows
	{ "[M]",      flextile,         { -1, -1, NO_SPLIT, MONOCLE, 0, 0, NULL } }, // monocle
	{ "||=",      flextile,         { -1, -1, SPLIT_VERTICAL, LEFT_TO_RIGHT, TOP_TO_BOTTOM, 0, NULL } }, // columns (col) layout
	{ ">M>",      flextile,         { -1, -1, FLOATING_MASTER, LEFT_TO_RIGHT, LEFT_TO_RIGHT, 0, NULL } }, // floating master
	{ "[D]",      flextile,         { -1, -1, SPLIT_VERTICAL, TOP_TO_BOTTOM, MONOCLE, 0, NULL } }, // deck
	{ "TTT",      flextile,         { -1, -1, SPLIT_HORIZONTAL, LEFT_TO_RIGHT, LEFT_TO_RIGHT, 0, NULL } }, // bstack
	{ "===",      flextile,         { -1, -1, SPLIT_HORIZONTAL, LEFT_TO_RIGHT, TOP_TO_BOTTOM, 0, NULL } }, // bstackhoriz
	{ "==#",      flextile,         { -1, -1, SPLIT_HORIZONTAL, TOP_TO_BOTTOM, GAPPLESSGRID, 0, NULL } }, // bstackgrid
	{ "|M|",      flextile,         { -1, -1, SPLIT_CENTERED_VERTICAL, TOP_TO_BOTTOM, TOP_TO_BOTTOM, TOP_TO_BOTTOM, NULL } }, // centeredmaster
	{ "-M-",      flextile,         { -1, -1, SPLIT_CENTERED_HORIZONTAL, LEFT_TO_RIGHT, LEFT_TO_RIGHT, LEFT_TO_RIGHT, NULL } }, // centeredmaster horiz
	{ ":::",      flextile,         { -1, -1, NO_SPLIT, GAPPLESSGRID, 0, 0, NULL } }, // gappless grid
	{ "[\\]",     flextile,         { -1, -1, NO_SPLIT, DWINDLE, 0, 0, NULL } }, // fibonacci dwindle
	{ "(@)",      flextile,         { -1, -1, NO_SPLIT, SPIRAL, 0, 0, NULL } }, // fibonacci spiral
 	{ "><>",      NULL,             {0} },    /* no layout function means floating behavior */
	{ NULL,       NULL,             {0} },
};

#define Shift ShiftMask
#define Ctrl ControlMask
#define Alt Mod1Mask
#define AltGr Mod3Mask
#define Super Mod4Mask
#define ShiftGr Mod5Mask

/* key definitions */
#define MODKEY Super

#define TAGKEYS(KEY,TAG) \
	{ MODKEY,                       KEY,      comboview,      {.ui = 1 << TAG} }, \
	{ MODKEY|Ctrl,                  KEY,      toggleview,     {.ui = 1 << TAG} }, \
	{ MODKEY|Shift,                 KEY,      combotag,       {.ui = 1 << TAG} }, \
	{ MODKEY|Ctrl|Shift,            KEY,      toggletag,      {.ui = 1 << TAG} }, \
	{ MODKEY|Alt|ShiftMask,         KEY,      swaptags,       {.ui = 1 << TAG} }, \
	{ MODKEY|Alt,                   KEY,      tagnextmon,     {.ui = 1 << TAG} }, \
	{ MODKEY|Alt|ControlMask,       KEY,      tagprevmon,     {.ui = 1 << TAG} },

#define STACKKEYS(MOD,ACTION) \
	{ MOD, XK_j, ACTION, {.i = INC(+1) } }, \
	{ MOD, XK_k, ACTION, {.i = INC(-1) } }, \
	{ MOD, XK_s, ACTION, {.i = PREVSEL } }, \
	{ MOD, XK_w, ACTION, {.i = 0 } }, \
	{ MOD, XK_e, ACTION, {.i = 1 } }, \
	{ MOD, XK_a, ACTION, {.i = 2 } }, \
	{ MOD, XK_z, ACTION, {.i = -1 } },

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }

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
	{ MODKEY,                       XK_d,            spawn,                  {.v = dmenucmd } },
	{ MODKEY,                       XK_Return,       spawn,                  {.v = termcmd } },
	{ MODKEY,                       XK_b,            togglebar,              {0} },

	STACKKEYS(MODKEY,                                stackfocus)
	STACKKEYS(MODKEY|Shift,                          stackpush)

	{ MODKEY,                       XK_j,            focusstack,             {.i = +1 } },
	{ MODKEY,                       XK_k,            focusstack,             {.i = -1 } },
	{ MODKEY|Alt|Shift,             XK_e,            focusstack,             {.i = +2 } },
	{ MODKEY|Alt|Shift,             XK_r,            focusstack,             {.i = -2 } },

	{ MODKEY|Ctrl,                  XK_j,            pushdown,               {0} },
	{ MODKEY|Ctrl,                  XK_k,            pushup,                 {0} },
	{ MODKEY,                       XK_i,            incnmaster,             {.i = +1 } },
	{ MODKEY,                       XK_u,            incnmaster,             {.i = -1 } },
	{ MODKEY|Ctrl,                  XK_i,            incnstack,              {.i = +1 } },
	{ MODKEY|Ctrl,                  XK_u,            incnstack,              {.i = -1 } },
	{ MODKEY,                       XK_h,            setmfact,               {.f = -0.05} },
	{ MODKEY,                       XK_l,            setmfact,               {.f = +0.05} },

	{ MODKEY|Shift,                 XK_h,            setcfact,               {.f = +0.25} },
	{ MODKEY|Shift,                 XK_l,            setcfact,               {.f = -0.25} },
	{ MODKEY|Shift,                 XK_o,            setcfact,               {0} },

	{ MODKEY,                       XK_Tab,          view,                   {0} },
	{ MODKEY,                       XK_comma,        shiftviewclients,       { .i = -1 } },
	{ MODKEY,                       XK_period,       shiftviewclients,       { .i = +1 } },
	{ MODKEY,                       XK_z,            showhideclient,         {0} },
	{ MODKEY,                       XK_q,            killclient,             {0} },
	{ MODKEY|Shift,                 XK_q,            quit,                   {0} }, // exit
	{ MODKEY|Ctrl|Shift,            XK_q,            quit,                   {1} }, // restart
	{ MODKEY,                       XK_o,            winview,                {0} },
	{ MODKEY,                       XK_bracketright, rotatelayoutaxis,       {.i = +1 } },   /* flextile, 1 = layout axis */
	{ MODKEY|Alt,                   XK_bracketright, rotatelayoutaxis,       {.i = +2 } },   /* flextile, 2 = master axis */
	{ MODKEY|Shift,                 XK_bracketright, rotatelayoutaxis,       {.i = +3 } },   /* flextile, 3 = stack axis */
	{ MODKEY|Ctrl,                  XK_bracketright, rotatelayoutaxis,       {.i = +4 } },   /* flextile, 4 = secondary stack axis */
	{ MODKEY,                       XK_bracketleft,  rotatelayoutaxis,       {.i = -1 } },   /* flextile, 1 = layout axis */
	{ MODKEY|Alt,                   XK_bracketleft,  rotatelayoutaxis,       {.i = -2 } },   /* flextile, 2 = master axis */
	{ MODKEY|Shift,                 XK_bracketleft,  rotatelayoutaxis,       {.i = -3 } },   /* flextile, 3 = stack axis */
	{ MODKEY|Ctrl,                  XK_bracketleft,  rotatelayoutaxis,       {.i = -4 } },   /* flextile, 4 = secondary stack axis */
	{ MODKEY|Ctrl,                  XK_m,            mirrorlayout,           {0} },          /* flextile, flip master and stack areas */
	{ MODKEY,                       XK_space,        setlayout,              {0} }, // toggles previous layout
	{ MODKEY|Shift,                 XK_s,            savefloats,             {0} }, // saves float posistion
	{ MODKEY|ShiftMask,             XK_space,        togglefloating,         {0} },
	{ Ctrl|Alt,                     XK_Tab,          togglenomodbuttons,     {0} },
	{ MODKEY,                       XK_w,            togglescratch,          {.ui = 0 } },
	{ MODKEY,                       XK_e,            togglescratch,          {.ui = 1 } },
	{ MODKEY,                       XK_r,            togglescratch,          {.ui = 2 } },
	{ MODKEY|Ctrl,                  XK_w,            setscratch,             {.ui = 0 } },
	{ MODKEY|Ctrl,                  XK_e,            setscratch,             {.ui = 1 } },
	{ MODKEY|Ctrl,                  XK_r,            setscratch,             {.ui = 2 } },
	{ MODKEY|Ctrl|Shift,            XK_w,            removescratch,          {.ui = 0 } },
	{ MODKEY|Ctrl|Shift,            XK_e,            removescratch,          {.ui = 1 } },
	{ MODKEY|Ctrl|Shift,            XK_r,            removescratch,          {.ui = 2 } },
	{ MODKEY,                       XK_f,            togglefullscreen,       {0} },
	{ MODKEY|Shift,                 XK_f,            togglefakefullscreen,   {0} },
	{ MODKEY,                       XK_0,            view,                   {.ui = ~SPTAGMASK } },
	{ MODKEY|Shift,                 XK_0,            tag,                    {.ui = ~SPTAGMASK } },
	{ MODKEY|Shift,                 XK_comma,        focusmon,               {.i = -1 } },
	{ MODKEY|Shift,                 XK_period,       focusmon,               {.i = +1 } },
	{ MODKEY|Alt,                   XK_comma,        tagmon,                 {.i = -1 } },
	{ MODKEY|Alt,                   XK_period,       tagmon,                 {.i = +1 } },
	{ MODKEY|Alt|Shift,             XK_comma,        tagallmon,              {.i = +1 } },
	{ MODKEY|Alt|Shift,             XK_period,       tagallmon,              {.i = -1 } },
	{ MODKEY|Alt|Ctrl,              XK_comma,        tagswapmon,             {.i = +1 } },
	{ MODKEY|Alt|Ctrl,              XK_period,       tagswapmon,             {.i = -1 } },
	{ MODKEY,                       XK_n,            togglealttag,           {0} },
	{ MODKEY,                       XK_Left,         focusdir,               {.i = 0 } }, // left
	{ MODKEY,                       XK_Right,        focusdir,               {.i = 1 } }, // right
	{ MODKEY,                       XK_Up,           focusdir,               {.i = 2 } }, // up
	{ MODKEY,                       XK_Down,         focusdir,               {.i = 3 } }, // down

	/* Unassigned key bidnings (available externally via the dawnc command) */
//	{ MODKEY,                       XK_,             incrgaps,               {.i = +1 } },
//	{ MODKEY,                       XK_,             incrgaps,               {.i = -1 } },
//	{ MODKEY,                       XK_,             incrigaps,              {.i = +1 } },
//	{ MODKEY,                       XK_,             incrigaps,              {.i = -1 } },
//	{ MODKEY,                       XK_,             incrogaps,              {.i = +1 } },
//	{ MODKEY,                       XK_,             incrogaps,              {.i = -1 } },
//	{ MODKEY,                       XK_,             incrihgaps,             {.i = +1 } },
//	{ MODKEY,                       XK_,             incrihgaps,             {.i = -1 } },
//	{ MODKEY,                       XK_,             incrivgaps,             {.i = +1 } },
//	{ MODKEY,                       XK_,             incrivgaps,             {.i = -1 } },
//	{ MODKEY,                       XK_,             incrohgaps,             {.i = +1 } },
//	{ MODKEY,                       XK_,             incrohgaps,             {.i = -1 } },
//	{ MODKEY,                       XK_,             incrovgaps,             {.i = +1 } },
//	{ MODKEY,                       XK_,             incrovgaps,             {.i = -1 } },
//	{ MODKEY,                       XK_,             togglegaps,             {0} },
//	{ MODKEY,                       XK_,             defaultgaps,            {0} },
//	{ MODKEY,                       XK_,             cyclelayout,            {.i = -1 } },
//	{ MODKEY,                       XK_,             cyclelayout,            {.i = +1 } },
//	{ MODKEY,                       XK_,             shiftview,              { .i = -1 } },
//	{ MODKEY,                       XK_,             shiftview,              { .i = +1 } },
//	{ MODKEY,                       XK_,             focusmaster,           {0} },
//	{ MODKEY,                       XK_,             tagallmon,              {.i = +1 } },
//	{ MODKEY,                       XK_,             tagallmon,              {.i = -1 } },
//	{ MODKEY,                       XK_,             tagswapmon,             {.i = +1 } },
//	{ MODKEY,                       XK_,             tagswapmon,             {.i = -1 } },
//	{ MODKEY,                       XK_,             transfer,               {0} },
//	{ MODKEY,                       XK_,             transferall,            {0} },
//	{ MODKEY,                       XK_,             togglesticky,           {0} },
//	{ MODKEY,                       XK_,             focusurgent,            {0} },
//	{ MODKEY,                       XK_,             inplacerotate,          {.i = +1} },
//	{ MODKEY,                       XK_,             inplacerotate,          {.i = -1} },
//	{ MODKEY,                       XK_,             rotatestack,            {.i = +1 } },
//	{ MODKEY,                       XK_,             rotatestack,            {.i = -1 } },
//	{ MODKEY,                       XK_,             unfloatvisible,         {0} },
//	{ MODKEY,                       XK_,             switchcol,              {0} },
//	{ MODKEY,                       XK_,             tagall,                 {.v = "F3"} },
//	{ MODKEY,                       XK_,             tagall,                 {.v = "4"} },
//	{ MODKEY,                       XK_,             setlayout,              {.v = &layouts[0]} },
//	{ MODKEY,                       XK_,             xrdb,                   {0 } },
//	{ MODKEY,                       XK_,             viewtoleft,             {0} },
//	{ MODKEY,                       XK_,             viewtoright,            {0} },
//	{ MODKEY,                       XK_,             tagtoleft,              {0} },
//	{ MODKEY,                       XK_,             tagtoright,             {0} },
//	{ MODKEY,                       XK_,             tagandviewtoleft,       {0} },
//	{ MODKEY,                       XK_,             tagandviewtoright,      {0} },
//  // taggrid - change tags by direction, e.g. left, right, up, down
//	{ MODKEY,                       XK_,             taggridmovetag,         { .ui = TAGGRID_UP    | TAGGRID_VIEW } },
//	{ MODKEY,                       XK_,             taggridmovetag,         { .ui = TAGGRID_DOWN  | TAGGRID_VIEW } },
//	{ MODKEY,                       XK_,             taggridmovetag,         { .ui = TAGGRID_RIGHT | TAGGRID_VIEW } },
//	{ MODKEY,                       XK_,             taggridmovetag,         { .ui = TAGGRID_LEFT  | TAGGRID_VIEW } },
//	{ MODKEY,                       XK_,             taggridmovetag,         { .ui = TAGGRID_UP    | TAGGRID_TAG | TAGGRID_VIEW } },
//	{ MODKEY,                       XK_,             taggridmovetag,         { .ui = TAGGRID_DOWN  | TAGGRID_TAG | TAGGRID_VIEW } },
//	{ MODKEY,                       XK_,             taggridmovetag,         { .ui = TAGGRID_RIGHT | TAGGRID_TAG | TAGGRID_VIEW } },
//	{ MODKEY,                       XK_,             taggridmovetag,         { .ui = TAGGRID_LEFT  | TAGGRID_TAG | TAGGRID_VIEW } },

	TAGKEYS(                        XK_1,                                    0)
	TAGKEYS(                        XK_2,                                    1)
	TAGKEYS(                        XK_3,                                    2)
	TAGKEYS(                        XK_4,                                    3)
	TAGKEYS(                        XK_5,                                    4)
	TAGKEYS(                        XK_6,                                    5)
	TAGKEYS(                        XK_7,                                    6)
	TAGKEYS(                        XK_8,                                    7)
	TAGKEYS(                        XK_9,                                    8)
};

/* button definitions */
/* click can be ClkButton, ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static Button buttons[] = {
	/* click                     event mask               button          function        argument */
	{ ClkButton,                 0,                       Button1,        spawn,          {.v = termcmd } },
	{ ClkLtSymbol,               0,                       Button1,        setlayout,      {0} },
	{ ClkLtSymbol,               0,                       Button4,        cyclelayout,    {.i = +1 } },
	{ ClkLtSymbol,               0,                       Button5,        cyclelayout,    {.i = -1 } },
	{ ClkWinTitle,               0,                       Button1,        focuswin,       {0} },
	{ ClkWinTitle,               0,                       Button3,        showhideclient, {0} },
	{ ClkWinTitle,               0,                       Button2,        zoom,           {0} },
	{ ClkStatusText,             0,                       Button1,        sigdwmblocks,   {.i = 1 } },
	{ ClkStatusText,             0,                       Button2,        sigdwmblocks,   {.i = 2 } },
	{ ClkStatusText,             0,                       Button3,        sigdwmblocks,   {.i = 3 } },
	{ ClkStatusText,             0,                       Button4,        sigdwmblocks,   {.i = 4 } },
	{ ClkStatusText,             0,                       Button5,        sigdwmblocks,   {.i = 5 } },
	{ ClkStatusText,             0,                       Button6,        sigdwmblocks,   {.i = 6 } },
	{ ClkStatusText,             0,                       Button7,        sigdwmblocks,   {.i = 7 } },
	{ ClkStatusText,             0,                       Button8,        sigdwmblocks,   {.i = 8 } },
	{ ClkStatusText,             0,                       Button9,        sigdwmblocks,   {.i = 9 } },
	{ ClkStatusText,             Shift,                   Button1,        sigdwmblocks,   {.i = 10 } },
	{ ClkStatusText,             Shift,                   Button2,        sigdwmblocks,   {.i = 11 } },
	{ ClkStatusText,             Shift,                   Button3,        sigdwmblocks,   {.i = 12 } },
	{ ClkClientWin,              MODKEY,                  Button1,        movemouse,      {0} },
	{ ClkClientWin,              MODKEY|Alt,              Button2,        togglefloating, {0} },
	{ ClkClientWin,              MODKEY,                  Button3,        resizemouse,    {0} },
	{ ClkClientWin,              0,                       Button8,        movemouse,      {0} },
	{ ClkClientWin,              0,                       Button9,        resizemouse,    {0} },
	{ ClkClientWin,              MODKEY,                  Button2,        zoom,           {0} },
	{ ClkClientWin,              MODKEY|Shift,            Button3,        dragcfact,      {0} },
	{ ClkClientWin,              MODKEY|Shift,            Button1,        dragmfact,      {0} },
	{ ClkRootWin,                MODKEY|Shift,            Button1,        dragmfact,      {0} },
	{ ClkClientWin,              MODKEY|Shift,            Button4,        rotatestack,    {.i = +1 } },
	{ ClkClientWin,              MODKEY|Shift,            Button5,        rotatestack,    {.i = -1 } },
	{ ClkClientWin,              MODKEY,                  Button4,        inplacerotate,  {.i = +1 } },
	{ ClkClientWin,              MODKEY,                  Button5,        inplacerotate,  {.i = -1 } },
	{ ClkClientWin,              MODKEY|Alt,              Button4,        cyclelayout,    {.i = -1 } },
	{ ClkClientWin,              MODKEY|Alt,              Button5,        cyclelayout,    {.i = +1 } },
	{ ClkTagBar,                 0,                       Button1,        view,           {0} },
	{ ClkTagBar,                 0,                       Button3,        toggleview,     {0} },
	{ ClkTagBar,                 MODKEY,                  Button1,        tag,            {0} },
	{ ClkTagBar,                 MODKEY,                  Button3,        toggletag,      {0} },
};

static const char *ipcsockpath = "/tmp/dawn.sock";
static IPCCommand ipccommands[] = {
	IPCCOMMAND( cyclelayout, 1, {ARG_TYPE_SINT} ),
	IPCCOMMAND( defaultgaps, 1, {ARG_TYPE_NONE} ),
	IPCCOMMAND( enable, 1, {ARG_TYPE_UINT} ),
	IPCCOMMAND( disable, 1, {ARG_TYPE_UINT} ),
	IPCCOMMAND( floatpos, 1, {ARG_TYPE_STR} ),
	IPCCOMMAND( focusmaster, 1, {ARG_TYPE_NONE} ),
	IPCCOMMAND( focusmon, 1, {ARG_TYPE_SINT} ),
	IPCCOMMAND( focusstack, 1, {ARG_TYPE_SINT} ),
	IPCCOMMAND( focusurgent, 1, {ARG_TYPE_NONE} ),
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
	IPCCOMMAND( mirrorlayout, 1, {ARG_TYPE_NONE} ),
	IPCCOMMAND( rotatelayoutaxis, 1, {ARG_TYPE_SINT} ),
	IPCCOMMAND( rotatestack, 1, {ARG_TYPE_SINT} ),
	IPCCOMMAND( pushdown, 1, {ARG_TYPE_NONE} ),
	IPCCOMMAND( pushup, 1, {ARG_TYPE_NONE} ),
	IPCCOMMAND( quit, 1, {ARG_TYPE_SINT} ), // 0 = quit, 1 = restart
	IPCCOMMAND( setborderpx, 1, {ARG_TYPE_SINT} ),
	IPCCOMMAND( setlayoutaxisex, 1, {ARG_TYPE_SINT} ),
	IPCCOMMAND( setlayoutex, 1, {ARG_TYPE_SINT} ),
	IPCCOMMAND( setlayoutsafe, 1, {ARG_TYPE_PTR} ),
	IPCCOMMAND( setcfact, 1, {ARG_TYPE_FLOAT} ),
	IPCCOMMAND( setmfact, 1, {ARG_TYPE_FLOAT} ),
	IPCCOMMAND( setgapsex, 1, {ARG_TYPE_SINT} ),
	IPCCOMMAND( setstatus, 1, {ARG_TYPE_STR} ),
	IPCCOMMAND( shiftview, 1, {ARG_TYPE_SINT} ),
	IPCCOMMAND( shiftviewclients, 1, {ARG_TYPE_SINT} ),
	IPCCOMMAND( stackpush, 1, {ARG_TYPE_SINT} ),
	IPCCOMMAND( stackfocus, 1, {ARG_TYPE_SINT} ),
	IPCCOMMAND( switchcol, 1, {ARG_TYPE_NONE} ),
	IPCCOMMAND( tag, 1, {ARG_TYPE_UINT} ),
	IPCCOMMAND( tagall, 1, {ARG_TYPE_STR} ), // e.g. "4" or "F4" to only move floating windows to tag 4
	IPCCOMMAND( tagallmon, 1, {ARG_TYPE_SINT} ),
	IPCCOMMAND( taggridmovetag, 1, {ARG_TYPE_UINT} ),
	IPCCOMMAND( tagmon, 1, {ARG_TYPE_UINT} ),
	IPCCOMMAND( tagnextmonex, 1, {ARG_TYPE_UINT} ),
	IPCCOMMAND( tagprevmonex, 1, {ARG_TYPE_UINT} ),
	IPCCOMMAND( tagswapmon, 1, {ARG_TYPE_SINT} ),
	IPCCOMMAND( tagtoleft, 1, {ARG_TYPE_NONE} ),
	IPCCOMMAND( tagtoright, 1, {ARG_TYPE_NONE} ),
	IPCCOMMAND( tagandviewtoleft, 1, {ARG_TYPE_NONE} ),
	IPCCOMMAND( tagandviewtoright, 1, {ARG_TYPE_NONE} ),
	IPCCOMMAND( toggle, 1, {ARG_TYPE_UINT} ), // toggle functionality on and off
	IPCCOMMAND( togglealttag, 1, {ARG_TYPE_NONE} ),
	IPCCOMMAND( togglebar, 1, {ARG_TYPE_NONE} ),
	IPCCOMMAND( togglefakefullscreen, 1, {ARG_TYPE_NONE} ),
	IPCCOMMAND( togglefloating, 1, {ARG_TYPE_NONE} ),
	IPCCOMMAND( togglefullscreen, 1, {ARG_TYPE_NONE} ),
	IPCCOMMAND( togglegaps, 1, {ARG_TYPE_NONE} ),
	IPCCOMMAND( togglescratch, 1, {ARG_TYPE_UINT} ),
	IPCCOMMAND( togglesticky, 1, {ARG_TYPE_NONE} ),
	IPCCOMMAND( toggletag, 1, {ARG_TYPE_UINT} ),
	IPCCOMMAND( toggleview, 1, {ARG_TYPE_UINT} ),
	IPCCOMMAND( transfer, 1, {ARG_TYPE_NONE} ),
	IPCCOMMAND( transferall, 1, {ARG_TYPE_NONE} ),
	IPCCOMMAND( unfloatvisible, 1, {ARG_TYPE_NONE} ),
	IPCCOMMAND( view, 1, {ARG_TYPE_UINT} ),
	IPCCOMMAND( viewtoleft, 1, {ARG_TYPE_NONE} ),
	IPCCOMMAND( viewtoright, 1, {ARG_TYPE_NONE} ),
	IPCCOMMAND( winview, 1, {ARG_TYPE_NONE} ),
	IPCCOMMAND( xrdb, 1, {ARG_TYPE_NONE} ), // reload xrdb / Xresources
	IPCCOMMAND( zoom, 1, {ARG_TYPE_NONE} ),
};
