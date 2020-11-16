#include "ll.h"
#include <stdlib.h>

#define LL_HEAD_SIZE sizeof(struct ll_head)
#define LL_NODE_SIZE sizeof(struct ll_node)

struct ll_head* ll_construct() {
  struct ll_head* head = malloc(LL_HEAD_SIZE);

  head->last = NULL;
  head->next = NULL;

  return head;
}

void ll_destroy(struct ll_head* list) {
  struct ll_node* node = list->next;
  struct ll_node* temp;

  while (node) {
    temp = node;
    node = node->next;
    free(temp);
  }

  free(list);
}

struct ll_node* ll_append(struct ll_head* list, void* buf, int len) {
  struct ll_node* new_node = malloc(LL_NODE_SIZE);

  new_node->buf = buf;
  new_node->len = len;
  new_node->prev = NULL;
  new_node->next = NULL;

  if (list->last) {
    list->last->next = new_node;
    new_node->prev = list->last;
  } else {
    list->next = new_node;
  }

  list->last = new_node;

  return new_node;
}