void
tagall(const Arg *arg)
{
	if (!selws->clients)
		return;
	/* if parameter starts with F, just move floating windows */
	int floating_only = (char *)arg->v && ((char *)arg->v)[0] == 'F' ? 1 : 0;
	int tag = (char *)arg->v ? atoi(((char *)arg->v) + floating_only) : 0;
	int j;
	Client* c;
	if (tag >= 0 && tag < NUMTAGS)
		for (c = selws->clients; c; c = c->next)
		{
			if (!floating_only || ISFLOATING(c))
				for (j = 0; j < NUMTAGS; j++)
				{
					if (c->tags & 1 << j && selmon->tagset[selws->seltags] & 1 << j)
					{
						c->tags = c->tags ^ (1 << j & TAGMASK);
						c->tags = c->tags | 1 << (tag-1);
					}
				}
		}
	arrange(selws);
}