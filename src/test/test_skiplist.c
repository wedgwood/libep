#include "skiplist.h"

#include <stdio.h>
#include <stdlib.h>

SKIPLIST_DECLARE_TYPE(priority, 8, long, 0.25)

int calc_level(skiplist_node_t(priority) *node) {
  int ret = 0;
  while (node->forward[ret++]) {}
  return ret;
}

void test1(int times) {
  typedef skiplist_t(priority) priority_t;
  typedef skiplist_node_t(priority) priority_node_t;
  priority_t p;
  skiplist_init(priority, &p);

  while (--times >= 0) {
    priority_node_t *node = (priority_node_t *)malloc(sizeof(priority_node_t));
    skiplist_node_init(priority, node, times);
    skiplist_insert(priority, &p, node);
    skiplist_delete(priority, &p, node);
  }

  priority_node_t *n;

  skiplist_for_each(n, &p) {
    printf("\n%d: %ld\n", calc_level(n), n->score);
  }

  priority_node_t *n1;
  priority_node_t *n2;
  skiplist_for_each_clear(n1, n2, &p, free);
}

void test2(int times) {
  typedef skiplist_t(priority) priority_t;
  typedef skiplist_node_t(priority) priority_node_t;
  priority_t p;
  skiplist_init(priority, &p);

  while (--times >= 0) {
    priority_node_t *node = (priority_node_t *)malloc(sizeof(priority_node_t));
    skiplist_node_init(priority, node, times);
    skiplist_insert(priority, &p, node);
  }

  priority_node_t *n;

  while ((n = skiplist_shift(priority, &p))) {
    printf("\nv: %ld\n", n->score);
  }
}

void test3(int times) {
  typedef skiplist_t(priority) priority_t;
  typedef skiplist_node_t(priority) priority_node_t;
  priority_t p;
  skiplist_init(priority, &p);

  while (--times >= 0) {
    priority_node_t *node = (priority_node_t *)malloc(sizeof(priority_node_t));
    skiplist_node_init(priority, node, times);
    skiplist_insert(priority, &p, node);

    if (times % 3) {
      skiplist_delete(priority, &p, node);
    }
  }

  priority_node_t *node, *n;

  skiplist_shift_lte(priority, &p, 123, node, n) {
    printf("\nv: %ld\n", node->score);
  }
}

int main(int argc, const char *argv[]) {
  /* test1(10000); */
  /* test2(10000); */
  test3(10000);
  return 0;
}
