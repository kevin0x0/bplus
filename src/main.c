#include "bplus.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
  BplusTreeNode* root = NULL;
  root = bplus_insert(root, 1);
  root = bplus_insert(root, 4);
  root = bplus_insert(root, 7);
  root = bplus_insert(root, 5);
  root = bplus_insert(root, 3);
  root = bplus_insert(root, 8);
  root = bplus_insert(root, 6);
  root = bplus_insert(root, 2);

  BplusIter begin = bplus_search_not_below(root, 2);
  BplusIter end = bplus_search_above(root, 7);
  while (begin.node != end.node || begin.index != end.index) {
    printf("%d ", begin.node->elems[begin.index]);
    begin = bplus_iter_next(begin);
  }
  return 0;
//  int num = atoi(argv[1]);
//  BplusTreeNode treenode;
//  treenode.nodeptr[BPLUS_DEGREE - 1] = NULL;
//  treenode.nodeptr[0] = NULL;
//  for (size_t i = 0; i < BPLUS_ELEM_NO; ++i) {
//    treenode.elems[i] = i * 2;
//    treenode.nodeptr[i] = (void*)(i + 5);
//  }
//  treenode.nodeptr[BPLUS_ELEM_NO] = (void*)(BPLUS_ELEM_NO + 5);
//  treenode.elem_no = BPLUS_ELEM_NO;
//  size_t insert_pos = (num + 1) / 2;
//  if (insert_pos > BPLUS_ELEM_NO)
//    insert_pos = BPLUS_ELEM_NO;
//  BplusTreeNode* node = bplus_split_inner(&treenode, num, NULL, insert_pos);
//  for (size_t i = 0; i < treenode.elem_no; ++i)
//    printf(" %d", treenode.elems[i]);
//  putchar('\n');
//  for (size_t i = 0; i <= treenode.elem_no; ++i)
//    printf("%d ", treenode.nodeptr[i]);
//  putchar('\n');
//  putchar('\n');
//  for (size_t i = 0; i < node->elem_no; ++i)
//    printf(" %d", node->elems[i]);
//  putchar('\n');
//  for (size_t i = 0; i <= node->elem_no; ++i)
//    printf("%d ", node->nodeptr[i]);
//  putchar('\n');
//  putchar('\n');
//  printf("middle: %d\n", treenode.elems[treenode.elem_no]);
//  putchar('\n');
//  free(node);
//  return 0;
}
