/*

    auto-generated file, do not modify!
    libcontext - a slightly more portable version of boost::context
    Copyright Martin Husemann 2013.
    Copyright Oliver Kowalke 2009.
    Copyright Sergue E. Leontiev 2013
    Copyright Thomas Sailer 2013.
    Minor modifications by Tomasz Wlostowski 2016.

 Distributed under the Boost Software License, Version 1.0.
      (See accompanying file LICENSE.BOOSTv1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)

*/
#include <csetjmp>
#include <cstdlib>
#include <libcontext.h>

#if defined(LIBCONTEXT_PLATFORM_windows_i386) && defined(LIBCONTEXT_COMPILER_gcc)
__asm (
".text\n"
".p2align 4,,15\n"
".globl	_jump_fcontext\n"
".def	_jump_fcontext;	.scl	2;	.type	32;	.endef\n"
"_jump_fcontext:\n"
"    mov    0x10(%esp),%ecx\n"
"    push   %ebp\n"
"    push   %ebx\n"
"    push   %esi\n"
"    push   %edi\n"
"    mov    %fs:0x18,%edx\n"
"    mov    (%edx),%eax\n"
"    push   %eax\n"
"    mov    0x4(%edx),%eax\n"
"    push   %eax\n"
"    mov    0x8(%edx),%eax\n"
"    push   %eax\n"
"    mov    0xe0c(%edx),%eax\n"
"    push   %eax\n"
"    mov    0x10(%edx),%eax\n"
"    push   %eax\n"
"    lea    -0x8(%esp),%esp\n"
"    test   %ecx,%ecx\n"
"    je     nxt1\n"
"    stmxcsr (%esp)\n"
"    fnstcw 0x4(%esp)\n"
"nxt1:\n"
"    mov    0x30(%esp),%eax\n"
"    mov    %esp,(%eax)\n"
"    mov    0x34(%esp),%edx\n"
"    mov    0x38(%esp),%eax\n"
"    mov    %edx,%esp\n"
"    test   %ecx,%ecx\n"
"    je     nxt2\n"
"    ldmxcsr (%esp)\n"
"    fldcw  0x4(%esp)\n"
"nxt2:\n"
"    lea    0x8(%esp),%esp\n"
"    mov    %fs:0x18,%edx\n"
"    pop    %ecx\n"
"    mov    %ecx,0x10(%edx)\n"
"    pop    %ecx\n"
"    mov    %ecx,0xe0c(%edx)\n"
"    pop    %ecx\n"
"    mov    %ecx,0x8(%edx)\n"
"    pop    %ecx\n"
"    mov    %ecx,0x4(%edx)\n"
"    pop    %ecx\n"
"    mov    %ecx,(%edx)\n"
"    pop    %edi\n"
"    pop    %esi\n"
"    pop    %ebx\n"
"    pop    %ebp\n"
"    pop    %edx\n"
"    mov    %eax,0x4(%esp)\n"
"    jmp    *%edx\n"
);

#endif

#if defined(LIBCONTEXT_PLATFORM_windows_i386) && defined(LIBCONTEXT_COMPILER_gcc)
__asm (
".text\n"
".p2align 4,,15\n"
".globl	_make_fcontext\n"
".def	_make_fcontext;	.scl	2;	.type	32;	.endef\n"
"_make_fcontext:\n"
"mov    0x4(%esp),%eax\n"
"lea    -0x8(%eax),%eax\n"
"and    $0xfffffff0,%eax\n"
"lea    -0x3c(%eax),%eax\n"
"mov    0x4(%esp),%ecx\n"
"mov    %ecx,0x14(%eax)\n"
"mov    0x8(%esp),%edx\n"
"neg    %edx\n"
"lea    (%ecx,%edx,1),%ecx\n"
"mov    %ecx,0x10(%eax)\n"
"mov    %ecx,0xc(%eax)\n"
"mov    0xc(%esp),%ecx\n"
"mov    %ecx,0x2c(%eax)\n"
"stmxcsr (%eax)\n"
"fnstcw 0x4(%eax)\n"
"mov    $finish,%ecx\n"
"mov    %ecx,0x30(%eax)\n"
"mov    %fs:0x0,%ecx\n"
"walk:\n"
"mov    (%ecx),%edx\n"
"inc    %edx\n"
"je     found\n"
"dec    %edx\n"
"xchg   %edx,%ecx\n"
"jmp    walk\n"
"found:\n"
"mov    0x4(%ecx),%ecx\n"
"mov    %ecx,0x3c(%eax)\n"
"mov    $0xffffffff,%ecx\n"
"mov    %ecx,0x38(%eax)\n"
"lea    0x38(%eax),%ecx\n"
"mov    %ecx,0x18(%eax)\n"
"ret\n"
"finish:\n"
"xor    %eax,%eax\n"
"mov    %eax,(%esp)\n"
"call   _exit\n"
"hlt\n"
".def	__exit;	.scl	2;	.type	32;	.endef  \n"
);

#endif

