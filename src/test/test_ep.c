#include "ep.h"

#include <fcntl.h>
#include <stdio.h>

void process(int events, ep_data_t data) {
  int fd = data.fd;

  if (events & EP_READABLE) {
    static char buffer[1024];
    int len;
    printf("\nresponse: \n");

    while ((len = read(fd, buffer, 1023)) >= 0) {
      buffer[len] = '\0';
      /* buffer[len + 1] = '\0'; */
      puts(buffer);

      if (len < 1023) {
        break;
      }
    }

    fflush(stdout);
  }
}

int main(int argc, const char *argv[])
{
  printf("ep engine: %s\n", ep_pollapi_name());
  ep_pollstate_t state;

  if (ep_pollapi_init(&state)) {
    puts("failed to init!");
    return -1;
  }

  int fd = 0;
  int flags = fcntl(fd, F_GETFL);
  fcntl(fd, F_SETFL, flags | O_NONBLOCK);

  int events = EP_READABLE;
  ep_data_t data;
  data.ptr = 0;
  data.fd = fd;

  ep_pollapi_add(&state, fd, events, data);

  while (1) {
    int ready = ep_pollapi_wait(&state, 200);
    int events_out = 0;
    ep_data_t data_out;

    if (ready > 0) {
      while (ep_pollapi_fetch(&state, &events_out, &data_out) > 0) {
        process(events_out, data_out);
      }
    }
  }

  return 0;
}
