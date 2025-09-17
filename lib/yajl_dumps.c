#include "yajl_dumps.h"

#include <stdint.h>

int
dump_settings(yajl_gen gen)
{
	int i;
	// clang-format off
	YMAP(
		YSTR("Functionality"); YMAP(
			for (i = 0; functionality_names[i].name != NULL; i++) {
				YSTR(functionality_names[i].name); YBOOL(enabled(functionality_names[i].value));
			}
		)
	)
	// clang-format on

	return 0;
}

int
dump_commands(yajl_gen gen)
{
	int i, a;
	IPCCommand c;
	// clang-format off
	YARR(
		for (i = 0; i < LENGTH(ipccommands); i++) {
			c = ipccommands[i];
			YMAP(
				YSTR("command"); YSTR(c.name);
				for (a = 0; a < c.argc; a++) {
					YSTR("argument"); YSTR(dbus_type_to_string(c.args[a]));
				}
			)
		}
	)
	// clang-format on

	return 0;
}

int
dump_bar_height(yajl_gen gen)
{
	// clang-format off
	YINT(bh);
	// clang-format on

	return 0;
}

int
dump_systray_icons(yajl_gen gen)
{
	Client *i;
	// clang-format off
	YARR(
		for (i = systray->icons; i; i = i->next) {
			YMAP(
				YSTR("name"); YSTR(i->name);
				YSTR("win"); YINT(i->win);
			)
		}
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
	int i;

	// clang-format off
	YMAP(
		YSTR("name"); YSTR(NVL(c->name, "NULL"));
		YSTR("alttitle"); YSTR(NVL(c->alttitle, "NULL"));
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
			for (i = 0; flag_names[i].name != NULL; i++) {
				YSTR(flag_names[i].name); YBOOL(c->flags & flag_names[i].value);
			}
		)
	)
	// clang-format on

	return 0;
}

int
dump_monitor(yajl_gen gen, Monitor *mon, int is_selected)
{
	Workspace *ws = mon->selws;
	if (selws->mon == mon && selws == stickyws)
		ws = stickyws;

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

			if (mon->bar) {
				YSTR("bar"); YMAP(
					YSTR("y"); YINT(mon->bar->by);
					YSTR("is_shown"); YBOOL(mon->showbar);
					YSTR("is_vert"); YBOOL(mon->bar->vert);
					YSTR("window_id"); YINT(mon->bar->win);
				)
			}
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
				YSTR("current"); YSTR(NVL(ws->ltsymbol, "NULL"));
				YSTR("old"); YSTR(NVL(ws->prevlayout->symbol, "NULL"));
			)
			YSTR("address"); YMAP(
				YSTR("current"); YINT((uintptr_t)ws->layout);
				YSTR("old"); YINT((uintptr_t)ws->prevlayout);
			)
		)

		if (mon->bar) {
			YSTR("bar"); YMAP(
				YSTR("y"); YINT(mon->bar->by);
				YSTR("is_shown"); YBOOL(mon->showbar);
				YSTR("is_vert"); YBOOL(mon->bar->vert);
				YSTR("window_id"); YINT(mon->bar->win);
			)
		}
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
				YSTR("name"); YSTR((layouts[i].name ? layouts[i].name : layouts[i].symbol ? layouts[i].symbol : ""));
				YSTR("symbol"); YSTR((layouts[i].symbol ? layouts[i].symbol : ""));
				YSTR("index"); YINT(i);
			)
		}
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
