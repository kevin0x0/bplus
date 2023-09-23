#include "bplus.h"
#include "windowsx.h"
#include "visualization.h"
#include <mutex>
#include <math.h>
#include <vector>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
std::mutex mut;
BplusTreeNode* root = NULL;
int total_elem_no = 0;
float r = 1.0;
constexpr int textwidth = 10;
constexpr int textheight = 18;
constexpr int hierarchy_distancce = 100;

int offset_x = 400;
int offset_y = 100;

constexpr COLORREF solid_block_color = RGB(0x09, 0xF7, 0xA0);
constexpr COLORREF line_color = RGB(0x37, 0x9A, 0xE6);

int main(int argc, char** argv) {
  viewport vp("B+", 800, 600, WndProc);
  vp.update();
  bplus_delete(root);
  char cmd[100];
  while (fgets(cmd, 100, stdin)) {
    if (cmd[0] == 'i') {
      size_t i = 0;
      for (i = 0; i < 99; ++i) {
        if (cmd[i] <= '9' && cmd[i] >= '0')
          break;
      }
      int num = atoi(cmd + i);
      mut.lock();
      if (bplus_search(root, num).node) {
        std::cerr << num << " already exists" << std::endl;
      } else {
        root = bplus_insert(root, num);
        total_elem_no++;
      }
      mut.unlock();
      vp.update();
    } else if (cmd[0] == 's') {
      size_t i = 0;
      for (i = 0; i < 99; ++i) {
        if (cmd[i] <= '9' && cmd[i] >= '0')
          break;
      }
      char* end = NULL;
      int num = strtoll(cmd + i, &end, 10);
      if (*end != '\0' && *end != '\n') {
        end++;
        int num2 = strtoll(end, NULL, 10);
        BplusIter begin = bplus_search_not_below(root, num);
        BplusIter end = bplus_search_above(root, num2);
        while (!bplus_iter_equal(begin, end)) {
          std::cout << bplus_iter_get_key(begin) << ' ';
          begin = bplus_iter_next(begin);
        }
        std::cout << std::endl;
      } else {
        if (bplus_search(root, num).node) {
          std::cout << num << " found" << std::endl;
        } else {
          std::cout << num << " not found" << std::endl;
        }
      }
    } else if (cmd[0] == 'd') {
      size_t i = 0;
      for (i = 0; i < 99; ++i) {
        if (cmd[i] <= '9' && cmd[i] >= '0')
          break;
      }
      int num = atoi(cmd + i);
      mut.lock();
      BplusIter iter = bplus_search(root, num);
      if (!iter.node) {
        std::cerr << num << " not found" << std::endl;
      } else {
        root = bplus_erase(root, iter);
        total_elem_no--;
      }
      mut.unlock();
      vp.update();
    } else if (cmd[0] == 'q') {
      mut.lock();
      bplus_delete(root);
      root = NULL;
      mut.unlock();
      break;
    } else {
      std::cerr << "unknown command" << std::endl;
    }
  }


  //std::cerr << "reach return 0" << std::endl;
  return 0;
  //  int num = atoi(argv[1]);
  //  BplusTreeNode treenode;
  //  treenode.nodeptr[BPLUS_DEGREE - 1] = NULL;
  //  treenode.nodeptr[0] = NULL;
  //  for (size_t i = 0; i < BPLUS_ELEM_NO; ++i) {
  //    treenode.elems[i] = i  2;
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

static inline int get_node_width(BplusTreeNode* node) {
  return (node->elem_no * 6 + 1) * textwidth;
}

void draw_number(HDC hdc, int x, int y, int number) {
  TCHAR str[32];
  char str1[32];
  sprintf(str1, "%05d", number);
  TCHAR* ptr = str;
  for (const char* ptr1 = str1; *ptr1 != '\0'; ++ptr1)
    *ptr++ = *ptr1;
  size_t len = ptr - str;
  TextOut(hdc, x + 4, y + 1, str, len);
}

void draw_node(HDC hdc, HBRUSH brush, int x, int y, BplusTreeNode* node) {
  RECT rect;
  rect.top = y;
  rect.left = x;
  rect.bottom = rect.top + textheight;
  rect.right = rect.left + textwidth;
  FillRect(hdc, &rect, brush);
  for (size_t i = 0; i < node->elem_no; ++i) {
    rect.left += textwidth;
    rect.right = rect.left + 5 * textwidth;
    FrameRect(hdc, &rect, brush);
    draw_number(hdc, rect.left, rect.top, node->elems[i]);
    rect.left += 5 * textwidth;
    rect.right = rect.left + textwidth;
    FillRect(hdc, &rect, brush);
  }
}

RECT draw_tree_rec(HDC hdc, HBRUSH brush, int x, int y, int line_from_x, int line_from_y, size_t tree_width, BplusTreeNode* node) {
  size_t node_width = get_node_width(node);
  size_t node_x = x + tree_width / 2 - node_width / 2;
  size_t node_y = y;
  if (node->father) {
    MoveToEx(hdc, line_from_x, line_from_y, NULL);
    LineTo(hdc, x + tree_width / 2, node_y);
  }
  draw_node(hdc, brush, node_x, node_y, node);
  if (!node->nodeptr[0]) {
    RECT rect = { .left = static_cast<SHORT>(node_x), .top = static_cast<SHORT>(node_y),
                  .right = static_cast<SHORT>(node_x + node_width), .bottom = static_cast<SHORT>(node_y + textheight) };
    return rect;
  } else {
    RECT prev_rect = draw_tree_rec(hdc, brush, x + 0 * tree_width / (node->elem_no + 1), node_y + hierarchy_distancce,
                                    node_x + textwidth / 2 + 0 * textwidth * 6, node_y + textheight, tree_width / (node->elem_no + 1), node->nodeptr[0]);
    for (size_t i = 1; i <= node->elem_no; ++i) {
      RECT curr_rect = draw_tree_rec(hdc, brush, x + i * tree_width / (node->elem_no + 1), node_y + hierarchy_distancce,
                                      node_x + textwidth / 2 + i * textwidth * 6, node_y + textheight, tree_width / (node->elem_no + 1), node->nodeptr[i]);
      MoveToEx(hdc, prev_rect.right, prev_rect.top + textheight / 2, NULL);
      LineTo(hdc, curr_rect.left, curr_rect.top + textheight / 2);
      prev_rect.right = curr_rect.right;
    }
    return prev_rect;
  }
}

void clear_window(HWND hwnd, HDC hdc) {
  RECT rect;
  GetClientRect(hwnd, &rect);
  FillRect(hdc, &rect, HBRUSH(COLOR_WINDOW + 1));
}

void draw_tree(HWND hwnd) {
  mut.lock();
  PAINTSTRUCT ps;
  HDC hdc = BeginPaint(hwnd, &ps);
  clear_window(hwnd, hdc);
  if (!root) {
    EndPaint(hwnd, &ps);
    mut.unlock();
    return;
  }
  size_t tree_height = bplus_height(root);
  size_t leaf_number = pow(4, tree_height - 1);
  size_t leaf_width = (BPLUS_ELEM_NO * 6 + 1) * textwidth;
  size_t tree_width = leaf_width * leaf_number;
  HBRUSH brush = CreateSolidBrush(solid_block_color);
  HPEN pen = CreatePen(PS_SOLID, 1, line_color);
  HPEN oldpen = (HPEN)SelectObject(hdc, pen);
  draw_tree_rec(hdc, brush, offset_x - tree_width / 2, offset_y, 0, 0, tree_width, root);
  SelectObject(hdc, oldpen);
  DeleteObject(pen);
  DeleteObject(brush);
  EndPaint(hwnd, &ps);
  mut.unlock();
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
  enum mouse_state { down, up };
  static enum mouse_state mouse = up;
  static int mouse_x = 0;
  static int mouse_y = 0;
  switch (message) {
    case WM_LBUTTONDOWN:
      mouse = down;
      mouse_x = GET_X_LPARAM(lParam);
      mouse_y = GET_Y_LPARAM(lParam);
      break;

    case WM_LBUTTONUP:
      mouse = up;
      break;

    case WM_MOUSEMOVE:
      if (mouse == down) {
        offset_x += GET_X_LPARAM(lParam) - mouse_x;
        offset_y += GET_Y_LPARAM(lParam) - mouse_y;
        mouse_x = GET_X_LPARAM(lParam);
        mouse_y = GET_Y_LPARAM(lParam);
        InvalidateRect(hwnd, NULL, false);
      }
      break;

    case WM_CREATE:
      break;

    case WM_PAINT:
      draw_tree(hwnd);
      break;

    case WM_CLOSE:
      DestroyWindow(hwnd);
      return 0;

    case WM_DESTROY:
      PostQuitMessage(0);
      return 0;
  }

  return DefWindowProc(hwnd, message, wParam, lParam);
}
