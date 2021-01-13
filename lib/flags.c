void
addflag(Client *c, const unsigned long flag)
{
	c->prevflags = (c->prevflags & ~flag) | (c->flags & flag);
	c->flags |= flag;
}

void
setflag(Client *c, const unsigned long flag, const int value)
{
	value ? addflag(c, flag) : removeflag(c, flag);
}

void
removeflag(Client *c, const unsigned long flag)
{
	c->prevflags = (c->prevflags & ~flag) | (c->flags & flag);
	c->flags &= ~flag;
}