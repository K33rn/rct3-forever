#pragma once
#include <windows.h>

void log(const char *fmt, ...);

namespace ui {
void init();
bool open();
int toggle_key();
void poll_toggle(HWND game_window);   // edge-detects the toggle key while the game window is in front
void apply_locks();                   // re-writes locked values every frame, menu open or not
void frame(bool toast);
}
