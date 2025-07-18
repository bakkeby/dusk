/* Utilities */
#include "defaults.h"
#include "lookup.c"

/* Bar functionality */
#include "bar_indicators.c"
#include "flags.c"
#include "func.c"
#include "warp.c"
#include "workspace.c"

#include "bar_alpha.c"
#include "bar.c"
#include "bar_ltsymbol.c"
#include "bar_config_error.c"
#include "bar_flexwintitle.c"
#include "bar_status2d.c"
#include "bar_wintitle_floating.c"
#include "bar_wintitle_hidden.c"
#include "bar_wintitle_sticky.c"
#include "bar_wintitle_single.c"
#include "bar_systray.c"
#include "bar_wintitleactions.c"
#include "bar_workspaces.c"
#include "bar_wspreview.c"
#include "bar_powerline.c"

/* Other functionality */
#include "attachx.c"
#include "autostart.c"
#include "cfacts.c"
#include "clientopacity.c"
#include "cyclelayouts.c"
#include "decorationhints.c"
#include "dragcfact.c"
#include "dragmfact.c"
#include "dragwfact.c"
#include "dragfact.c"
#include "ewmh.c"
#include "group.c"
#include "fakefullscreenclient.c"
#include "floatpos.c"
#include "focusdir.c"
#include "placedir.c"
#include "focusmaster.c"
#include "focusurgent.c"
#include "inplacerotate.c"
#include "killunsel.c"
#include "mark.c"
#include "maximize.c"
#include "movemouse.c"
#include "resizemouse.c"
#include "placemouse.c"
#include "swallowmouse.c"
#include "nomodbuttons.c"
#include "push.c"
#include "semi_scratchpads.c"
#include "renamed_scratchpads.c"
#include "restartsig.c"
#include "riodraw.c"
#include "rotatestack.c"
#include "savefloats.c"
#include "setborderpx.c"
#include "settitle.c"
#ifdef XINERAMA
#include "sortscreens.c"
#endif // XINERAMA
#include "stacker.c"
#include "sticky.c"
#include "swallow.c"
#include "switchcol.c"
#include "togglefullscreen.c"
#include "transfer.c"
#include "transferall.c"
#include "unfloatvisible.c"
#include "vanitygaps.c"
#include "winicon.c"
#include "xrdb.c"

#ifdef HAVE_LIBXI
#include "xban.c"
#endif

/* Debug */
#include "x11_debug.c"

/* Layouts */
#include "layout_flextile-deluxe.c"

/* dbus */
#ifdef HAVE_DBUS
#include "dbus_server.c"
#endif