#if defined(LIBCONTEXT_PLATFORM_windows_x86_64) && defined(LIBCONTEXT_COMPILER_gcc)
__asm (
".text\n"
".p2align 4,,15\n"
".globl	jump_fcontext\n"
".def	jump_fcontext;	.scl	2;	.type	32;	.endef\n"
".seh_proc	jump_fcontext\n"
"jump_fcontext:\n"
".seh_endprologue\n"
"	push   %rbp\n"
"	push   %rbx\n"
"	push   %rsi\n"
"	push   %rdi\n"
"	push   %r15\n"
"	push   %r14\n"
"	push   %r13\n"
"	push   %r12\n"
"	mov    %gs:0x30,%r10\n"
"	mov    0x8(%r10),%rax\n"
"	push   %rax\n"
"	mov    0x10(%r10),%rax\n"
"	push   %rax\n"
"	mov    0x1478(%r10),%rax\n"
"	push   %rax\n"
"	mov    0x18(%r10),%rax\n"
"	push   %rax\n"
"	lea    -0xa8(%rsp),%rsp\n"
"	test   %r9,%r9\n"
"	je     nxt1\n"
"	stmxcsr 0xa0(%rsp)\n"
"	fnstcw 0xa4(%rsp)\n"
"	movaps %xmm6,(%rsp)\n"
"	movaps %xmm7,0x10(%rsp)\n"
"	movaps %xmm8,0x20(%rsp)\n"
"	movaps %xmm9,0x30(%rsp)\n"
"	movaps %xmm10,0x40(%rsp)\n"
"	movaps %xmm11,0x50(%rsp)\n"
"	movaps %xmm12,0x60(%rsp)\n"
"	movaps %xmm13,0x70(%rsp)\n"
"	movaps %xmm14,0x80(%rsp)\n"
"	movaps %xmm15,0x90(%rsp)\n"
"nxt1:\n"
"	xor    %r10,%r10\n"
"	push   %r10\n"
"	mov    %rsp,(%rcx)\n"
"	mov    %rdx,%rsp\n"
"	pop    %r10\n"
"	test   %r9,%r9\n"
"	je     nxt2\n"
"	ldmxcsr 0xa0(%rsp)\n"
"	fldcw  0xa4(%rsp)\n"
"	movaps (%rsp),%xmm6\n"
"	movaps 0x10(%rsp),%xmm7\n"
"	movaps 0x20(%rsp),%xmm8\n"
"	movaps 0x30(%rsp),%xmm9\n"
"	movaps 0x40(%rsp),%xmm10\n"
"	movaps 0x50(%rsp),%xmm11\n"
"	movaps 0x60(%rsp),%xmm12\n"
"	movaps 0x70(%rsp),%xmm13\n"
"	movaps 0x80(%rsp),%xmm14\n"
"	movaps 0x90(%rsp),%xmm15\n"
"nxt2:\n"
"	mov    $0xa8,%rcx\n"
"    test   %r10,%r10\n"
"    je     nxt3\n"
"    add    $0x8,%rcx\n"
"nxt3:\n"
"	lea    (%rsp,%rcx,1),%rsp\n"
"	mov    %gs:0x30,%r10\n"
"	pop    %rax\n"
"	mov    %rax,0x18(%r10)\n"
"	pop    %rax\n"
"	mov    %rax,0x1478(%r10)\n"
"	pop    %rax\n"
"	mov    %rax,0x10(%r10)\n"
"	pop    %rax\n"
"	mov    %rax,0x8(%r10)\n"
"	pop    %r12\n"
"	pop    %r13\n"
"	pop    %r14\n"
"	pop    %r15\n"
"	pop    %rdi\n"
"	pop    %rsi\n"
"	pop    %rbx\n"
"	pop    %rbp\n"
"	pop    %r10\n"
"	mov    %r8,%rax\n"
"	mov    %r8,%rcx\n"
"	jmpq   *%r10\n"
".seh_endproc\n"
);

#endif

#if defined(LIBCONTEXT_PLATFORM_windows_x86_64) && defined(LIBCONTEXT_COMPILER_gcc)
__asm (
".text\n"
".p2align 4,,15\n"
".globl	make_fcontext\n"
".def	make_fcontext;	.scl	2;	.type	32;	.endef\n"
".seh_proc	make_fcontext\n"
"make_fcontext:\n"
".seh_endprologue\n"
"mov    %rcx,%rax\n"
"sub    $0x28,%rax\n"
"and    $0xfffffffffffffff0,%rax\n"
"sub    $0x128,%rax\n"
"mov    %r8,0x118(%rax)\n"
"mov    %rcx,0xd0(%rax)\n"
"neg    %rdx\n"
"lea    (%rcx,%rdx,1),%rcx\n"
"mov    %rcx,0xc8(%rax)\n"
"mov    %rcx,0xc0(%rax)\n"
"stmxcsr 0xa8(%rax)\n"
"fnstcw 0xac(%rax)\n"
"leaq  finish(%rip), %rcx\n"
"mov    %rcx,0x120(%rax)\n"
"mov    $0x1,%rcx\n"
"mov    %rcx,(%rax)\n"
"retq\n"
"finish:\n"
"xor    %rcx,%rcx\n"
"callq  0x63\n"
"hlt\n"
"   .seh_endproc\n"
".def	_exit;	.scl	2;	.type	32;	.endef  \n"
);

#endif

#if defined(LIBCONTEXT_PLATFORM_linux_i386) && defined(LIBCONTEXT_COMPILER_gcc)
__asm (
".text\n"
".globl jump_fcontext\n"
".align 2\n"
".type jump_fcontext,@function\n"
"jump_fcontext:\n"
"    movl  0x10(%esp), %ecx\n"
"    pushl  %ebp  \n"
"    pushl  %ebx  \n"
"    pushl  %esi  \n"
"    pushl  %edi  \n"
"    leal  -0x8(%esp), %esp\n"
"    test  %ecx, %ecx\n"
"    je  1f\n"
"    stmxcsr  (%esp)\n"
"    fnstcw  0x4(%esp)\n"
"1:\n"
"    movl  0x1c(%esp), %eax\n"
"    movl  %esp, (%eax)\n"
"    movl  0x20(%esp), %edx\n"
"    movl  0x24(%esp), %eax\n"
"    movl  %edx, %esp\n"
"    test  %ecx, %ecx\n"
"    je  2f\n"
"    ldmxcsr  (%esp)\n"
"    fldcw  0x4(%esp)\n"
"2:\n"
"    leal  0x8(%esp), %esp\n"
"    popl  %edi  \n"
"    popl  %esi  \n"
"    popl  %ebx  \n"
"    popl  %ebp  \n"
"    popl  %edx\n"
"    movl  %eax, 0x4(%esp)\n"
"    jmp  *%edx\n"
".size jump_fcontext,.-jump_fcontext\n"
);

#endif

#if defined(LIBCONTEXT_PLATFORM_linux_i386) && defined(LIBCONTEXT_COMPILER_gcc)
__asm (
".text\n"
".globl make_fcontext\n"
".align 2\n"
".type make_fcontext,@function\n"
"make_fcontext:\n"
"    movl  0x4(%esp), %eax\n"
"    leal  -0x8(%eax), %eax\n"
"    andl  $-16, %eax\n"
"    leal  -0x20(%eax), %eax\n"
"    movl  0xc(%esp), %edx\n"
"    movl  %edx, 0x18(%eax)\n"
"    stmxcsr  (%eax)\n"
"    fnstcw  0x4(%eax)\n"
"    call  1f\n"
"1:  popl  %ecx\n"
"    addl  $finish-1b, %ecx\n"
"    movl  %ecx, 0x1c(%eax)\n"
"    ret \n"
"finish:\n"
"    call  2f\n"
"2:  popl  %ebx\n"
"    addl  $_GLOBAL_OFFSET_TABLE_+[.-2b], %ebx\n"
"    xorl  %eax, %eax\n"
"    movl  %eax, (%esp)\n"
"    call  _exit@PLT\n"
"    hlt\n"
".size make_fcontext,.-make_fcontext\n"
);

#endif

