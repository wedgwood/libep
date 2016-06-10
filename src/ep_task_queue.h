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

#ifndef __EP_TASK_QUEUE_H__
#define __EP_TASK_QUEUE_H__

#include "ep_callback.h"
#include "list.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct ep_task_queue_s ep_task_queue_t;
typedef ep_callback_t * ep_task_id_t;

struct ep_task_queue_s {
  list_head_t list;
};

static inline int ep_task_queue_init(ep_task_queue_t *q) {
  INIT_LIST_HEAD(&q->list);
  return 0;
}

static inline ep_task_id_t ep_task_queue_push(ep_task_queue_t *q, void (*fn)(void *), void *arg) {
  ep_task_id_t ret = NULL;
  ep_callback_t *cb;
  cb = (ep_callback_t *)malloc(sizeof(ep_callback_t));

  if (cb) {
    INIT_LIST_HEAD(&cb->list);
    list_add_tail(&cb->list, &q->list);
    cb->fn  = fn;
    cb->arg = arg;
    ret     = cb;
  }

  return ret;
}

static inline void ep_task_queue_fini(ep_task_queue_t *q) {
  list_head_t *pos, *n;

  list_for_each_safe(pos, n, &q->list) {
    ep_callback_t *cb = list_entry(pos, ep_callback_t, list);
    list_del(pos);
    free(cb);
  }
}

static inline void ep_task_queue_run(ep_task_queue_t *q) {
  list_head_t *pos, *n;

  list_for_each_safe(pos, n, &q->list) {
    list_del(pos);
    ep_callback_t *cb = list_entry(pos, ep_callback_t, list);
    ep_call(cb);
    free(cb);
  }
}

static inline void ep_task_queue_run_keep(ep_task_queue_t *q) {
  list_head_t *pos;

  list_for_each(pos, &q->list) {
    ep_callback_t *cb = list_entry(pos, ep_callback_t, list);
    cb->fn(cb->arg);
  }
}

static inline void ep_task_queue_run_once(ep_task_queue_t *q) {
  if (!list_empty(&q->list)) {
    ep_callback_t *cb = list_entry(q->list.next, ep_callback_t, list);
    list_del(&cb->list);
    ep_call(cb);
    free(cb);
  }
}

static inline void ep_task_cancel(ep_task_id_t id) {
  list_del(&id->list);
  free(id);
}

#endif //!__EP_TASK_QUEUE_H__
