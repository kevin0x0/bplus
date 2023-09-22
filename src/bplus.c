#include "bplus.h"
#include <stdlib.h>
#include <string.h>

static inline BplusTreeNode* bplus_right_sibling(BplusTreeNode* node) {
  return node->nodeptr[BPLUS_ELEM_NO];
}

static size_t bplus_find_insert_pos(BplusTreeNode* node, KeyType key);
static BplusIter bplus_find_insert_pos_rec(BplusTreeNode* node, KeyType key);
static inline size_t bplus_find_nodeptr(BplusTreeNode* node, BplusTreeNode* ptr);
static BplusTreeNode* bplus_rebalance_leaf(BplusTreeNode* node);
static BplusTreeNode* bplus_rebalance_inner(BplusTreeNode* node);


BplusTreeNode* bplus_create(KeyType key) {
  BplusTreeNode* node = (BplusTreeNode*)malloc(sizeof (BplusTreeNode));
  if (!node) return NULL;
  node->elem_no = 0;
  node->father = NULL;
  memset(node, 0, sizeof(BplusTreeNode));
  node->elem_no = 1;
  node->elems[0] = key;
  return node;
}

void bplus_delete(BplusTreeNode* node) {
  if (!node) return;
  /* if node->nodeptr[0] is not NULL, this node is inner node, else is leaf node. */
  if (node->nodeptr[0]) {
    size_t ptr_no = node->elem_no + 1;
    BplusTreeNode** nodeptr = node->nodeptr;
    for (size_t i = 0; i < ptr_no; ++i)
      bplus_delete(nodeptr[i]);
  }
  free(node);
}

BplusIter bplus_search_above(BplusTreeNode* node, KeyType key) {
  if (!node) {
    BplusIter iter = { .node = NULL, .index = 0 };
    return iter;
  }
  while (true) {
  size_t left = 0;
  size_t right = node->elem_no - 1;
    if (key < node->elems[0]) {
      if (!node->nodeptr[0]) {
        BplusIter iter = { .node = node, .index = 0 };
        return iter;
      } else {
        node = node->nodeptr[0];
      }
    } else if (key >= node->elems[right]) {
      if (!node->nodeptr[0]) {
        BplusIter iter = { .node = bplus_right_sibling(node), .index = 0 };
        return iter;
      } else {
        node = node->nodeptr[right + 1];
      }
    } else {
      while (left + 1 != right) {
        size_t mid = (left + right) / 2;
        if (key < node->elems[mid])
          right = mid;
        else
          left = mid;
      }
      if (!node->nodeptr[0]) {
        BplusIter iter = { .node = node, .index = right };
        return iter;
      } else {
        node = node->nodeptr[right];
      }
    }
  }
}

static BplusIter bplus_find_insert_pos_rec(BplusTreeNode* node, KeyType key) {
  while (true) {
  size_t left = 0;
  size_t right = node->elem_no - 1;
    if (key < node->elems[0]) {
      if (!node->nodeptr[0]) {
        BplusIter iter = { .node = node, .index = 0 };
        return iter;
      } else {
        node = node->nodeptr[0];
      }
    } else if (key >= node->elems[right]) {
      if (!node->nodeptr[0]) {
        BplusIter iter = { .node = node, .index = right + 1 };
        return iter;
      } else {
        node = node->nodeptr[right + 1];
      }
    } else {
      while (left + 1 != right) {
        size_t mid = (left + right) / 2;
        if (key < node->elems[mid])
          right = mid;
        else
          left = mid;
      }
      if (!node->nodeptr[0]) {
        BplusIter iter = { .node = node->elems[left] == key ? NULL : node, .index = right };
        return iter;
      } else {
        node = node->nodeptr[right];
      }
    }
  }
}