#if defined(LIBCONTEXT_PLATFORM_linux_x86_64) && defined(LIBCONTEXT_COMPILER_gcc)
__asm (
".text\n"
".globl jump_fcontext\n"
".type jump_fcontext,@function\n"
".align 16\n"
"jump_fcontext:\n"
"    pushq  %rbp  \n"
"    pushq  %rbx  \n"
"    pushq  %r15  \n"
"    pushq  %r14  \n"
"    pushq  %r13  \n"
"    pushq  %r12  \n"
"    leaq  -0x8(%rsp), %rsp\n"
"    cmp  $0, %rcx\n"
"    je  1f\n"
"    stmxcsr  (%rsp)\n"
"    fnstcw   0x4(%rsp)\n"
"1:\n"
"    movq  %rsp, (%rdi)\n"
"    movq  %rsi, %rsp\n"
"    cmp  $0, %rcx\n"
"    je  2f\n"
"    ldmxcsr  (%rsp)\n"
"    fldcw  0x4(%rsp)\n"
"2:\n"
"    leaq  0x8(%rsp), %rsp\n"
"    popq  %r12  \n"
"    popq  %r13  \n"
"    popq  %r14  \n"
"    popq  %r15  \n"
"    popq  %rbx  \n"
"    popq  %rbp  \n"
"    popq  %r8\n"
"    movq  %rdx, %rax\n"
"    movq  %rdx, %rdi\n"
"    jmp  *%r8\n"
".size jump_fcontext,.-jump_fcontext\n"
);

#endif

#if defined(LIBCONTEXT_PLATFORM_linux_x86_64) && defined(LIBCONTEXT_COMPILER_gcc)
__asm (
".text\n"
".globl make_fcontext\n"
".type make_fcontext,@function\n"
".align 16\n"
"make_fcontext:\n"
"    movq  %rdi, %rax\n"
"    andq  $-16, %rax\n"
"    leaq  -0x48(%rax), %rax\n"
"    movq  %rdx, 0x38(%rax)\n"
"    stmxcsr  (%rax)\n"
"    fnstcw   0x4(%rax)\n"
"    leaq  finish(%rip), %rcx\n"
"    movq  %rcx, 0x40(%rax)\n"
"    ret \n"
"finish:\n"
"    xorq  %rdi, %rdi\n"
"    call  _exit@PLT\n"
"    hlt\n"
".size make_fcontext,.-make_fcontext\n"
);

#endif

#if defined(LIBCONTEXT_PLATFORM_apple_x86_64) && defined(LIBCONTEXT_COMPILER_gcc)
__asm (
".text\n"
".globl _jump_fcontext\n"
".align 8\n"
"_jump_fcontext:\n"
"    pushq  %rbp  \n"
"    pushq  %rbx  \n"
"    pushq  %r15  \n"
"    pushq  %r14  \n"
"    pushq  %r13  \n"
"    pushq  %r12  \n"
"    leaq  -0x8(%rsp), %rsp\n"
"    cmp  $0, %rcx\n"
"    je  1f\n"
"    stmxcsr  (%rsp)\n"
"    fnstcw   0x4(%rsp)\n"
"1:\n"
"    movq  %rsp, (%rdi)\n"
"    movq  %rsi, %rsp\n"
"    cmp  $0, %rcx\n"
"    je  2f\n"
"    ldmxcsr  (%rsp)\n"
"    fldcw  0x4(%rsp)\n"
"2:\n"
"    leaq  0x8(%rsp), %rsp\n"
"    popq  %r12  \n"
"    popq  %r13  \n"
"    popq  %r14  \n"
"    popq  %r15  \n"
"    popq  %rbx  \n"
"    popq  %rbp  \n"
"    popq  %r8\n"
"    movq  %rdx, %rax\n"
"    movq  %rdx, %rdi\n"
"    jmp  *%r8\n"
);

#endif

#if defined(LIBCONTEXT_PLATFORM_apple_x86_64) && defined(LIBCONTEXT_COMPILER_gcc)
__asm (
".text\n"
".globl _make_fcontext\n"
".align 8\n"
"_make_fcontext:\n"
"    movq  %rdi, %rax\n"
"    movabs  $-16,           %r8\n"
"    andq    %r8,            %rax\n"
"    leaq  -0x48(%rax), %rax\n"
"    movq  %rdx, 0x38(%rax)\n"
"    stmxcsr  (%rax)\n"
"    fnstcw   0x4(%rax)\n"
"    leaq  finish(%rip), %rcx\n"
"    movq  %rcx, 0x40(%rax)\n"
"    ret \n"
"finish:\n"
"    xorq  %rdi, %rdi\n"
"    call  __exit\n"
"    hlt\n"
);

#endif

#if defined(LIBCONTEXT_PLATFORM_apple_i386) && defined(LIBCONTEXT_COMPILER_gcc)
__asm (
".text\n"
".globl _jump_fcontext\n"
".align 2\n"
"_jump_fcontext:\n"
"    movl  0x10(%esp), %ecx\n"
"    pushl  %ebp  \n"
"    pushl  %ebx  \n"
"    pushl  %esi  \n"
"    pushl  %edi  \n"
"    leal  -0x8(%esp), %esp\n"
"    test  %ecx, %ecx\n"
"    je  1f\n"
"    stmxcsr  (%esp)\n"
"    fnstcw  0x4(%esp)\n"
"1:\n"
"    movl  0x1c(%esp), %eax\n"
"    movl  %esp, (%eax)\n"
"    movl  0x20(%esp), %edx\n"
"    movl  0x24(%esp), %eax\n"
"    movl  %edx, %esp\n"
"    test  %ecx, %ecx\n"
"    je  2f\n"
"    ldmxcsr  (%esp)\n"
"    fldcw  0x4(%esp)\n"
"2:\n"
"    leal  0x8(%esp), %esp\n"
"    popl  %edi  \n"
"    popl  %esi  \n"
"    popl  %ebx  \n"
"    popl  %ebp  \n"
"    popl  %edx\n"
"    movl  %eax, 0x4(%esp)\n"
"    jmp  *%edx\n"
);

#endif

#if defined(LIBCONTEXT_PLATFORM_apple_i386) && defined(LIBCONTEXT_COMPILER_gcc)
__asm (
".text\n"
".globl _make_fcontext\n"
".align 2\n"
"_make_fcontext:\n"
"    movl  0x4(%esp), %eax\n"
"    leal  -0x8(%eax), %eax\n"
"    andl  $-16, %eax\n"
"    leal  -0x20(%eax), %eax\n"
"    movl  0xc(%esp), %edx\n"
"    movl  %edx, 0x18(%eax)\n"
"    stmxcsr  (%eax)\n"
"    fnstcw  0x4(%eax)\n"
"    call  1f\n"
"1:  popl  %ecx\n"
"    addl  $finish-1b, %ecx\n"
"    movl  %ecx, 0x1c(%eax)\n"
"    ret \n"
"finish:\n"
"    xorl  %eax, %eax\n"
"    movl  %eax, (%esp)\n"
"    call  __exit\n"
"    hlt\n"
);

#endif

