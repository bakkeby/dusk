void
addflag(Client *c, const uint64_t flag)
{
	c->prevflags = (c->prevflags & ~flag) | (c->flags & flag);
	c->flags |= flag;
}

void
setflag(Client *c, const uint64_t flag, const int value)
{
	value ? addflag(c, flag) : removeflag(c, flag);
}

void
removeflag(Client *c, const uint64_t flag)
{
	c->prevflags = (c->prevflags & ~flag) | (c->flags & flag);
	c->flags &= ~flag;
}

void
toggleflag(Client *c, const uint64_t flag)
{
	c->flags ^= flag;
}

void
toggleflagop(Client *c, const uint64_t flag, int op)
{
	switch (op) {
	default:
	case 0: /* _NET_WM_STATE_REMOVE */
		removeflag(c, flag);
		break;
	case 1: /* _NET_WM_STATE_ADD */
		addflag(c, flag);
		break;
	case 2: /* _NET_WM_STATE_TOGGLE */
		toggleflag(c, flag);
		break;
	}
}

void
toggleclientflag(const Arg *arg)
{
	if (!selws->sel)
		return;

	toggleflagop(selws->sel, getflagbyname(arg->v), 2);
	arrangews(selws);
}

const uint64_t
getflagbyname(const char *name)
{
	int i;

	if (!name)
		return 0;

	for (i = 0; flag_names[i].name != NULL; i++) {
		if (strcmp(flag_names[i].name, name) == 0)
			return flag_names[i].value;
	}

	return 0;
}

const char *
getnamebyflag(const uint64_t flag)
{
	int i;

	for (i = 0; flag_names[i].name != NULL; i++) {
		if (flag_names[i].value == flag)
			return flag_names[i].name;
	}

	return 0;
}
