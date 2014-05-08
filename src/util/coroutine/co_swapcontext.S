
/* from glibc-2.1.7 source code ... */

#if defined(__i386__)

/* glibc-2.17\sysdeps\unix\sysv\linux\i386 */

#define oFS 24
#define oEDI 36
#define oESI 40
#define oEBP 44
#define oESP 48
#define oEBX 52
#define oEDX 56
#define oECX 60
#define oEAX 64
#define oEIP 76

.globl co_swapcontext
.type  co_swapcontext, @function
co_swapcontext:

	
	/* Load address of the context data structure we save in.  */
	movl	4(%esp), %eax

	/* Return value of swapcontext.  EAX is the only register whose
	   value is not preserved.  */
	movl	$0, oEAX(%eax)

	/* Save the 32-bit register values and the return address.  */
	movl	%ecx, oECX(%eax)
	movl	%edx, oEDX(%eax)
	movl	%edi, oEDI(%eax)
	movl	%esi, oESI(%eax)
	movl	%ebp, oEBP(%eax)
	movl	(%esp), %ecx
	movl	%ecx, oEIP(%eax)
	
	leal	4(%esp), %ecx
	movl	%ecx, oESP(%eax)
	movl	%ebx, oEBX(%eax)

	/* Save the FS segment register.  */
	xorl	%edx, %edx
	movw	%fs, %dx
	movl	%edx, oFS(%eax)

	
	/* EAX was modified, reload it.  */
	movl	8(%esp), %eax

	

	/* Restore the FS segment register.  We don't touch the GS register
	   since it is used for threads.  */
	movl	oFS(%eax), %edx
	movw	%dx, %fs


	/* Fetch the address to return to.  */
	movl	oEIP(%eax), %ecx

	/* Load the new stack pointer.  */
	movl	oESP(%eax), %esp

	/* Push the return address on the new stack so we can return there.  */
	pushl	%ecx

	/* Load the values of all the 32-bit registers (except ESP).
	   Since we are loading from EAX, it must be last.  */
	movl	oEDI(%eax), %edi
	movl	oESI(%eax), %esi
	movl	oEBP(%eax), %ebp
	movl	oEBX(%eax), %ebx
	movl	oEDX(%eax), %edx
	movl	oECX(%eax), %ecx
	movl	oEAX(%eax), %eax

	/* The following 'ret' will pop the address of the code and jump
	   to it.  */
	ret

#elif defined(__x86_64__)

/* glibc-2.17\sysdeps\unix\sysv\linux\x86_64 */

#define oR8 40
#define oR9 48
#define oR12 72
#define oR13 80
#define oR14 88
#define oR15 96
#define oRDI 104
#define oRSI 112
#define oRBP 120
#define oRBX 128
#define oRDX 136
#define oRCX 152
#define oRSP 160
#define oRIP 168

.globl co_swapcontext
.type  co_swapcontext, @function
co_swapcontext:

	/* Save the preserved registers, the registers used for passing args,
	   and the return address.  */
	movq	%rbx, oRBX(%rdi)
	movq	%rbp, oRBP(%rdi)
	movq	%r12, oR12(%rdi)
	movq	%r13, oR13(%rdi)
	movq	%r14, oR14(%rdi)
	movq	%r15, oR15(%rdi)

	movq	%rdi, oRDI(%rdi)
	movq	%rsi, oRSI(%rdi)
	movq	%rdx, oRDX(%rdi)
	movq	%rcx, oRCX(%rdi)
	movq	%r8, oR8(%rdi)
	movq	%r9, oR9(%rdi)

	movq	(%rsp), %rcx
	movq	%rcx, oRIP(%rdi)
	leaq	8(%rsp), %rcx		/* Exclude the return address.  */
	movq	%rcx, oRSP(%rdi)


	/* Load the new stack pointer and the preserved registers.  */
	movq	oRSP(%rsi), %rsp
	movq	oRBX(%rsi), %rbx
	movq	oRBP(%rsi), %rbp
	movq	oR12(%rsi), %r12
	movq	oR13(%rsi), %r13
	movq	oR14(%rsi), %r14
	movq	oR15(%rsi), %r15

	/* The following ret should return to the address set with
	getcontext.  Therefore push the address on the stack.  */
	movq	oRIP(%rsi), %rcx
	pushq	%rcx

	/* Setup registers used for passing args.  */
	movq	oRDI(%rsi), %rdi
	movq	oRDX(%rsi), %rdx
	movq	oRCX(%rsi), %rcx
	movq	oR8(%rsi), %r8
	movq	oR9(%rsi), %r9

	/* Setup finally  %rsi.  */
	movq	oRSI(%rsi), %rsi

	/* Clear rax to indicate success.  */
	xorl	%eax, %eax
	ret

#endif
/* a faster impl is http://www.rethinkdb.com/blog/making-coroutines-fast */