BplusIter bplus_search_not_below(BplusTreeNode* node, KeyType key) {
  if (!node) {
    BplusIter iter = { .node = NULL, .index = 0 };
    return iter;
  }
  while (true) {
  size_t left = 0;
  size_t right = node->elem_no - 1;
    if (key < node->elems[0]) {
      if (!node->nodeptr[0]) {
        BplusIter iter = { .node = node, .index = 0 };
        return iter;
      } else {
        node = node->nodeptr[0];
      }
    } else if (key == node->elems[right]) {
      if (!node->nodeptr[0]) {
        BplusIter iter = { .node = node, .index = right };
        return iter;
      } else {
        node = node->nodeptr[right + 1];
      }
    } else if (key > node->elems[right]) {
      if (!node->nodeptr[0]) {
        BplusIter iter = { .node = bplus_right_sibling(node), .index = 0 };
        return iter;
      } else {
        node = node->nodeptr[right + 1];
      }
    } else {
      while (left + 1 != right) {
        size_t mid = (left + right) / 2;
        if (key < node->elems[mid])
          right = mid;
        else
          left = mid;
      }
      if (!node->nodeptr[0]) {
        BplusIter iter = { .node = node, .index = node->elems[left] == key ? left : right };
        return iter;
      } else {
        node = node->nodeptr[right];
      }
    }
  }
}

BplusTreeNode* bplus_insert(BplusTreeNode* root, KeyType key) {
  if (!root) return bplus_create(key);

  /* find where to insert the key(what index in which leaf node) */
  BplusIter iter = bplus_find_insert_pos_rec(root, key);
  if (!iter.node) return NULL; /* this element has already been inserted to the B+ tree */
  BplusTreeNode* node = iter.node;
  if (node->elem_no != BPLUS_ELEM_NO) { /* the 'node' is not full */
    memmove(node->elems + iter.index + 1, node->elems + iter.index, (node->elem_no - iter.index) * sizeof (node->elems[0]));
    node->elems[iter.index] = key;
    node->elem_no++;
    return root;
  }
  /* if 'node' is full, split it. The right part is returned and received by 'split_node'. */
  BplusTreeNode* split_node = bplus_split_leaf(node, key, iter.index);
  if (!split_node) return NULL;
  KeyType middle = split_node->elems[0];
  
  /* insert 'middle' to father of 'node' */
  while (node->father) {
    node = node->father;
    /* find where to insert 'middle' in the 'node' */
    size_t index = bplus_find_insert_pos(node, middle);
    if (node->elem_no != BPLUS_ELEM_NO) {
      /* the 'node' is not full, insert and return. */
      memmove(node->elems + index + 1, node->elems + index, (node->elem_no - index) * sizeof (node->elems[0]));
      memmove(node->nodeptr + index + 2, node->nodeptr + index + 1, (node->elem_no - index) * sizeof (BplusTreeNode*));
      node->elems[index] = middle;
      node->nodeptr[index + 1] = split_node;
      node->elem_no++;
      return root;
    } else {
      /* the 'node' is full, split it. The right part is returned and received by 'split_node'. */
      split_node = bplus_split_inner(node, middle, split_node, index);
      if (!split_node) return NULL;
      /* when spliting inner node, the middle element will be stored in the trailing position of left part. */
      middle = node->elems[node->elem_no];
    }
  }
  /* the 'node' is split and is root node, so create a new root. */
  BplusTreeNode* new_root = bplus_create(middle);
  new_root->nodeptr[0] = node;
  new_root->nodeptr[1] = split_node;
  node->father = new_root;
  split_node->father = new_root;
  return new_root;
}

BplusTreeNode* bplus_erase(BplusTreeNode* root, BplusIter iter) {
  if (!iter.node) return NULL;
  /* remove the element */
  memmove(iter.node->elems + iter.index, iter.node->elems + iter.index + 1, (iter.node->elem_no - iter.index - 1) * sizeof (KeyType));
  iter.node->elem_no--;
  /* no need to rebalance this leaf node */
  if (iter.node->elem_no >= BPLUS_ELEM_NO / 2)
    return root;

  BplusTreeNode* node = iter.node;
  if (!node->father) {  /* 'node' is root */
    if (node->elem_no == 0) {
      free(node);
      return NULL;
    } else {
      return node;
    }
  }
  BplusTreeNode* merge_node = bplus_rebalance_leaf(node);
  /* succeed to steal a key from a sibling node of 'node' */
  if (!merge_node) return root;
  /* 'node' is merged with one of its sibling node,
   * now check whether their father need to rebalance(they must have a father). */ 
  node = merge_node->father;
  while (node->father) {
    if (node->elem_no >= BPLUS_ELEM_NO / 2) /* no need to rebalance */
      return root;
    merge_node = bplus_rebalance_inner(node);
    if (!merge_node) return root;
    /* check their father */
    node = merge_node->father;
  }
  /* now 'node' is root */
  if (node->elem_no != 0)
    return root;
  else {
    /* 'merge_node' bacomes root */
    free(node);
    merge_node->father = NULL;
    return merge_node;
  }
}

