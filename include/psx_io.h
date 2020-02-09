//psx_io.h

#include "types.h"

#define MAXFDS  128

int fflib_init();

int fflib_attach(int idx, u64 id, int now);

int fflib_detach(int idx, u64 id);

u64 fflib_id_get(int idx);

int fflib_fd_get(int idx);

int fflib_ss_get(int idx);

int fflib_fd_set(int idx, int fd);

int fflib_ss_set(int idx, int ss);
