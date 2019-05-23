#if WIN32
#include "windows.h"
#endif
#include "Common.hpp"

namespace t2
{
  static int g_previous_mouse_x;
  static int g_previous_mouse_y;

  static uint64_t g_timeOfLastHumanInteraction = -1;

#if WIN32
  static HHOOK g_keyboardHook = nullptr;
  static HHOOK g_mouseHook = nullptr;

  LRESULT CALLBACK MouseProc(int nCode,
    WPARAM wParam,
    LPARAM lParam)
  {
    MSLLHOOKSTRUCT* s = (MSLLHOOKSTRUCT*)lParam;

    if (g_previous_mouse_x != s->pt.x || g_previous_mouse_y != s->pt.y)
    {
      g_previous_mouse_x = s->pt.x;
      g_previous_mouse_y = s->pt.y;
      g_timeOfLastHumanInteraction = TimerGet();
    }
    return CallNextHookEx(0, nCode, wParam, lParam);
  }

  LRESULT CALLBACK LowLevelKeyboardProc(int nCode,
    WPARAM wParam,
    LPARAM lParam)
  {
    KBDLLHOOKSTRUCT* s = (KBDLLHOOKSTRUCT*)lParam;
    g_timeOfLastHumanInteraction = TimerGet();
    return CallNextHookEx(0, nCode, wParam, lParam);
  }

  void HumanActivityDetectionInit()
  {
    // Install the low-level keyboard & mouse hooks
    g_mouseHook = SetWindowsHookEx(WH_MOUSE_LL,
      MouseProc,
      GetModuleHandle(0),
      0);

    // Install the low-level keyboard & mouse hooks
    g_keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL,
      LowLevelKeyboardProc,
      GetModuleHandle(0),
      0);
  }

  void HumanActivityDetectionDestroy()
  {
    if (g_mouseHook)
      UnhookWindowsHookEx(g_mouseHook);
    if (g_keyboardHook)
      UnhookWindowsHookEx(g_keyboardHook);
  }

  void PumpOSMessageLoop()
  {
    MSG msg;
    while (PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE))
    {
      //Translate message
      TranslateMessage(&msg);

      //Dispatch message
      DispatchMessage(&msg);
    }
  }
#else
  //dummy implementation that always will say we haven't seen any human activity
  void HumanActivityDetectionInit()
  {
  }
  void HumanActivityDetectionDestroy()
  {
  }
  void PumpOSMessageLoop()
  {
  }
#endif

  double TimeSinceLastDetectedHumanActivityOnMachine()
  {
     if (g_timeOfLastHumanInteraction == -1)
       return -1;
     return TimerDiffSeconds(g_timeOfLastHumanInteraction, TimerGet());
  }
}