#include "ep_task_queue.h"

#include <stdio.h>

void test(void *arg) {
  int *a = arg;
  printf("%d\n", (*a)++);
}

int main(int argc, const char *argv[]) {
  ep_task_queue_t queue;
  ep_task_queue_init(&queue);
  int a = 1;
  void *arg = &a;
  ep_task_queue_push(&queue, test, arg);
  ep_task_queue_push(&queue, test, arg);
  ep_task_queue_push(&queue, test, arg);
  ep_task_queue_push(&queue, test, arg);
  ep_task_id_t task = ep_task_queue_push(&queue, test, arg);
  ep_task_cancel(task);
  ep_task_queue_run(&queue);
  ep_task_queue_push(&queue, test, arg);
  ep_task_queue_push(&queue, test, arg);
  ep_task_queue_run(&queue);
  ep_task_queue_fini(&queue);
  return 0;
}
