/* Bar functionality */
#include "bar_indicators.c"
#include "bar_tagicons.c"
#include "flags.c"
#include "func.c"
#include "warp.c"


#include "bar_alpha.c"
#if BAR_ALTERNATIVE_TAGS_PATCH
#include "bar_alternativetags.c"
#endif
#include "bar_dwmblocks.c"
#if BAR_EWMHTAGS_PATCH
#include "bar_ewmhtags.c"
#endif

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

#if DECORATION_HINTS_PATCH
#include "decorationhints.c"
#endif

#include "dragcfact.c"
#include "dragmfact.c"
#include "fakefullscreenclient.c"
#include "floatpos.c"

#if FOCUSADJACENTTAG_PATCH
#include "focusadjacenttag.c"
#endif

#include "focusmaster.c"

#if FOCUSURGENT_PATCH
#include "focusurgent.c"
#endif
#if INPLACEROTATE_PATCH
#include "inplacerotate.c"
#endif
#if IPC_PATCH
#include "ipc.c"
#ifdef VERSION
#include "ipc/IPCClient.c"
#include "ipc/yajl_dumps.c"
#include "ipc/ipc.c"
#include "ipc/util.c"
#endif
#endif // IPC_PATCH
#if INSETS_PATCH
#include "insets.c"
#endif
#if KEYMODES_PATCH
#include "keymodes.c"
#endif

#include "killunsel.c"

#if MOVESTACK_PATCH
#include "movestack.c"
#endif
#if NO_MOD_BUTTONS_PATCH
#include "nomodbuttons.c"
#endif

#include "pertag.c"

#if PUSH_NO_MASTER_PATCH
#include "push_no_master.c"
#elif PUSH_PATCH
#include "push.c"
#endif
#if REORGANIZETAGS_PATCH
#include "reorganizetags.c"
#endif
#if RESTARTSIG_PATCH
#include "restartsig.c"
#endif
#if ROTATESTACK_PATCH
#include "rotatestack.c"
#endif
#if ROUNDED_CORNERS_PATCH
#include "roundedcorners.c"
#endif

#include "scratchpad.c"
#include "setborderpx.c"
#include "shiftview.c"
#include "shiftviewclients.c"

#if SORTSCREENS_PATCH
#ifdef XINERAMA
#include "sortscreens.c"
#endif // XINERAMA
#endif
#if STACKER_PATCH
#include "stacker.c"
#endif

#include "sticky.c"
#include "swallow.c"

#if SWAPFOCUS_PATCH
#include "swapfocus.c"
#endif

#include "swaptags.c"

#if SWITCHCOL_PATCH
#include "switchcol.c"
#endif
#if TAGALL_PATCH
#include "tagall.c"
#endif

#include "tagallmon.c"

#if TAGOTHERMONITOR_PATCH
#include "tagothermonitor.c"
#endif

#include "tagswapmon.c"
#include "togglefullscreen.c"
#include "transfer.c"
#include "transferall.c"

#if UNFLOATVISIBLE_PATCH
#include "unfloatvisible.c"
#endif

#include "vanitygaps.c"
#include "winview.c"
#include "zoomswap.c"

#if XRDB_PATCH
#include "xrdb.c"
#endif

/* Layouts */
#include "layout_flextile-deluxe.c"
