/* Bar functionality */
#include "bar_indicators.c"
#include "bar_tagicons.c"
#include "flags.c"
#include "func.c"
#include "warp.c"


#include "bar_alpha.c"
#include "bar_alternativetags.c"
#include "bar_dwmblocks.c"

#include "combo.c"

#include "bar_ltsymbol.c"
#include "bar_status2d.c"
#include "bar_statusbutton.c"
#include "bar_statuscmd.c"
#include "bar_tags.c"
#include "bar_taggrid.c"
#include "bar_flexwintitle.c"
#include "bar_wintitle_floating.c"
#include "bar_wintitle_hidden.c"
#include "bar_systray.c"
#include "bar_wintitleactions.c"

/* Other patches */
#include "attachx.c"
#include "cfacts.c"
#include "cool_autostart.c"
#include "cyclelayouts.c"
#include "decorationhints.c"
#include "dragcfact.c"
#include "dragmfact.c"
#include "ewmhtags.c"
#include "fakefullscreenclient.c"
#include "floatpos.c"

#if FOCUSADJACENTTAG_PATCH
#include "focusadjacenttag.c"
#endif

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
#if INSETS_PATCH
#include "insets.c"
#endif
#if KEYMODES_PATCH
#include "keymodes.c"
#endif

#include "killunsel.c"
#include "nomodbuttons.c"
#include "pertag.c"

#if PUSH_NO_MASTER_PATCH
#include "push_no_master.c"
#elif PUSH_PATCH
#include "push.c"
#endif
#include "restartsig.c"
#include "rotatestack.c"
#include "scratchpad.c"
#include "setborderpx.c"
#include "shiftview.c"
#include "shiftviewclients.c"

#ifdef XINERAMA
#include "sortscreens.c"
#endif // XINERAMA

#if STACKER_PATCH
#include "stacker.c"
#endif

#include "sticky.c"
#include "swallow.c"
#include "swaptags.c"
#include "switchcol.c"
#include "tagall.c"
#include "tagallmon.c"
#include "tagothermonitor.c"
#include "tagswapmon.c"
#include "togglefullscreen.c"
#include "transfer.c"
#include "transferall.c"
#include "unfloatvisible.c"
#include "vanitygaps.c"
#include "winview.c"
#include "zoomswap.c"

#if XRDB_PATCH
#include "xrdb.c"
#endif

/* Layouts */
#include "layout_flextile-deluxe.c"
