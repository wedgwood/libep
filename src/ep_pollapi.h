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

#ifndef __EP_API_H__
#define __EP_API_H__

#include "platform.h"

#include <stdint.h>
#include <string.h>

#define EP_NONE      0u
#define EP_READABLE  1u
#define EP_WRITABLE  (1u << 1)
#define EP_EXCEPT    (1u << 2)
#define EP_ERROR     (1u << 3)

typedef union {
  void *ptr;
  int fd;
  uint32_t u32;
  uint64_t u64;
} ep_data_t;

#include "ep_select.h"

#ifdef USE_SELECT
  #define __default_ep_engine select
#else
  #ifdef EP_HAVE_EPOLL
    #include "ep_epoll.h"
    #define __default_ep_engine epoll
  #else
    #ifdef EP_HAVE_KQUEUE
      #include "ep_kqueue.h"
      #define __default_ep_engine kqueue
    #else
      #define __default_ep_engine select
    #endif
  #endif
#endif

#define ep_pollapi_define(engine, api) __ep_pollapi_define(engine, api)
#define __ep_pollapi_define(engine, api) ep_##engine##_##api

#define ep_pollstate_define(engine) __ep_pollstate_define(engine)
#define __ep_pollstate_define(engine)  ep_##engine##_state_t

typedef ep_pollstate_define(__default_ep_engine) ep_pollstate_t;

#define ep_pollapi_init    ep_pollapi_define(__default_ep_engine, init)
#define ep_pollapi_fini    ep_pollapi_define(__default_ep_engine, fini)
#define ep_pollapi_add     ep_pollapi_define(__default_ep_engine, add)
#define ep_pollapi_mod     ep_pollapi_define(__default_ep_engine, mod)
#define ep_pollapi_del     ep_pollapi_define(__default_ep_engine, del)
#define ep_pollapi_wait    ep_pollapi_define(__default_ep_engine, wait)
#define ep_pollapi_fetch   ep_pollapi_define(__default_ep_engine, fetch)
#define ep_pollapi_name    ep_pollapi_define(__default_ep_engine, name)
#define ep_pollapi_ready   ep_pollapi_define(__default_ep_engine, ready)

#endif //!__EP_API_H__
