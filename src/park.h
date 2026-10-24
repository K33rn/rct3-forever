// Park editor tab: the park's live statistics, its finances and spawning guests (park.cpp).
#pragma once

namespace park {
void tab();           // draws the Park tab (game thread: spawning calls game code)
void apply_locks();   // re-writes locked park values every frame
}
