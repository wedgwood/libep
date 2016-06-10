#include "ep.h"
#include "ep_fcntl.h"
#include "ep_sock.h"

#include <stdlib.h>

int main()
{
  char ip[16] = {'\0'};
  int port;

  ep_state_t state;
  ep_init(&state);

  int server = ep_tcpserver(13681, "127.0.0.1", 128);

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
  data.ptr = NULL;
  data.fd = server;
  ep_add(&state, server, EP_READABLE, data);
  perror(NULL);

  ep_data_t client_data;
  ep_data_t data_out;

  while (1) {
    puts("====== iterate");
    fflush(stdout);
    if (ep_iterate(&state, -1, &events, &data_out) > 0) {
      if (data_out.fd == server) {
        int client;

        while ((client = ep_tcpaccept(server, ip, sizeof(ip), &port)) > 0) {
          printf("new connection: ip(%s) port(%d)\n", ip, port);
          fflush(stdout);
          ep_nonblock(client);
          client_data.fd = client;
          ep_add(&state, client, EP_READABLE, client_data);
        }
      } else {
        int len;
        static char buffer[1024];
        int client = data_out.fd;

        while ((len = read(client, buffer, 1022)) >= 0) {
          if (!len) {
            close(client);
            break;
          }

          buffer[len] = '\n';
          buffer[len + 1] = '\0';
          write(client, buffer, len + 2);
          fflush(stdout);

          if (len < 1022) {
            break;
          }
        }
      }
    }
  }

  puts("finish");
  ep_fini(&state);
  return 0;
}
