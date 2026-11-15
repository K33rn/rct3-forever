// call_in_place (in_place_call.h): room for the arguments on the stack, filled where they lie, then the call.
#include "in_place_call.h"

extern "C" __attribute__((naked)) uint32_t __cdecl call_in_place(uintptr_t, void *, uint32_t, void(__cdecl *)(uint8_t *, void *), void *) {
    __asm__ volatile(
        "pushl %ebp\n\t"
        "movl %esp, %ebp\n\t"
        "pushl %ebx\n\t"
        "pushl %esi\n\t"
        "pushl %edi\n\t"
        "subl 16(%ebp), %esp\n\t"   // the arguments' room
        "movl %esp, %edi\n\t"
        "pushl 24(%ebp)\n\t"        // fill(area, context)
        "pushl %edi\n\t"
        "calll *20(%ebp)\n\t"
        "addl $8, %esp\n\t"
        "movl 12(%ebp), %ecx\n\t"   // fn(self, the arguments): it takes them off
        "calll *8(%ebp)\n\t"
        "leal -12(%ebp), %esp\n\t"
        "popl %edi\n\t"
        "popl %esi\n\t"
        "popl %ebx\n\t"
        "popl %ebp\n\t"
        "retl\n\t");
}
