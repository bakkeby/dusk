int
size_config_error(Bar *bar, BarArg *a)
{
	return (config_error ? a->w : 0);
}

int
draw_config_error(Bar *bar, BarArg *a)
{
	return drw_2dtext(drw, a->x, a->y, a->w, a->h, lrpad / 2, config_error, 0, 1, a->scheme);
}
