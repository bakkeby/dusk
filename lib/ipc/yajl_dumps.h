#ifndef YAJL_DUMPS_H_
#define YAJL_DUMPS_H_

#include <string.h>
#include <yajl/yajl_gen.h>

#define YSTR(str) yajl_gen_string(gen, (unsigned char *)str, strlen(str))
#define YINT(num) yajl_gen_integer(gen, num)
#define YDOUBLE(num) yajl_gen_double(gen, num)
#define YBOOL(v) yajl_gen_bool(gen, v)
#define YNULL() yajl_gen_null(gen)
#define YARR(body)                                                             \
  {                                                                            \
    yajl_gen_array_open(gen);                                                  \
    body;                                                                      \
    yajl_gen_array_close(gen);                                                 \
  }
#define YMAP(body)                                                             \
  {                                                                            \
    yajl_gen_map_open(gen);                                                    \
    body;                                                                      \
    yajl_gen_map_close(gen);                                                   \
  }

int dump_client(yajl_gen gen, Client *c);

int dump_monitor(yajl_gen gen, Monitor *mon, int is_selected);

int dump_monitors(yajl_gen gen, Monitor *mons, Monitor *selmon);

int dump_layouts(yajl_gen gen, const Layout layouts[], const int layouts_len);

int dump_client_state(yajl_gen gen, const ClientState *state);

int dump_error_message(yajl_gen gen, const char *reason);

#endif  // YAJL_DUMPS_H_
