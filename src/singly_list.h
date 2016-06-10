#ifndef __SINGLY_LIST_H__
#define __SINGLY_LIST_H__

#include <stddef.h>

typedef struct singly_list_head_s singly_list_head_t;
typedef struct singly_list_node_s singly_list_node_t;

struct singly_list_head_s {
  singly_list_node_t *begin;
  singly_list_node_t **end;
};

struct singly_list_node_s {
  singly_list_node_t *next;
};

#define SINGLY_LIST_HEAD_INIT(name) {NULL, &name.begin}

#define SINGLY_LIST_HEAD(name) \
  singly_list_head_t name = SINGLY_LIST_HEAD_INIT(name)

#define INIT_SINGLY_LIST_HEAD(ptr) do { \
  (ptr)->begin = NULL; \
  (ptr)->end = &(ptr)->begin; \
} while (0)

#define SINGLY_LIST_NODE_INIT(name) {NULL;}

#define SINGLY_LIST_NODE(name) \
  singly_list_node_t name = SINGLY_LIST_NODE_INIT(name)

#define INIT_SINGLY_LIST_NODE(ptr) do { \
  (ptr)->next = NULL; \
} while (0)

static inline void singly_list_push(singly_list_node_t *new_node, singly_list_head_t *head) {
  *head->end = new_node;
  head->end = &new_node->next;
  new_node->next = NULL;
}

static inline void singly_list_unshift(singly_list_node_t *new_node, singly_list_head_t *head) {
  if (head->begin) {
    new_node->next = head->begin;
    head->begin = new_node;
  } else {
    head->begin = new_node;
    head->end = &new_node->next;
    new_node->next = NULL;
  }
}

static inline singly_list_node_t *singly_list_shift(singly_list_head_t *head) {
  singly_list_node_t *ret = NULL;

  if (head->begin) {
    ret = head->begin;
    head->begin = head->begin->next;

    if (head->begin) {
      head->end = &head->begin;
    }
  }

  return ret;
}

static inline int singly_list_empty(singly_list_head_t *head) {
  return !head->begin;
}

static inline void __singly_list_splice(singly_list_head_t *list, singly_list_head_t *head) {
  *head->end = list->begin;
}

static inline void singly_list_splice(singly_list_head_t *list, singly_list_head_t *head) {
  if (!singly_list_empty(list)) {
    __singly_list_splice(list, head);
  }
}

static inline void singly_list_splice_init(singly_list_head_t *list, singly_list_head_t *head) {
  if (!singly_list_empty(list)) {
    __singly_list_splice(list, head);
    INIT_SINGLY_LIST_HEAD(list);
  }
}

#define singly_list_entry(ptr, type, member) \
  ((type *)((char *)(ptr) - (unsigned long)(&((type *)0)->member)))

#define singly_list_for_each(pos, head) \
  for (pos = (head)->next; pos; pos = pos->next)

#define singly_list_for_each_safe(pos, n, head) \
  for (pos = (head)->next, n = pos ? pos->next : NULL; pos; pos = n, n = n ? n->next : NULL)

#define singly_list_end_entry(type, member) singly_list_entry(NULL, type, member)

#define singly_list_for_each_entry(pos, head, member) \
  for (pos = singly_list_entry((head)->next, typeof(*pos), member); \
    pos != singly_list_end_entry(typeof(*pos), member); \
    pos = singly_list_entry(pos->member.next, typeof(*pos), member))

#endif //!__SINGLY_LIST_H__
