#include "visualization.h"
#include <future>
#include <iostream>

static WNDCLASSEX initialize_wndclass(void);

WNDCLASSEX viewport::wndclass_;
static bool wndclass_initialized = false;

void viewport::winmanager(std::string viewport_name, int width, int height, std::promise<HWND>& phwnd) {
  TCHAR* wnd_name = new TCHAR[(viewport_name.size() + 1) * sizeof(TCHAR)];
  for (size_t i = 0; i < viewport_name.size(); ++i)
    wnd_name[i] = viewport_name[i];
  wnd_name[viewport_name.size()] = '\0';
  HWND hwnd = CreateWindowEx(0,
      TEXT("viewport"),
      wnd_name,
      WS_OVERLAPPEDWINDOW,// ^ WS_THICKFRAME,
      CW_USEDEFAULT,
      CW_USEDEFAULT,
      int(width),
      int(height),
      NULL,
      NULL,
      wndclass_.hInstance,
      NULL);
  if (hwnd == NULL) {
    MessageBox(NULL, TEXT("Failed to create a window"), TEXT("ERROR"), MB_ICONERROR);
    exit(EXIT_FAILURE);
  }
  phwnd.set_value(hwnd);
  ShowWindow(hwnd, SW_SHOW);
  UpdateWindow(hwnd);

  MSG msg;
  while (GetMessage(&msg, NULL, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
}

viewport::viewport(const std::string& viewport_name, int width, int height, WndCallBack* WndProc) {
  if (!wndclass_initialized) {
    /* create and register a window class */
    HINSTANCE hInstance = GetModuleHandle(NULL);
    wndclass_.cbSize = sizeof(WNDCLASSEX);
    wndclass_.style = CS_HREDRAW | CS_VREDRAW;
    wndclass_.lpfnWndProc = WndProc;
    wndclass_.cbClsExtra = 0;
    wndclass_.cbWndExtra = 0;
    wndclass_.hInstance = hInstance;
    wndclass_.hIcon = LoadIcon(hInstance, NULL);
    wndclass_.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wndclass_.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wndclass_.lpszMenuName = NULL;
    wndclass_.lpszClassName = TEXT("viewport");
    wndclass_.hIconSm = LoadIcon(wndclass_.hInstance, NULL);
    if (!RegisterClassEx(&wndclass_)) {
      MessageBox(NULL, TEXT("Failed to register a window"), TEXT("ERROR"), MB_ICONERROR);
      exit(EXIT_FAILURE);
    }
    wndclass_initialized = true;
  }
  std::promise<HWND> phwnd;
  win_manager_ = std::thread(winmanager, viewport_name, width, height, std::ref(phwnd));
  auto fut = phwnd.get_future();
  hwnd_ = fut.get();
}
