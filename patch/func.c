void
enable(const Arg *arg)
{
	settings |= arg->ui;
	reload();
}

int
enabled(const long functionality)
{
	return settings & functionality;
}

void
disable(const Arg *arg)
{
	settings &= ~arg->ui;
	reload();
}

int
disabled(const long functionality)
{
	return !(settings & functionality);
}

void
reload()
{
	arrange(NULL);
	grabkeys();
}