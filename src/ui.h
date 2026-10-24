#pragma once
#include <windows.h>

void log(const char *fmt, ...);
// world (metres, y up) -> screen pixels with the game's camera of the last frame; false when behind or unknown
bool world_to_screen(float x, float y, float z, float sw, float sh, float *sx, float *sy);

namespace ui {
void init();
bool open();
int toggle_key();
void poll_toggle(HWND game_window);   // edge-detects the toggle key while the game window is in front
void toggle();                        // also called for the key's window message (remote desktop, input tools)
void apply_locks();                   // re-writes locked values every frame, menu open or not
bool overlay_active();                // something to draw in the world (tracked guests)
void frame(bool toast);
}
