#include "ep_timer.h"

#include <stdio.h>
#include <unistd.h>

void time_handler(void *arg) {
  puts("after an interval!");
}

void test1() {
  ep_timer_t timer;
  ep_timer_init(&timer);
  ep_timer_updatetime(&timer);
  ep_timer_id_t id = ep_timer_run_every(&timer, 10000, time_handler, NULL);
  ep_timer_execute(&timer, ep_timer_gettime(&timer) + 5000, 10);
  ep_timer_execute(&timer, ep_timer_gettime(&timer) + 10000, 10);
  ep_timer_execute(&timer, ep_timer_gettime(&timer) + 15000, 10);
  ep_timer_execute(&timer, ep_timer_gettime(&timer) + 20000, 10);
  ep_timer_execute(&timer, ep_timer_gettime(&timer) + 25000, 10);
  ep_timer_execute(&timer, ep_timer_gettime(&timer) + 30000, 10);
  ep_timer_clear(id);
}

void test2() {
  ep_timer_t timer;
  ep_timer_init(&timer);
  /* timer_run_after(&timer, 10000, time_handler, NULL); */
  ep_timer_updatetime(&timer);
  ep_timer_run_every(&timer, 10000, time_handler, NULL);

  while (1) {
    ep_timer_updatetime(&timer);
    printf("nearest time: %d\n", ep_timer_nearest_diff(&timer));
    sleep(1);
  }
}

int main(int argc, const char *argv[])
{
  test1();
  test2();
  return 0;
}
