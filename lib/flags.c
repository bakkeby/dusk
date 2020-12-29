// int
// hadflag(Client *c, const int flag)
// {
// 	return c->prevflags & (1 << flag);
// }

// int
// hasflag(Client *c, const int flag)
// {
// 	return c->flags & (1 << flag);
// }

// void
// addflag(Client *c, const int flag)
// {
// 	long binflag = 1 << flag;
// 	c->prevflags = (c->prevflags ^ binflag) | (c->flags & binflag);
// 	c->flags |= binflag;
// }

// void
// setflag(Client *c, const int flag, const int value)
// {
// 	value ? addflag(c, flag) : removeflag(c, flag);
// }

// void
// setflags(Client *c, const long flags)
// {
// 	c->prevflags = c->flags;
// 	c->flags = flags;
// }

// void
// removeflag(Client *c, const int flag)
// {
// 	long binflag = 1 << flag;
// 	c->prevflags = (c->prevflags ^ binflag) | (c->flags & binflag);
// 	c->flags ^= binflag;
// }

// int
// hadflag(Client *c, const unsigned int flag)
// {
// 	return c->prevflags & flag;
// }

// int
// hasflag(Client *c, const unsigned int flag)
// {
// 	return c->flags & flag;
// }

void
addflag(Client *c, const unsigned long flag)
{
	// if (ISLOCKED(c))
	// 	return;
	c->prevflags = (c->prevflags & ~flag) | (c->flags & flag);
	c->flags |= flag;


}

void
setflag(Client *c, const unsigned long flag, const int value)
{
	value ? addflag(c, flag) : removeflag(c, flag);
}

// void
// setflags(Client *c, const unsigned int flags)
// {
// 	c->prevflags = c->flags;
// 	c->flags = flags;
// }

void
removeflag(Client *c, const unsigned long flag)
{
	// if (ISLOCKED(c) && flag != Locked)
	// 	return;

	c->prevflags = (c->prevflags & ~flag) | (c->flags & flag);
	c->flags &= ~flag;
}