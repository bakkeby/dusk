void
enable(const Arg *arg)
{
	// settings |= arg->ui;
	enablefunc(arg->ui);
	reload();
}

void
disable(const Arg *arg)
{
	disablefunc(arg->ui);
	reload();
}

void
toggle(const Arg *arg)
{
	if (disabled(arg->ui))
		enable(arg);
	else
		disable(arg);
}

void
reload()
{
	arrange(NULL);
	grabkeys();
}