BplusTreeNode* bplus_split_leaf(BplusTreeNode* node, KeyType inserted_entry, size_t insert_pos) {
  BplusTreeNode* right_node = (BplusTreeNode*)malloc(sizeof (BplusTreeNode));
  if (!right_node) return NULL;
  size_t right_elem_no = (node->elem_no + 1) / 2;
  right_node->elem_no = right_elem_no;
  size_t left_elem_no = node->elem_no + 1 - right_elem_no;
  node->elem_no = left_elem_no;
  size_t total_elem_no = left_elem_no + right_elem_no;
  if (insert_pos < left_elem_no) {
    memcpy(right_node->elems, node->elems + left_elem_no - 1, right_elem_no * sizeof (node->elems[0]));
    if (insert_pos + 1 != left_elem_no)
      memmove(node->elems + insert_pos + 1, node->elems + insert_pos, (left_elem_no - insert_pos - 1) * sizeof (node->elems[0]));
    node->elems[insert_pos] = inserted_entry;
  } else {
    memcpy(right_node->elems, node->elems + left_elem_no, (insert_pos - left_elem_no) * sizeof (node->elems));
    memcpy(right_node->elems + insert_pos - left_elem_no + 1, node->elems + insert_pos, (total_elem_no - insert_pos) * sizeof (node->elems[0]));
    right_node->elems[insert_pos - left_elem_no] = inserted_entry;
  }
  /* nodeptr[0] show whether a node is a leaf. NULL indicates yes */
  right_node->nodeptr[0] = NULL;
  right_node->father = node->father;
  right_node->nodeptr[BPLUS_ELEM_NO] = node->nodeptr[BPLUS_ELEM_NO];
  node->nodeptr[BPLUS_ELEM_NO] = right_node;
  return right_node;
}

BplusTreeNode* bplus_split_inner(BplusTreeNode* node, KeyType inserted_entry, BplusTreeNode* inserted_node, size_t insert_pos) {
  BplusTreeNode* right_node = (BplusTreeNode*)malloc(sizeof (BplusTreeNode));
  if (!right_node) return NULL;
  size_t right_elem_no = (node->elem_no - 1) / 2;
  right_node->elem_no = right_elem_no;
  size_t left_elem_no = node->elem_no - right_elem_no;
  node->elem_no = left_elem_no;
  size_t total_elem_no = left_elem_no + right_elem_no;
  if (insert_pos <= left_elem_no) {
    memcpy(right_node->elems, node->elems + left_elem_no, right_elem_no * sizeof (node->elems[0]));
    memcpy(right_node->nodeptr, node->nodeptr + left_elem_no, (right_elem_no + 1) * sizeof (BplusTreeNode*));
    if (insert_pos != left_elem_no) {
      memmove(node->elems + insert_pos + 1, node->elems + insert_pos, (left_elem_no - insert_pos) * sizeof (node->elems[0]));
      memmove(node->nodeptr + insert_pos + 2, node->nodeptr + insert_pos + 1, (left_elem_no - insert_pos - 1) * sizeof (BplusTreeNode*));
      node->elems[insert_pos] = inserted_entry;
      node->nodeptr[insert_pos + 1] = inserted_node;
    } else {
      right_node->nodeptr[0] = inserted_node;
      node->elems[left_elem_no] = inserted_entry;
    }
  } else {
    memcpy(right_node->elems + insert_pos - left_elem_no, node->elems + insert_pos, (total_elem_no - insert_pos) * sizeof (node->elems[0]));
    memcpy(right_node->nodeptr + insert_pos - left_elem_no, node->nodeptr + insert_pos, (total_elem_no - insert_pos) * sizeof (BplusTreeNode*));
    memcpy(right_node->elems, node->elems + left_elem_no + 1, (insert_pos - left_elem_no - 1) * sizeof (node->elems[0]));
    memcpy(right_node->nodeptr, node->nodeptr + left_elem_no + 1, (insert_pos - left_elem_no) * sizeof (BplusTreeNode*));
    right_node->nodeptr[insert_pos - left_elem_no] = inserted_node;
    right_node->elems[insert_pos - left_elem_no - 1] = inserted_entry;
  }
  right_node->father = node->father;
  for (size_t i = 0; i <= right_node->elem_no; ++i)
    right_node->nodeptr[i]->father = right_node;
  return right_node;
}

