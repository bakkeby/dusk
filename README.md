![https://github.com/bakkeby/dusk/wiki/dusk.jpg](https://github.com/bakkeby/dusk/wiki/dusk.jpg)

---

Please ignore. This is just my personal build of what was once dwm.

If you are looking for dwm then you may want to check out
[dwm-flexipatch](https://github.com/bakkeby/dwm-flexipatch/) or just build your own
[from scratch](https://dwm.suckless.org/).

---

![https://raw.githubusercontent.com/wiki/bakkeby/dwm-vanitygaps/dawn_scr01.png](https://raw.githubusercontent.com/wiki/bakkeby/dwm-vanitygaps/dawn_scr01.png)

---

That said if you do decide to try this build out then familiarity with dwm or another tiling window
manager is not strictly needed, but it would be highly recommended to at least have some basic
understanding of how window managers work in general and what your desired workflow is before diving
in.

### Features

There are too many to list them all, but here are some features where dusk differentiates itself
from both [dwm](https://dwm.suckless.org/) as well as
[dwm-flexipatch](https://github.com/bakkeby/dwm-flexipatch/) (and perhaps a few other tiling window
managers):

- it uses workspaces instead of tags where the workspace can be freely moved between monitors or be
  pinned to designated monitors if so desired
- flexible dynamic layouts
- seamless restarts (hint: applications stay on their designated workspaces rather than being
  jumbled up on the first workspace)
- comprehensive multi-monitor support (e.g. special features for portrait vs landscape monitors)
- intuitive mouse support (e.g. rearrange or resize tiled windows using the mouse)
- highly configurable bar(s)
- extensive EWMH support (which ultimately gives applications and external tools more control)
- feature rich scratchpads that can be assigned on demand
- asynchronous status updates with full click support (each status block is updated individually)

For a more comprehensive list refer to the [wiki](https://github.com/bakkeby/dusk/wiki).

### Non-features

There are some things that are simply out of scope for this project. Here are a few ideas or topics
that there are no plans to integrate:

- manual tiling - while technically not that complicated the general workflow of dynamic tiling is
  just so much faster and ideally the end user should not have to think about where to place the
  next window. If manual tiling is your thing then perhaps refer to herbstluftwm or i3.
- polybar - no additional support or integration for external bars such as polybar or lemonbar is
  planned
- dwmblocks, slstatus, xsetroot, etc. - all these status updater programs for dwm rely on setting
  the X root name to update the status which is not supported here - instead each status block /
  segment is updated individually and asynchronously from other blocks via the external dusk client
- plain text configuration file - while possible this would make things a lot more complicated and
  ultimately give the user less configuration options, so re-compilation will still be necessary to
  apply configuration changes

### Support groups

If dusk gave you a feeling of loneliness, distress, depression, anxiety or mental fatigue then you
may want to seek help in one of these support groups:

   - [r/duskwm](https://www.reddit.com/r/duskwm/)
   - [Discord](https://discord.gg/DMA7BDc4RA)
