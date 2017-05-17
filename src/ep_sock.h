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

#ifndef __EP_SOCK_H__
#define __EP_SOCK_H__

#include "ep_fcntl.h"

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

static inline int ep_setkeepalive(int fd, int interval) {
  int ret = -1;

  do {
    int val = 1;

    if (setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &val, sizeof(val)) == -1) {
      break;
    }

#ifdef __linux__
    val = interval;

    if (setsockopt(fd, IPPROTO_IP, TCP_KEEPIDLE, &val, sizeof(val)) < 0) {
      break;
    }

    val = interval / 3;

    if (val == 0) {
      val = 1;
    }

    if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPINTVL, &val, sizeof(val)) < 0) {
      break;
    }

    val = 3;

    if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPCNT, &val, sizeof(val)) < 0) {
      break;
    }
#else
    ((void) interval);
#endif

    ret = 0;
  } while (0);

  return ret;
}

static inline int ep_settcpnodelay(int fd, int val) {
  return setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &val, sizeof(val));
}

static inline int ep_getsoerror(int fd, int *out) {
  socklen_t len = sizeof(int);
  return getsockopt(fd, SOL_SOCKET, SO_ERROR, (void *)out, &len);
}

static inline int ep_tcpnodelay_enable(int fd) {
  return ep_settcpnodelay(fd, 1);
}

static inline int ep_tcpnodelay_disable(int fd) {
  return ep_settcpnodelay(fd, 0);
}

static inline int ep_setsendbuffer(int fd, int sz) {
  return setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &sz, sizeof(sz) == -1);
}

static inline int ep_settcpkeepalive(int fd) {
  int val = 1;
  return setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &val, sizeof(val));
}

static inline int ep_genericresolve(const char *host, char *ipbuf, size_t ipbuf_len, int ip_only) {
  struct addrinfo hints, *info;
  int rv;

  bzero(&hints, sizeof(hints));

  if (ip_only) {
    hints.ai_flags = AI_NUMERICHOST;
  }

  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  if ((rv = getaddrinfo(host, NULL, &hints, &info)) != 0) {
    return -1;
  }

  if (info->ai_family == AF_INET) {
    struct sockaddr_in *sa = (struct sockaddr_in *)info->ai_addr;
    inet_ntop(AF_INET, &(sa->sin_addr), ipbuf, ipbuf_len);
  } else {
    struct sockaddr_in6 *sa = (struct sockaddr_in6 *)info->ai_addr;
    inet_ntop(AF_INET6, &(sa->sin6_addr), ipbuf, ipbuf_len);
  }

  freeaddrinfo(info);
  return 0;
}

static inline int ep_resolve(const char *host, char *ipbuf, size_t ipbuf_len) {
  return ep_genericresolve(host, ipbuf, ipbuf_len, 0);
}

static inline int ep_resolveip(const char *host, char *ipbuf, size_t ipbuf_len) {
  return ep_genericresolve(host, ipbuf, ipbuf_len, 1);
}

static inline int ep_reuseaddr_enable(int fd) {
  int on = 1;
  return setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
}

static inline int ep_createsocket(int domain) {
  int ret = -1;

  do {
    int s;

    if ((s = socket(domain, SOCK_STREAM, 0)) == -1) {
      break;
    }

    if (ep_reuseaddr_enable(s) == -1) {
      close(s);
      break;
    }

    ret = s;
  } while (0);

  return ret;
}

