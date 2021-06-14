void
enable(const Arg *arg)
{
	enablefunc(getfuncbyname(arg->v));
	reload();
}

void
disable(const Arg *arg)
{
	disablefunc(getfuncbyname(arg->v));
	reload();
}

void
toggle(const Arg *arg)
{
	togglefunc(getfuncbyname(arg->v));
	reload();
}

void
reload()
{
	arrange(NULL);
	grabkeys(NULL);
}
