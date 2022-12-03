#include <X11/Xlib-xcb.h>
#include <xcb/res.h>
#ifdef __OpenBSD__
#include <sys/sysctl.h>
#include <kvm.h>
#endif /* __OpenBSD__ */

static int scanner;
static xcb_connection_t *xcon;

void
swallow(const Arg *arg)
{
	Client *c, *next, *last, *s = CLIENT;

	if (!s)
		return;

	for (c = nextmarked(NULL, s); c; c = nextmarked(next, NULL)) {
		next = c->next;
		if (c != s) {
			detach(c);
			detachstack(c);
			for (last = c->swallowing; last && last->swallowing; last = last->swallowing);
			if (last)
				last->swallowing = s->swallowing;
			else
				c->swallowing = s->swallowing;
			s->swallowing = c;
			XMoveWindow(dpy, c->win, c->x, HEIGHT(c) * -2);
		}
	}
	arrange(NULL);
}

int
swallowclient(Client *t, Client *c)
{
	if (disabled(Swallow) || NOSWALLOW(c))
		return 0;
	if (!RULED(c) && disabled(SwallowFloating) && ISFLOATING(c))
		return 0;

	if (ISFULLSCREEN(t))
		setfullscreen(c, 1, ISFAKEFULLSCREEN(t));

	replaceclient(t, c);
	XMoveWindow(dpy, t->win, t->x, HEIGHT(t) * -2);
	addflag(c, IgnoreCfgReqPos);
	c->swallowing = t;
	c->revertws = NULL;

	return 1;
}

int
replaceclient(Client *old, Client *new)
{
	if (!old || !new || old == new)
		return 0;

	Client *c = NULL;
	Workspace *ws = old->ws;

	new->ws = ws;
	setflag(new, Floating, old->flags & Floating);

	new->scratchkey = old->scratchkey;
	old->scratchkey = 0;

	new->next = old->next;
	new->snext = old->snext;

	if (old == ws->clients)
		ws->clients = new;
	else {
		for (c = ws->clients; c && c->next != old; c = c->next);
		c->next = new;
	}

	if (old == ws->stack)
		ws->stack = new;
	else {
		for (c = ws->stack; c && c->snext != old; c = c->snext);
		c->snext = new;
	}

	old->next = NULL;
	old->snext = NULL;

	if (ISVISIBLE(new) && !ISFULLSCREEN(new)) {
		if (ISFLOATING(new))
			resize(new, old->x, old->y, new->w, new->h, 0);
		else
			resize(new, old->x, old->y, old->w, old->h, 0);
	}

	return 1;
}

void
unswallow(const Arg *arg)
{
	Client *c = CLIENT, *s;

	if (!c || !c->swallowing)
		return;

	s = c->swallowing;
	if (c && replaceclient(c, s)) {
		c->swallowing = s->swallowing;
		s->swallowing = NULL;
		attach(c);
		attachstack(c);
		focus(c);
		arrange(c->ws);
	}
}

pid_t
winpid(Window w)
{
	pid_t result = 0;

	#ifdef __linux__
	xcb_res_client_id_spec_t spec = {0};
	spec.client = w;
	spec.mask = XCB_RES_CLIENT_ID_MASK_LOCAL_CLIENT_PID;

	xcb_generic_error_t *e = NULL;
	xcb_res_query_client_ids_cookie_t c = xcb_res_query_client_ids(xcon, 1, &spec);
	xcb_res_query_client_ids_reply_t *r = xcb_res_query_client_ids_reply(xcon, c, &e);

	if (!r)
		return (pid_t)0;

	xcb_res_client_id_value_iterator_t i = xcb_res_query_client_ids_ids_iterator(r);
	for (; i.rem; xcb_res_client_id_value_next(&i)) {
		spec = i.data->spec;
		if (spec.mask & XCB_RES_CLIENT_ID_MASK_LOCAL_CLIENT_PID) {
			uint32_t *t = xcb_res_client_id_value_value(i.data);
			result = *t;
			break;
		}
	}

	free(r);

	if (result == (pid_t)-1)
		result = 0;

	#endif /* __linux__ */
	#ifdef __OpenBSD__
	Atom type;
	int format;
	unsigned long len, bytes;
	unsigned char *prop;
	pid_t ret;

	if (XGetWindowProperty(dpy, w, XInternAtom(dpy, "_NET_WM_PID", 1), 0, 1, False, AnyPropertyType, &type, &format, &len, &bytes, &prop) != Success || !prop)
		return 0;

	ret = *(pid_t*)prop;
	XFree(prop);
	result = ret;
	#endif /* __OpenBSD__ */

	return result;
}

pid_t
getparentprocess(pid_t p)
{
	unsigned int v = 0;

#ifdef __linux__
	FILE *f;
	char buf[256] = {0};
	snprintf(buf, sizeof(buf) - 1, "/proc/%u/stat", (unsigned)p);

	if (!(f = fopen(buf, "r")))
		return (pid_t)0;

	if (fscanf(f, "%*u %*s %*c %u", (unsigned *)&v) != 1)
		v = (pid_t)0;
	fclose(f);
#endif /* __linux__ */
#ifdef __OpenBSD__
	int n;
	kvm_t *kd;
	struct kinfo_proc *kp;

	kd = kvm_openfiles(NULL, NULL, NULL, KVM_NO_FILES, NULL);
	if (!kd)
		return 0;

	kp = kvm_getprocs(kd, KERN_PROC_PID, p, sizeof(*kp), &n);
	v = kp->p_ppid;
#endif /* __OpenBSD__ */
	return (pid_t)v;
}

int
isdescprocess(pid_t p, pid_t c)
{
	while (p != c && c != 0)
		c = getparentprocess(c);

	return (int)c;
}

Client *
termforwin(const Client *w)
{
	Workspace *ws;
	Client *c;
	char key = w->swallowkey;

	if (!w->pid)
		return NULL;

	c = selws->sel;
	if (c && ISTERMINAL(c) && ((key && c->scratchkey == key) || (c->pid && isdescprocess(c->pid, w->pid))))
		return c;

	for (ws = workspaces; ws; ws = ws->next)
		for (c = ws->stack; c; c = c->snext)
			if (ISTERMINAL(c) && ((key && c->scratchkey == key) || (c->pid && isdescprocess(c->pid, w->pid))))
				return c;

	return NULL;
}

Client *
swallowingparent(Window w)
{
	Workspace *ws;
	Client *c, *next;

	for (ws = workspaces; ws; ws = ws->next) {
		for (c = ws->clients; c; c = next) {
			next = c->next;
			while (c->swallowing) {
				if (c->swallowing->win == w)
					return c;
				c = c->swallowing;
			}
		}
	}

	return NULL;
}

Client *
swallowingroot(Window w)
{
	Workspace *ws;
	Client *c, *s, *next;

	for (ws = workspaces; ws; ws = ws->next) {
		for (c = ws->clients; c; c = next) {
			next = c->next;
			s = c;
			while (s->swallowing) {
				if (s->swallowing->win == w)
					return c;
				s = s->swallowing;
			}
		}
	}

	return NULL;
}