#if defined(LIBCONTEXT_PLATFORM_linux_arm32) && defined(LIBCONTEXT_COMPILER_gcc)
__asm (
".text\n"
".globl jump_fcontext\n"
".align 2\n"
".type jump_fcontext,%function\n"
"jump_fcontext:\n"
"    @ save LR as PC\n"
"    push {lr}\n"
"    @ save V1-V8,LR\n"
"    push {v1-v8,lr}\n"
"    @ prepare stack for FPU\n"
"    sub  sp, sp, #64\n"
"    @ test if fpu env should be preserved\n"
"    cmp  a4, #0\n"
"    beq  1f\n"
"    @ save S16-S31\n"
"    vstmia  sp, {d8-d15}\n"
"1:\n"
"    @ store RSP (pointing to context-data) in A1\n"
"    str  sp, [a1]\n"
"    @ restore RSP (pointing to context-data) from A2\n"
"    mov  sp, a2\n"
"    @ test if fpu env should be preserved\n"
"    cmp  a4, #0\n"
"    beq  2f\n"
"    @ restore S16-S31\n"
"    vldmia  sp, {d8-d15}\n"
"2:\n"
"    @ prepare stack for FPU\n"
"    add  sp, sp, #64\n"
"    @ use third arg as return value after jump\n"
"    @ and as first arg in context function\n"
"    mov  a1, a3\n"
"    @ restore v1-V8,LR,PC\n"
"    pop {v1-v8,lr}\n"
"    pop {pc}\n"
".size jump_fcontext,.-jump_fcontext\n"
);

#endif

#if defined(LIBCONTEXT_PLATFORM_linux_arm32) && defined(LIBCONTEXT_COMPILER_gcc)
__asm (
".text\n"
".globl make_fcontext\n"
".align 2\n"
".type make_fcontext,%function\n"
"make_fcontext:\n"
"    @ shift address in A1 to lower 16 byte boundary\n"
"    bic  a1, a1, #15\n"
"    @ reserve space for context-data on context-stack\n"
"    sub  a1, a1, #104\n"
"    @ third arg of make_fcontext() == address of context-function\n"
"    str  a3, [a1,#100]\n"
"    @ compute abs address of label finish\n"
"    adr  a2, finish\n"
"    @ save address of finish as return-address for context-function\n"
"    @ will be entered after context-function returns\n"
"    str  a2, [a1,#96]\n"
"    bx  lr @ return pointer to context-data\n"
"finish:\n"
"    @ exit code is zero\n"
"    mov  a1, #0\n"
"    @ exit application\n"
"    bl  _exit@PLT\n"
".size make_fcontext,.-make_fcontext\n"
);

#endif

#if defined(LIBCONTEXT_PLATFORM_linux_arm64) && defined(LIBCONTEXT_COMPILER_gcc)
__asm (
".cpu    generic+fp+simd\n"
".text\n"
".align  2\n"
".global jump_fcontext\n"
".type   jump_fcontext, %function\n"
"jump_fcontext:\n"
"    # prepare stack for GP + FPU\n"
"    sub  sp, sp, #0xb0\n"
"# Because gcc may save integer registers in fp registers across a\n"
"# function call we cannot skip saving the fp registers.\n"
"#\n"
"# Do not reinstate this test unless you fully understand what you\n"
"# are doing.\n"
"#\n"
"#    # test if fpu env should be preserved\n"
"#    cmp  w3, #0\n"
"#    b.eq  1f\n"
"    # save d8 - d15\n"
"    stp  d8,  d9,  [sp, #0x00]\n"
"    stp  d10, d11, [sp, #0x10]\n"
"    stp  d12, d13, [sp, #0x20]\n"
"    stp  d14, d15, [sp, #0x30]\n"
"1:\n"
"    # save x19-x30\n"
"    stp  x19, x20, [sp, #0x40]\n"
"    stp  x21, x22, [sp, #0x50]\n"
"    stp  x23, x24, [sp, #0x60]\n"
"    stp  x25, x26, [sp, #0x70]\n"
"    stp  x27, x28, [sp, #0x80]\n"
"    stp  x29, x30, [sp, #0x90]\n"
"    # save LR as PC\n"
"    str  x30, [sp, #0xa0]\n"
"    # store RSP (pointing to context-data) in first argument (x0).\n"
"    # STR cannot have sp as a target register\n"
"    mov  x4, sp\n"
"    str  x4, [x0]\n"
"    # restore RSP (pointing to context-data) from A2 (x1)\n"
"    mov  sp, x1\n"
"#    # test if fpu env should be preserved\n"
"#    cmp  w3, #0\n"
"#    b.eq  2f\n"
"    # load d8 - d15\n"
"    ldp  d8,  d9,  [sp, #0x00]\n"
"    ldp  d10, d11, [sp, #0x10]\n"
"    ldp  d12, d13, [sp, #0x20]\n"
"    ldp  d14, d15, [sp, #0x30]\n"
"2:\n"
"    # load x19-x30\n"
"    ldp  x19, x20, [sp, #0x40]\n"
"    ldp  x21, x22, [sp, #0x50]\n"
"    ldp  x23, x24, [sp, #0x60]\n"
"    ldp  x25, x26, [sp, #0x70]\n"
"    ldp  x27, x28, [sp, #0x80]\n"
"    ldp  x29, x30, [sp, #0x90]\n"
"    # use third arg as return value after jump\n"
"    # and as first arg in context function\n"
"    mov  x0, x2\n"
"    # load pc\n"
"    ldr  x4, [sp, #0xa0]\n"
"    # restore stack from GP + FPU\n"
"    add  sp, sp, #0xb0\n"
"    ret x4\n"
".size   jump_fcontext,.-jump_fcontext\n"
);

#endif

#if defined(LIBCONTEXT_PLATFORM_linux_arm64) && defined(LIBCONTEXT_COMPILER_gcc)
__asm (
".cpu    generic+fp+simd\n"
".text\n"
".align  2\n"
".global make_fcontext\n"
".type   make_fcontext, %function\n"
"make_fcontext:\n"
"    # shift address in x0 (allocated stack) to lower 16 byte boundary\n"
"    and x0, x0, ~0xF\n"
"    # reserve space for context-data on context-stack\n"
"    sub  x0, x0, #0xb0\n"
"    # third arg of make_fcontext() == address of context-function\n"
"    # store address as a PC to jump in\n"
"    str  x2, [x0, #0xa0]\n"
"    # save address of finish as return-address for context-function\n"
"    # will be entered after context-function returns (LR register)\n"
"    adr  x1, finish\n"
"    str  x1, [x0, #0x98]\n"
"    ret  x30 \n"
"finish:\n"
"    # exit code is zero\n"
"    mov  x0, #0\n"
"    # exit application\n"
"    bl  _exit\n"
".size   make_fcontext,.-make_fcontext\n"
);

#endif