static inline int ep_generictcpconnect(const char *host, int port, char *source, int nonblock) {
  int s = -1;
  char portstr[6];
  struct addrinfo hints, *servinfo, *bservinfo, *p, *b;

  snprintf(portstr, sizeof(portstr), "%d", port);
  bzero(&hints, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  int rv;

  if ((rv = getaddrinfo(host, portstr, &hints, &servinfo)) != 0) {
    return -1;
  }

  for (p = servinfo; p != NULL; p = p->ai_next) {
    if ((s = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
      continue;
    }

    if (ep_reuseaddr_enable(s) == -1) {
      goto err;
    }

    if (nonblock && ep_nonblock(s) == -1) {
      goto err;
    }

    if (source) {
      int bound = 0;

      if ((rv = getaddrinfo(source, NULL, &hints, &bservinfo)) != 0) {
        goto err;
      }

      for (b = bservinfo; b != NULL; b = b->ai_next) {
        if (bind(s, b->ai_addr, b->ai_addrlen) != -1) {
          bound = 1;
          break;
        }
      }

      freeaddrinfo(bservinfo);

      if (!bound) {
        goto ret;
      }
    }

    if (connect(s, p->ai_addr, p->ai_addrlen) == -1) {
      if (EINPROGRESS == errno && nonblock) {
        goto ret;
      } else {
        close(s);
        s = -1;
        continue;
      }
    }

    goto ret;
  }

err:
  if (s != -1) {
    close(s);
    s = -1;
  }

ret:
  freeaddrinfo(servinfo);
  return s;
}

static inline int ep_tcpconnect(const char *host, int port) {
  return ep_generictcpconnect(host, port, NULL, 0);
}

static inline int ep_tcpconnect_nb(const char *host, int port) {
  return ep_generictcpconnect(host, port, NULL, 1);
}

static inline int ep_tcpbindconnect(const char *host, int port, char *source) {
  return ep_generictcpconnect(host, port, source, 0);
}

static inline int ep_tcpbindconnect_nb(const char *host, int port, char *source) {
  return ep_generictcpconnect(host, port, source, 1);
}

static inline int ep_genericudpconnect(const char *host, int port, char *source, int nonblock) {
  int s = -1;
  char portstr[6];
  struct addrinfo hints, *servinfo, *bservinfo, *p, *b;

  snprintf(portstr, sizeof(portstr), "%d", port);
  bzero(&hints, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_DGRAM;

  int rv;

  if ((rv = getaddrinfo(host, portstr, &hints, &servinfo)) != 0) {
    return -1;
  }

  for (p = servinfo; p != NULL; p = p->ai_next) {
    if ((s = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
      continue;
    }

    if (ep_reuseaddr_enable(s) == -1) {
      goto err;
    }

    if (nonblock && ep_nonblock(s) == -1) {
      goto err;
    }

    if (source) {
      int bound = 0;

      if ((rv = getaddrinfo(source, NULL, &hints, &bservinfo)) != 0) {
        goto err;
      }

      for (b = bservinfo; b != NULL; b = b->ai_next) {
        if (bind(s, b->ai_addr, b->ai_addrlen) != -1) {
          bound = 1;
          break;
        }
      }

      freeaddrinfo(bservinfo);

      if (!bound) {
        goto ret;
      }
    }

    if (connect(s, p->ai_addr, p->ai_addrlen) == -1) {
      close(s);
      s = -1;
      continue;
    }

    goto ret;
  }

err:
  if (s != -1) {
    close(s);
    s = -1;
  }

ret:
  freeaddrinfo(servinfo);
  return s;
}

static inline int ep_udpconnect(const char *host, int port) {
  return ep_genericudpconnect(host, port, NULL, 0);
}

static inline int ep_udpconnect_nb(const char *host, int port) {
  return ep_genericudpconnect(host, port, NULL, 1);
}

static inline int ep_udpbindconnect(const char *host, int port, char *source) {
  return ep_genericudpconnect(host, port, source, 0);
}

static inline int ep_udpbindconnect_nb(const char *host, int port, char *source) {
  return ep_genericudpconnect(host, port, source, 1);
}

static inline int ep_genericunixconnect(const char *path, int nonblock) {
  int ret = -1;
  int s;

  do {
    struct sockaddr_un sa;

    if ((s = ep_createsocket(AF_LOCAL)) == -1) {
      break;
    }

    sa.sun_family = AF_LOCAL;
    strncpy(sa.sun_path, path, sizeof(sa.sun_path) - 1);

    if (nonblock) {
      close(s);
      break;
    }

    if (connect(s, (struct sockaddr *)&sa, sizeof(sa)) == -1) {
      if (!nonblock || errno != EINPROGRESS) {
        close(s);
      }
    }

    ret = s;
  } while (0);

  return s;
}

static inline int ep_unixconnect(char *path) {
  return ep_genericunixconnect(path, 0);
}

static inline int ep_unixconnect_nb(char *path) {
  return ep_genericunixconnect(path, 1);
}

static inline int ep_read(int fd, char *buf, int count) {
  int n, total = 0;

  while (total != count) {
    n = read(fd, buf, count - total);

    if (0 == n) {
      break;
    }

    if (-1 == n) {
      total = -1;
    }

    total += n;
    buf += n;
  }

  return total;
}

static inline int ep_write(int fd, char *buf, int count) {
  int n, total = 0;

  while (total != count) {
    n = write(fd, buf, count - total);

    if (0 == n) {
      break;
    }

    if (-1 == n) {
      total = -1;
      break;
    }

    total += n;
    buf += n;
  }

  return total;
}

static inline int ep_listen(int s, struct sockaddr *sa, socklen_t len, int backlog) {
  return (bind(s, sa, len) == 0 && listen(s, backlog) == 0) ? 0 : -1;
}

static inline int ep_v6only(int s) {
  int ret = 0;
  int val = 1;

  if (setsockopt(s, IPPROTO_IPV6, IPV6_V6ONLY, &val, sizeof(val))) {
    close(s);
    ret = -1;
  }

  return ret;
}

static inline int ep__tcpserver(int port, const char *bindaddr, int af, int backlog) {
  int s, rv;
  char portstr[6];
  struct addrinfo hints, *servinfo, *p;

  snprintf(portstr, 6, "%d", port);
  bzero(&hints, sizeof(hints));
  hints.ai_family = af;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  if ((rv = getaddrinfo(bindaddr, portstr, &hints, &servinfo))) {
    goto err;
  }

  for (p = servinfo; p != NULL; p = p->ai_next) {
    if ((s = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0) {
      continue;
    }

    if (af == AF_INET6 && ep_v6only(s)) {
      goto err;
    }

    if (ep_reuseaddr_enable(s)) {
      goto err;
    }

    if (ep_listen(s, p->ai_addr, p->ai_addrlen, backlog)) {
      goto err;
    }

    goto end;
  }

  if (NULL == p) {
    goto err;
  }

err:
  s = -1;
end:
  freeaddrinfo(servinfo);
  return s;
}

static inline int ep_tcpserver(int port, const char *bindaddr, int backlog) {
  return ep__tcpserver(port, bindaddr, AF_INET, backlog);
}

static inline int ep_tcp6server(int port, const char *bindaddr, int backlog) {
  return ep__tcpserver(port, bindaddr, AF_INET6, backlog);
}

static inline int ep_unixserver(const char *path, mode_t perm, int backlog) {
  int ret = -1;
  int s;

  do {
    struct sockaddr_un sa;

    if ((s = ep_createsocket(AF_LOCAL)) < 0) {
      break;
    }

    memset(&sa, 0, sizeof(sa));
    sa.sun_family = AF_LOCAL;
    strncpy(sa.sun_path, path, sizeof(sa.sun_path) - 1);

    if (ep_listen(s, (struct sockaddr *)&sa, sizeof(sa), backlog)) {
      break;
    }

    if (perm) {
      chmod(sa.sun_path, perm);
    }

    ret = s;
  } while (0);

  return ret;
}

static inline int ep_genericaccept(int s, struct sockaddr *sa, socklen_t *len) {
  int ret = -1;

  while (1) {
    int fd = accept(s, sa, len);

    if (-1 == fd) {
      if (EINTR == errno) {
        continue;
      } else {
        break;
      }
    }

    ret = fd;
    break;
  }

  return ret;
}

static inline void ep_getinetinfo(struct sockaddr_storage *ss, char *ip, size_t ip_len, int *port) {
  if (ss->ss_family == AF_INET) {
    struct sockaddr_in *s = (struct sockaddr_in *)ss;

    if (ip) {
      inet_ntop(AF_INET, (void *)&(s->sin_addr), ip, ip_len);
    }

    if (port) {
      *port = ntohs(s->sin_port);
    }
  } else {
    struct sockaddr_in6 *s = (struct sockaddr_in6 *)ss;

    if (ip) {
      inet_ntop(AF_INET6, (void *)&(s->sin6_addr), ip, ip_len);
    }

    if (port) {
      *port = ntohs(s->sin6_port);
    }
  }
}

static inline int ep_tcpaccept(int s, char *ip, size_t ip_len, int *port) {
  int fd;
  struct sockaddr_storage ss;
  socklen_t ss_len = sizeof(ss);

  if ((fd = ep_genericaccept(s, (struct sockaddr *)&ss, &ss_len)) >= 0) {
    ep_getinetinfo(&ss, ip, ip_len, port);
  }

  return fd;
}

static inline int ep_unixaccept(int s) {
  struct sockaddr_un sa;
  socklen_t sa_len = sizeof(sa);
  return ep_genericaccept(s, (struct sockaddr *)&sa, &sa_len);
}

static inline int ep__udpserver(int port, const char *bindaddr, int af) {
  int s, rv;
  char portstr[6];
  struct addrinfo hints, *servinfo, *p;

  snprintf(portstr, 6, "%d", port);
  bzero(&hints, sizeof(hints));
  hints.ai_family = af;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_flags = AI_PASSIVE;

  if ((rv = getaddrinfo(bindaddr, portstr, &hints, &servinfo))) {
    goto err;
  }

  for (p = servinfo; p != NULL; p = p->ai_next) {
    if ((s = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0) {
      continue;
    }

    if (af == AF_INET6 && ep_v6only(s)) {
      goto err;
    }

    if (ep_reuseaddr_enable(s)) {
      goto err;
    }

    if (bind(s, p->ai_addr, p->ai_addrlen)) {
      goto err;
    }

    goto end;
  }

  if (NULL == p) {
    goto err;
  }

err:
  s = -1;
end:
  freeaddrinfo(servinfo);
  return s;
}

static inline int ep_udpserver(int port, const char *bindaddr) {
  return ep__udpserver(port, bindaddr, AF_INET);
}

static inline int ep_udp6server(int port, const char *bindaddr) {
  return ep__udpserver(port, bindaddr, AF_INET6);
}

static inline ssize_t ep_udprecvfrom(int s, void *buf, size_t length, char *ip, size_t ip_len, int *port) {
  struct sockaddr_storage ss;
  socklen_t ss_len = sizeof(ss);
  ssize_t len = recvfrom(s, buf, length, 0, (struct sockaddr *)&ss, &ss_len);

  if (len > 0) {
    ep_getinetinfo(&ss, ip, ip_len, port);
  }

  return len;
}

static inline int ep_tcppeername(int fd, char *ip, size_t ip_len, int *port) {
  struct sockaddr_storage sa;
  socklen_t salen = sizeof(sa);

  if (getpeername(fd, (struct sockaddr*)&sa, &salen) == -1) {
    if (port) {
      *port = 0;
    }

    ip[0] = '?';
    ip[1] = '\0';
    return -1;
  }

  if (sa.ss_family == AF_INET) {
    struct sockaddr_in *s = (struct sockaddr_in *)&sa;

    if (ip) {
      inet_ntop(AF_INET, (void*)&(s->sin_addr), ip, ip_len);
    }

    if (port) {
      *port = ntohs(s->sin_port);
    }
  } else {
    struct sockaddr_in6 *s = (struct sockaddr_in6 *)&sa;

    if (ip) {
      inet_ntop(AF_INET6, (void*)&(s->sin6_addr), ip, ip_len);
    }

    if (port) {
      *port = ntohs(s->sin6_port);
    }
  }

  return 0;
}

// static inline int sockname(int fd, char *ip, size_t ip_len, int *port);

#endif //!__EP_SOCK_H__
