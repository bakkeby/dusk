#define compareNameToFunc(F) else if (!strcasecmp(name, #F)) return F;
#define mapfunc(N,F) else if (!strcasecmp(name, N)) F(NULL);

void
enable(const Arg *arg)
{
	const char *name = arg->v;
	uint64_t f = getfuncbyname(name);

	if (f) {
		enablefunc(f);
		reload(f);
	}
}

void
disable(const Arg *arg)
{
	const char *name = arg->v;
	uint64_t f = getfuncbyname(name);

	if (f) {
		disablefunc(f);
		reload(f);
	}
}

void
toggle(const Arg *arg)
{
	const char *name = arg->v;
	uint64_t f = getfuncbyname(name);

	if (f) {
		togglefunc(f);
		reload(f);
	}
	mapfunc("bar", togglebar)
	mapfunc("barpadding", togglebarpadding)
	mapfunc("compact", togglecompact)
	mapfunc("fakefullscreen", togglefakefullscreen)
	mapfunc("floating", togglefloating)
	mapfunc("fullscreen", togglefullscreen)
	mapfunc("gaps", togglegaps)
	mapfunc("mark", togglemark)
	mapfunc("nomodbuttons", togglenomodbuttons)
	mapfunc("pinnedws", togglepinnedws)
	mapfunc("sticky", togglesticky)
}

void
reload(const uint64_t functionality)
{
	Workspace *ws;
	Client *c;
	int func_enabled = enabled(functionality);

	/* If the NoBorders functionality was disabled, then loop through and force resize all clients
	 * that previously had the NoBorder flag set in order to restore borders. */
	if (!func_enabled && functionality == NoBorders) {
		for (ws = workspaces; ws; ws = ws->next) {
			for (c = ws->clients; c; c = c->next) {
				if (WASNOBORDER(c)) {
					restoreborder(c);
				}
			}
		}
	}

	arrange(NULL);
	grabkeys();
}

const uint64_t
getfuncbyname(const char *name)
{
	int i;

	if (!name)
		return 0;

	for (i = 0; functionality_names[i].name != NULL; i++) {
		if (strcmp(functionality_names[i].name, name) == 0)
			return functionality_names[i].value;
	}

	return 0;
}

const char *
getnamebyfunc(const uint64_t functionality)
{
	int i;

	for (i = 0; functionality_names[i].name != NULL; i++) {
		if (functionality_names[i].value == functionality)
			return functionality_names[i].name;
	}

	return 0;
}

#undef mapfunc