#if defined(LIBCONTEXT_PLATFORM_linux_mips_n64) && defined(LIBCONTEXT_COMPILER_gcc)
__asm (
".text\n"
".globl jump_fcontext\n"
".align 2\n"
".set noreorder\n"
".type jump_fcontext,@function\n"
".ent jump_fcontext\n"
"jump_fcontext:\n"
"    # reserve space on stack\n"
"    daddiu $sp, $sp, -176\n"
"    sd  $s0, 64($sp)  # save S0\n"
"    sd  $s1, 72($sp)  # save S1\n"
"    sd  $s2, 80($sp)  # save S2\n"
"    sd  $s3, 88($sp)  # save S3\n"
"    sd  $s4, 96($sp)  # save S4\n"
"    sd  $s5, 104($sp) # save S5\n"
"    sd  $s6, 112($sp) # save S6\n"
"    sd  $s7, 120($sp) # save S7\n"
"    sd  $fp, 128($sp) # save FP\n"
"    sd  $ra, 144($sp) # save RA\n"
"    sd  $ra, 152($sp) # save RA as PC\n"
"    s.d  $f24, 0($sp)   # save F24\n"
"    s.d  $f25, 8($sp)   # save F25\n"
"    s.d  $f26, 16($sp)  # save F26\n"
"    s.d  $f27, 24($sp)  # save F27\n"
"    s.d  $f28, 32($sp)  # save F28\n"
"    s.d  $f29, 40($sp)  # save F29\n"
"    s.d  $f30, 48($sp)  # save F30\n"
"    s.d  $f31, 56($sp)  # save F31\n"
"    # store SP (pointing to old context-data) in pointer a0(first arg)\n"
"    sd  $sp, 0($a0)\n"
"    # get SP (pointing to new context-data) from a1 param\n"
"    move  $sp, $a1\n"
"    l.d  $f24, 0($sp)   # restore F24\n"
"    l.d  $f25, 8($sp)   # restore F25\n"
"    l.d  $f26, 16($sp)  # restore F26\n"
"    l.d  $f27, 24($sp)  # restore F27\n"
"    l.d  $f28, 32($sp)  # restore F28\n"
"    l.d  $f29, 40($sp)  # restore F29\n"
"    l.d  $f30, 48($sp)  # restore F30\n"
"    l.d  $f31, 56($sp)  # restore F31\n"
"    ld  $s0, 64($sp)  # restore S0\n"
"    ld  $s1, 72($sp)  # restore S1\n"
"    ld  $s2, 80($sp)  # restore S2\n"
"    ld  $s3, 88($sp)  # restore S3\n"
"    ld  $s4, 96($sp)  # restore S4\n"
"    ld  $s5, 104($sp) # restore S5\n"
"    ld  $s6, 112($sp) # restore S6\n"
"    ld  $s7, 120($sp) # restore S7\n"
"    ld  $fp, 128($sp) # restore FP\n"
"    ld  $ra, 144($sp) # restore RA\n"
"    # load PC\n"
"    ld  $t9, 152($sp)\n"
"    sd  $a2, 160($sp)\n"
"    # adjust stack\n"
"    daddiu $sp, $sp, 176\n"
"    move  $a0, $a2 # move *data from a2 to a0 as param\n"
"    move  $v0, $a2 # move *data from a2 to v0 as return\n"
"    # jump to context\n"
"    jr  $t9\n"
"    nop\n"
".end jump_fcontext\n"
".size jump_fcontext, .-jump_fcontext\n"
);

#endif

#if defined(LIBCONTEXT_PLATFORM_linux_mips_n64) && defined(LIBCONTEXT_COMPILER_gcc)
__asm (
".text\n"
".globl make_fcontext\n"
".align 2\n"
".set noreorder\n"
".type make_fcontext,@function\n"
".ent make_fcontext\n"
"make_fcontext:\n"
"#ifdef __PIC__\n"
".set    noreorder\n"
".cpload $t9\n"
".set    reorder\n"
"#endif\n"
"    # shift address in A0 to lower 16 byte boundary\n"
"    li $v1, 0xfffffffffffffff0\n"
"    and $v0, $v1, $a0\n"
"    # reserve space for context-data on context-stack\n"
"    daddiu $v0, $v0, -176\n"
"    # third arg of make_fcontext() == address of context-function\n"
"    sd  $a2, 152($v0)\n"
"    # save global pointer in context-data\n"
"    sd  $gp, 136($v0)\n"
"    # psudo instruction compute abs address of label finish based on GP\n"
"    dla  $t9, finish\n"
"    # save address of finish as return-address for context-function\n"
"    # will be entered after context-function returns\n"
"    sd  $t9, 144($v0)\n"
"    jr  $ra # return pointer to context-data\n"
"    nop\n"
"finish:\n"
"    # reload our gp register (needed for la)\n"
"    daddiu $t0, $sp, -176\n"
"    ld $gp, 136($t0)\n"
"    ld $v0, 160($t0)\n"
"    # call _exit(0)\n"
"    dla $t9, _exit\n"
"    move $a0, $zero\n"
"    jr $t9\n"
"    nop\n"
".end make_fcontext\n"
".size make_fcontext, .-make_fcontext\n"
);

#endif

