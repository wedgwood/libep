/* The MIT License

  Copyright © 2016 by Yuan B.J. <wedgwood@qq.com>

  Permission is hereby granted, free of charge, to any person obtaining
  a copy of this software and associated documentation files (the "Software"),
  to deal in the Software without restriction, including without limitation
  the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom the
  Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included
  in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
  OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
  DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
  OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef __EP_SELECT_H__
#define __EP_SELECT_H__

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#ifndef FD_COPY
#define FD_COPY(src, dest) (*(dest) = *(src))
#endif

typedef struct {
  ep_data_t data;
  uint32_t events;
} ep_select_event_t;

typedef struct {
  fd_set fds;
  fd_set rfds;
  fd_set wfds;
  fd_set efds;
  int maxfd;
  int ready;
  ep_data_t *data;
  ep_select_event_t *ee;
} ep_select_state_t;

static inline int ep_select_init(ep_select_state_t *state) {
  ep_data_t *data = (ep_data_t *)calloc(FD_SETSIZE, sizeof(ep_data_t));

  if (!data) {
    return -1;
  }

  ep_select_event_t *ee = (ep_select_event_t *)malloc(sizeof(ep_select_event_t) * FD_SETSIZE);

  if (!ee) {
    free(data);
    return -1;
  }

  FD_ZERO(&state->fds);
  FD_ZERO(&state->rfds);
  FD_ZERO(&state->wfds);
  FD_ZERO(&state->efds);

  state->ready = 0;
  state->data  = data;
  state->ee    = ee;
  state->maxfd = -1;
  return 0;
}

static inline int ep_select_fini(ep_select_state_t *state) {
  free(state->data);
  free(state->ee);
  return 0;
}

static inline int ep_select_add(ep_select_state_t *state, int fd, int events, ep_data_t data) {
  if (fd >= FD_SETSIZE) {
    return -1;
  }

  if (events & EP_READABLE) {
    FD_SET(fd, &state->rfds);
  }

  if (events & EP_WRITABLE) {
    FD_SET(fd, &state->wfds);
  }

  if (events & EP_EXCEPT) {
    FD_SET(fd, &state->efds);
  }

  if (fd > state->maxfd) {
    state->maxfd = fd;
  }

  state->data[fd].u64 = data.u64;
  return 0;
}

static inline int ep_select_mod(ep_select_state_t *state, int fd, int events, ep_data_t data) {
  if (fd >= FD_SETSIZE) {
    return -1;
  }

  if (events & EP_READABLE) {
    FD_SET(fd, &state->rfds);
  } else {
    FD_CLR(fd, &state->rfds);
  }

  if (events & EP_WRITABLE) {
    FD_SET(fd, &state->wfds);
  } else {
    FD_CLR(fd, &state->wfds);
  }

  if (events & EP_EXCEPT) {
    FD_SET(fd, &state->efds);
  } else {
    FD_CLR(fd, &state->efds);
  }

  state->data[fd].u64 = data.u64;
  return 0;
}

static inline int ep_select_del(ep_select_state_t *state, int fd) {
  FD_CLR(fd, &state->fds);
  FD_CLR(fd, &state->rfds);
  FD_CLR(fd, &state->wfds);
  FD_CLR(fd, &state->efds);

  // state->ptr[fd] = NULL;

  if (fd == state->maxfd) {
    int maxfd = fd;

    while (--maxfd >= 0) {
      if (FD_ISSET(maxfd, &state->fds)) {
        break;
      }
    }

    state->maxfd = maxfd;
  }

  return 0;
}

static inline int ep_select_wait(ep_select_state_t *state, int timeout) {
  fd_set rfds;
  fd_set wfds;
  fd_set efds;

  FD_COPY(&state->rfds, &rfds);
  FD_COPY(&state->wfds, &wfds);
  FD_COPY(&state->efds, &efds);

  int ret;

  if (timeout >= 0) {
    struct timeval tv;
    tv.tv_sec = timeout / 1000;
    tv.tv_usec = timeout % 1000 * 1000;
    ret = select(state->maxfd + 1, &rfds, &wfds, &efds, &tv);
  } else {
    ret = select(state->maxfd + 1, &rfds, &wfds, &efds, NULL);
  }

  state->ready = ret;

  int i, j;

  for (i = 0, j = 0; j < ret && i < FD_SETSIZE; ++i) {
    uint32_t events = 0;

    if (FD_ISSET(i, &rfds)) {
      events |= EP_READABLE;
    }

    if (FD_ISSET(i, &wfds)) {
      events |= EP_WRITABLE;
    }

    if (FD_ISSET(i, &efds)) {
      events |= EP_EXCEPT;
    }

    if (events) {
      state->ee[j].events = events;
      state->ee[j].data.u64 = state->data[i].u64;
      ++j;
    }
  }

  return ret;
}

static inline int ep_select_fetch(ep_select_state_t *state, int *events, ep_data_t *data) {
  int ret = 0;

  if (state->ready > 0) {
    ret = state->ready;
    ep_select_event_t *ee = state->ee + --state->ready;

    if (events) {
      *events = ee->events;
    }

    if (data) {
      data->u64 = ee->data.u64;
    }
  }

  return ret;
}

static inline const char *ep_select_name() {
  return "select";
}

#endif //!__EP_SELECT_H__
