#include "ep.h"
#include "ep_fcntl.h"
#include "ep_sock.h"

#include <stdlib.h>

int main()
{
  /* char path[255] = "/var/run/test.sock"; */
  char path[255] = "./test.sock";

  ep_state_t state;
  ep_init(&state);

  int server = ep_unixserver(path, S_IRWXU, 128);
  puts("listen ./test.sock");

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

  ep_data_t client_data;
  ep_data_t data_out;

  while (1) {
    puts(" ====== iterate");
    fflush(stdout);
    if (ep_iterate(&state, -1, &events, &data_out) > 0) {
      if (data_out.fd == server) {
        int client;

        while ((client = ep_unixaccept(server)) > 0) {
          puts("new connection\n");
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
          buffer[len] = '\n';
          buffer[len + 1] = '\0';
          write(client, buffer, len + 2);
          fflush(stdout);
        }
      }
    }
  }

  ep_fini(&state);
  return 0;
}
