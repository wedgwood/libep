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

#ifndef __EP_FCNTL_H__
#define __EP_FCNTL_H__

#include <fcntl.h>

static inline int ep_nonblock(int fd) {
  int flags = fcntl(fd, F_GETFL);

  if (-1 == flags) {
    return -1;
  }

  return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

static inline int ep_block(int fd) {
  int flags = fcntl(fd, F_GETFL);

  if (-1 == flags) {
    return -1;
  }

  return fcntl(fd, F_SETFL, flags & ~O_NONBLOCK);
}

#endif //!__EP_FCNTL_H__