static BplusTreeNode* bplus_rebalance_inner(BplusTreeNode* node) {
  BplusTreeNode* father = node->father;
  size_t index = bplus_find_nodeptr(father, node);
  size_t sibling_index = 0;
  if (index == 0) {
    sibling_index = 1;
  } else if (index == father->elem_no) {
    sibling_index = father->elem_no - 1;
  } else {
    sibling_index = father->nodeptr[index - 1]->elem_no < father->nodeptr[index + 1]->elem_no ?
                    index + 1 : index - 1;
  }
  if (index < sibling_index) {
    BplusTreeNode* right_sibling = father->nodeptr[sibling_index];
    if (right_sibling->elem_no > BPLUS_ELEM_NO / 2) {
      node->elems[node->elem_no++] = father->elems[index];
      node->nodeptr[node->elem_no] = right_sibling->nodeptr[0];
      node->nodeptr[node->elem_no]->father = node;
      father->elems[index] = right_sibling->elems[0];
      memmove(right_sibling->nodeptr, right_sibling->nodeptr + 1, right_sibling->elem_no * sizeof (BplusTreeNode*));
      memmove(right_sibling->elems, right_sibling->elems + 1, --right_sibling->elem_no * sizeof (KeyType));
      return NULL;
    } else {
      node->elems[node->elem_no++] = father->elems[index];
      memcpy(node->elems + node->elem_no, right_sibling->elems, right_sibling->elem_no * sizeof (KeyType));
      memcpy(node->nodeptr + node->elem_no, right_sibling->nodeptr, (right_sibling->elem_no + 1) * sizeof (BplusTreeNode*));
      node->elem_no += right_sibling->elem_no;
      for (size_t i = 0; i <= right_sibling->elem_no; ++i)
        right_sibling->nodeptr[i]->father = node;
      memmove(father->elems + sibling_index - 1, father->elems + sibling_index, (father->elem_no - sibling_index) * sizeof (KeyType));
      memmove(father->nodeptr + sibling_index, father->nodeptr + sibling_index + 1, (father->elem_no - sibling_index) * sizeof (BplusTreeNode*));
      free(right_sibling);
      return node;
    }
  } else {
    BplusTreeNode* left_sibling = father->nodeptr[sibling_index];
    if (left_sibling->elem_no > BPLUS_ELEM_NO / 2) {
      memmove(node->elems + 1, node->elems, node->elem_no++ * sizeof (KeyType));
      memmove(node->nodeptr + 1, node->nodeptr, node->elem_no * sizeof (BplusTreeNode*));
      node->elems[0] = father->elems[index - 1];
      node->nodeptr[0] = left_sibling->nodeptr[left_sibling->elem_no];
      node->nodeptr[0]->father = node;
      father->elems[index - 1] = left_sibling->elems[--left_sibling->elem_no];
      return NULL;
    } else {
      left_sibling->elems[left_sibling->elem_no++] = father->elems[sibling_index];
      memcpy(left_sibling->elems + left_sibling->elem_no, node->elems, node->elem_no * sizeof (KeyType));
      memcpy(left_sibling->nodeptr + left_sibling->elem_no, node->nodeptr, (node->elem_no + 1) * sizeof (BplusTreeNode*));
      left_sibling->elem_no += node->elem_no;
      for (size_t i = 0; i <= node->elem_no; ++i)
        node->nodeptr[i]->father = left_sibling;
      memmove(father->elems + index - 1, father->elems + index, (father->elem_no - index) * sizeof (KeyType));
      memmove(father->nodeptr + index, father->nodeptr + index + 1, (father->elem_no - index) * sizeof (BplusTreeNode*));
      free(node);
      return left_sibling;
    }
  }
}

