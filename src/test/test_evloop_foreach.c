#include "ep.h"
#include "ep_fcntl.h"

#include <fcntl.h>
#include <stdio.h>

void hello(void *arg) {
  puts("world");
}

void onetick(void *arg) {
  puts("=== onetick ===");
}

void test1() {
  int fd = 0;
  ep_nonblock(fd);

  ep_state_t state;
  ep_init(&state);

  ep_data_t data;
  data.ptr = 0;
  data.fd = 0;

  ep_add(&state, fd, EP_READABLE, data);
  ep_timer_id_t id = ep_setinterval(&state, 1500, hello, NULL);
  ep_clearinterval(id);

  int events_out;
  ep_data_t data_out;
  ep_everytick(&state, onetick, NULL);

  while (1) {
    ep_for_each(&state, -1, &events_out, &data_out) {
      int len;
      int fd_out = data_out.fd;
      puts("\n--------------------\n");
      static char buffer[1024];
      printf("process: %d\n", fd_out);
      fflush(stdout);

      while ((len = read(fd_out, buffer, 1022)) >= 0) {
        buffer[len] = '\0';
        puts(buffer);
      }
    }
  }

  ep_fini(&state);
  puts("\n=== test1 end ===\n");
}

int main(int argc, const char *argv[]) {
  printf("\nusing %s\n", ep_pollapi_name());
  test1();
  return 0;
}
