char *
tagicon(Monitor *m, int tag)
{
	Client *c;
	char *icon;
	int tagindex = tag + NUMTAGS * m->num;
	if (tagindex >= LENGTH(tagicons[DEFAULT_TAGS]))
		tagindex = tagindex % LENGTH(tagicons[DEFAULT_TAGS]);
	icon = tagicons[m->alttag ? ALTERNATIVE_TAGS : DEFAULT_TAGS][tagindex];
	for (c = m->clients; c && (!(c->tags & 1 << tag) || HIDDEN(c)); c = c->next);
	if (c)
		icon = tagicons[OCCUPIED_TAGS][tagindex];
	else if (TEXTW(icon) <= lrpad && m->tagset[m->seltags] & 1 << tag)
		icon = tagicons[VACANT_TAGS][tagindex];
	return icon;
}