static BplusTreeNode* bplus_rebalance_leaf(BplusTreeNode* node) {
  BplusTreeNode* father = node->father;
  size_t index = bplus_find_nodeptr(father, node);
  size_t sibling_index = 0;
  if (index == 0) {
    sibling_index = 1;
  } else if (index == father->elem_no) {
    sibling_index = father->elem_no - 1;
  } else {
    sibling_index = father->nodeptr[index - 1]->elem_no < father->nodeptr[index + 1]->elem_no ?
                    index + 1 : index - 1;
  }
  if (index < sibling_index) {
    BplusTreeNode* right_sibling = father->nodeptr[sibling_index];
    if (right_sibling->elem_no > BPLUS_ELEM_NO / 2) {
      node->elems[node->elem_no++] = right_sibling->elems[0];
      memmove(right_sibling->elems, right_sibling->elems + 1, --right_sibling->elem_no * sizeof (KeyType));
      father->elems[index] = right_sibling->elems[0];
      return NULL;
    } else {
      memcpy(node->elems + node->elem_no, right_sibling->elems, right_sibling->elem_no * sizeof (KeyType));
      node->elem_no += right_sibling->elem_no;
      memmove(father->elems + sibling_index - 1, father->elems + sibling_index, (father->elem_no - sibling_index) * sizeof (KeyType));
      memmove(father->nodeptr + sibling_index, father->nodeptr + sibling_index + 1, (father->elem_no - sibling_index) * sizeof (BplusTreeNode*));
      node->nodeptr[BPLUS_ELEM_NO] = bplus_right_sibling(right_sibling);
      free(right_sibling);
      return node;
    }
  } else {
    BplusTreeNode* left_sibling = father->nodeptr[sibling_index];
    if (left_sibling->elem_no > BPLUS_ELEM_NO / 2) {
      memmove(node->elems + 1, node->elems, node->elem_no++ * sizeof (KeyType));
      node->elems[0] = left_sibling->elems[--left_sibling->elem_no];
      father->elems[sibling_index] = node->elems[0];
      return NULL;
    } else {
      memcpy(left_sibling->elems + left_sibling->elem_no, node->elems, node->elem_no * sizeof (KeyType));
      left_sibling->elem_no += node->elem_no;
      memmove(father->elems + index - 1, father->elems + index, (father->elem_no - index) * sizeof (KeyType));
      memmove(father->nodeptr + index, father->nodeptr + index + 1, (father->elem_no - index) * sizeof (BplusTreeNode*));
      left_sibling->nodeptr[BPLUS_ELEM_NO] = bplus_right_sibling(node);
      free(node);
      return left_sibling;
    }
  }
}

static size_t bplus_find_insert_pos(BplusTreeNode* node, KeyType key) {
  size_t left = 0;
  size_t right = node->elem_no - 1;
  if (key < node->elems[0]) {
      return 0;
  } else if (key >= node->elems[right]) {
      return right + 1;
  } else {
    while (left + 1 != right) {
      size_t mid = (left + right) / 2;
      if (key < node->elems[mid])
        right = mid;
      else
        left = mid;
    }
    return right;
  }
}

static inline size_t bplus_find_nodeptr(BplusTreeNode* node, BplusTreeNode* ptr) {
  for (size_t i = 0; i <= node->elem_no; ++i) {
    if (ptr == node->nodeptr[i]) return i;
  }
  return node->elem_no + 1;
}
