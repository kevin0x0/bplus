#include <thread>
#include <future>
#include <iostream>
#include <string>
#include <windows.h>

class viewport {
  private:
  static WNDCLASSEX wndclass_;
  HWND hwnd_;
  std::thread win_manager_;

  using WndCallBack = LRESULT CALLBACK(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

  static void winmanager(std::string viewport_name, int width, int height, std::promise<HWND>& phwnd);

  public:
  viewport(const std::string& viewport_name, int width, int height, WndCallBack* WndProc);
  ~viewport(void) {
    SendMessage(hwnd_, WM_CLOSE, 0, 0);
    win_manager_.join();
  }
  inline HWND hwnd(void) { return hwnd_; };
  inline void update(void) {
    InvalidateRect(hwnd_, NULL, false);
    UpdateWindow(hwnd_);
  };
};