#if defined(LIBCONTEXT_PLATFORM_linux_ppc32) && defined(LIBCONTEXT_COMPILER_gcc)
__asm (
".text\n"
".globl jump_fcontext\n"
".align 2\n"
".type jump_fcontext,@function\n"
"jump_fcontext:\n"
"    # reserve space on stack\n"
"    subi  %r1, %r1, 240\n"
"    stw  %r13, 152(%r1)  # save R13\n"
"    stw  %r14, 156(%r1)  # save R14\n"
"    stw  %r15, 160(%r1)  # save R15\n"
"    stw  %r16, 164(%r1)  # save R16\n"
"    stw  %r17, 168(%r1)  # save R17\n"
"    stw  %r18, 172(%r1)  # save R18\n"
"    stw  %r19, 176(%r1)  # save R19\n"
"    stw  %r20, 180(%r1)  # save R20\n"
"    stw  %r21, 184(%r1)  # save R21\n"
"    stw  %r22, 188(%r1)  # save R22\n"
"    stw  %r23, 192(%r1)  # save R23\n"
"    stw  %r24, 196(%r1)  # save R24\n"
"    stw  %r25, 200(%r1)  # save R25\n"
"    stw  %r26, 204(%r1)  # save R26\n"
"    stw  %r27, 208(%r1)  # save R27\n"
"    stw  %r28, 212(%r1)  # save R28\n"
"    stw  %r29, 216(%r1)  # save R29\n"
"    stw  %r30, 220(%r1)  # save R30\n"
"    stw  %r31, 224(%r1)  # save R31\n"
"    # save CR\n"
"    mfcr  %r0\n"
"    stw  %r0, 228(%r1)\n"
"    # save LR\n"
"    mflr  %r0\n"
"    stw  %r0, 232(%r1)\n"
"    # save LR as PC\n"
"    stw  %r0, 236(%r1)\n"
"    # test if fpu env should be preserved\n"
"    cmpwi  cr7, %r6, 0\n"
"    beq  cr7, 1f\n"
"    stfd  %f14, 0(%r1)  # save F14\n"
"    stfd  %f15, 8(%r1)  # save F15\n"
"    stfd  %f16, 16(%r1)  # save F16\n"
"    stfd  %f17, 24(%r1)  # save F17\n"
"    stfd  %f18, 32(%r1)  # save F18\n"
"    stfd  %f19, 40(%r1)  # save F19\n"
"    stfd  %f20, 48(%r1)  # save F20\n"
"    stfd  %f21, 56(%r1)  # save F21\n"
"    stfd  %f22, 64(%r1)  # save F22\n"
"    stfd  %f23, 72(%r1)  # save F23\n"
"    stfd  %f24, 80(%r1)  # save F24\n"
"    stfd  %f25, 88(%r1)  # save F25\n"
"    stfd  %f26, 96(%r1)  # save F26\n"
"    stfd  %f27, 104(%r1)  # save F27\n"
"    stfd  %f28, 112(%r1)  # save F28\n"
"    stfd  %f29, 120(%r1)  # save F29\n"
"    stfd  %f30, 128(%r1)  # save F30\n"
"    stfd  %f31, 136(%r1)  # save F31\n"
"    mffs  %f0  # load FPSCR\n"
"    stfd  %f0, 144(%r1)  # save FPSCR\n"
"1:\n"
"    # store RSP (pointing to context-data) in R3\n"
"    stw  %r1, 0(%r3)\n"
"    # restore RSP (pointing to context-data) from R4\n"
"    mr  %r1, %r4\n"
"    # test if fpu env should be preserved\n"
"    cmpwi  cr7, %r6, 0\n"
"    beq  cr7, 2f\n"
"    lfd  %f14, 0(%r1)  # restore F14\n"
"    lfd  %f15, 8(%r1)  # restore F15\n"
"    lfd  %f16, 16(%r1)  # restore F16\n"
"    lfd  %f17, 24(%r1)  # restore F17\n"
"    lfd  %f18, 32(%r1)  # restore F18\n"
"    lfd  %f19, 40(%r1)  # restore F19\n"
"    lfd  %f20, 48(%r1)  # restore F20\n"
"    lfd  %f21, 56(%r1)  # restore F21\n"
"    lfd  %f22, 64(%r1)  # restore F22\n"
"    lfd  %f23, 72(%r1)  # restore F23\n"
"    lfd  %f24, 80(%r1)  # restore F24\n"
"    lfd  %f25, 88(%r1)  # restore F25\n"
"    lfd  %f26, 96(%r1)  # restore F26\n"
"    lfd  %f27, 104(%r1)  # restore F27\n"
"    lfd  %f28, 112(%r1)  # restore F28\n"
"    lfd  %f29, 120(%r1)  # restore F29\n"
"    lfd  %f30, 128(%r1)  # restore F30\n"
"    lfd  %f31, 136(%r1)  # restore F31\n"
"    lfd  %f0,  144(%r1)  # load FPSCR\n"
"    mtfsf  0xff, %f0  # restore FPSCR\n"
"2:\n"
"    lwz  %r13, 152(%r1)  # restore R13\n"
"    lwz  %r14, 156(%r1)  # restore R14\n"
"    lwz  %r15, 160(%r1)  # restore R15\n"
"    lwz  %r16, 164(%r1)  # restore R16\n"
"    lwz  %r17, 168(%r1)  # restore R17\n"
"    lwz  %r18, 172(%r1)  # restore R18\n"
"    lwz  %r19, 176(%r1)  # restore R19\n"
"    lwz  %r20, 180(%r1)  # restore R20\n"
"    lwz  %r21, 184(%r1)  # restore R21\n"
"    lwz  %r22, 188(%r1)  # restore R22\n"
"    lwz  %r23, 192(%r1)  # restore R23\n"
"    lwz  %r24, 196(%r1)  # restore R24\n"
"    lwz  %r25, 200(%r1)  # restore R25\n"
"    lwz  %r26, 204(%r1)  # restore R26\n"
"    lwz  %r27, 208(%r1)  # restore R27\n"
"    lwz  %r28, 212(%r1)  # restore R28\n"
"    lwz  %r29, 216(%r1)  # restore R29\n"
"    lwz  %r30, 220(%r1)  # restore R30\n"
"    lwz  %r31, 224(%r1)  # restore R31\n"
"    # restore CR\n"
"    lwz  %r0, 228(%r1)\n"
"    mtcr  %r0\n"
"    # restore LR\n"
"    lwz  %r0, 232(%r1)\n"
"    mtlr  %r0\n"
"    # load PC\n"
"    lwz  %r0, 236(%r1)\n"
"    # restore CTR\n"
"    mtctr  %r0\n"
"    # adjust stack\n"
"    addi  %r1, %r1, 240\n"
"    # use third arg as return value after jump\n"
"    # use third arg as first arg in context function\n"
"    mr  %r3, %r5\n"
"    # jump to context\n"
"    bctr\n"
".size jump_fcontext, .-jump_fcontext\n"
);

#endif

#if defined(LIBCONTEXT_PLATFORM_linux_ppc32) && defined(LIBCONTEXT_COMPILER_gcc)
__asm (
".text\n"
".globl make_fcontext\n"
".align 2\n"
".type make_fcontext,@function\n"
"make_fcontext:\n"
"    # save return address into R6\n"
"    mflr  %r6\n"
"    # first arg of make_fcontext() == top address of context-function\n"
"    # shift address in R3 to lower 16 byte boundary\n"
"    clrrwi  %r3, %r3, 4\n"
"    # reserve space for context-data on context-stack\n"
"    # including 64 byte of linkage + parameter area (R1 % 16 == 0)\n"
"    subi  %r3, %r3, 304\n"
"    # third arg of make_fcontext() == address of context-function\n"
"    stw  %r5, 236(%r3)\n"
"    # load LR\n"
"    mflr  %r0\n"
"    # jump to label 1\n"
"    bl  1f\n"
"1:\n"
"    # load LR into R4\n"
"    mflr  %r4\n"
"    # compute abs address of label finish\n"
"    addi  %r4, %r4, finish - 1b\n"
"    # restore LR\n"
"    mtlr  %r0\n"
"    # save address of finish as return-address for context-function\n"
"    # will be entered after context-function returns\n"
"    stw  %r4, 232(%r3)\n"
"    # restore return address from R6\n"
"    mtlr  %r6\n"
"    blr  # return pointer to context-data\n"
"finish:\n"
"    # save return address into R0\n"
"    mflr  %r0\n"
"    # save return address on stack, set up stack frame\n"
"    stw  %r0, 4(%r1)\n"
"    # allocate stack space, R1 % 16 == 0\n"
"    stwu  %r1, -16(%r1)\n"
"    # exit code is zero\n"
"    li  %r3, 0\n"
"    # exit application\n"
"    bl  _exit@plt\n"
".size make_fcontext, .-make_fcontext\n"
);

#endif

