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

#ifndef __EP_KQUEUE_H__
#define __EP_KQUEUE_H__

#include <errno.h>
#include <stdlib.h>
#include <sys/event.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

typedef struct {
  int efd;
  int ready;
  int sz;
  struct kevent *ee;
} ep_kqueue_state_t;

static inline int ep_kqueue_init(ep_kqueue_state_t *state) {
  const int sz = 128;
  int efd = kqueue();

  if (efd < 0) {
    return -1;
  }

  struct kevent *ee = (struct kevent *)calloc(sz * sizeof(struct kevent), 1);

  if (!ee) {
    return -1;
  }

  state->ee    = ee;
  state->efd   = efd;
  state->ready = 0;
  state->sz    = sz;
  return 0;
}

static inline int ep_kqueue_fini(ep_kqueue_state_t *state) {
  close(state->efd);
  free(state->ee);
  return 0;
}

static inline int ep_kqueue_add(ep_kqueue_state_t *state, int fd, int events, ep_data_t data) {
  struct kevent ev;

  if (events & EP_READABLE) {
    EV_SET(&ev, fd, EVFILT_READ, EV_ADD, 0, 0, data.ptr);

    if (kevent(state->efd, &ev, 1, NULL, 0, NULL) == -1) {
      return -1;
    }
  }

  if (events & EP_WRITABLE) {
    EV_SET(&ev, fd, EVFILT_WRITE, EV_ADD, 0, 0, data.ptr);

    if (kevent(state->efd, &ev, 1, NULL, 0, NULL) == -1) {
      return -1;
    }
  }

  return 0;
}

static inline int ep_kqueue_mod(ep_kqueue_state_t *state, int fd, int events, ep_data_t data) {
  struct kevent ev;

  if (events & EP_READABLE) {
    EV_SET(&ev, fd, EVFILT_READ, EV_ADD, 0, 0, data.ptr);
  } else {
    EV_SET(&ev, fd, EVFILT_READ, EV_DISABLE, 0, 0, data.ptr);
  }

  if (kevent(state->efd, &ev, 1, NULL, 0, NULL) == -1) {
    return -1;
  }

  if (events & EP_WRITABLE) {
    EV_SET(&ev, fd, EVFILT_WRITE, EV_ADD, 0, 0, data.ptr);
  } else {
    EV_SET(&ev, fd, EVFILT_WRITE, EV_DISABLE, 0, 0, data.ptr);
  }

  if (kevent(state->efd, &ev, 1, NULL, 0, data.ptr) == -1) {
    return -1;
  }

  return 0;
}

static inline int ep_kqueue_del(ep_kqueue_state_t *state, int fd) {
  struct kevent ev;
  EV_SET(&ev, fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);

  if (kevent(state->efd, &ev, 1, NULL, 0, NULL) == -1) {
    return -1;
  }

  EV_SET(&ev, fd, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);

  if (kevent(state->efd, &ev, 1, NULL, 0, NULL) == -1) {
    return -1;
  }

  return 0;
}

static inline int ep_kqueue_wait(ep_kqueue_state_t *state, int timeout) {
  int ret;

  if (timeout >= 0) {
    struct timespec t;
    t.tv_sec = timeout / 1000;
    t.tv_nsec = timeout % 1000 * 1000000;
    ret = kevent(state->efd, NULL, 0, state->ee, state->sz, &t);
  } else {
    ret = kevent(state->efd, NULL, 0, state->ee, state->sz, NULL);
  }

  state->ready = ret;
  return ret;
}

static inline int ep_kqueue_fetch(ep_kqueue_state_t *state, int *events, ep_data_t *data) {
  int ret = 0;

  if (state->ready > 0) {
    ret = state->ready;
    struct kevent *ee = state->ee + --state->ready;

    if (events) {
      *events = 0;

      if (ee->filter == EVFILT_READ || ee->flags & EV_EOF) {
        *events |= EP_READABLE;
      } else if (ee->filter == EVFILT_WRITE) {
        *events |= EP_WRITABLE;
      } else if (ee->filter == EV_OOBAND) {
        *events |= EP_EXCEPT;
      }

      if (ee->flags & EV_ERROR) {
        *events |= EP_ERROR;
      }
    }

    if (data) {
      data->ptr = ee->udata;
    }
  }

  return ret;
}

static inline const char *ep_kqueue_name() {
  return "kqueue";
}

#endif //!__EP_KQUEUE_H__
