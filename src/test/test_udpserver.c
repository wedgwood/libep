#include "ep.h"
#include "ep_fcntl.h"
#include "ep_sock.h"

#include <stdlib.h>

int main()
{
  ep_state_t state;
  ep_init(&state);

  int server = ep_udpserver(13681, "127.0.0.1");

  puts("listen 127.0.0.1:13681");

  if (server < 0) {
    printf("failed set server!\n");
    return 0;
  } else {
    printf("server setup(%d)", server);
  }

  ep_nonblock(server);

  int events;
  ep_data_t data;
  data.fd = server;
  ep_add(&state, server, EP_READABLE, data);

  ep_data_t data_out;

  while (1) {
    puts("====== iterate");
    fflush(stdout);
    if (ep_iterate(&state, -1, &events, &data_out) > 0) {
      if (data_out.fd == server) {
        struct sockaddr_storage ss;
        socklen_t sl;
        char buf[10240];
        ssize_t len = recvfrom(server, buf, sizeof(buf), 0, (struct sockaddr *)&ss, &sl);

        if (len > 0) {
          buf[len] = '\0';
          printf("-> %s\n", buf);
        }

        sendto(server, buf, len, 0, (struct sockaddr *)&ss, sl);
      }
    }
  }

  ep_fini(&state);
  return 0;
}