#if defined(LIBCONTEXT_PLATFORM_linux_ppc64) && defined(LIBCONTEXT_COMPILER_gcc)
__asm (
".globl jump_fcontext\n"
#if _CALL_ELF == 2
"  .text\n"
"  .align 2\n"
"jump_fcontext:\n"
"        addis   %r2, %r12, .TOC.-jump_fcontext@ha\n"
"        addi    %r2, %r2, .TOC.-jump_fcontext@l\n"
"        .localentry jump_fcontext, . - jump_fcontext\n"
#else
"  .section \".opd\",\"aw\"\n"
"  .align 3\n"
"jump_fcontext:\n"
# ifdef _CALL_LINUX
"        .quad   .L.jump_fcontext,.TOC.@tocbase,0\n"
"        .type   jump_fcontext,@function\n"
"        .text\n"
"        .align 2\n"
".L.jump_fcontext:\n"
# else
"        .hidden .jump_fcontext\n"
"        .globl  .jump_fcontext\n"
"        .quad   .jump_fcontext,.TOC.@tocbase,0\n"
"        .size   jump_fcontext,24\n"
"        .type   .jump_fcontext,@function\n"
"        .text\n"
"        .align 2\n"
".jump_fcontext:\n"
# endif
#endif
"    # reserve space on stack\n"
"    subi  %r1, %r1, 320\n"
#if _CALL_ELF != 2
"    std  %r2,  144(%r1)  # save TOC\n"
#endif
"    std  %r14, 152(%r1)  # save R14\n"
"    std  %r15, 160(%r1)  # save R15\n"
"    std  %r16, 168(%r1)  # save R16\n"
"    std  %r17, 176(%r1)  # save R17\n"
"    std  %r18, 184(%r1)  # save R18\n"
"    std  %r19, 192(%r1)  # save R19\n"
"    std  %r20, 200(%r1)  # save R20\n"
"    std  %r21, 208(%r1)  # save R21\n"
"    std  %r22, 216(%r1)  # save R22\n"
"    std  %r23, 224(%r1)  # save R23\n"
"    std  %r24, 232(%r1)  # save R24\n"
"    std  %r25, 240(%r1)  # save R25\n"
"    std  %r26, 248(%r1)  # save R26\n"
"    std  %r27, 256(%r1)  # save R27\n"
"    std  %r28, 264(%r1)  # save R28\n"
"    std  %r29, 272(%r1)  # save R29\n"
"    std  %r30, 280(%r1)  # save R30\n"
"    std  %r31, 288(%r1)  # save R31\n"
"    # save CR\n"
"    mfcr  %r0\n"
"    std  %r0, 296(%r1)\n"
"    # save LR\n"
"    mflr  %r0\n"
"    std  %r0, 304(%r1)\n"
"    # save LR as PC\n"
"    std  %r0, 312(%r1)\n"
"    # test if fpu env should be preserved\n"
"    cmpwi  cr7, %r6, 0\n"
"    beq  cr7, 1f\n"
"    stfd  %f14, 0(%r1)  # save F14\n"
"    stfd  %f15, 8(%r1)  # save F15\n"
"    stfd  %f16, 16(%r1)  # save F16\n"
"    stfd  %f17, 24(%r1)  # save F17\n"
"    stfd  %f18, 32(%r1)  # save F18\n"
"    stfd  %f19, 40(%r1)  # save F19\n"
"    stfd  %f20, 48(%r1)  # save F20\n"
"    stfd  %f21, 56(%r1)  # save F21\n"
"    stfd  %f22, 64(%r1)  # save F22\n"
"    stfd  %f23, 72(%r1)  # save F23\n"
"    stfd  %f24, 80(%r1)  # save F24\n"
"    stfd  %f25, 88(%r1)  # save F25\n"
"    stfd  %f26, 96(%r1)  # save F26\n"
"    stfd  %f27, 104(%r1)  # save F27\n"
"    stfd  %f28, 112(%r1)  # save F28\n"
"    stfd  %f29, 120(%r1)  # save F29\n"
"    stfd  %f30, 128(%r1)  # save F30\n"
"    stfd  %f31, 136(%r1)  # save F31\n"
"1:\n"
"    # store RSP (pointing to context-data) in R3\n"
"    std  %r1, 0(%r3)\n"
"    # restore RSP (pointing to context-data) from R4\n"
"    mr  %r1, %r4\n"
"    # test if fpu env should be preserved\n"
"    cmpwi  cr7, %r6, 0\n"
"    beq  cr7, 2f\n"
"    lfd  %f14, 0(%r1)  # restore F14\n"
"    lfd  %f15, 8(%r1)  # restore F15\n"
"    lfd  %f16, 16(%r1)  # restore F16\n"
"    lfd  %f17, 24(%r1)  # restore F17\n"
"    lfd  %f18, 32(%r1)  # restore F18\n"
"    lfd  %f19, 40(%r1)  # restore F19\n"
"    lfd  %f20, 48(%r1)  # restore F20\n"
"    lfd  %f21, 56(%r1)  # restore F21\n"
"    lfd  %f22, 64(%r1)  # restore F22\n"
"    lfd  %f23, 72(%r1)  # restore F23\n"
"    lfd  %f24, 80(%r1)  # restore F24\n"
"    lfd  %f25, 88(%r1)  # restore F25\n"
"    lfd  %f26, 96(%r1)  # restore F26\n"
"    lfd  %f27, 104(%r1)  # restore F27\n"
"    lfd  %f28, 112(%r1)  # restore F28\n"
"    lfd  %f29, 120(%r1)  # restore F29\n"
"    lfd  %f30, 128(%r1)  # restore F30\n"
"    lfd  %f31, 136(%r1)  # restore F31\n"
"2:\n"
#if _CALL_ELF != 2
"    ld  %r2,  144(%r1)  # restore TOC\n"
#endif
"    ld  %r14, 152(%r1)  # restore R14\n"
"    ld  %r15, 160(%r1)  # restore R15\n"
"    ld  %r16, 168(%r1)  # restore R16\n"
"    ld  %r17, 176(%r1)  # restore R17\n"
"    ld  %r18, 184(%r1)  # restore R18\n"
"    ld  %r19, 192(%r1)  # restore R19\n"
"    ld  %r20, 200(%r1)  # restore R20\n"
"    ld  %r21, 208(%r1)  # restore R21\n"
"    ld  %r22, 216(%r1)  # restore R22\n"
"    ld  %r23, 224(%r1)  # restore R23\n"
"    ld  %r24, 232(%r1)  # restore R24\n"
"    ld  %r25, 240(%r1)  # restore R25\n"
"    ld  %r26, 248(%r1)  # restore R26\n"
"    ld  %r27, 256(%r1)  # restore R27\n"
"    ld  %r28, 264(%r1)  # restore R28\n"
"    ld  %r29, 272(%r1)  # restore R29\n"
"    ld  %r30, 280(%r1)  # restore R30\n"
"    ld  %r31, 288(%r1)  # restore R31\n"
"    # restore CR\n"
"    ld  %r0, 296(%r1)\n"
"    mtcr  %r0\n"
"    # restore LR\n"
"    ld  %r0, 304(%r1)\n"
"    mtlr  %r0\n"
"    # load PC\n"
"    ld  %r12, 312(%r1)\n"
"    # restore CTR\n"
"    mtctr  %r12\n"
"    # adjust stack\n"
"    addi  %r1, %r1, 320\n"
"    # use third arg as return value after jump\n"
"    # use third arg as first arg in context function\n"
"    mr  %r3, %r5\n"
"    # jump to context\n"
"    bctr\n"
#if _CALL_ELF == 2
"  .size jump_fcontext, .-jump_fcontext\n"
#else
# ifdef _CALL_LINUX
"  .size .jump_fcontext, .-.L.jump_fcontext\n"
# else
"  .size .jump_fcontext, .-.jump_fcontext\n"
# endif
#endif
);

