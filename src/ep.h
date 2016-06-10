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

#ifndef __EP_H__
#define __EP_H__

#include "ep_task_queue.h"
#include "ep_timer.h"
#include "ep_pollapi.h"

#include <errno.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

typedef struct {
  ep_pollstate_t ps;
  ep_task_queue_t nexttick;
  ep_task_queue_t everytick;
  ep_timer_t timer;
} ep_state_t;

static inline int ep_init(ep_state_t *state) {
  int ret = 0;

  if (ep_pollapi_init(&state->ps) < 0) {
    ret = -1;
  }

  ep_task_queue_init(&state->nexttick);
  ep_task_queue_init(&state->everytick);
  ep_timer_init(&state->timer);

  return ret;
}

static inline int ep_fini(ep_state_t *state) {
  ep_pollapi_fini(&state->ps);
  ep_timer_fini(&state->timer);
  ep_task_queue_fini(&state->nexttick);
  ep_task_queue_fini(&state->everytick);
  return 0;
}

static inline int ep_add(ep_state_t *state, int fd, int events, ep_data_t data) {
  return ep_pollapi_add(&state->ps, fd, events, data);
}

static inline int ep_mod(ep_state_t *state, int fd, int events, ep_data_t data) {
  return ep_pollapi_mod(&state->ps, fd, events, data);
}

static inline int ep_del(ep_state_t *state, int fd) {
  return ep_pollapi_del(&state->ps, fd);
}

static inline int __ep_waittime(ep_timer_t *timer, int hint) {
  ep_timer_updatetime(timer);
  int waittime = ep_timer_nearest_diff(timer);

  if (waittime < 0) {
    waittime = hint;
  } else {
    waittime = (hint < 0 || waittime < hint) ? waittime : hint;
  }

  return waittime;
}


static inline int ep_iterate(ep_state_t *state, int waittime, int *events, ep_data_t *data) {
  int ret = ep_pollapi_fetch(&state->ps, events, data);

  if (ret <= 0) {
    ep_task_queue_run(&state->nexttick);
    ep_task_queue_run_keep(&state->everytick);
    ep_timer_execute_now(&state->timer, 128);

    ret = ep_pollapi_wait(&state->ps, __ep_waittime(&state->timer, waittime));

    if (ret > 0) {
      ep_pollapi_fetch(&state->ps, events, data);
    }
  }

  return ret;
}

static inline ep_timer_id_t ep_settimeout(ep_state_t *state, uint64_t ms, void (*cb)(void *), void *arg) {
  ep_timer_updatetime(&state->timer);
  return ep_timer_run_after(&state->timer, ms, cb, arg);
}

static inline ep_timer_id_t ep_setinterval(ep_state_t *state, uint64_t ms, void (*cb)(void *), void *arg) {
  ep_timer_updatetime(&state->timer);
  return ep_timer_run_every(&state->timer, ms, cb, arg);
}

static inline void ep_cleartimeout(ep_timer_id_t id) {
  ep_timer_clear(id);
}

static inline void ep_clearinterval(ep_timer_id_t id) {
  ep_timer_clear(id);
}

static inline ep_task_id_t ep_nexttick(ep_state_t *state, void (*cb)(void *), void *arg) {
  return ep_task_queue_push(&state->nexttick, cb, arg);
}

static inline ep_task_id_t ep_everytick(ep_state_t *state, void (*cb)(void *), void *arg) {
  return ep_task_queue_push(&state->everytick, cb, arg);
}

static inline void ep_cleartask(ep_task_id_t id) {
  ep_task_cancel(id);
}

#endif //!__EP_H__
