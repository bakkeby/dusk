#include "yajl_dumps.h"

#include <stdint.h>

int
dump_settings(yajl_gen gen)
{
  // clang-format off
  YMAP(
    YSTR("Functionality"); YMAP(
      YSTR("SmartGaps"); YBOOL(enabled(SmartGaps));
      YSTR("SmartGapsMonocle"); YBOOL(enabled(SmartGapsMonocle));
      YSTR("Systray"); YBOOL(enabled(Systray));
      YSTR("Swallow"); YBOOL(enabled(Swallow));
      YSTR("SwallowFloating"); YBOOL(enabled(SwallowFloating));
      YSTR("CenteredWindowName"); YBOOL(enabled(CenteredWindowName));
      YSTR("BarActiveGroupBorderColor"); YBOOL(enabled(BarActiveGroupBorderColor));
      YSTR("SpawnCwd"); YBOOL(enabled(SpawnCwd));
      YSTR("ColorEmoji"); YBOOL(enabled(ColorEmoji));
      YSTR("Status2DNoAlpha"); YBOOL(enabled(Status2DNoAlpha));
      YSTR("BarBorder"); YBOOL(enabled(BarBorder));
      YSTR("NoBorders"); YBOOL(enabled(NoBorders));
      YSTR("Warp"); YBOOL(enabled(Warp));
      YSTR("FocusedOnTop"); YBOOL(enabled(FocusedOnTop));
      YSTR("DecorationHints"); YBOOL(enabled(DecorationHints));
      YSTR("FocusOnNetActive"); YBOOL(enabled(FocusOnNetActive));
      YSTR("AllowNoModifierButtons"); YBOOL(enabled(AllowNoModifierButtons));
      YSTR("CenterSizeHintsClients"); YBOOL(enabled(CenterSizeHintsClients));
      YSTR("ResizeHints"); YBOOL(enabled(ResizeHints));
      YSTR("SortScreens"); YBOOL(enabled(SortScreens));
      YSTR("ViewOnWs"); YBOOL(enabled(ViewOnWs));
      YSTR("Xresources"); YBOOL(enabled(Xresources));
      YSTR("Debug"); YBOOL(enabled(Debug));
      YSTR("AltWorkspaceIcons"); YBOOL(enabled(AltWorkspaceIcons));
      YSTR("GreedyMonitor"); YBOOL(enabled(GreedyMonitor));
      YSTR("AutoHideScratchpads"); YBOOL(enabled(AutoHideScratchpads));
    )
  )
  // clang-format on

  return 0;
}

int
dump_workspace(yajl_gen gen, const char *name, const int mon, const int visible, const int pinned, const int num_clients)
{
  // clang-format off
  YMAP(
    YSTR("monitor"); YINT(mon);
    YSTR("visible"); YINT(visible);
    YSTR("pinned"); YINT(pinned);
    YSTR("clients"); YINT(num_clients);
    YSTR("name"); YSTR(name);
  )
  // clang-format on

  return 0;
}

int
dump_workspaces(yajl_gen gen)
{
  Workspace *ws;
  Client *c;
  int num_clients;
  // clang-format off
  YARR(
    for (ws = workspaces; ws; ws = ws->next) {
      for (num_clients = 0, c = ws->clients; c; c = c->next, ++num_clients);
      dump_workspace(gen, ws->name, ws->mon->num, ws->visible, ws->pinned, num_clients);
    }
  )
  // clang-format on

  return 0;
}

