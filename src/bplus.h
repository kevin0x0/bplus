#ifndef _BPLUS_H
#define _BPLUS_H

#include <stdint.h>
#include <stdbool.h>

#define BPLUS_DEGREE    (4)
#define BPLUS_ELEM_NO   (BPLUS_DEGREE - 1)

typedef int KeyType;
typedef struct tagBplusTreeNode {
  struct tagBplusTreeNode* nodeptr[BPLUS_DEGREE];
  struct tagBplusTreeNode* father;
  KeyType elems[BPLUS_ELEM_NO];
  size_t elem_no;
} BplusTreeNode;

typedef struct tagBplusIter {
  BplusTreeNode* node;
  size_t index;
} BplusIter;

BplusTreeNode* bplus_create(KeyType key);
void bplus_delete(BplusTreeNode* node);

BplusTreeNode* bplus_insert(BplusTreeNode* root, KeyType key);
BplusTreeNode* bplus_erase(BplusTreeNode* root, BplusIter iter);
BplusIter bplus_search_above(BplusTreeNode* node, KeyType key);
BplusIter bplus_search_not_below(BplusTreeNode* node, KeyType key);

static inline BplusIter bplus_iter_next(BplusIter iter) {
  BplusIter next;
  if (!iter.node) return iter;
  if (++iter.index != iter.node->elem_no) {
    return iter;
  } else {
    iter.node = iter.node->nodeptr[BPLUS_ELEM_NO];
    iter.index = 0;
    return iter;
  }
}

/* private */
BplusTreeNode* bplus_split_leaf(BplusTreeNode* node, KeyType inserted_entry, size_t insert_pos);
BplusTreeNode* bplus_split_inner(BplusTreeNode* node, KeyType inserted_entry, BplusTreeNode* inserted_node, size_t insert_pos);



#endif
