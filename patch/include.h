/* Bar functionality */
#include "bar_indicators.h"
#include "bar_tagicons.h"
#include "flags.h"
#include "func.h"
#include "warp.h"

#include "bar_alpha.h"
#include "bar_alternativetags.h"
#include "bar_dwmblocks.h"
#include "combo.h"

#include "bar_ltsymbol.h"
#include "bar_status2d.h"
#include "bar_statusbutton.h"
#include "bar_statuscmd.h"
#include "bar_tags.h"
#include "bar_taggrid.h"
#include "bar_flexwintitle.h"
#include "bar_wintitle_floating.h"
#include "bar_wintitle_hidden.h"
#include "bar_systray.h"
#include "bar_wintitleactions.h"

/* Other patches */
#include "attachx.h"
#include "cfacts.h"
#include "cool_autostart.h"
#include "cyclelayouts.h"
#include "decorationhints.h"
#include "dragcfact.h"
#include "dragmfact.h"
#include "ewmhtags.h"
#include "fakefullscreenclient.h"
#include "floatpos.h"

#if FOCUSADJACENTTAG_PATCH
#include "focusadjacenttag.h"
#endif

#include "focusmaster.h"
#include "focusurgent.h"
#include "inplacerotate.h"
#include "ipc.h"
#include "ipc/ipc.h"
#include "ipc/util.h"

#if INSETS_PATCH
#include "insets.h"
#endif
#if KEYMODES_PATCH
#include "keymodes.h"
#endif

#include "killunsel.h"
#include "nomodbuttons.h"
#include "pertag.h"
#include "push.h"
#include "restartsig.h"
#include "rotatestack.h"
#include "scratchpad.h"
#include "setborderpx.h"
#include "shiftview.h"
#include "shiftviewclients.h"

#ifdef XINERAMA
#include "sortscreens.h"
#endif // XINERAMA

#if STACKER_PATCH
#include "stacker.h"
#endif

#include "sticky.h"
#include "swallow.h"
#include "swaptags.h"
#include "switchcol.h"
#include "tagall.h"
#include "tagallmon.h"
#include "tagothermonitor.h"
#include "tagswapmon.h"
#include "togglefullscreen.h"
#include "transfer.h"
#include "transferall.h"
#include "unfloatvisible.h"
#include "vanitygaps.h"
#include "winview.h"
#include "zoomswap.h"

#if XRDB_PATCH
#include "xrdb.h"
#endif
/* Layouts */

#include "layout_flextile-deluxe.h"
