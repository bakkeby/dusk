static int riodimensions[4] = { -1, -1, -1, -1 };
static Client *rioclient = NULL;
static pid_t riopid = 0;

void
riodraw(const char slopstyle[])
{
	int i;
	char str[100];
	char strout[100];
	char tmpstring[30] = {0};
	char slopcmd[100] = "slop -f x%xx%yx%wx%hx ";
	int firstchar = 0;
	int counter = 0;

	if (!riopid && !rioclient)
		return;

	strcat(slopcmd, slopstyle);
	FILE *fp = popen(slopcmd, "r");

	while (fgets(str, 100, fp) != NULL)
		strcat(strout, str);

	pclose(fp);

	if (strlen(strout) < 6)
		return;

	for (i = 0; i < strlen(strout); i++){
		if (!firstchar) {
			if (strout[i] == 'x')
				firstchar = 1;
			continue;
		}

		if (strout[i] != 'x')
			tmpstring[strlen(tmpstring)] = strout[i];
		else {
			riodimensions[counter] = atoi(tmpstring);
			counter++;
			memset(tmpstring,0,strlen(tmpstring));
		}
	}

	if (rioclient)
		rioposition(rioclient, riodimensions[0], riodimensions[1], riodimensions[2], riodimensions[3]);
}

void
rioposition(Client *c, int x, int y, int w, int h)
{
	Workspace *ws;
	if (w > 50 && h > 50 && x > -40 && y > -40 &&
			(abs(c->w - w) > 20 || abs(c->h - h) > 20 || abs(c->x - x) > 20 || abs(c->y - y) > 20)) {
		if ((ws = recttows(x, y, w, h)) && ws != c->ws) {
			detach(c);
			detachstack(c);
			attachx(c, AttachBottom, ws);
			attachstack(c);
			selws = ws;
			selmon = ws->mon;
			focus(c);
		}

		SETFLOATING(c);
		if (enabled(RioDrawIncludeBorders))
			resizeclient(c, x, y, w - (c->bw * 2), h - (c->bw * 2));
		else
			resizeclient(c, x - c->bw, y - c->bw, w, h);
		drawbar(c->ws->mon);
		arrange(c->ws);
	} else
		fprintf(stderr, "rioposition: refusing to position client %s at x (%d), y (%d), w (%d), h (%d)\n", c->name, x, y, w, h);

	riodimensions[3] = -1;
	rioclient = NULL;
	riopid = 0;
}

/* drag out an area using slop and resize the selected window to it */
void
rioresize(const Arg *arg)
{
	rioclient = CLIENT;
	riodraw(slopresizestyle);
}

/* spawn a new window and drag out an area using slop to postiion it */
void
riospawn(const Arg *arg)
{
	riopid = spawncmd(arg);;
	riodraw(slopspawnstyle);
}
