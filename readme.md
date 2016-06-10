# libep

a tiny event poll library with only header files.

## supported event multiplexing model

* `epoll`
* `kqueue`
* `select`

## Example

```c
int main()
{
  ep_state_t state;
  ep_init(&state);

  int server = ep_tcpserver(13681, "127.0.0.1", 128);

  puts("listen 127.0.0.1:13681");
  ep_nonblock(server);

  int events;
  ep_data_t data;
  data.ptr = NULL;
  data.fd = server;
  ep_add(&state, server, EP_READABLE, data);

  ep_data_t client_data;
  ep_data_t data_out;

  while (1) {
    if (ep_iterate(&state, -1, &events, &data_out) > 0) {
      if (data_out.fd == server) {
        int client;
        char ip[16] = {'\0'};
        int port;

        while ((client = ep_tcpaccept(server, ip, sizeof(ip), &port)) > 0) {
          printf("new connection: ip(%s) port(%d)\n", ip, port);
          ep_nonblock(client);
          client_data.fd = client;
          ep_add(&state, client, EP_READABLE, client_data);
        }
      } else {
        int len;
        static char buffer[1024];
        int client = data_out.fd;

        while ((len = read(client, buffer, 1024)) >= 0) {
          if (!len) {
            close(client);
            break;
          }

          write(client, buffer, len);
          fflush(stdout);
        }
      }
    }
  }

  ep_fini(&state);
  return 0;
}
```
