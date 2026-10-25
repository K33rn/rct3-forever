// Intrusive two-way lists: objects that sit in a list carry a node {prev, next} themselves.
#include "rewrite.h"

struct ListNode {
    ListNode *prev;   // whose next is this node
    ListNode *next;   // whose prev is this node
};

static volatile long g_unlinks;

// Unlink: take the node out of its list and clear it. A node that isn't properly linked both ways means the list
// is broken; the game stops there with a breakpoint (int 3), and so does this.
static void __fastcall unlink(ListNode *node) {
    g_unlinks++;
    if (!node || !node->next || !node->prev || node->prev->next != node || node->next->prev != node)
        __builtin_debugtrap();
    node->prev->next = node->next;
    node->next->prev = node->prev;
    node->prev = nullptr;
    node->next = nullptr;
}

static const uint32_t unlink_copies[] = {
    0x40a890, 0x468370, 0x475e30, 0x50f0d0, 0x5304a0, 0x555dd0, 0x5561e0, 0x79e460,
    0x87f3b0, 0x9659a0, 0xb32a00, 0xbeaad0, 0xbfe520, 0xc06050, 0xc0dd70, 0xc0deb0,
    0xc0dfd0, 0xc0e380, 0xc0e690, 0xc0e9e0, 0xc0edb0, 0xc0f0d0, 0xc0f390, 0xc0f590,
    0xc0f760, 0xc9be80,
};
static const uint8_t unlink_original[] = {0x85, 0xc9, 0x74, 0x2e, 0x8b, 0x51, 0x04, 0x85, 0xd2, 0x74, 0x27, 0x8b,
                                          0x01, 0x85, 0xc0, 0x74};
REWRITE_COPIES("ListNode::Unlink", unlink_copies, unlink_original, unlink, &g_unlinks);
