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

#ifndef __EP_TIMER_H__
#define __EP_TIMER_H__

#include <stdint.h>
#include <stdlib.h>
#include <sys/time.h>

#include "skiplist.h"

typedef void * ep_timer_id_t;

SKIPLIST_DECLARE_TYPE(timer, 8, uint64_t, 0.25)

typedef struct {
  skiplist_t(timer) sl;
  uint64_t now;
} ep_timer_t;

typedef struct {
  skiplist_node_t(timer) n;
  uint64_t ms;
  void (*cb)(void *);
  void *arg;
  ep_timer_t *timer;
} ep_timer_node_t;

#define __ep_timer_node_entry(node) skiplist_entry((node), ep_timer_node_t, n)

static inline void ep_timer_init(ep_timer_t *timer) {
  skiplist_init(timer, &timer->sl);
}

static inline ep_timer_node_t *__ep_timer_node_create() {
  return (ep_timer_node_t *)malloc(sizeof(ep_timer_node_t));
}

static inline void __ep_timer_node_destroy(void *node) {
  free(node);
}

static inline uint64_t ep_timer_updatetime(ep_timer_t *timer) {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return timer->now = tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

static inline uint64_t ep_timer_gettime(ep_timer_t *timer) {
  return timer->now;
}

static inline ep_timer_id_t ep_timer_run_after(ep_timer_t *timer, uint64_t ms, void (*cb)(void *), void *arg) {
  ep_timer_id_t ret = 0;
  ep_timer_node_t *node = __ep_timer_node_create();

  if (node) {
    node->ms = 0;
    skiplist_node_init(timer, &node->n, ep_timer_gettime(timer) + ms);
    node->cb = cb;
    node->arg = arg;
    node->timer = timer;
    skiplist_insert(timer, &timer->sl, &node->n);
    ret = node;
  }

  return ret;
}

static inline ep_timer_id_t ep_timer_run_every(ep_timer_t *timer, uint64_t ms, void (*cb)(void *), void *arg) {
  ep_timer_id_t ret = NULL;
  ep_timer_node_t *node = __ep_timer_node_create();

  if (node) {
    node->ms = ms;
    skiplist_node_init(timer, &node->n, ep_timer_gettime(timer) + ms);
    node->cb = cb;
    node->arg = arg;
    node->timer = timer;
    skiplist_insert(timer, &timer->sl, &node->n);
    ret = node;
  }

  return ret;
}

static inline void ep_timer_execute(ep_timer_t *timer, uint64_t now, int max_times) {
  skiplist_node_t(timer) *node, *n;

  skiplist_shift_lte(timer, &timer->sl, now, node, n) {
    ep_timer_node_t *timer_node = __ep_timer_node_entry(node);
    timer_node->cb(timer_node->arg);

    if (timer_node->ms > 0) {
      timer_node->n.score += timer_node->ms;
      skiplist_insert(timer, &timer->sl, &timer_node->n);
    } else {
      __ep_timer_node_destroy(timer_node);
    }

    if (max_times >= 0 && --max_times <= 0) {
      break;
    }
  }
}

static inline int ep_timer_nearest_diff(ep_timer_t *timer) {
  uint64_t ret;

  if (timer->sl.header.forward[0]) {
    uint64_t now = ep_timer_gettime(timer);
    uint64_t nearest = timer->sl.header.forward[0]->score;
    ret = nearest > now ? nearest - now : 0;
  } else {
    ret = -1;
  }

  return ret;
}

static inline void ep_timer_clear(ep_timer_id_t id) {
  ep_timer_node_t *node = (ep_timer_node_t *)id;
  ep_timer_t *timer = node->timer;
  skiplist_delete(timer, &timer->sl, id);
  __ep_timer_node_destroy(node);
}

static inline void ep_timer_fini(ep_timer_t *timer) {
  skiplist_node_t(timer) *n1;
  skiplist_node_t(timer) *n2;
  skiplist_for_each_clear(n1, n2, &timer->sl, __ep_timer_node_destroy);
}

static inline ep_timer_id_t ep_timer_settimeout(ep_timer_t *timer, uint64_t ms, void (*cb)(void *), void *arg) {
  ep_timer_updatetime(timer);
  return ep_timer_run_after(timer, ms, cb, arg);
}

static inline ep_timer_id_t ep_timer_setinterval(ep_timer_t *timer, uint64_t ms, void (*cb)(void *), void *arg) {
  ep_timer_updatetime(timer);
  return ep_timer_run_every(timer, ms, cb, arg);
}

static inline void ep_timer_cleartimeout(ep_timer_id_t id) {
  ep_timer_clear(id);
}

static inline void ep_timer_clearinterval(ep_timer_id_t id) {
  ep_timer_clear(id);
}

static inline void ep_timer_execute_now(ep_timer_t *timer, int limit) {
  ep_timer_execute(timer, ep_timer_updatetime(timer), limit);
}

#endif //!__EP_TIMER_H__