int
dump_client(yajl_gen gen, Client *c)
{
  // clang-format off
  YMAP(
    YSTR("name"); YSTR(c->name);
    YSTR("window_id"); YINT(c->win);
    YSTR("workspace"); YSTR(c->ws->name);
    YSTR("monitor_number"); YINT(c->ws->mon->num);

    YSTR("geometry"); YMAP(
      YSTR("current"); YMAP (
        YSTR("x"); YINT(c->x);
        YSTR("y"); YINT(c->y);
        YSTR("width"); YINT(c->w);
        YSTR("height"); YINT(c->h);
      )
      YSTR("old"); YMAP(
        YSTR("x"); YINT(c->oldx);
        YSTR("y"); YINT(c->oldy);
        YSTR("width"); YINT(c->oldw);
        YSTR("height"); YINT(c->oldh);
      )
    )

    YSTR("size_hints"); YMAP(
      YSTR("base"); YMAP(
        YSTR("width"); YINT(c->basew);
        YSTR("height"); YINT(c->baseh);
      )
      YSTR("step"); YMAP(
        YSTR("width"); YINT(c->incw);
        YSTR("height"); YINT(c->inch);
      )
      YSTR("max"); YMAP(
        YSTR("width"); YINT(c->maxw);
        YSTR("height"); YINT(c->maxh);
      )
      YSTR("min"); YMAP(
        YSTR("width"); YINT(c->minw);
        YSTR("height"); YINT(c->minh);
      )
      YSTR("aspect_ratio"); YMAP(
        YSTR("min"); YDOUBLE(c->mina);
        YSTR("max"); YDOUBLE(c->maxa);
      )
    )

    YSTR("border_width"); YMAP(
      YSTR("current"); YINT(c->bw);
      YSTR("old"); YINT(c->oldbw);
    )

    YSTR("flags"); YMAP(
      YSTR("AlwaysOnTop"); YBOOL(ALWAYSONTOP(c));
      YSTR("Floating"); YBOOL(ISFLOATING(c));
      YSTR("Fixed"); YBOOL(ISFIXED(c));
      YSTR("Locked"); YBOOL(ISLOCKED(c));
      YSTR("Sticky"); YBOOL(ISSTICKY(c));
      YSTR("Centered"); YBOOL(ISCENTERED(c));
      YSTR("FullScreen"); YBOOL(ISFULLSCREEN(c));
      YSTR("FakeFullScreen"); YBOOL(ISFAKEFULLSCREEN(c));
      YSTR("Permanent"); YBOOL(ISPERMANENT(c));
      YSTR("Terminal"); YBOOL(ISTERMINAL(c));
      YSTR("Transient"); YBOOL(ISTRANSIENT(c));
      YSTR("Urgent"); YBOOL(ISURGENT(c));
      YSTR("Visible"); YBOOL(ISVISIBLE(c));
      YSTR("IgnoreCfgReq"); YBOOL(IGNORECFGREQ(c));
      YSTR("IgnoreCfgReqPos"); YBOOL(IGNORECFGREQPOS(c));
      YSTR("IgnoreCfgReqSize"); YBOOL(IGNORECFGREQSIZE(c));
      YSTR("IgnorePropTransientFor"); YBOOL(IGNOREPROPTRANSIENTFOR(c));
      YSTR("IgnoreSizeHints"); YBOOL(IGNORESIZEHINTS(c));
      YSTR("IgnoreDecorationHints"); YBOOL(IGNOREDECORATIONHINTS(c));
      YSTR("Marked"); YBOOL(ISMARKED(c));
      YSTR("NeedResize"); YBOOL(NEEDRESIZE(c));
      YSTR("NeverFocus"); YBOOL(NEVERFOCUS(c));
      YSTR("NoBorder"); YBOOL(NOBORDER(c));
      YSTR("NoSwallow"); YBOOL(NOSWALLOW(c));
      YSTR("OnlyModButtons"); YBOOL(ONLYMODBUTTONS(c));
      YSTR("RestoreFakeFullScreen"); YBOOL(RESTOREFAKEFULLSCREEN(c));
      YSTR("Ruled"); YBOOL(RULED(c));
      YSTR("SwitchWorkspace"); YBOOL(SWITCHWORKSPACE(c));
      YSTR("EnableWorkspace"); YBOOL(ENABLEWORKSPACE(c));
      YSTR("RevertWorkspace"); YBOOL(REVERTWORKSPACE(c));
      YSTR("MoveResize"); YBOOL(MOVERESIZE(c));
      YSTR("WasFloating"); YBOOL(WASFLOATING(c));
      YSTR("WasFakeFullscreen"); YBOOL(WASFAKEFULLSCREEN(c));
      YSTR("WasFullscreen"); YBOOL(WASFULLSCREEN(c));
      YSTR("WasFullscreen"); YBOOL(WASFULLSCREEN(c));
    )
  )
  // clang-format on

  return 0;
}

