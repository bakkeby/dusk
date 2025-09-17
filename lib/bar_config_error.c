int
size_config_error(Bar *bar, BarArg *a)
{
	int textw = (config_error ? TEXT2DW(config_error) + lrpad : 0);
	return MIN(textw, a->w);
}

int
draw_config_error(Bar *bar, BarArg *a)
{
	return drw_2dtext(drw, a->x, a->y, a->w, a->h, lrpad / 2, config_error, 0, 1, a->scheme);
}
