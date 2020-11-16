#ifndef _LL_H
#define _LL_H

// 双向链表
struct ll_head {
  struct ll_node* last;
  struct ll_node* next;
};

struct ll_node {
  void* buf;
  int len;
  struct ll_node* prev;
  struct ll_node* next;
};

struct ll_head* ll_construct();
void ll_destroy(struct ll_head* list);
struct ll_node* ll_append(struct ll_head* list, void* buf, int len);

#endif