int
dump_monitor(yajl_gen gen, Monitor *mon, int is_selected)
{
  Workspace *ws = mon->selws;

  // clang-format off
  if (!ws) {
    YMAP(
      YSTR("num"); YINT(mon->num);
      YSTR("is_selected"); YBOOL(is_selected);

      YSTR("monitor_geometry"); YMAP(
        YSTR("x"); YINT(mon->mx);
        YSTR("y"); YINT(mon->my);
        YSTR("width"); YINT(mon->mw);
        YSTR("height"); YINT(mon->mh);
      )

      YSTR("window_geometry"); YMAP(
        YSTR("x"); YINT(mon->wx);
        YSTR("y"); YINT(mon->wy);
        YSTR("width"); YINT(mon->ww);
        YSTR("height"); YINT(mon->wh);
      )

      YSTR("workspace"); YMAP(
        YSTR("current"); YSTR("NULL");
      )

      YSTR("bar"); YMAP(
        YSTR("y"); YINT(mon->bar->by);
        YSTR("is_shown"); YBOOL(mon->showbar);
        YSTR("is_top"); YBOOL(mon->bar->topbar);
        YSTR("window_id"); YINT(mon->bar->win);
      )
    )

    return 0;
  }

  YMAP(
    YSTR("master_factor"); YDOUBLE(ws->mfact);
    YSTR("num_master"); YINT(ws->nmaster);
    YSTR("num"); YINT(mon->num);
    YSTR("is_selected"); YBOOL(is_selected);

    YSTR("monitor_geometry"); YMAP(
      YSTR("x"); YINT(mon->mx);
      YSTR("y"); YINT(mon->my);
      YSTR("width"); YINT(mon->mw);
      YSTR("height"); YINT(mon->mh);
    )

    YSTR("window_geometry"); YMAP(
      YSTR("x"); YINT(mon->wx);
      YSTR("y"); YINT(mon->wy);
      YSTR("width"); YINT(mon->ww);
      YSTR("height"); YINT(mon->wh);
    )

    YSTR("workspace"); YMAP(
      YSTR("current"); YSTR(ws->name);
    )

    YSTR("clients"); YMAP(
      YSTR("selected"); YINT(ws->sel ? ws->sel->win : 0);
      YSTR("stack"); YARR(
        for (Client* c = ws->stack; c; c = c->snext)
          YINT(c->win);
      )
      YSTR("all"); YARR(
        for (Client* c = ws->clients; c; c = c->next)
          YINT(c->win);
      )
    )

    YSTR("layout"); YMAP(
      YSTR("symbol"); YMAP(
        YSTR("current"); YSTR(ws->ltsymbol);
        YSTR("old"); YSTR(ws->prevlayout->symbol);
      )
      YSTR("address"); YMAP(
        YSTR("current"); YINT((uintptr_t)ws->layout);
        YSTR("old"); YINT((uintptr_t)ws->prevlayout);
      )
    )

    YSTR("bar"); YMAP(
      YSTR("y"); YINT(mon->bar->by);
      YSTR("is_shown"); YBOOL(mon->showbar);
      YSTR("is_top"); YBOOL(mon->bar->topbar);
      YSTR("window_id"); YINT(mon->bar->win);
    )
  )
  // clang-format on

  return 0;
}

int
dump_monitors(yajl_gen gen, Monitor *mons, Monitor *selmon)
{
  // clang-format off
  YARR(
    for (Monitor *mon = mons; mon; mon = mon->next) {
      if (mon == selmon)
        dump_monitor(gen, mon, 1);
      else
        dump_monitor(gen, mon, 0);
    }
  )
  // clang-format on

  return 0;
}

