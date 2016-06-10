#include "singly_list.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

typedef struct {
  singly_list_node_t list;
  int i;
} int_node_t;

int main(int argc, char *argv[])
{
  SINGLY_LIST_HEAD(list);

  for (int i = 0; i < 100; ++i) {
    int_node_t *node = (int_node_t *)malloc(sizeof(int_node_t));
    node->i = i;
    singly_list_push(&node->list, &list);
  }

  singly_list_node_t *node;

  while ((node = singly_list_shift(&list))) {
    printf("node = %d\n", singly_list_entry(node, int_node_t, list)->i);
  }

  assert(singly_list_empty(&list));

  return 0;
}
