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

#ifndef __EP_CALLBACK_H__
#define __EP_CALLBACK_H__

#include "list.h"

struct ep_task_queue_s;

typedef struct ep_callback_s ep_callback_t;

struct ep_callback_s {
  list_head_t list;
  void (*fn)(void *);
  void *arg;
};

#define ep_call(cb) ((cb)->fn((cb)->arg))

#define ep_callback_init(cb, fn, arg) \
  do { \
    INIT_LIST_HEAD(&(cb)->list); \
    (cb)->fn = (fn); \
    (cb)->arg = (arg); \
  } while (0)

#endif //!__EP_CALLBACK_H__