int
dump_layouts(yajl_gen gen, const Layout layouts[], const int layouts_len)
{
  // clang-format off
  YARR(
    for (int i = 0; i < layouts_len; i++) {
      YMAP(
        // Check for a NULL pointer. The cycle layouts patch adds an entry at
        // the end of the layouts array with a NULL pointer for the symbol
        YSTR("symbol"); YSTR((layouts[i].symbol ? layouts[i].symbol : ""));
        YSTR("address"); YINT((uintptr_t)(layouts + i));
      )
    }
  )
  // clang-format on

  return 0;
}

int
dump_client_focus_change_event(yajl_gen gen, Client *old_client,
                               Client *new_client, int mon_num)
{
  // clang-format off
  YMAP(
    YSTR("client_focus_change_event"); YMAP(
      YSTR("monitor_number"); YINT(mon_num);
      YSTR("old_win_id"); old_client == NULL ? YNULL() : YINT(old_client->win);
      YSTR("new_win_id"); new_client == NULL ? YNULL() : YINT(new_client->win);
    )
  )
  // clang-format on

  return 0;
}

int
dump_layout_change_event(yajl_gen gen, const int mon_num,
                         const char *old_symbol, const Layout *old_layout,
                         const char *new_symbol, const Layout *new_layout)
{
  // clang-format off
  YMAP(
    YSTR("layout_change_event"); YMAP(
      YSTR("monitor_number"); YINT(mon_num);
      YSTR("old_symbol"); YSTR(old_symbol);
      YSTR("old_address"); YINT((uintptr_t)old_layout);
      YSTR("new_symbol"); YSTR(new_symbol);
      YSTR("new_address"); YINT((uintptr_t)new_layout);
    )
  )
  // clang-format on

  return 0;
}

int
dump_monitor_focus_change_event(yajl_gen gen, const int last_mon_num,
                                const int new_mon_num)
{
  // clang-format off
  YMAP(
    YSTR("monitor_focus_change_event"); YMAP(
      YSTR("old_monitor_number"); YINT(last_mon_num);
      YSTR("new_monitor_number"); YINT(new_mon_num);
    )
  )
  // clang-format on

  return 0;
}

int
dump_focused_title_change_event(yajl_gen gen, const int mon_num,
                                const Window client_id, const char *old_name,
                                const char *new_name)
{
  // clang-format off
  YMAP(
    YSTR("focused_title_change_event"); YMAP(
      YSTR("monitor_number"); YINT(mon_num);
      YSTR("client_window_id"); YINT(client_id);
      YSTR("old_name"); YSTR(old_name);
      YSTR("new_name"); YSTR(new_name);
    )
  )
  // clang-format on

  return 0;
}

int
dump_client_state(yajl_gen gen, const ClientState *state)
{
  // clang-format off
  YMAP(
    YSTR("old_state"); YBOOL(state->oldstate);
    YSTR("is_fixed"); YBOOL(state->isfixed);
    YSTR("is_floating"); YBOOL(state->isfloating);
    YSTR("is_fullscreen"); YBOOL(state->isfullscreen);
    YSTR("is_urgent"); YBOOL(state->isurgent);
    YSTR("never_focus"); YBOOL(state->neverfocus);
  )
  // clang-format on

  return 0;
}

int
dump_focused_state_change_event(yajl_gen gen, const int mon_num,
                                const Window client_id,
                                const ClientState *old_state,
                                const ClientState *new_state)
{
  // clang-format off
  YMAP(
    YSTR("focused_state_change_event"); YMAP(
      YSTR("monitor_number"); YINT(mon_num);
      YSTR("client_window_id"); YINT(client_id);
      YSTR("old_state"); dump_client_state(gen, old_state);
      YSTR("new_state"); dump_client_state(gen, new_state);
    )
  )
  // clang-format on

  return 0;
}

int
dump_error_message(yajl_gen gen, const char *reason)
{
  // clang-format off
  YMAP(
    YSTR("result"); YSTR("error");
    YSTR("reason"); YSTR(reason);
  )
  // clang-format on

  return 0;
}
