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

#ifndef __EP_EPOLL_H__
#define __EP_EPOLL_H__

#include <stddef.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <unistd.h>

typedef struct {
  int efd;
  int ready;
  int sz;
  struct epoll_event *ee;
} ep_epoll_state_t;

static inline int ep_epoll_init(ep_epoll_state_t *state) {
  const int sz = 128;
  int efd = epoll_create(sz);

  if (efd < 0) {
    return -1;
  }

  struct epoll_event *ee = (struct epoll_event *)calloc(sz * sizeof(struct epoll_event), 1);

  if (!ee) {
    return -1;
  }

  state->ee    = ee;
  state->efd   = efd;
  state->ready = 0;
  state->sz    = sz;
  return 0;
}

static inline int ep_epoll_fini(ep_epoll_state_t *state) {
  close(state->efd);
  free(state->ee);
  return 0;
}

static inline int ep_epoll_add(ep_epoll_state_t *state, int fd, int events, ep_data_t data) {
  struct epoll_event ee;
  ee.events = 0;

  if (events & EP_READABLE) {
    ee.events |= EPOLLIN;
  }

  if (events & EP_WRITABLE) {
    ee.events |= EPOLLOUT;
  }

  if (events & EP_EXCEPT) {
    ee.events |= EPOLLPRI;
  }

  ee.data.u64 = data.u64;
  return epoll_ctl(state->efd, EPOLL_CTL_ADD, fd, &ee);
}

static inline int ep_epoll_mod(ep_epoll_state_t *state, int fd, int events, ep_data_t data) {
  struct epoll_event ee;
  ee.events = 0;

  if (events & EP_READABLE) {
    ee.events |= EPOLLIN;
  }

  if (events & EP_WRITABLE) {
    ee.events |= EPOLLOUT;
  }

  if (events & EP_EXCEPT) {
    ee.events |= EPOLLPRI;
  }

  ee.data.u64 = data.u64;
  return epoll_ctl(state->efd, EPOLL_CTL_MOD, fd, &ee);
}

static inline int ep_epoll_del(ep_epoll_state_t *state, int fd) {
  return epoll_ctl(state->efd, EPOLL_CTL_DEL, fd, NULL);
}

static inline int ep_epoll_wait(ep_epoll_state_t *state, int timeout) {
  return (state->ready = epoll_wait(state->efd, state->ee, state->sz, timeout));
}

static inline int ep_epoll_fetch(ep_epoll_state_t *state, int *events, ep_data_t *data) {
  int ret = 0;

  if (state->ready > 0) {
    ret = state->ready;
    struct epoll_event *ee = state->ee + --state->ready;

    if (events) {
      *events = 0;

      if (ee->events & EPOLLIN || ee->events & EPOLLHUP || ee->events & EPOLLRDHUP) {
        *events |= EP_READABLE;
      }

      if (ee->events & EPOLLOUT) {
        *events |= EP_WRITABLE;
      }

      if (ee->events & EPOLLPRI) {
        *events |= EP_EXCEPT;
      }

      if (ee->events & EPOLLERR) {
        *events |= EP_ERROR;
      }
    }

    if (data) {
      data->u64 = ee->data.u64;
    }
  }

  return ret;
}

static inline const char *ep_epoll_name() {
  return "epoll";
}

#endif //!__EP_EPOLL_H__