#endif

#if defined(LIBCONTEXT_PLATFORM_linux_ppc64) && defined(LIBCONTEXT_COMPILER_gcc)
__asm (
".globl make_fcontext\n"
#if _CALL_ELF == 2
"  .text\n"
"  .align 2\n"
"make_fcontext:\n"
"  addis   %r2, %r12, .TOC.-make_fcontext@ha\n"
"  addi    %r2, %r2, .TOC.-make_fcontext@l\n"
"  .localentry make_fcontext, . - make_fcontext\n"
#else
"  .section \".opd\",\"aw\"\n"
"  .align 3\n"
"make_fcontext:\n"
# ifdef _CALL_LINUX
"  .quad   .L.make_fcontext,.TOC.@tocbase,0\n"
"  .type   make_fcontext,@function\n"
"  .text\n"
"  .align 2\n"
".L.make_fcontext:\n"
# else
"  .hidden .make_fcontext\n"
"  .globl  .make_fcontext\n"
"  .quad   .make_fcontext,.TOC.@tocbase,0\n"
"  .size   make_fcontext,24\n"
"  .type   .make_fcontext,@function\n"
"  .text\n"
"  .align 2\n"
".make_fcontext:\n"
# endif
#endif
"    # save return address into R6\n"
"    mflr  %r6\n"
"    # first arg of make_fcontext() == top address of context-stack\n"
"    # shift address in R3 to lower 16 byte boundary\n"
"    clrrdi  %r3, %r3, 4\n"
"    # reserve space for context-data on context-stack\n"
"    # including 64 byte of linkage + parameter area (R1 % 16 == 0)\n"
"    subi  %r3, %r3, 384\n"
"    # third arg of make_fcontext() == address of context-function\n"
"    # entry point (ELFv2) or descriptor (ELFv1)\n"
#if _CALL_ELF == 2
"    # save address of context-function entry point\n"
"    std  %r5, 312(%r3)\n"
#else
"    # save address of context-function entry point\n"
"    ld   %r4, 0(%r5)\n"
"    std  %r4, 312(%r3)\n"
"    # save TOC of context-function\n"
"    ld   %r4, 8(%r5)\n"
"    std  %r4, 144(%r3)\n"
#endif
"    # load LR\n"
"    mflr  %r0\n"
"    # jump to label 1\n"
"    bl  1f\n"
"1:\n"
"    # load LR into R4\n"
"    mflr  %r4\n"
"    # compute abs address of label finish\n"
"    addi  %r4, %r4, finish - 1b\n"
"    # restore LR\n"
"    mtlr  %r0\n"
"    # save address of finish as return-address for context-function\n"
"    # will be entered after context-function returns\n"
"    std  %r4, 304(%r3)\n"
"    # restore return address from R6\n"
"    mtlr  %r6\n"
"    blr  # return pointer to context-data\n"
"finish:\n"
"    # save return address into R0\n"
"    mflr  %r0\n"
"    # save return address on stack, set up stack frame\n"
"    std  %r0, 8(%r1)\n"
"    # allocate stack space, R1 % 16 == 0\n"
"    stdu  %r1, -32(%r1)\n"
"    # exit code is zero\n"
"    li  %r3, 0\n"
"    # exit application\n"
"    bl  _exit\n"
"    nop\n"
#if _CALL_ELF == 2
"  .size make_fcontext, .-make_fcontext\n"
#else
# ifdef _CALL_LINUX
"  .size .make_fcontext, .-.L.make_fcontext\n"
# else
"  .size .make_fcontext, .-.make_fcontext\n"
# endif
#endif
);

#endif

#if defined(LIBCONTEXT_PLATFORM_msvc_x86_64) || defined(LIBCONTEXT_PLATFORM_msvc_i386)

#include <map>

#ifdef __cplusplus
extern "C" {
#endif

#include <windows.h>

namespace libcontext
{

static int threadHasFibers = 0;

struct FiberData
{
	intptr_t inValue;
	intptr_t outValue;
	void(*entry)(intptr_t);
};

static std::map<fcontext_t, FiberData> fiberParams;

static void fiberEntry(LPVOID params)
{
	auto ctx = (fcontext_t) GetCurrentFiber();
	auto& d = fiberParams[ctx];
	d.entry(d.inValue);
}

fcontext_t LIBCONTEXT_CALL_CONVENTION make_fcontext(void* sp, size_t size, void(*fn)(intptr_t))
{
	if (!threadHasFibers)
	{
		ConvertThreadToFiber(nullptr);
		threadHasFibers = 1;
	}

	fcontext_t ctx = CreateFiber(size, (LPFIBER_START_ROUTINE) fiberEntry, nullptr );
	fiberParams[ctx].entry = fn;

	return ctx;
}

intptr_t LIBCONTEXT_CALL_CONVENTION jump_fcontext(fcontext_t* ofc, fcontext_t nfc,
	intptr_t vp, bool preserve_fpu)
{
	auto current = (void*)GetCurrentFiber();
	fiberParams[current].outValue = vp;
	*ofc = GetCurrentFiber();
	fiberParams[nfc].inValue = vp;
	SwitchToFiber(nfc);
	return fiberParams[*ofc].outValue;
}


void LIBCONTEXT_CALL_CONVENTION release_fcontext( fcontext_t ctx )
{
	if( fiberParams.find( ctx ) != fiberParams.end() )
	{
		fiberParams.erase( ctx );
	}
}


}; // namespace libcontext

#ifdef __cplusplus
};
#endif

#else // defined(LIBCONTEXT_PLATFORM_msvc_x86_64) || defined(LIBCONTEXT_PLATFORM_msvc_i386)

#warning nowindows

#ifdef __cplusplus
extern "C" {
#endif

namespace libcontext
{

void LIBCONTEXT_CALL_CONVENTION release_fcontext( fcontext_t ctx )
{
	// do nothing...
}

}; // namespace libcontext

#ifdef __cplusplus
};
#endif // defined(LIBCONTEXT_PLATFORM_msvc_x86_64) || defined(LIBCONTEXT_PLATFORM_msvc_i386)

#endif

