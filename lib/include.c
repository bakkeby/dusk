/* Bar functionality */
#include "bar_indicators.c"
#include "flags.c"
#include "func.c"
#include "warp.c"
#include "workspace.c"
// #include "combo.c"

#include "bar_alpha.c"
#include "bar_dwmblocks.c"
#include "bar_ltsymbol.c"
#include "bar_status2d.c"
#include "bar_statusbutton.c"
#include "bar_statuscmd.c"
#include "bar_flexwintitle.c"
#include "bar_wintitle_floating.c"
#include "bar_wintitle_hidden.c"
#include "bar_systray.c"
#include "bar_wintitleactions.c"
#include "bar_workspaces.c"

/* Other patches */
#include "attachx.c"
#include "cfacts.c"
#include "clientopacity.c"
#include "cool_autostart.c"
#include "cyclelayouts.c"
#include "decorationhints.c"
#include "dragcfact.c"
#include "dragmfact.c"
#include "ewmhtags.c"
#include "fakefullscreenclient.c"
#include "floatpos.c"
#include "focusdir.c"
#include "focusmaster.c"
#include "focusurgent.c"
#include "inplacerotate.c"
#include "ipc.c"
#ifdef VERSION
#include "ipc/IPCClient.c"
#include "ipc/yajl_dumps.c"
#include "ipc/ipc.c"
#include "ipc/util.c"
#endif
#include "killunsel.c"
#include "nomodbuttons.c"
#include "push.c"
#include "renamed_scratchpads.c"
#include "restartsig.c"
#include "rotatestack.c"
#include "savefloats.c"
#include "setborderpx.c"
#include "shiftview.c"
#include "shiftviewclients.c"
#ifdef XINERAMA
#include "sortscreens.c"
#endif // XINERAMA
#include "stacker.c"
#include "sticky.c"
#include "swallow.c"
#include "swaptags.c"
#include "switchcol.c"
#include "togglefullscreen.c"
#include "transfer.c"
#include "transferall.c"
#include "unfloatvisible.c"
#include "vanitygaps.c"
#include "zoomswap.c"
#include "xrdb.c"

/* Layouts */
#include "layout_flextile-deluxe.c"
