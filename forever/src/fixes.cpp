// Fixes to the game's own behaviour, as opposed to rewrites, which do what the original does. These let a park saved
// with custom content this install doesn't have load instead of crashing. GameFixes=0 in the ini leaves them out.
#include "forever.h"

// Placement::Step(shape owner...): a path piece of a style that isn't installed has no shape (+0xcc of the second
// argument), and the game crashes working out its footprint. The entry number -1 means "none", which clean-up ignores.
// Otherwise the original runs: its first three instructions, then the rest of it.
__attribute__((naked)) static void placement_step_guard() {
    __asm__ volatile(
        "movl 8(%esp), %eax\n"
        "cmpl $0, 0xcc(%eax)\n"
        "je 1f\n"
        "pushl %ebp\n"
        "movl %esp, %ebp\n"
        "andl $-16, %esp\n"
        "movl $0x42e996, %eax\n"
        "jmp *%eax\n"
        "1:\n"
        "orl $-1, %eax\n"
        "ret $0x14\n");
}

// QueueTypeGroundSurface's destructor leaves its queue type's entry (+0x5c), which a surface of a queue type that isn't
// installed never had (see QueueSurface::Activate). Without one it goes on to the rest of the destructor.
__attribute__((naked)) static void queue_surface_leave_guard() {
    __asm__ volatile(
        "movl 0x5c(%esi), %eax\n"
        "testl %eax, %eax\n"
        "je 1f\n"
        "movl 0x16c(%eax), %ecx\n"
        "movl $0x738bf2, %edx\n"
        "jmp *%edx\n"
        "1:\n"
        "popl %edi\n"
        "movl $0x738c17, %edx\n"
        "jmp *%edx\n");
}

void install_fixes() {
    static const uint8_t placement_step_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0};
    if (replace_function("Fix: Placement::Step", 0x42e990, placement_step_original, sizeof placement_step_original,
                         (void *)placement_step_guard))
        flog("game fix in use: Placement::Step without a shape");
    static const uint8_t queue_surface_leave_original[] = {0x8b, 0x46, 0x5c, 0x8b, 0x88, 0x6c, 0x01, 0x00, 0x00};
    if (replace_function("Fix: QueueSurface leaving", 0x738be9, queue_surface_leave_original,
                         sizeof queue_surface_leave_original, (void *)queue_surface_leave_guard))
        flog("game fix in use: queue surface without an entry");
}
