	.def	@feat.00;
	.scl	3;
	.type	0;
	.endef
	.globl	@feat.00
@feat.00 = 0
	.file	"\347\250\213\345\272\217"
	.def	"断言相等";
	.scl	2;
	.type	32;
	.endef
	.text
	.globl	"断言相等"                      # -- Begin function 断言相等
	.p2align	4
"断言相等":                             # @"\E6\96\AD\E8\A8\80\E7\9B\B8\E7\AD\89"
.seh_proc "断言相等"
# %bb.0:                                # %entry
	pushq	%rsi
	.seh_pushreg %rsi
	subq	$32, %rsp
	.seh_stackalloc 32
	.seh_endprologue
	movl	(%rcx), %eax
	cmpl	(%rdx), %eax
	jne	.LBB0_1
# %bb.3:                                # %then
	incl	"测试通过"(%rip)
	jmp	.LBB0_2
.LBB0_1:                                # %else
	movq	%r8, %rsi
	incl	"测试失败"(%rip)
	leaq	.L__unnamed_1(%rip), %rcx
	leaq	.L__unnamed_2(%rip), %rdx
	callq	printf
	movl	(%rsi), %edx
	leaq	.L__unnamed_3(%rip), %rcx
	callq	printf
	nop
.LBB0_2:                                # %ifcont
	.seh_startepilogue
	addq	$32, %rsp
	popq	%rsi
	.seh_endepilogue
	retq
	.seh_endproc
                                        # -- End function
	.def	"扫描标记";
	.scl	2;
	.type	32;
	.endef
	.globl	"扫描标记"                      # -- Begin function 扫描标记
	.p2align	4
"扫描标记":                             # @"\E6\89\AB\E6\8F\8F\E6\A0\87\E8\AE\B0"
.seh_proc "扫描标记"
# %bb.0:                                # %entry
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%r13
	.seh_pushreg %r13
	pushq	%r12
	.seh_pushreg %r12
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	pushq	%rax
	.seh_stackalloc 8
	movq	%rsp, %rbp
	.seh_setframe %rbp, 0
	.seh_endprologue
	xorl	%edi, %edi
	leaq	.L__unnamed_4(%rip), %rsi
	leaq	.L__unnamed_5(%rip), %rbx
	leaq	.L__unnamed_6(%rip), %r14
	.p2align	4
.LBB1_1:                                # %whilecond
                                        # =>This Inner Loop Header: Depth=1
	movl	"源位置"(%rip), %r15d
	movq	"源代码"(%rip), %rcx
	subq	$32, %rsp
	callq	"获取字符数"
	addq	$32, %rsp
	cmpl	%eax, %r15d
	jge	.LBB1_10
# %bb.2:                                # %whilebody
                                        #   in Loop: Header=BB1_1 Depth=1
	movq	"源代码"(%rip), %r15
	movl	"源位置"(%rip), %r12d
	subq	$32, %rsp
	movq	%r15, %rcx
	movl	%r12d, %edx
	callq	"字符位置到字节位置"
	addq	$32, %rsp
	movl	%eax, %r13d
	incl	%r12d
	subq	$32, %rsp
	movq	%r15, %rcx
	movl	%r12d, %edx
	callq	"字符位置到字节位置"
	addq	$32, %rsp
	movl	%eax, %r12d
	subl	%r13d, %r12d
	cmovsl	%edi, %r12d
	movslq	%r13d, %r13
	addq	%r15, %r13
	leal	1(%r12), %ecx
	subq	$32, %rsp
	callq	malloc
	addq	$32, %rsp
	movq	%rax, %r15
	subq	$32, %rsp
	movq	%rax, %rcx
	movq	%r13, %rdx
	movq	%r12, %r8
	callq	strncpy
	addq	$32, %rsp
	movslq	%r12d, %rax
	movb	$0, (%r15,%rax)
	movl	$16, %eax
	callq	___chkstk_ms
	subq	%rax, %rsp
	movq	%rsp, %r12
	movq	%r15, (%r12)
	subq	$32, %rsp
	movq	%r15, %rcx
	movq	%rsi, %rdx
	callq	strcmp
	addq	$32, %rsp
	testl	%eax, %eax
	movb	$1, %r15b
	movb	$1, %al
	jne	.LBB1_3
# %bb.4:                                # %lor_merge
                                        #   in Loop: Header=BB1_1 Depth=1
	testb	%al, %al
	je	.LBB1_5
.LBB1_6:                                # %lor_merge14
                                        #   in Loop: Header=BB1_1 Depth=1
	movb	$1, %al
	testb	%r15b, %r15b
	jne	.LBB1_8
# %bb.7:                                # %lor_rhs18
                                        #   in Loop: Header=BB1_1 Depth=1
	movq	(%r12), %rcx
	subq	$32, %rsp
	leaq	.L__unnamed_7(%rip), %rdx
	callq	strcmp
	addq	$32, %rsp
	testl	%eax, %eax
	sete	%al
.LBB1_8:                                # %lor_merge23
                                        #   in Loop: Header=BB1_1 Depth=1
	testb	%al, %al
	je	.LBB1_10
# %bb.9:                                # %then
                                        #   in Loop: Header=BB1_1 Depth=1
	incl	"源位置"(%rip)
	jmp	.LBB1_1
	.p2align	4
.LBB1_3:                                # %lor_rhs
                                        #   in Loop: Header=BB1_1 Depth=1
	movq	(%r12), %rcx
	subq	$32, %rsp
	movq	%r14, %rdx
	callq	strcmp
	addq	$32, %rsp
	testl	%eax, %eax
	sete	%al
	testb	%al, %al
	jne	.LBB1_6
.LBB1_5:                                # %lor_rhs9
                                        #   in Loop: Header=BB1_1 Depth=1
	movq	(%r12), %rcx
	subq	$32, %rsp
	movq	%rbx, %rdx
	callq	strcmp
	addq	$32, %rsp
	testl	%eax, %eax
	sete	%r15b
	jmp	.LBB1_6
	.p2align	4
.LBB1_11:                               # %whilebody28
                                        #   in Loop: Header=BB1_10 Depth=1
	movq	"源代码"(%rip), %rdi
	movl	"源位置"(%rip), %esi
	subq	$32, %rsp
	movq	%rdi, %rcx
	movl	%esi, %edx
	callq	"字符位置到字节位置"
	addq	$32, %rsp
	movl	%eax, %ebx
	incl	%esi
	subq	$32, %rsp
	movq	%rdi, %rcx
	movl	%esi, %edx
	callq	"字符位置到字节位置"
	addq	$32, %rsp
	movl	%eax, %r14d
	xorl	%esi, %esi
	subl	%ebx, %r14d
	cmovsl	%esi, %r14d
	movslq	%ebx, %rbx
	addq	%rdi, %rbx
	leal	1(%r14), %ecx
	subq	$32, %rsp
	callq	malloc
	addq	$32, %rsp
	movq	%rax, %rdi
	subq	$32, %rsp
	movq	%rax, %rcx
	movq	%rbx, %rdx
	movq	%r14, %r8
	callq	strncpy
	addq	$32, %rsp
	movslq	%r14d, %rax
	movb	$0, (%rdi,%rax)
	subq	$32, %rsp
	leaq	.L__unnamed_8(%rip), %rdx
	movq	%rdi, %rcx
	callq	strcmp
	addq	$32, %rsp
	testl	%eax, %eax
	jne	.LBB1_13
# %bb.12:                               # %land_rhs
                                        #   in Loop: Header=BB1_10 Depth=1
	movq	"源代码"(%rip), %rdi
	movl	"源位置"(%rip), %ebx
	leal	1(%rbx), %edx
	subq	$32, %rsp
	movq	%rdi, %rcx
	callq	"字符位置到字节位置"
	addq	$32, %rsp
	movl	%eax, %r14d
	addl	$2, %ebx
	subq	$32, %rsp
	movq	%rdi, %rcx
	movl	%ebx, %edx
	callq	"字符位置到字节位置"
	addq	$32, %rsp
	subl	%r14d, %eax
	cmovnsl	%eax, %esi
	movslq	%r14d, %rbx
	addq	%rdi, %rbx
	leal	1(%rsi), %ecx
	subq	$32, %rsp
	callq	malloc
	addq	$32, %rsp
	movq	%rax, %rdi
	subq	$32, %rsp
	movq	%rax, %rcx
	movq	%rbx, %rdx
	movq	%rsi, %r8
	callq	strncpy
	addq	$32, %rsp
	movslq	%esi, %rax
	movb	$0, (%rdi,%rax)
	subq	$32, %rsp
	leaq	.L__unnamed_9(%rip), %rdx
	movq	%rdi, %rcx
	callq	strcmp
	addq	$32, %rsp
	testl	%eax, %eax
	sete	%sil
.LBB1_13:                               # %land_merge
                                        #   in Loop: Header=BB1_10 Depth=1
	testb	%sil, %sil
	jne	.LBB1_14
	jmp	.LBB1_26
	.p2align	4
.LBB1_172:                              # %whilebody67
                                        #   in Loop: Header=BB1_14 Depth=2
	incl	"源位置"(%rip)
.LBB1_14:                               # %whilecond66
                                        #   Parent Loop BB1_10 Depth=1
                                        # =>  This Inner Loop Header: Depth=2
	movl	"源位置"(%rip), %edi
	movq	"源代码"(%rip), %rcx
	subq	$32, %rsp
	callq	"获取字符数"
	addq	$32, %rsp
	xorl	%esi, %esi
	cmpl	%eax, %edi
	jge	.LBB1_16
# %bb.15:                               # %land_rhs74
                                        #   in Loop: Header=BB1_14 Depth=2
	movq	"源代码"(%rip), %rdi
	movl	"源位置"(%rip), %ebx
	subq	$32, %rsp
	movq	%rdi, %rcx
	movl	%ebx, %edx
	callq	"字符位置到字节位置"
	addq	$32, %rsp
	movl	%eax, %r14d
	incl	%ebx
	subq	$32, %rsp
	movq	%rdi, %rcx
	movl	%ebx, %edx
	callq	"字符位置到字节位置"
	addq	$32, %rsp
	subl	%r14d, %eax
	cmovnsl	%eax, %esi
	movslq	%r14d, %rbx
	addq	%rdi, %rbx
	leal	1(%rsi), %ecx
	subq	$32, %rsp
	callq	malloc
	addq	$32, %rsp
	movq	%rax, %rdi
	subq	$32, %rsp
	movq	%rax, %rcx
	movq	%rbx, %rdx
	movq	%rsi, %r8
	callq	strncpy
	addq	$32, %rsp
	movslq	%esi, %rax
	movb	$0, (%rdi,%rax)
	leaq	.L__unnamed_10(%rip), %rax
	cmpq	%rax, %rdi
	setne	%sil
.LBB1_16:                               # %land_merge87
                                        #   in Loop: Header=BB1_14 Depth=2
	testb	%sil, %sil
	jne	.LBB1_172
	jmp	.LBB1_17
	.p2align	4
.LBB1_25:                               # %then146
                                        #   in Loop: Header=BB1_17 Depth=2
	incl	"源位置"(%rip)
	jmp	.LBB1_17
	.p2align	4
.LBB1_19:                               # %lor_rhs117
                                        #   in Loop: Header=BB1_17 Depth=2
	movq	(%rdi), %rcx
	subq	$32, %rsp
	leaq	.L__unnamed_11(%rip), %rdx
	callq	strcmp
	addq	$32, %rsp
	testl	%eax, %eax
	sete	%al
	testb	%al, %al
	jne	.LBB1_22
.LBB1_21:                               # %lor_rhs126
                                        #   in Loop: Header=BB1_17 Depth=2
	movq	(%rdi), %rcx
	subq	$32, %rsp
	leaq	.L__unnamed_12(%rip), %rdx
	callq	strcmp
	addq	$32, %rsp
	testl	%eax, %eax
	sete	%bl
	jmp	.LBB1_22
	.p2align	4
.LBB1_17:                               # %whilecond93
                                        #   Parent Loop BB1_10 Depth=1
                                        # =>  This Inner Loop Header: Depth=2
	movl	"源位置"(%rip), %esi
	movq	"源代码"(%rip), %rcx
	subq	$32, %rsp
	callq	"获取字符数"
	addq	$32, %rsp
	cmpl	%eax, %esi
	jge	.LBB1_10
# %bb.18:                               # %whilebody94
                                        #   in Loop: Header=BB1_17 Depth=2
	movq	"源代码"(%rip), %rsi
	movl	"源位置"(%rip), %edi
	subq	$32, %rsp
	movq	%rsi, %rcx
	movl	%edi, %edx
	callq	"字符位置到字节位置"
	addq	$32, %rsp
	movl	%eax, %ebx
	incl	%edi
	subq	$32, %rsp
	movq	%rsi, %rcx
	movl	%edi, %edx
	callq	"字符位置到字节位置"
	addq	$32, %rsp
	movl	%eax, %edi
	xorl	%eax, %eax
	subl	%ebx, %edi
	cmovsl	%eax, %edi
	movslq	%ebx, %rbx
	addq	%rsi, %rbx
	leal	1(%rdi), %ecx
	subq	$32, %rsp
	callq	malloc
	addq	$32, %rsp
	movq	%rax, %rsi
	subq	$32, %rsp
	movq	%rax, %rcx
	movq	%rbx, %rdx
	movq	%rdi, %r8
	callq	strncpy
	addq	$32, %rsp
	movslq	%edi, %rax
	movb	$0, (%rsi,%rax)
	movl	$16, %eax
	callq	___chkstk_ms
	subq	%rax, %rsp
	movq	%rsp, %rdi
	movq	%rsi, (%rdi)
	subq	$32, %rsp
	leaq	.L__unnamed_13(%rip), %rdx
	movq	%rsi, %rcx
	callq	strcmp
	addq	$32, %rsp
	testl	%eax, %eax
	movb	$1, %bl
	movb	$1, %al
	jne	.LBB1_19
# %bb.20:                               # %lor_merge122
                                        #   in Loop: Header=BB1_17 Depth=2
	testb	%al, %al
	je	.LBB1_21
.LBB1_22:                               # %lor_merge131
                                        #   in Loop: Header=BB1_17 Depth=2
	movb	$1, %al
	testb	%bl, %bl
	jne	.LBB1_24
# %bb.23:                               # %lor_rhs135
                                        #   in Loop: Header=BB1_17 Depth=2
	movq	(%rdi), %rcx
	subq	$32, %rsp
	leaq	.L__unnamed_14(%rip), %rdx
	callq	strcmp
	addq	$32, %rsp
	testl	%eax, %eax
	sete	%al
.LBB1_24:                               # %lor_merge140
                                        #   in Loop: Header=BB1_17 Depth=2
	testb	%al, %al
	jne	.LBB1_25
.LBB1_10:                               # %whilecond27
                                        # =>This Loop Header: Depth=1
                                        #     Child Loop BB1_14 Depth 2
                                        #     Child Loop BB1_17 Depth 2
	movl	"源位置"(%rip), %esi
	movq	"源代码"(%rip), %rcx
	subq	$32, %rsp
	callq	"获取字符数"
	addq	$32, %rsp
	decl	%eax
	cmpl	%eax, %esi
	jl	.LBB1_11
.LBB1_26:                               # %whileend29
	movl	"源位置"(%rip), %esi
	movq	"源代码"(%rip), %rcx
	subq	$32, %rsp
	callq	"获取字符数"
	addq	$32, %rsp
	cmpl	%eax, %esi
	jl	.LBB1_27
# %bb.75:                               # %then157
	movl	T_EOF(%rip), %eax
	jmp	.LBB1_76
.LBB1_27:                               # %else155
	movq	"源代码"(%rip), %rsi
	movl	"源位置"(%rip), %edi
	subq	$32, %rsp
	movq	%rsi, %rcx
	movl	%edi, %edx
	callq	"字符位置到字节位置"
	addq	$32, %rsp
	movl	%eax, %ebx
	incl	%edi
	subq	$32, %rsp
	movq	%rsi, %rcx
	movl	%edi, %edx
	callq	"字符位置到字节位置"
	addq	$32, %rsp
	movl	%eax, %edi
	xorl	%eax, %eax
	subl	%ebx, %edi
	cmovsl	%eax, %edi
	movslq	%ebx, %rbx
	addq	%rsi, %rbx
	leal	1(%rdi), %ecx
	subq	$32, %rsp
	callq	malloc
	addq	$32, %rsp
	movq	%rax, %rsi
	subq	$32, %rsp
	movq	%rax, %rcx
	movq	%rbx, %rdx
	movq	%rdi, %r8
	callq	strncpy
	addq	$32, %rsp
	movslq	%edi, %rax
	movb	$0, (%rsi,%rax)
	movl	$16, %eax
	callq	___chkstk_ms
	subq	%rax, %rsp
	movq	%rsp, %rdi
	movq	%rsi, (%rdi)
	movl	$16, %eax
	callq	___chkstk_ms
	subq	%rax, %rsp
	movq	%rsp, %rsi
	movl	$0, (%rsi)
	movq	(%rdi), %rcx
	subq	$32, %rsp
	leaq	.L__unnamed_15(%rip), %rdx
	callq	strcmp
	addq	$32, %rsp
	testl	%eax, %eax
	jne	.LBB1_28
# %bb.78:                               # %then176
	movl	$1, (%rsi)
.LBB1_28:                               # %ifcont175
	movq	(%rdi), %rcx
	subq	$32, %rsp
	leaq	.L__unnamed_16(%rip), %rdx
	callq	strcmp
	addq	$32, %rsp
	testl	%eax, %eax
	jne	.LBB1_29
# %bb.79:                               # %then183
	movl	$1, (%rsi)
.LBB1_29:                               # %ifcont182
	movq	(%rdi), %rcx
	subq	$32, %rsp
	leaq	.L__unnamed_17(%rip), %rdx
	callq	strcmp
	addq	$32, %rsp
	testl	%eax, %eax
	jne	.LBB1_30
# %bb.80:                               # %then190
	movl	$1, (%rsi)
.LBB1_30:                               # %ifcont189
	movq	(%rdi), %rcx
	subq	$32, %rsp
	leaq	.L__unnamed_18(%rip), %rdx
	callq	strcmp
	addq	$32, %rsp
	testl	%eax, %eax
	jne	.LBB1_31
# %bb.81:                               # %then197
	movl	$1, (%rsi)
.LBB1_31:                               # %ifcont196
	movq	(%rdi), %rcx
	subq	$32, %rsp
	leaq	.L__unnamed_19(%rip), %rdx
	callq	strcmp
	addq	$32, %rsp
	testl	%eax, %eax
	jne	.LBB1_32
# %bb.82:                               # %then204
	movl	$1, (%rsi)
.LBB1_32:                               # %ifcont203
	movq	(%rdi), %rcx
	subq	$32, %rsp
	leaq	.L__unnamed_20(%rip), %rdx
	callq	strcmp
	addq	$32, %rsp
	testl	%eax, %eax
	jne	.LBB1_33
# %bb.83:                               # %then211
	movl	$1, (%rsi)
.LBB1_33:                               # %ifcont210
	movq	(%rdi), %rcx
	subq	$32, %rsp
	leaq	.L__unnamed_21(%rip), %rdx
	callq	strcmp
	addq	$32, %rsp
	testl	%eax, %eax
	jne	.LBB1_34
# %bb.84:                               # %then218
	movl	$1, (%rsi)
.LBB1_34:                               # %ifcont217
	movq	(%rdi), %rcx
	subq	$32, %rsp
	leaq	.L__unnamed_22(%rip), %rdx
	callq	strcmp
	addq	$32, %rsp
	testl	%eax, %eax
	jne	.LBB1_35
# %bb.85:                               # %then225
	movl	$1, (%rsi)
.LBB1_35:                               # %ifcont224
	movq	(%rdi), %rcx
	subq	$32, %rsp
	leaq	.L__unnamed_23(%rip), %rdx
	callq	strcmp
	addq	$32, %rsp
	testl	%eax, %eax
	jne	.LBB1_36
# %bb.86:                               # %then232
	movl	$1, (%rsi)
.LBB1_36:                               # %ifcont231
	movq	(%rdi), %rcx
	subq	$32, %rsp
	leaq	.L__unnamed_24(%rip), %rdx
	callq	strcmp
	addq	$32, %rsp
	testl	%eax, %eax
	jne	.LBB1_37
# %bb.87:                               # %then239
	movl	$1, (%rsi)
.LBB1_37:                               # %ifcont238
	cmpl	$1, (%rsi)
	jne	.LBB1_38
# %bb.88:                               # %then244
	movl	$0, "当前标记数值"(%rip)
	leaq	.L__unnamed_25(%rip), %rsi
	leaq	.L__unnamed_26(%rip), %rdi
	leaq	.L__unnamed_27(%rip), %rbx
	leaq	.L__unnamed_28(%rip), %r14
	jmp	.LBB1_89
	.p2align	4
.LBB1_112:                              # %ifcont417
                                        #   in Loop: Header=BB1_89 Depth=1
	incl	"源位置"(%rip)
.LBB1_89:                               # %whilecond245
                                        # =>This Inner Loop Header: Depth=1
	movl	"源位置"(%rip), %r15d
	movq	"源代码"(%rip), %rcx
	subq	$32, %rsp
	callq	"获取字符数"
	addq	$32, %rsp
	cmpl	%eax, %r15d
	jge	.LBB1_101
# %bb.90:                               # %whilebody246
                                        #   in Loop: Header=BB1_89 Depth=1
	movq	"源代码"(%rip), %r12
	movl	"源位置"(%rip), %r15d
	subq	$32, %rsp
	movq	%r12, %rcx
	movl	%r15d, %edx
	callq	"字符位置到字节位置"
	addq	$32, %rsp
	movl	%eax, %r13d
	incl	%r15d
	subq	$32, %rsp
	movq	%r12, %rcx
	movl	%r15d, %edx
	callq	"字符位置到字节位置"
	addq	$32, %rsp
	movl	%eax, %r15d
	subl	%r13d, %r15d
	movl	$0, %eax
	cmovsl	%eax, %r15d
	movslq	%r13d, %r13
	addq	%r12, %r13
	leal	1(%r15), %ecx
	subq	$32, %rsp
	callq	malloc
	addq	$32, %rsp
	movq	%rax, %r12
	subq	$32, %rsp
	movq	%rax, %rcx
	movq	%r13, %rdx
	movq	%r15, %r8
	callq	strncpy
	addq	$32, %rsp
	movslq	%r15d, %rax
	movb	$0, (%r12,%rax)
	movl	$16, %eax
	callq	___chkstk_ms
	subq	%rax, %rsp
	movq	%rsp, %r15
	movq	%r12, (%r15)
	movl	$16, %eax
	callq	___chkstk_ms
	subq	%rax, %rsp
	movq	%rsp, %r12
	movl	$0, (%r12)
	movq	(%r15), %rcx
	subq	$32, %rsp
	leaq	.L__unnamed_29(%rip), %rdx
	callq	strcmp
	addq	$32, %rsp
	testl	%eax, %eax
	jne	.LBB1_91
# %bb.173:                              # %then270
                                        #   in Loop: Header=BB1_89 Depth=1
	movl	$1, (%r12)
.LBB1_91:                               # %ifcont269
                                        #   in Loop: Header=BB1_89 Depth=1
	movq	(%r15), %rcx
	subq	$32, %rsp
	leaq	.L__unnamed_30(%rip), %rdx
	callq	strcmp
	addq	$32, %rsp
	testl	%eax, %eax
	jne	.LBB1_92
# %bb.174:                              # %then277
                                        #   in Loop: Header=BB1_89 Depth=1
	movl	$1, (%r12)
.LBB1_92:                               # %ifcont276
                                        #   in Loop: Header=BB1_89 Depth=1
	movq	(%r15), %rcx
	subq	$32, %rsp
	leaq	.L__unnamed_31(%rip), %rdx
	callq	strcmp
	addq	$32, %rsp
	testl	%eax, %eax
	jne	.LBB1_93
# %bb.175:                              # %then284
                                        #   in Loop: Header=BB1_89 Depth=1
	movl	$1, (%r12)
.LBB1_93:                               # %ifcont283
                                        #   in Loop: Header=BB1_89 Depth=1
	movq	(%r15), %rcx
	subq	$32, %rsp
	leaq	.L__unnamed_32(%rip), %rdx
	callq	strcmp
	addq	$32, %rsp
	testl	%eax, %eax
	jne	.LBB1_94
# %bb.176:                              # %then291
                                        #   in Loop: Header=BB1_89 Depth=1
	movl	$1, (%r12)
.LBB1_94:                               # %ifcont290
                                        #   in Loop: Header=BB1_89 Depth=1
	movq	(%r15), %rcx
	subq	$32, %rsp
	leaq	.L__unnamed_33(%rip), %rdx
	callq	strcmp
	addq	$32, %rsp
	testl	%eax, %eax
	jne	.LBB1_95
# %bb.177:                              # %then298
                                        #   in Loop: Header=BB1_89 Depth=1
	movl	$1, (%r12)
.LBB1_95:                               # %ifcont297
                                        #   in Loop: Header=BB1_89 Depth=1
	movq	(%r15), %rcx
	subq	$32, %rsp
	leaq	.L__unnamed_34(%rip), %rdx
	callq	strcmp
	addq	$32, %rsp
	testl	%eax, %eax
	jne	.LBB1_96
# %bb.178:                              # %then305
                                        #   in Loop: Header=BB1_89 Depth=1
	movl	$1, (%r12)
.LBB1_96:                               # %ifcont304
                                        #   in Loop: Header=BB1_89 Depth=1
	movq	(%r15), %rcx
	subq	$32, %rsp
	leaq	.L__unnamed_35(%rip), %rdx
	callq	strcmp
	addq	$32, %rsp
	testl	%eax, %eax
	jne	.LBB1_97
# %bb.179:                              # %then312
                                        #   in Loop: Header=BB1_89 Depth=1
	movl	$1, (%r12)
.LBB1_97:                               # %ifcont311
                                        #   in Loop: Header=BB1_89 Depth=1
	movq	(%r15), %rcx
	subq	$32, %rsp
	movq	%rsi, %rdx
	callq	strcmp
	addq	$32, %rsp
	testl	%eax, %eax
	jne	.LBB1_98
# %bb.180:                              # %then319
                                        #   in Loop: Header=BB1_89 Depth=1
	movl	$1, (%r12)
.LBB1_98:                               # %ifcont318
                                        #   in Loop: Header=BB1_89 Depth=1
	movq	(%r15), %rcx
	subq	$32, %rsp
	movq	%rdi, %rdx
	callq	strcmp
	addq	$32, %rsp
	testl	%eax, %eax
	jne	.LBB1_99
# %bb.181:                              # %then326
                                        #   in Loop: Header=BB1_89 Depth=1
	movl	$1, (%r12)
.LBB1_99:                               # %ifcont325
                                        #   in Loop: Header=BB1_89 Depth=1
	movq	(%r15), %rcx
	subq	$32, %rsp
	movq	%rbx, %rdx
	callq	strcmp
	addq	$32, %rsp
	testl	%eax, %eax
	jne	.LBB1_100
# %bb.102:                              # %then333
                                        #   in Loop: Header=BB1_89 Depth=1
	movl	$1, (%r12)
.LBB1_100:                              # %ifcont332
                                        #   in Loop: Header=BB1_89 Depth=1
	cmpl	$1, (%r12)
	jne	.LBB1_101
# %bb.103:                              # %then339
                                        #   in Loop: Header=BB1_89 Depth=1
	movl	"当前标记数值"(%rip), %eax
	addl	%eax, %eax
	leal	(%rax,%rax,4), %eax
	movl	%eax, "当前标记数值"(%rip)
	movq	(%r15), %rcx
	subq	$32, %rsp
	leaq	.L__unnamed_36(%rip), %rdx
	callq	strcmp
	addq	$32, %rsp
	testl	%eax, %eax
	jne	.LBB1_104
# %bb.182:                              # %then346
                                        #   in Loop: Header=BB1_89 Depth=1
	incl	"当前标记数值"(%rip)
.LBB1_104:                              # %ifcont345
                                        #   in Loop: Header=BB1_89 Depth=1
	movq	(%r15), %rcx
	subq	$32, %rsp
	leaq	.L__unnamed_37(%rip), %rdx
	callq	strcmp
	addq	$32, %rsp
	testl	%eax, %eax
	jne	.LBB1_105
# %bb.183:                              # %then355
                                        #   in Loop: Header=BB1_89 Depth=1
	addl	$2, "当前标记数值"(%rip)
.LBB1_105:                              # %ifcont354
                                        #   in Loop: Header=BB1_89 Depth=1
	movq	(%r15), %rcx
	subq	$32, %rsp
	leaq	.L__unnamed_38(%rip), %rdx
	callq	strcmp
	addq	$32, %rsp
	testl	%eax, %eax
	jne	.LBB1_106
# %bb.184:                              # %then364
                                        #   in Loop: Header=BB1_89 Depth=1
	addl	$3, "当前标记数值"(%rip)
.LBB1_106:                              # %ifcont363
                                        #   in Loop: Header=BB1_89 Depth=1
	movq	(%r15), %rcx
	subq	$32, %rsp
	leaq	.L__unnamed_39(%rip), %rdx
	callq	strcmp
	addq	$32, %rsp
	testl	%eax, %eax
	jne	.LBB1_107
# %bb.185:                              # %then373
                                        #   in Loop: Header=BB1_89 Depth=1
	addl	$4, "当前标记数值"(%rip)
.LBB1_107:                              # %ifcont372
                                        #   in Loop: Header=BB1_89 Depth=1
	movq	(%r15), %rcx
	subq	$32, %rsp
	leaq	.L__unnamed_40(%rip), %rdx
	callq	strcmp
	addq	$32, %rsp
	testl	%eax, %eax
	jne	.LBB1_108
# %bb.186:                              # %then382
                                        #   in Loop: Header=BB1_89 Depth=1
	addl	$5, "当前标记数值"(%rip)
.LBB1_108:                              # %ifcont381
                                        #   in Loop: Header=BB1_89 Depth=1
	movq	(%r15), %rcx
	subq	$32, %rsp
	leaq	.L__unnamed_41(%rip), %rdx
	callq	strcmp
	addq	$32, %rsp
	testl	%eax, %eax
	jne	.LBB1_109
# %bb.187:                              # %then391
                                        #   in Loop: Header=BB1_89 Depth=1
	addl	$6, "当前标记数值"(%rip)
.LBB1_109:                              # %ifcont390
                                        #   in Loop: Header=BB1_89 Depth=1
	movq	(%r15), %rcx
	subq	$32, %rsp
	leaq	.L__unnamed_42(%rip), %rdx
	callq	strcmp
	addq	$32, %rsp
	testl	%eax, %eax
	jne	.LBB1_110
# %bb.188:                              # %then400
                                        #   in Loop: Header=BB1_89 Depth=1
	addl	$7, "当前标记数值"(%rip)
.LBB1_110:                              # %ifcont399
                                        #   in Loop: Header=BB1_89 Depth=1
	movq	(%r15), %rcx
	subq	$32, %rsp
	leaq	.L__unnamed_43(%rip), %rdx
	callq	strcmp
	addq	$32, %rsp
	testl	%eax, %eax
	jne	.LBB1_111
# %bb.189:                              # %then409
                                        #   in Loop: Header=BB1_89 Depth=1
	addl	$8, "当前标记数值"(%rip)
.LBB1_111:                              # %ifcont408
                                        #   in Loop: Header=BB1_89 Depth=1
	movq	(%r15), %rcx
	subq	$32, %rsp
	movq	%r14, %rdx
	callq	strcmp
	addq	$32, %rsp
	testl	%eax, %eax
	jne	.LBB1_112
# %bb.113:                              # %then418
                                        #   in Loop: Header=BB1_89 Depth=1
	addl	$9, "当前标记数值"(%rip)
	incl	"源位置"(%rip)
	jmp	.LBB1_89
.LBB1_101:                              # %whileend247
	movl	T_NUM(%rip), %eax
	jmp	.LBB1_76
.LBB1_38:                               # %else242
	movl	$16, %eax
	callq	___chkstk_ms
	subq	%rax, %rsp
	movq	%rsp, %rsi
	movl	$0, (%rsi)
	movq	(%rdi), %rcx
	subq	$32, %rsp
	leaq	.L__unnamed_44(%rip), %rdx
	callq	strcmp
	addq	$32, %rsp
	testl	%eax, %eax
	jne	.LBB1_39
# %bb.114:                              # %then431
	movl	$1, (%rsi)
.LBB1_39:                               # %ifcont430
	movq	(%rdi), %rcx
	subq	$32, %rsp
	leaq	.L__unnamed_45(%rip), %rdx
	callq	strcmp
	addq	$32, %rsp
	testl	%eax, %eax
	jne	.LBB1_40
# %bb.115:                              # %then438
	movl	$1, (%rsi)
.LBB1_40:                               # %ifcont437
	movq	(%rdi), %rcx
	subq	$32, %rsp
	leaq	.L__unnamed_46(%rip), %rdx
	callq	strcmp
	addq	$32, %rsp
	testl	%eax, %eax
	jne	.LBB1_41
# %bb.116:                              # %then445
	movl	$1, (%rsi)
.LBB1_41:                               # %ifcont444
	movq	(%rdi), %rcx
	subq	$32, %rsp
	leaq	.L__unnamed_47(%rip), %rdx
	callq	strcmp
	addq	$32, %rsp
	testl	%eax, %eax
	jne	.LBB1_42
# %bb.117:                              # %then452
	movl	$1, (%rsi)
.LBB1_42:                               # %ifcont451
	movq	(%rdi), %rcx
	subq	$32, %rsp
	leaq	.L__unnamed_48(%rip), %rdx
	callq	strcmp
	addq	$32, %rsp
	testl	%eax, %eax
	jne	.LBB1_43
# %bb.118:                              # %then459
	movl	$1, (%rsi)
.LBB1_43:                               # %ifcont458
	movq	(%rdi), %rcx
	subq	$32, %rsp
	leaq	.L__unnamed_49(%rip), %rdx
	callq	strcmp
	addq	$32, %rsp
	testl	%eax, %eax
	jne	.LBB1_44
# %bb.119:                              # %then466
	movl	$1, (%rsi)
.LBB1_44:                               # %ifcont465
	movq	(%rdi), %rcx
	subq	$32, %rsp
	leaq	.L__unnamed_50(%rip), %rdx
	callq	strcmp
	addq	$32, %rsp
	testl	%eax, %eax
	jne	.LBB1_45
# %bb.120:                              # %then473
	movl	$1, (%rsi)
.LBB1_45:                               # %ifcont472
	movq	(%rdi), %rcx
	subq	$32, %rsp
	leaq	.L__unnamed_51(%rip), %rdx
	callq	strcmp
	addq	$32, %rsp
	testl	%eax, %eax
	jne	.LBB1_46
# %bb.121:                              # %then480
	movl	$1, (%rsi)
.LBB1_46:                               # %ifcont479
	movq	(%rdi), %rcx
	subq	$32, %rsp
	leaq	.L__unnamed_52(%rip), %rdx
	callq	strcmp
	addq	$32, %rsp
	testl	%eax, %eax
	jne	.LBB1_47
# %bb.122:                              # %then487
	movl	$1, (%rsi)
.LBB1_47:                               # %ifcont486
	movq	(%rdi), %rcx
	subq	$32, %rsp
	leaq	.L__unnamed_53(%rip), %rdx
	callq	strcmp
	addq	$32, %rsp
	testl	%eax, %eax
	jne	.LBB1_48
# %bb.123:                              # %then494
	movl	$1, (%rsi)
.LBB1_48:                               # %ifcont493
	movq	(%rdi), %rcx
	subq	$32, %rsp
	leaq	.L__unnamed_54(%rip), %rdx
	callq	strcmp
	addq	$32, %rsp
	testl	%eax, %eax
	jne	.LBB1_49
# %bb.124:                              # %then501
	movl	$1, (%rsi)
.LBB1_49:                               # %ifcont500
	movq	(%rdi), %rcx
	subq	$32, %rsp
	leaq	.L__unnamed_55(%rip), %rdx
	callq	strcmp
	addq	$32, %rsp
	testl	%eax, %eax
	jne	.LBB1_50
# %bb.125:                              # %then508
	movl	$1, (%rsi)
.LBB1_50:                               # %ifcont507
	movq	(%rdi), %rcx
	subq	$32, %rsp
	leaq	.L__unnamed_56(%rip), %rdx
	callq	strcmp
	addq	$32, %rsp
	testl	%eax, %eax
	jne	.LBB1_51
# %bb.126:                              # %then515
	movl	$1, (%rsi)
.LBB1_51:                               # %ifcont514
	movq	(%rdi), %rcx
	subq	$32, %rsp
	leaq	.L__unnamed_57(%rip), %rdx
	callq	strcmp
	addq	$32, %rsp
	testl	%eax, %eax
	jne	.LBB1_52
# %bb.127:                              # %then522
	movl	$1, (%rsi)
.LBB1_52:                               # %ifcont521
	movq	(%rdi), %rcx
	subq	$32, %rsp
	leaq	.L__unnamed_58(%rip), %rdx
	callq	strcmp
	addq	$32, %rsp
	testl	%eax, %eax
	jne	.LBB1_53
# %bb.128:                              # %then529
	movl	$1, (%rsi)
.LBB1_53:                               # %ifcont528
	movq	(%rdi), %rcx
	subq	$32, %rsp
	leaq	.L__unnamed_59(%rip), %rdx
	callq	strcmp
	addq	$32, %rsp
	testl	%eax, %eax
	jne	.LBB1_54
# %bb.129:                              # %then536
	movl	$1, (%rsi)
.LBB1_54:                               # %ifcont535
	movq	(%rdi), %rcx
	subq	$32, %rsp
	leaq	.L__unnamed_60(%rip), %rdx
	callq	strcmp
	addq	$32, %rsp
	testl	%eax, %eax
	jne	.LBB1_55
# %bb.130:                              # %then543
	movl	$1, (%rsi)
.LBB1_55:                               # %ifcont542
	movq	(%rdi), %rcx
	subq	$32, %rsp
	leaq	.L__unnamed_61(%rip), %rdx
	callq	strcmp
	addq	$32, %rsp
	testl	%eax, %eax
	jne	.LBB1_56
# %bb.131:                              # %then550
	movl	$1, (%rsi)
.LBB1_56:                               # %ifcont549
	cmpl	$0, (%rsi)
	jne	.LBB1_57
# %bb.132:                              # %then556
	incl	"源位置"(%rip)
	movl	T_IDENT(%rip), %eax
.LBB1_76:                               # %then157
	movl	%eax, "当前标记类型"(%rip)
.LBB1_77:                               # %then157
	.seh_startepilogue
	leaq	8(%rbp), %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	.seh_endepilogue
	retq
.LBB1_57:                               # %else554
	movq	(%rdi), %rcx
	subq	$32, %rsp
	leaq	.L__unnamed_62(%rip), %rdx
	callq	strcmp
	addq	$32, %rsp
	testl	%eax, %eax
	jne	.LBB1_58
# %bb.133:                              # %then566
	movl	"源位置"(%rip), %edi
	incl	%edi
	movl	%edi, "源位置"(%rip)
	movq	"源代码"(%rip), %rcx
	subq	$32, %rsp
	callq	"获取字符数"
	addq	$32, %rsp
	xorl	%esi, %esi
	cmpl	%eax, %edi
	jge	.LBB1_135
# %bb.134:                              # %land_rhs574
	movq	"源代码"(%rip), %rdi
	movl	"源位置"(%rip), %ebx
	subq	$32, %rsp
	movq	%rdi, %rcx
	movl	%ebx, %edx
	callq	"字符位置到字节位置"
	addq	$32, %rsp
	movl	%eax, %r14d
	incl	%ebx
	subq	$32, %rsp
	movq	%rdi, %rcx
	movl	%ebx, %edx
	callq	"字符位置到字节位置"
	addq	$32, %rsp
	subl	%r14d, %eax
	cmovnsl	%eax, %esi
	movslq	%r14d, %rbx
	addq	%rdi, %rbx
	leal	1(%rsi), %ecx
	subq	$32, %rsp
	callq	malloc
	addq	$32, %rsp
	movq	%rax, %rdi
	subq	$32, %rsp
	movq	%rax, %rcx
	movq	%rbx, %rdx
	movq	%rsi, %r8
	callq	strncpy
	addq	$32, %rsp
	movslq	%esi, %rax
	movb	$0, (%rdi,%rax)
	subq	$32, %rsp
	leaq	.L__unnamed_63(%rip), %rdx
	movq	%rdi, %rcx
	callq	strcmp
	addq	$32, %rsp
	testl	%eax, %eax
	sete	%sil
.LBB1_135:                              # %land_merge589
	testb	%sil, %sil
	je	.LBB1_136
# %bb.137:                              # %then595
	incl	"源位置"(%rip)
	movl	T_EQ(%rip), %eax
	jmp	.LBB1_76
.LBB1_58:                               # %else564
	movq	(%rdi), %rcx
	subq	$32, %rsp
	leaq	.L__unnamed_64(%rip), %rdx
	callq	strcmp
	addq	$32, %rsp
	testl	%eax, %eax
	jne	.LBB1_59
# %bb.138:                              # %then606
	movl	"源位置"(%rip), %edi
	incl	%edi
	movl	%edi, "源位置"(%rip)
	movq	"源代码"(%rip), %rcx
	subq	$32, %rsp
	callq	"获取字符数"
	addq	$32, %rsp
	xorl	%esi, %esi
	cmpl	%eax, %edi
	jge	.LBB1_140
# %bb.139:                              # %land_rhs614
	movq	"源代码"(%rip), %rdi
	movl	"源位置"(%rip), %ebx
	subq	$32, %rsp
	movq	%rdi, %rcx
	movl	%ebx, %edx
	callq	"字符位置到字节位置"
	addq	$32, %rsp
	movl	%eax, %r14d
	incl	%ebx
	subq	$32, %rsp
	movq	%rdi, %rcx
	movl	%ebx, %edx
	callq	"字符位置到字节位置"
	addq	$32, %rsp
	subl	%r14d, %eax
	cmovnsl	%eax, %esi
	movslq	%r14d, %rbx
	addq	%rdi, %rbx
	leal	1(%rsi), %ecx
	subq	$32, %rsp
	callq	malloc
	addq	$32, %rsp
	movq	%rax, %rdi
	subq	$32, %rsp
	movq	%rax, %rcx
	movq	%rbx, %rdx
	movq	%rsi, %r8
	callq	strncpy
	addq	$32, %rsp
	movslq	%esi, %rax
	movb	$0, (%rdi,%rax)
	subq	$32, %rsp
	leaq	.L__unnamed_65(%rip), %rdx
	movq	%rdi, %rcx
	callq	strcmp
	addq	$32, %rsp
	testl	%eax, %eax
	sete	%sil
.LBB1_140:                              # %land_merge629
	testb	%sil, %sil
	je	.LBB1_141
# %bb.142:                              # %then635
	incl	"源位置"(%rip)
	movl	T_NEQ(%rip), %eax
	jmp	.LBB1_76
.LBB1_136:                              # %else593
	movl	T_ASSIGN(%rip), %eax
	jmp	.LBB1_76
.LBB1_59:                               # %else604
	movq	(%rdi), %rcx
	subq	$32, %rsp
	leaq	.L__unnamed_66(%rip), %rdx
	callq	strcmp
	addq	$32, %rsp
	testl	%eax, %eax
	jne	.LBB1_60
# %bb.143:                              # %then646
	movl	"源位置"(%rip), %edi
	incl	%edi
	movl	%edi, "源位置"(%rip)
	movq	"源代码"(%rip), %rcx
	subq	$32, %rsp
	callq	"获取字符数"
	addq	$32, %rsp
	xorl	%esi, %esi
	cmpl	%eax, %edi
	jge	.LBB1_145
# %bb.144:                              # %land_rhs654
	movq	"源代码"(%rip), %rdi
	movl	"源位置"(%rip), %ebx
	subq	$32, %rsp
	movq	%rdi, %rcx
	movl	%ebx, %edx
	callq	"字符位置到字节位置"
	addq	$32, %rsp
	movl	%eax, %r14d
	incl	%ebx
	subq	$32, %rsp
	movq	%rdi, %rcx
	movl	%ebx, %edx
	callq	"字符位置到字节位置"
	addq	$32, %rsp
	subl	%r14d, %eax
	cmovnsl	%eax, %esi
	movslq	%r14d, %rbx
	addq	%rdi, %rbx
	leal	1(%rsi), %ecx
	subq	$32, %rsp
	callq	malloc
	addq	$32, %rsp
	movq	%rax, %rdi
	subq	$32, %rsp
	movq	%rax, %rcx
	movq	%rbx, %rdx
	movq	%rsi, %r8
	callq	strncpy
	addq	$32, %rsp
	movslq	%esi, %rax
	movb	$0, (%rdi,%rax)
	subq	$32, %rsp
	leaq	.L__unnamed_67(%rip), %rdx
	movq	%rdi, %rcx
	callq	strcmp
	addq	$32, %rsp
	testl	%eax, %eax
	sete	%sil
.LBB1_145:                              # %land_merge669
	testb	%sil, %sil
	je	.LBB1_146
# %bb.147:                              # %then675
	incl	"源位置"(%rip)
	movl	T_LE(%rip), %eax
	jmp	.LBB1_76
.LBB1_141:                              # %else633
	movl	T_NOT(%rip), %eax
	jmp	.LBB1_76
.LBB1_60:                               # %else644
	movq	(%rdi), %rcx
	subq	$32, %rsp
	leaq	.L__unnamed_68(%rip), %rdx
	callq	strcmp
	addq	$32, %rsp
	testl	%eax, %eax
	jne	.LBB1_61
# %bb.148:                              # %then686
	movl	"源位置"(%rip), %edi
	incl	%edi
	movl	%edi, "源位置"(%rip)
	movq	"源代码"(%rip), %rcx
	subq	$32, %rsp
	callq	"获取字符数"
	addq	$32, %rsp
	xorl	%esi, %esi
	cmpl	%eax, %edi
	jge	.LBB1_150
# %bb.149:                              # %land_rhs694
	movq	"源代码"(%rip), %rdi
	movl	"源位置"(%rip), %ebx
	subq	$32, %rsp
	movq	%rdi, %rcx
	movl	%ebx, %edx
	callq	"字符位置到字节位置"
	addq	$32, %rsp
	movl	%eax, %r14d
	incl	%ebx
	subq	$32, %rsp
	movq	%rdi, %rcx
	movl	%ebx, %edx
	callq	"字符位置到字节位置"
	addq	$32, %rsp
	subl	%r14d, %eax
	cmovnsl	%eax, %esi
	movslq	%r14d, %rbx
	addq	%rdi, %rbx
	leal	1(%rsi), %ecx
	subq	$32, %rsp
	callq	malloc
	addq	$32, %rsp
	movq	%rax, %rdi
	subq	$32, %rsp
	movq	%rax, %rcx
	movq	%rbx, %rdx
	movq	%rsi, %r8
	callq	strncpy
	addq	$32, %rsp
	movslq	%esi, %rax
	movb	$0, (%rdi,%rax)
	subq	$32, %rsp
	leaq	.L__unnamed_69(%rip), %rdx
	movq	%rdi, %rcx
	callq	strcmp
	addq	$32, %rsp
	testl	%eax, %eax
	sete	%sil
.LBB1_150:                              # %land_merge709
	testb	%sil, %sil
	je	.LBB1_151
# %bb.152:                              # %then715
	incl	"源位置"(%rip)
	movl	T_GE(%rip), %eax
	jmp	.LBB1_76
.LBB1_146:                              # %else673
	movl	T_LT(%rip), %eax
	jmp	.LBB1_76
.LBB1_61:                               # %else684
	movq	(%rdi), %rcx
	subq	$32, %rsp
	leaq	.L__unnamed_70(%rip), %rdx
	callq	strcmp
	addq	$32, %rsp
	testl	%eax, %eax
	jne	.LBB1_62
# %bb.153:                              # %then726
	movl	"源位置"(%rip), %edi
	incl	%edi
	movl	%edi, "源位置"(%rip)
	movq	"源代码"(%rip), %rcx
	subq	$32, %rsp
	callq	"获取字符数"
	addq	$32, %rsp
	xorl	%esi, %esi
	cmpl	%eax, %edi
	jge	.LBB1_155
# %bb.154:                              # %land_rhs734
	movq	"源代码"(%rip), %rdi
	movl	"源位置"(%rip), %ebx
	subq	$32, %rsp
	movq	%rdi, %rcx
	movl	%ebx, %edx
	callq	"字符位置到字节位置"
	addq	$32, %rsp
	movl	%eax, %r14d
	incl	%ebx
	subq	$32, %rsp
	movq	%rdi, %rcx
	movl	%ebx, %edx
	callq	"字符位置到字节位置"
	addq	$32, %rsp
	subl	%r14d, %eax
	cmovnsl	%eax, %esi
	movslq	%r14d, %rbx
	addq	%rdi, %rbx
	leal	1(%rsi), %ecx
	subq	$32, %rsp
	callq	malloc
	addq	$32, %rsp
	movq	%rax, %rdi
	subq	$32, %rsp
	movq	%rax, %rcx
	movq	%rbx, %rdx
	movq	%rsi, %r8
	callq	strncpy
	addq	$32, %rsp
	movslq	%esi, %rax
	movb	$0, (%rdi,%rax)
	subq	$32, %rsp
	leaq	.L__unnamed_71(%rip), %rdx
	movq	%rdi, %rcx
	callq	strcmp
	addq	$32, %rsp
	testl	%eax, %eax
	sete	%sil
.LBB1_155:                              # %land_merge749
	testb	%sil, %sil
	je	.LBB1_156
# %bb.157:                              # %then755
	incl	"源位置"(%rip)
	movl	T_AND(%rip), %eax
	jmp	.LBB1_76
.LBB1_151:                              # %else713
	movl	T_GT(%rip), %eax
	jmp	.LBB1_76
.LBB1_62:                               # %else724
	movq	(%rdi), %rcx
	subq	$32, %rsp
	leaq	.L__unnamed_72(%rip), %rdx
	callq	strcmp
	addq	$32, %rsp
	testl	%eax, %eax
	jne	.LBB1_63
# %bb.158:                              # %then766
	movl	"源位置"(%rip), %edi
	incl	%edi
	movl	%edi, "源位置"(%rip)
	movq	"源代码"(%rip), %rcx
	subq	$32, %rsp
	callq	"获取字符数"
	addq	$32, %rsp
	xorl	%esi, %esi
	cmpl	%eax, %edi
	jge	.LBB1_160
# %bb.159:                              # %land_rhs774
	movq	"源代码"(%rip), %rdi
	movl	"源位置"(%rip), %ebx
	subq	$32, %rsp
	movq	%rdi, %rcx
	movl	%ebx, %edx
	callq	"字符位置到字节位置"
	addq	$32, %rsp
	movl	%eax, %r14d
	incl	%ebx
	subq	$32, %rsp
	movq	%rdi, %rcx
	movl	%ebx, %edx
	callq	"字符位置到字节位置"
	addq	$32, %rsp
	subl	%r14d, %eax
	cmovnsl	%eax, %esi
	movslq	%r14d, %rbx
	addq	%rdi, %rbx
	leal	1(%rsi), %ecx
	subq	$32, %rsp
	callq	malloc
	addq	$32, %rsp
	movq	%rax, %rdi
	subq	$32, %rsp
	movq	%rax, %rcx
	movq	%rbx, %rdx
	movq	%rsi, %r8
	callq	strncpy
	addq	$32, %rsp
	movslq	%esi, %rax
	movb	$0, (%rdi,%rax)
	subq	$32, %rsp
	leaq	.L__unnamed_73(%rip), %rdx
	movq	%rdi, %rcx
	callq	strcmp
	addq	$32, %rsp
	testl	%eax, %eax
	sete	%sil
.LBB1_160:                              # %land_merge789
	testb	%sil, %sil
	je	.LBB1_156
# %bb.161:                              # %then795
	incl	"源位置"(%rip)
	movl	T_OR(%rip), %eax
	jmp	.LBB1_76
.LBB1_63:                               # %else764
	incl	"源位置"(%rip)
	movq	(%rdi), %rcx
	subq	$32, %rsp
	leaq	.L__unnamed_74(%rip), %rdx
	callq	strcmp
	addq	$32, %rsp
	testl	%eax, %eax
	jne	.LBB1_64
# %bb.162:                              # %then808
	movl	T_PLUS(%rip), %eax
	jmp	.LBB1_76
.LBB1_64:                               # %else806
	movq	(%rdi), %rcx
	subq	$32, %rsp
	leaq	.L__unnamed_75(%rip), %rdx
	callq	strcmp
	addq	$32, %rsp
	testl	%eax, %eax
	jne	.LBB1_65
# %bb.163:                              # %then816
	movl	T_MINUS(%rip), %eax
	jmp	.LBB1_76
.LBB1_65:                               # %else814
	movq	(%rdi), %rcx
	subq	$32, %rsp
	leaq	.L__unnamed_76(%rip), %rdx
	callq	strcmp
	addq	$32, %rsp
	testl	%eax, %eax
	jne	.LBB1_66
# %bb.164:                              # %then824
	movl	T_STAR(%rip), %eax
	jmp	.LBB1_76
.LBB1_66:                               # %else822
	movq	(%rdi), %rcx
	subq	$32, %rsp
	leaq	.L__unnamed_77(%rip), %rdx
	callq	strcmp
	addq	$32, %rsp
	testl	%eax, %eax
	jne	.LBB1_67
# %bb.165:                              # %then832
	movl	T_SLASH(%rip), %eax
	jmp	.LBB1_76
.LBB1_67:                               # %else830
	movq	(%rdi), %rcx
	subq	$32, %rsp
	leaq	.L__unnamed_78(%rip), %rdx
	callq	strcmp
	addq	$32, %rsp
	testl	%eax, %eax
	jne	.LBB1_68
# %bb.166:                              # %then840
	movl	T_PCT(%rip), %eax
	jmp	.LBB1_76
.LBB1_68:                               # %else838
	movq	(%rdi), %rcx
	subq	$32, %rsp
	leaq	.L__unnamed_79(%rip), %rdx
	callq	strcmp
	addq	$32, %rsp
	testl	%eax, %eax
	jne	.LBB1_69
# %bb.167:                              # %then848
	movl	T_LPAREN(%rip), %eax
	jmp	.LBB1_76
.LBB1_69:                               # %else846
	movq	(%rdi), %rcx
	subq	$32, %rsp
	leaq	.L__unnamed_80(%rip), %rdx
	callq	strcmp
	addq	$32, %rsp
	testl	%eax, %eax
	jne	.LBB1_70
# %bb.168:                              # %then856
	movl	T_RPAREN(%rip), %eax
	jmp	.LBB1_76
.LBB1_70:                               # %else854
	movq	(%rdi), %rcx
	subq	$32, %rsp
	leaq	.L__unnamed_81(%rip), %rdx
	callq	strcmp
	addq	$32, %rsp
	testl	%eax, %eax
	jne	.LBB1_71
# %bb.169:                              # %then864
	movl	T_LBRACE(%rip), %eax
	jmp	.LBB1_76
.LBB1_71:                               # %else862
	movq	(%rdi), %rcx
	subq	$32, %rsp
	leaq	.L__unnamed_82(%rip), %rdx
	callq	strcmp
	addq	$32, %rsp
	testl	%eax, %eax
	jne	.LBB1_72
# %bb.170:                              # %then872
	movl	T_RBRACE(%rip), %eax
	jmp	.LBB1_76
.LBB1_72:                               # %else870
	movq	(%rdi), %rcx
	subq	$32, %rsp
	leaq	.L__unnamed_83(%rip), %rdx
	callq	strcmp
	addq	$32, %rsp
	testl	%eax, %eax
	jne	.LBB1_73
# %bb.171:                              # %then880
	movl	T_SEMI(%rip), %eax
	jmp	.LBB1_76
.LBB1_73:                               # %else878
	movq	(%rdi), %rcx
	subq	$32, %rsp
	leaq	.L__unnamed_84(%rip), %rdx
	callq	strcmp
	addq	$32, %rsp
	testl	%eax, %eax
	jne	.LBB1_156
# %bb.74:                               # %then888
	movl	T_COMMA(%rip), %eax
	jmp	.LBB1_76
.LBB1_156:                              # %else753
	movl	$0, "当前标记类型"(%rip)
	jmp	.LBB1_77
	.seh_endproc
                                        # -- End function
	.def	"获取优先级";
	.scl	2;
	.type	32;
	.endef
	.globl	"获取优先级"                    # -- Begin function 获取优先级
	.p2align	4
"获取优先级":                           # @"\E8\8E\B7\E5\8F\96\E4\BC\98\E5\85\88\E7\BA\A7"
# %bb.0:                                # %entry
	movl	"当前标记类型"(%rip), %eax
	cmpl	T_OR(%rip), %eax
	jne	.LBB2_1
# %bb.22:                               # %then
	movl	$1, %eax
	retq
.LBB2_1:                                # %else
	movl	"当前标记类型"(%rip), %eax
	cmpl	T_AND(%rip), %eax
	jne	.LBB2_2
# %bb.23:                               # %then6
	movl	$2, %eax
	retq
.LBB2_2:                                # %else4
	movl	"当前标记类型"(%rip), %eax
	cmpl	T_EQ(%rip), %eax
	movb	$1, %cl
	movb	$1, %al
	je	.LBB2_4
# %bb.3:                                # %lor_rhs
	movl	"当前标记类型"(%rip), %eax
	cmpl	T_NEQ(%rip), %eax
	sete	%al
.LBB2_4:                                # %lor_merge
	testb	%al, %al
	jne	.LBB2_6
# %bb.5:                                # %lor_rhs13
	movl	"当前标记类型"(%rip), %eax
	cmpl	T_LT(%rip), %eax
	sete	%cl
.LBB2_6:                                # %lor_merge17
	movb	$1, %al
	testb	%cl, %cl
	movb	$1, %cl
	jne	.LBB2_8
# %bb.7:                                # %lor_rhs21
	movl	"当前标记类型"(%rip), %ecx
	cmpl	T_GT(%rip), %ecx
	sete	%cl
.LBB2_8:                                # %lor_merge25
	testb	%cl, %cl
	jne	.LBB2_10
# %bb.9:                                # %lor_rhs29
	movl	"当前标记类型"(%rip), %eax
	cmpl	T_LE(%rip), %eax
	sete	%al
.LBB2_10:                               # %lor_merge33
	movb	$1, %cl
	testb	%al, %al
	je	.LBB2_11
# %bb.12:                               # %lor_merge41
	testb	%cl, %cl
	je	.LBB2_13
.LBB2_24:                               # %then47
	movl	$3, %eax
	retq
.LBB2_11:                               # %lor_rhs37
	movl	"当前标记类型"(%rip), %eax
	cmpl	T_GE(%rip), %eax
	sete	%cl
	testb	%cl, %cl
	jne	.LBB2_24
.LBB2_13:                               # %else45
	movl	"当前标记类型"(%rip), %eax
	cmpl	T_PLUS(%rip), %eax
	movb	$1, %al
	je	.LBB2_15
# %bb.14:                               # %lor_rhs52
	movl	"当前标记类型"(%rip), %eax
	cmpl	T_MINUS(%rip), %eax
	sete	%al
.LBB2_15:                               # %lor_merge56
	testb	%al, %al
	je	.LBB2_16
# %bb.25:                               # %then62
	movl	$4, %eax
	retq
.LBB2_16:                               # %else60
	movl	"当前标记类型"(%rip), %eax
	cmpl	T_STAR(%rip), %eax
	movb	$1, %al
	movb	$1, %cl
	jne	.LBB2_17
# %bb.18:                               # %lor_merge71
	testb	%cl, %cl
	je	.LBB2_19
.LBB2_20:                               # %lor_merge79
	testb	%al, %al
	je	.LBB2_26
.LBB2_21:                               # %then85
	movl	$5, %eax
	retq
.LBB2_17:                               # %lor_rhs67
	movl	"当前标记类型"(%rip), %ecx
	cmpl	T_SLASH(%rip), %ecx
	sete	%cl
	testb	%cl, %cl
	jne	.LBB2_20
.LBB2_19:                               # %lor_rhs75
	movl	"当前标记类型"(%rip), %eax
	cmpl	T_PCT(%rip), %eax
	sete	%al
	testb	%al, %al
	jne	.LBB2_21
.LBB2_26:                               # %else83
	xorl	%eax, %eax
	retq
                                        # -- End function
	.def	"解析原子";
	.scl	2;
	.type	32;
	.endef
	.globl	"解析原子"                      # -- Begin function 解析原子
	.p2align	4
"解析原子":                             # @"\E8\A7\A3\E6\9E\90\E5\8E\9F\E5\AD\90"
.seh_proc "解析原子"
# %bb.0:                                # %entry
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rax
	.seh_stackalloc 8
	movq	%rsp, %rbp
	.seh_setframe %rbp, 0
	.seh_endprologue
	movl	"当前标记类型"(%rip), %eax
	cmpl	T_MINUS(%rip), %eax
	jne	.LBB3_1
# %bb.12:                               # %then
	subq	$32, %rsp
	callq	"扫描标记"
	callq	"解析原子"
	addq	$32, %rsp
	negl	%eax
.LBB3_8:                                # %else11
	.seh_startepilogue
	leaq	8(%rbp), %rsp
	popq	%rsi
	popq	%rbp
	.seh_endepilogue
	retq
.LBB3_1:                                # %else
	movl	"当前标记类型"(%rip), %eax
	cmpl	T_NOT(%rip), %eax
	jne	.LBB3_2
# %bb.6:                                # %then6
	subq	$32, %rsp
	callq	"扫描标记"
	callq	"解析原子"
	addq	$32, %rsp
	movl	%eax, %ecx
	movl	$16, %eax
	callq	___chkstk_ms
	subq	%rax, %rsp
	movq	%rsp, %rax
	movl	%ecx, (%rax)
	testl	%ecx, %ecx
	jne	.LBB3_7
# %bb.9:                                # %then13
	movl	$1, %eax
	jmp	.LBB3_8
.LBB3_2:                                # %else4
	movl	"当前标记类型"(%rip), %eax
	cmpl	T_NUM(%rip), %eax
	jne	.LBB3_3
# %bb.10:                               # %then21
	movl	"当前标记数值"(%rip), %ecx
	movl	$16, %eax
	callq	___chkstk_ms
	subq	%rax, %rsp
	movq	%rsp, %rsi
	movl	%ecx, (%rsi)
.LBB3_11:                               # %then37
	subq	$32, %rsp
	callq	"扫描标记"
	addq	$32, %rsp
	movl	(%rsi), %eax
	jmp	.LBB3_8
.LBB3_3:                                # %else19
	movl	"当前标记类型"(%rip), %eax
	cmpl	T_LPAREN(%rip), %eax
	jne	.LBB3_7
# %bb.4:                                # %then30
	subq	$32, %rsp
	callq	"扫描标记"
	addq	$32, %rsp
	movl	$16, %eax
	callq	___chkstk_ms
	subq	%rax, %rsp
	movq	%rsp, %rcx
	movl	$1, (%rcx)
	subq	$32, %rsp
	callq	"解析表达式_内部"
	addq	$32, %rsp
	movl	%eax, %ecx
	movl	$16, %eax
	callq	___chkstk_ms
	subq	%rax, %rsp
	movq	%rsp, %rsi
	movl	%ecx, (%rsi)
	movl	"当前标记类型"(%rip), %eax
	cmpl	T_RPAREN(%rip), %eax
	je	.LBB3_11
# %bb.5:                                # %ifcont36
	movl	(%rsi), %eax
	jmp	.LBB3_8
.LBB3_7:                                # %else11
	xorl	%eax, %eax
	jmp	.LBB3_8
	.seh_endproc
                                        # -- End function
	.def	"解析表达式_内部";
	.scl	2;
	.type	32;
	.endef
	.globl	"解析表达式_内部"               # -- Begin function 解析表达式_内部
	.p2align	4
"解析表达式_内部":                      # @"\E8\A7\A3\E6\9E\90\E8\A1\A8\E8\BE\BE\E5\BC\8F_\E5\86\85\E9\83\A8"
.seh_proc "解析表达式_内部"
# %bb.0:                                # %entry
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	pushq	%rax
	.seh_stackalloc 8
	movq	%rsp, %rbp
	.seh_setframe %rbp, 0
	.seh_endprologue
	movq	%rcx, %rsi
	subq	$32, %rsp
	callq	"解析原子"
	addq	$32, %rsp
	jmp	.LBB4_1
	.p2align	4
.LBB4_18:                               # %lor_merge
                                        #   in Loop: Header=BB4_1 Depth=1
	movzbl	%al, %eax
.LBB4_1:                                # %whilecond
                                        # =>This Loop Header: Depth=1
                                        #     Child Loop BB4_2 Depth 2
	movl	%eax, 4(%rbp)
.LBB4_2:                                # %whilecond
                                        #   Parent Loop BB4_1 Depth=1
                                        # =>  This Inner Loop Header: Depth=2
	subq	$32, %rsp
	callq	"获取优先级"
	addq	$32, %rsp
	cmpl	(%rsi), %eax
	jl	.LBB4_33
# %bb.3:                                # %whilebody
                                        #   in Loop: Header=BB4_2 Depth=2
	movl	"当前标记类型"(%rip), %ecx
	movl	$16, %eax
	callq	___chkstk_ms
	subq	%rax, %rsp
	movq	%rsp, %rdi
	movl	%ecx, (%rdi)
	subq	$32, %rsp
	callq	"获取优先级"
	addq	$32, %rsp
	movl	%eax, %ecx
	movl	$16, %eax
	callq	___chkstk_ms
	subq	%rax, %rsp
	movq	%rsp, %rbx
	movl	%ecx, (%rbx)
	subq	$32, %rsp
	callq	"扫描标记"
	addq	$32, %rsp
	movl	(%rbx), %edx
	incl	%edx
	movl	$16, %eax
	callq	___chkstk_ms
	subq	%rax, %rsp
	movq	%rsp, %rcx
	movl	%edx, (%rcx)
	subq	$32, %rsp
	callq	"解析表达式_内部"
	addq	$32, %rsp
	movl	%eax, %ecx
	movl	$16, %eax
	callq	___chkstk_ms
	subq	%rax, %rsp
	movq	%rsp, %r8
	movl	%ecx, (%r8)
	movl	(%rdi), %eax
	cmpl	T_PLUS(%rip), %eax
	jne	.LBB4_4
# %bb.34:                               # %then
                                        #   in Loop: Header=BB4_2 Depth=2
	movl	(%r8), %eax
	addl	%eax, 4(%rbp)
.LBB4_4:                                # %ifcont
                                        #   in Loop: Header=BB4_2 Depth=2
	movl	(%rdi), %eax
	cmpl	T_MINUS(%rip), %eax
	jne	.LBB4_5
# %bb.35:                               # %then14
                                        #   in Loop: Header=BB4_2 Depth=2
	movl	(%r8), %eax
	subl	%eax, 4(%rbp)
.LBB4_5:                                # %ifcont13
                                        #   in Loop: Header=BB4_2 Depth=2
	movl	(%rdi), %eax
	cmpl	T_STAR(%rip), %eax
	jne	.LBB4_6
# %bb.36:                               # %then22
                                        #   in Loop: Header=BB4_2 Depth=2
	movl	4(%rbp), %eax
	imull	(%r8), %eax
	movl	%eax, 4(%rbp)
.LBB4_6:                                # %ifcont21
                                        #   in Loop: Header=BB4_2 Depth=2
	movl	(%rdi), %eax
	cmpl	T_SLASH(%rip), %eax
	jne	.LBB4_7
# %bb.19:                               # %then30
                                        #   in Loop: Header=BB4_2 Depth=2
	cmpl	$0, (%r8)
	je	.LBB4_7
# %bb.20:                               # %then35
                                        #   in Loop: Header=BB4_2 Depth=2
	movl	4(%rbp), %eax
	cltd
	idivl	(%r8)
	movl	%eax, 4(%rbp)
.LBB4_7:                                # %ifcont29
                                        #   in Loop: Header=BB4_2 Depth=2
	movl	(%rdi), %eax
	cmpl	T_PCT(%rip), %eax
	jne	.LBB4_8
# %bb.21:                               # %then43
                                        #   in Loop: Header=BB4_2 Depth=2
	cmpl	$0, (%r8)
	je	.LBB4_8
# %bb.22:                               # %then49
                                        #   in Loop: Header=BB4_2 Depth=2
	movl	4(%rbp), %eax
	cltd
	idivl	(%r8)
	movl	%edx, 4(%rbp)
.LBB4_8:                                # %ifcont42
                                        #   in Loop: Header=BB4_2 Depth=2
	movl	(%rdi), %eax
	cmpl	T_EQ(%rip), %eax
	jne	.LBB4_9
# %bb.23:                               # %then57
                                        #   in Loop: Header=BB4_2 Depth=2
	movl	4(%rbp), %eax
	xorl	%ecx, %ecx
	cmpl	(%r8), %eax
	sete	%cl
	movl	%ecx, 4(%rbp)
.LBB4_9:                                # %ifcont56
                                        #   in Loop: Header=BB4_2 Depth=2
	movl	(%rdi), %eax
	cmpl	T_NEQ(%rip), %eax
	jne	.LBB4_10
# %bb.24:                               # %then66
                                        #   in Loop: Header=BB4_2 Depth=2
	movl	4(%rbp), %eax
	xorl	%ecx, %ecx
	cmpl	(%r8), %eax
	setne	%cl
	movl	%ecx, 4(%rbp)
.LBB4_10:                               # %ifcont65
                                        #   in Loop: Header=BB4_2 Depth=2
	movl	(%rdi), %eax
	cmpl	T_LT(%rip), %eax
	jne	.LBB4_11
# %bb.25:                               # %then75
                                        #   in Loop: Header=BB4_2 Depth=2
	movl	4(%rbp), %eax
	xorl	%ecx, %ecx
	cmpl	(%r8), %eax
	setl	%cl
	movl	%ecx, 4(%rbp)
.LBB4_11:                               # %ifcont74
                                        #   in Loop: Header=BB4_2 Depth=2
	movl	(%rdi), %eax
	cmpl	T_GT(%rip), %eax
	jne	.LBB4_12
# %bb.26:                               # %then83
                                        #   in Loop: Header=BB4_2 Depth=2
	movl	4(%rbp), %eax
	xorl	%ecx, %ecx
	cmpl	(%r8), %eax
	setg	%cl
	movl	%ecx, 4(%rbp)
.LBB4_12:                               # %ifcont82
                                        #   in Loop: Header=BB4_2 Depth=2
	movl	(%rdi), %eax
	cmpl	T_LE(%rip), %eax
	jne	.LBB4_13
# %bb.27:                               # %then91
                                        #   in Loop: Header=BB4_2 Depth=2
	movl	4(%rbp), %eax
	xorl	%ecx, %ecx
	cmpl	(%r8), %eax
	setle	%cl
	movl	%ecx, 4(%rbp)
.LBB4_13:                               # %ifcont90
                                        #   in Loop: Header=BB4_2 Depth=2
	movl	(%rdi), %eax
	cmpl	T_GE(%rip), %eax
	jne	.LBB4_14
# %bb.28:                               # %then99
                                        #   in Loop: Header=BB4_2 Depth=2
	movl	4(%rbp), %eax
	xorl	%ecx, %ecx
	cmpl	(%r8), %eax
	setge	%cl
	movl	%ecx, 4(%rbp)
.LBB4_14:                               # %ifcont98
                                        #   in Loop: Header=BB4_2 Depth=2
	movl	(%rdi), %eax
	cmpl	T_AND(%rip), %eax
	jne	.LBB4_15
# %bb.29:                               # %then108
                                        #   in Loop: Header=BB4_2 Depth=2
	cmpl	$0, 4(%rbp)
	je	.LBB4_30
# %bb.31:                               # %land_rhs
                                        #   in Loop: Header=BB4_2 Depth=2
	cmpl	$0, (%r8)
	setne	%al
	jmp	.LBB4_32
	.p2align	4
.LBB4_30:                               #   in Loop: Header=BB4_2 Depth=2
	xorl	%eax, %eax
.LBB4_32:                               # %land_merge
                                        #   in Loop: Header=BB4_2 Depth=2
	movzbl	%al, %eax
	movl	%eax, 4(%rbp)
.LBB4_15:                               # %ifcont107
                                        #   in Loop: Header=BB4_2 Depth=2
	movl	(%rdi), %eax
	cmpl	T_OR(%rip), %eax
	jne	.LBB4_2
# %bb.16:                               # %then116
                                        #   in Loop: Header=BB4_1 Depth=1
	movb	$1, %al
	cmpl	$0, 4(%rbp)
	jne	.LBB4_18
# %bb.17:                               # %lor_rhs
                                        #   in Loop: Header=BB4_1 Depth=1
	cmpl	$0, (%r8)
	setne	%al
	jmp	.LBB4_18
.LBB4_33:                               # %whileend
	movl	4(%rbp), %eax
	.seh_startepilogue
	leaq	8(%rbp), %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%rbp
	.seh_endepilogue
	retq
	.seh_endproc
                                        # -- End function
	.def	"解析表达式";
	.scl	2;
	.type	32;
	.endef
	.globl	"解析表达式"                    # -- Begin function 解析表达式
	.p2align	4
"解析表达式":                           # @"\E8\A7\A3\E6\9E\90\E8\A1\A8\E8\BE\BE\E5\BC\8F"
.seh_proc "解析表达式"
# %bb.0:                                # %entry
	subq	$40, %rsp
	.seh_stackalloc 40
	.seh_endprologue
	movl	$1, 32(%rsp)
	leaq	32(%rsp), %rcx
	callq	"解析表达式_内部"
	nop
	.seh_startepilogue
	addq	$40, %rsp
	.seh_endepilogue
	retq
	.seh_endproc
                                        # -- End function
	.def	"测试词法数字";
	.scl	2;
	.type	32;
	.endef
	.globl	"测试词法数字"                  # -- Begin function 测试词法数字
	.p2align	4
"测试词法数字":                         # @"\E6\B5\8B\E8\AF\95\E8\AF\8D\E6\B3\95\E6\95\B0\E5\AD\97"
.seh_proc "测试词法数字"
# %bb.0:                                # %entry
	subq	$136, %rsp
	.seh_stackalloc 136
	.seh_endprologue
	leaq	.L__unnamed_85(%rip), %rax
	movq	%rax, "源代码"(%rip)
	movl	$0, "源位置"(%rip)
	callq	"扫描标记"
	movl	"当前标记类型"(%rip), %eax
	movl	%eax, 128(%rsp)
	movl	T_NUM(%rip), %eax
	movl	%eax, 120(%rsp)
	leaq	.L__unnamed_86(%rip), %rax
	movq	%rax, 112(%rsp)
	leaq	128(%rsp), %rcx
	leaq	120(%rsp), %rdx
	leaq	112(%rsp), %r8
	callq	"断言相等"
	movl	"当前标记数值"(%rip), %eax
	movl	%eax, 104(%rsp)
	movl	$123, 96(%rsp)
	leaq	.L__unnamed_87(%rip), %rax
	movq	%rax, 88(%rsp)
	leaq	104(%rsp), %rcx
	leaq	96(%rsp), %rdx
	leaq	88(%rsp), %r8
	callq	"断言相等"
	callq	"扫描标记"
	movl	"当前标记类型"(%rip), %eax
	movl	%eax, 80(%rsp)
	movl	T_NUM(%rip), %eax
	movl	%eax, 72(%rsp)
	leaq	.L__unnamed_88(%rip), %rax
	movq	%rax, 64(%rsp)
	leaq	80(%rsp), %rcx
	leaq	72(%rsp), %rdx
	leaq	64(%rsp), %r8
	callq	"断言相等"
	movl	"当前标记数值"(%rip), %eax
	movl	%eax, 56(%rsp)
	movl	$456, 48(%rsp)                  # imm = 0x1C8
	leaq	.L__unnamed_89(%rip), %rax
	movq	%rax, 40(%rsp)
	leaq	56(%rsp), %rcx
	leaq	48(%rsp), %rdx
	leaq	40(%rsp), %r8
	callq	"断言相等"
	nop
	.seh_startepilogue
	addq	$136, %rsp
	.seh_endepilogue
	retq
	.seh_endproc
                                        # -- End function
	.def	"测试词法运算符";
	.scl	2;
	.type	32;
	.endef
	.globl	"测试词法运算符"                # -- Begin function 测试词法运算符
	.p2align	4
"测试词法运算符":                       # @"\E6\B5\8B\E8\AF\95\E8\AF\8D\E6\B3\95\E8\BF\90\E7\AE\97\E7\AC\A6"
.seh_proc "测试词法运算符"
# %bb.0:                                # %entry
	subq	$520, %rsp                      # imm = 0x208
	.seh_stackalloc 520
	.seh_endprologue
	leaq	.L__unnamed_90(%rip), %rax
	movq	%rax, "源代码"(%rip)
	movl	$0, "源位置"(%rip)
	callq	"扫描标记"
	movl	"当前标记类型"(%rip), %eax
	movl	%eax, 512(%rsp)
	movl	T_PLUS(%rip), %eax
	movl	%eax, 504(%rsp)
	leaq	.L__unnamed_91(%rip), %rax
	movq	%rax, 496(%rsp)
	leaq	512(%rsp), %rcx
	leaq	504(%rsp), %rdx
	leaq	496(%rsp), %r8
	callq	"断言相等"
	callq	"扫描标记"
	movl	"当前标记类型"(%rip), %eax
	movl	%eax, 488(%rsp)
	movl	T_MINUS(%rip), %eax
	movl	%eax, 480(%rsp)
	leaq	.L__unnamed_92(%rip), %rax
	movq	%rax, 472(%rsp)
	leaq	488(%rsp), %rcx
	leaq	480(%rsp), %rdx
	leaq	472(%rsp), %r8
	callq	"断言相等"
	callq	"扫描标记"
	movl	"当前标记类型"(%rip), %eax
	movl	%eax, 464(%rsp)
	movl	T_STAR(%rip), %eax
	movl	%eax, 456(%rsp)
	leaq	.L__unnamed_93(%rip), %rax
	movq	%rax, 448(%rsp)
	leaq	464(%rsp), %rcx
	leaq	456(%rsp), %rdx
	leaq	448(%rsp), %r8
	callq	"断言相等"
	callq	"扫描标记"
	movl	"当前标记类型"(%rip), %eax
	movl	%eax, 440(%rsp)
	movl	T_SLASH(%rip), %eax
	movl	%eax, 432(%rsp)
	leaq	.L__unnamed_94(%rip), %rax
	movq	%rax, 424(%rsp)
	leaq	440(%rsp), %rcx
	leaq	432(%rsp), %rdx
	leaq	424(%rsp), %r8
	callq	"断言相等"
	callq	"扫描标记"
	movl	"当前标记类型"(%rip), %eax
	movl	%eax, 416(%rsp)
	movl	T_PCT(%rip), %eax
	movl	%eax, 408(%rsp)
	leaq	.L__unnamed_95(%rip), %rax
	movq	%rax, 400(%rsp)
	leaq	416(%rsp), %rcx
	leaq	408(%rsp), %rdx
	leaq	400(%rsp), %r8
	callq	"断言相等"
	callq	"扫描标记"
	movl	"当前标记类型"(%rip), %eax
	movl	%eax, 392(%rsp)
	movl	T_LPAREN(%rip), %eax
	movl	%eax, 384(%rsp)
	leaq	.L__unnamed_96(%rip), %rax
	movq	%rax, 376(%rsp)
	leaq	392(%rsp), %rcx
	leaq	384(%rsp), %rdx
	leaq	376(%rsp), %r8
	callq	"断言相等"
	callq	"扫描标记"
	movl	"当前标记类型"(%rip), %eax
	movl	%eax, 368(%rsp)
	movl	T_RPAREN(%rip), %eax
	movl	%eax, 360(%rsp)
	leaq	.L__unnamed_97(%rip), %rax
	movq	%rax, 352(%rsp)
	leaq	368(%rsp), %rcx
	leaq	360(%rsp), %rdx
	leaq	352(%rsp), %r8
	callq	"断言相等"
	callq	"扫描标记"
	movl	"当前标记类型"(%rip), %eax
	movl	%eax, 344(%rsp)
	movl	T_LBRACE(%rip), %eax
	movl	%eax, 336(%rsp)
	leaq	.L__unnamed_98(%rip), %rax
	movq	%rax, 328(%rsp)
	leaq	344(%rsp), %rcx
	leaq	336(%rsp), %rdx
	leaq	328(%rsp), %r8
	callq	"断言相等"
	callq	"扫描标记"
	movl	"当前标记类型"(%rip), %eax
	movl	%eax, 320(%rsp)
	movl	T_RBRACE(%rip), %eax
	movl	%eax, 312(%rsp)
	leaq	.L__unnamed_99(%rip), %rax
	movq	%rax, 304(%rsp)
	leaq	320(%rsp), %rcx
	leaq	312(%rsp), %rdx
	leaq	304(%rsp), %r8
	callq	"断言相等"
	callq	"扫描标记"
	movl	"当前标记类型"(%rip), %eax
	movl	%eax, 296(%rsp)
	movl	T_SEMI(%rip), %eax
	movl	%eax, 288(%rsp)
	leaq	.L__unnamed_100(%rip), %rax
	movq	%rax, 280(%rsp)
	leaq	296(%rsp), %rcx
	leaq	288(%rsp), %rdx
	leaq	280(%rsp), %r8
	callq	"断言相等"
	callq	"扫描标记"
	movl	"当前标记类型"(%rip), %eax
	movl	%eax, 272(%rsp)
	movl	T_ASSIGN(%rip), %eax
	movl	%eax, 264(%rsp)
	leaq	.L__unnamed_101(%rip), %rax
	movq	%rax, 256(%rsp)
	leaq	272(%rsp), %rcx
	leaq	264(%rsp), %rdx
	leaq	256(%rsp), %r8
	callq	"断言相等"
	callq	"扫描标记"
	movl	"当前标记类型"(%rip), %eax
	movl	%eax, 248(%rsp)
	movl	T_EQ(%rip), %eax
	movl	%eax, 240(%rsp)
	leaq	.L__unnamed_102(%rip), %rax
	movq	%rax, 232(%rsp)
	leaq	248(%rsp), %rcx
	leaq	240(%rsp), %rdx
	leaq	232(%rsp), %r8
	callq	"断言相等"
	callq	"扫描标记"
	movl	"当前标记类型"(%rip), %eax
	movl	%eax, 224(%rsp)
	movl	T_NEQ(%rip), %eax
	movl	%eax, 216(%rsp)
	leaq	.L__unnamed_103(%rip), %rax
	movq	%rax, 208(%rsp)
	leaq	224(%rsp), %rcx
	leaq	216(%rsp), %rdx
	leaq	208(%rsp), %r8
	callq	"断言相等"
	callq	"扫描标记"
	movl	"当前标记类型"(%rip), %eax
	movl	%eax, 200(%rsp)
	movl	T_LT(%rip), %eax
	movl	%eax, 192(%rsp)
	leaq	.L__unnamed_104(%rip), %rax
	movq	%rax, 184(%rsp)
	leaq	200(%rsp), %rcx
	leaq	192(%rsp), %rdx
	leaq	184(%rsp), %r8
	callq	"断言相等"
	callq	"扫描标记"
	movl	"当前标记类型"(%rip), %eax
	movl	%eax, 176(%rsp)
	movl	T_GT(%rip), %eax
	movl	%eax, 168(%rsp)
	leaq	.L__unnamed_105(%rip), %rax
	movq	%rax, 160(%rsp)
	leaq	176(%rsp), %rcx
	leaq	168(%rsp), %rdx
	leaq	160(%rsp), %r8
	callq	"断言相等"
	callq	"扫描标记"
	movl	"当前标记类型"(%rip), %eax
	movl	%eax, 152(%rsp)
	movl	T_LE(%rip), %eax
	movl	%eax, 144(%rsp)
	leaq	.L__unnamed_106(%rip), %rax
	movq	%rax, 136(%rsp)
	leaq	152(%rsp), %rcx
	leaq	144(%rsp), %rdx
	leaq	136(%rsp), %r8
	callq	"断言相等"
	callq	"扫描标记"
	movl	"当前标记类型"(%rip), %eax
	movl	%eax, 128(%rsp)
	movl	T_GE(%rip), %eax
	movl	%eax, 120(%rsp)
	leaq	.L__unnamed_107(%rip), %rax
	movq	%rax, 112(%rsp)
	leaq	128(%rsp), %rcx
	leaq	120(%rsp), %rdx
	leaq	112(%rsp), %r8
	callq	"断言相等"
	callq	"扫描标记"
	movl	"当前标记类型"(%rip), %eax
	movl	%eax, 104(%rsp)
	movl	T_AND(%rip), %eax
	movl	%eax, 96(%rsp)
	leaq	.L__unnamed_108(%rip), %rax
	movq	%rax, 88(%rsp)
	leaq	104(%rsp), %rcx
	leaq	96(%rsp), %rdx
	leaq	88(%rsp), %r8
	callq	"断言相等"
	callq	"扫描标记"
	movl	"当前标记类型"(%rip), %eax
	movl	%eax, 80(%rsp)
	movl	T_OR(%rip), %eax
	movl	%eax, 72(%rsp)
	leaq	.L__unnamed_109(%rip), %rax
	movq	%rax, 64(%rsp)
	leaq	80(%rsp), %rcx
	leaq	72(%rsp), %rdx
	leaq	64(%rsp), %r8
	callq	"断言相等"
	callq	"扫描标记"
	movl	"当前标记类型"(%rip), %eax
	movl	%eax, 56(%rsp)
	movl	T_NOT(%rip), %eax
	movl	%eax, 48(%rsp)
	leaq	.L__unnamed_110(%rip), %rax
	movq	%rax, 40(%rsp)
	leaq	56(%rsp), %rcx
	leaq	48(%rsp), %rdx
	leaq	40(%rsp), %r8
	callq	"断言相等"
	nop
	.seh_startepilogue
	addq	$520, %rsp                      # imm = 0x208
	.seh_endepilogue
	retq
	.seh_endproc
                                        # -- End function
	.def	"测试词法注释";
	.scl	2;
	.type	32;
	.endef
	.globl	"测试词法注释"                  # -- Begin function 测试词法注释
	.p2align	4
"测试词法注释":                         # @"\E6\B5\8B\E8\AF\95\E8\AF\8D\E6\B3\95\E6\B3\A8\E9\87\8A"
.seh_proc "测试词法注释"
# %bb.0:                                # %entry
	subq	$136, %rsp
	.seh_stackalloc 136
	.seh_endprologue
	leaq	.L__unnamed_111(%rip), %rax
	movq	%rax, "源代码"(%rip)
	movl	$0, "源位置"(%rip)
	callq	"扫描标记"
	movl	"当前标记类型"(%rip), %eax
	movl	%eax, 128(%rsp)
	movl	T_NUM(%rip), %eax
	movl	%eax, 120(%rsp)
	leaq	.L__unnamed_112(%rip), %rax
	movq	%rax, 112(%rsp)
	leaq	128(%rsp), %rcx
	leaq	120(%rsp), %rdx
	leaq	112(%rsp), %r8
	callq	"断言相等"
	movl	"当前标记数值"(%rip), %eax
	movl	%eax, 104(%rsp)
	movl	$1, 96(%rsp)
	leaq	.L__unnamed_113(%rip), %rax
	movq	%rax, 88(%rsp)
	leaq	104(%rsp), %rcx
	leaq	96(%rsp), %rdx
	leaq	88(%rsp), %r8
	callq	"断言相等"
	callq	"扫描标记"
	movl	"当前标记类型"(%rip), %eax
	movl	%eax, 80(%rsp)
	movl	T_NUM(%rip), %eax
	movl	%eax, 72(%rsp)
	leaq	.L__unnamed_114(%rip), %rax
	movq	%rax, 64(%rsp)
	leaq	80(%rsp), %rcx
	leaq	72(%rsp), %rdx
	leaq	64(%rsp), %r8
	callq	"断言相等"
	movl	"当前标记数值"(%rip), %eax
	movl	%eax, 56(%rsp)
	movl	$2, 48(%rsp)
	leaq	.L__unnamed_115(%rip), %rax
	movq	%rax, 40(%rsp)
	leaq	56(%rsp), %rcx
	leaq	48(%rsp), %rdx
	leaq	40(%rsp), %r8
	callq	"断言相等"
	nop
	.seh_startepilogue
	addq	$136, %rsp
	.seh_endepilogue
	retq
	.seh_endproc
                                        # -- End function
	.def	"测试表达式基本";
	.scl	2;
	.type	32;
	.endef
	.globl	"测试表达式基本"                # -- Begin function 测试表达式基本
	.p2align	4
"测试表达式基本":                       # @"\E6\B5\8B\E8\AF\95\E8\A1\A8\E8\BE\BE\E5\BC\8F\E5\9F\BA\E6\9C\AC"
.seh_proc "测试表达式基本"
# %bb.0:                                # %entry
	subq	$152, %rsp
	.seh_stackalloc 152
	.seh_endprologue
	leaq	.L__unnamed_116(%rip), %rax
	movq	%rax, "源代码"(%rip)
	movl	$0, "源位置"(%rip)
	callq	"扫描标记"
	callq	"解析表达式"
	movl	%eax, 144(%rsp)
	movl	$5, 136(%rsp)
	leaq	.L__unnamed_117(%rip), %rax
	movq	%rax, 128(%rsp)
	leaq	144(%rsp), %rcx
	leaq	136(%rsp), %rdx
	leaq	128(%rsp), %r8
	callq	"断言相等"
	leaq	.L__unnamed_118(%rip), %rax
	movq	%rax, "源代码"(%rip)
	movl	$0, "源位置"(%rip)
	callq	"扫描标记"
	callq	"解析表达式"
	movl	%eax, 120(%rsp)
	movl	$6, 112(%rsp)
	leaq	.L__unnamed_119(%rip), %rax
	movq	%rax, 104(%rsp)
	leaq	120(%rsp), %rcx
	leaq	112(%rsp), %rdx
	leaq	104(%rsp), %r8
	callq	"断言相等"
	leaq	.L__unnamed_120(%rip), %rax
	movq	%rax, "源代码"(%rip)
	movl	$0, "源位置"(%rip)
	callq	"扫描标记"
	callq	"解析表达式"
	movl	%eax, 96(%rsp)
	movl	$21, 88(%rsp)
	leaq	.L__unnamed_121(%rip), %rax
	movq	%rax, 80(%rsp)
	leaq	96(%rsp), %rcx
	leaq	88(%rsp), %rdx
	leaq	80(%rsp), %r8
	callq	"断言相等"
	leaq	.L__unnamed_122(%rip), %rax
	movq	%rax, "源代码"(%rip)
	movl	$0, "源位置"(%rip)
	callq	"扫描标记"
	callq	"解析表达式"
	movl	%eax, 72(%rsp)
	movl	$5, 64(%rsp)
	leaq	.L__unnamed_123(%rip), %rax
	movq	%rax, 56(%rsp)
	leaq	72(%rsp), %rcx
	leaq	64(%rsp), %rdx
	leaq	56(%rsp), %r8
	callq	"断言相等"
	leaq	.L__unnamed_124(%rip), %rax
	movq	%rax, "源代码"(%rip)
	movl	$0, "源位置"(%rip)
	callq	"扫描标记"
	callq	"解析表达式"
	movl	%eax, 48(%rsp)
	movl	$2, 40(%rsp)
	leaq	.L__unnamed_125(%rip), %rax
	movq	%rax, 32(%rsp)
	leaq	48(%rsp), %rcx
	leaq	40(%rsp), %rdx
	leaq	32(%rsp), %r8
	callq	"断言相等"
	nop
	.seh_startepilogue
	addq	$152, %rsp
	.seh_endepilogue
	retq
	.seh_endproc
                                        # -- End function
	.def	"测试表达式优先级";
	.scl	2;
	.type	32;
	.endef
	.globl	"测试表达式优先级"              # -- Begin function 测试表达式优先级
	.p2align	4
"测试表达式优先级":                     # @"\E6\B5\8B\E8\AF\95\E8\A1\A8\E8\BE\BE\E5\BC\8F\E4\BC\98\E5\85\88\E7\BA\A7"
.seh_proc "测试表达式优先级"
# %bb.0:                                # %entry
	subq	$184, %rsp
	.seh_stackalloc 184
	.seh_endprologue
	leaq	.L__unnamed_126(%rip), %rax
	movq	%rax, "源代码"(%rip)
	movl	$0, "源位置"(%rip)
	callq	"扫描标记"
	callq	"解析表达式"
	movl	%eax, 176(%rsp)
	movl	$14, 168(%rsp)
	leaq	.L__unnamed_127(%rip), %rax
	movq	%rax, 160(%rsp)
	leaq	176(%rsp), %rcx
	leaq	168(%rsp), %rdx
	leaq	160(%rsp), %r8
	callq	"断言相等"
	leaq	.L__unnamed_128(%rip), %rax
	movq	%rax, "源代码"(%rip)
	movl	$0, "源位置"(%rip)
	callq	"扫描标记"
	callq	"解析表达式"
	movl	%eax, 152(%rsp)
	movl	$20, 144(%rsp)
	leaq	.L__unnamed_129(%rip), %rax
	movq	%rax, 136(%rsp)
	leaq	152(%rsp), %rcx
	leaq	144(%rsp), %rdx
	leaq	136(%rsp), %r8
	callq	"断言相等"
	leaq	.L__unnamed_130(%rip), %rax
	movq	%rax, "源代码"(%rip)
	movl	$0, "源位置"(%rip)
	callq	"扫描标记"
	callq	"解析表达式"
	movl	%eax, 128(%rsp)
	movl	$7, 120(%rsp)
	leaq	.L__unnamed_131(%rip), %rax
	movq	%rax, 112(%rsp)
	leaq	128(%rsp), %rcx
	leaq	120(%rsp), %rdx
	leaq	112(%rsp), %r8
	callq	"断言相等"
	leaq	.L__unnamed_132(%rip), %rax
	movq	%rax, "源代码"(%rip)
	movl	$0, "源位置"(%rip)
	callq	"扫描标记"
	callq	"解析表达式"
	movl	%eax, 104(%rsp)
	movl	$26, 96(%rsp)
	leaq	.L__unnamed_133(%rip), %rax
	movq	%rax, 88(%rsp)
	leaq	104(%rsp), %rcx
	leaq	96(%rsp), %rdx
	leaq	88(%rsp), %r8
	callq	"断言相等"
	leaq	.L__unnamed_134(%rip), %rax
	movq	%rax, "源代码"(%rip)
	movl	$0, "源位置"(%rip)
	callq	"扫描标记"
	callq	"解析表达式"
	movl	%eax, 80(%rsp)
	movl	$5, 72(%rsp)
	leaq	.L__unnamed_135(%rip), %rax
	movq	%rax, 64(%rsp)
	leaq	80(%rsp), %rcx
	leaq	72(%rsp), %rdx
	leaq	64(%rsp), %r8
	callq	"断言相等"
	leaq	.L__unnamed_136(%rip), %rax
	movq	%rax, "源代码"(%rip)
	movl	$0, "源位置"(%rip)
	callq	"扫描标记"
	callq	"解析表达式"
	movl	%eax, 56(%rsp)
	movl	$21, 48(%rsp)
	leaq	.L__unnamed_137(%rip), %rax
	movq	%rax, 40(%rsp)
	leaq	56(%rsp), %rcx
	leaq	48(%rsp), %rdx
	leaq	40(%rsp), %r8
	callq	"断言相等"
	nop
	.seh_startepilogue
	addq	$184, %rsp
	.seh_endepilogue
	retq
	.seh_endproc
                                        # -- End function
	.def	"测试表达式比较";
	.scl	2;
	.type	32;
	.endef
	.globl	"测试表达式比较"                # -- Begin function 测试表达式比较
	.p2align	4
"测试表达式比较":                       # @"\E6\B5\8B\E8\AF\95\E8\A1\A8\E8\BE\BE\E5\BC\8F\E6\AF\94\E8\BE\83"
.seh_proc "测试表达式比较"
# %bb.0:                                # %entry
	subq	$184, %rsp
	.seh_stackalloc 184
	.seh_endprologue
	leaq	.L__unnamed_138(%rip), %rax
	movq	%rax, "源代码"(%rip)
	movl	$0, "源位置"(%rip)
	callq	"扫描标记"
	callq	"解析表达式"
	movl	%eax, 176(%rsp)
	movl	$1, 168(%rsp)
	leaq	.L__unnamed_139(%rip), %rax
	movq	%rax, 160(%rsp)
	leaq	176(%rsp), %rcx
	leaq	168(%rsp), %rdx
	leaq	160(%rsp), %r8
	callq	"断言相等"
	leaq	.L__unnamed_140(%rip), %rax
	movq	%rax, "源代码"(%rip)
	movl	$0, "源位置"(%rip)
	callq	"扫描标记"
	callq	"解析表达式"
	movl	%eax, 152(%rsp)
	movl	$1, 144(%rsp)
	leaq	.L__unnamed_141(%rip), %rax
	movq	%rax, 136(%rsp)
	leaq	152(%rsp), %rcx
	leaq	144(%rsp), %rdx
	leaq	136(%rsp), %r8
	callq	"断言相等"
	leaq	.L__unnamed_142(%rip), %rax
	movq	%rax, "源代码"(%rip)
	movl	$0, "源位置"(%rip)
	callq	"扫描标记"
	callq	"解析表达式"
	movl	%eax, 128(%rsp)
	movl	$1, 120(%rsp)
	leaq	.L__unnamed_143(%rip), %rax
	movq	%rax, 112(%rsp)
	leaq	128(%rsp), %rcx
	leaq	120(%rsp), %rdx
	leaq	112(%rsp), %r8
	callq	"断言相等"
	leaq	.L__unnamed_144(%rip), %rax
	movq	%rax, "源代码"(%rip)
	movl	$0, "源位置"(%rip)
	callq	"扫描标记"
	callq	"解析表达式"
	movl	%eax, 104(%rsp)
	movl	$1, 96(%rsp)
	leaq	.L__unnamed_145(%rip), %rax
	movq	%rax, 88(%rsp)
	leaq	104(%rsp), %rcx
	leaq	96(%rsp), %rdx
	leaq	88(%rsp), %r8
	callq	"断言相等"
	leaq	.L__unnamed_146(%rip), %rax
	movq	%rax, "源代码"(%rip)
	movl	$0, "源位置"(%rip)
	callq	"扫描标记"
	callq	"解析表达式"
	movl	%eax, 80(%rsp)
	movl	$1, 72(%rsp)
	leaq	.L__unnamed_147(%rip), %rax
	movq	%rax, 64(%rsp)
	leaq	80(%rsp), %rcx
	leaq	72(%rsp), %rdx
	leaq	64(%rsp), %r8
	callq	"断言相等"
	leaq	.L__unnamed_148(%rip), %rax
	movq	%rax, "源代码"(%rip)
	movl	$0, "源位置"(%rip)
	callq	"扫描标记"
	callq	"解析表达式"
	movl	%eax, 56(%rsp)
	movl	$1, 48(%rsp)
	leaq	.L__unnamed_149(%rip), %rax
	movq	%rax, 40(%rsp)
	leaq	56(%rsp), %rcx
	leaq	48(%rsp), %rdx
	leaq	40(%rsp), %r8
	callq	"断言相等"
	nop
	.seh_startepilogue
	addq	$184, %rsp
	.seh_endepilogue
	retq
	.seh_endproc
                                        # -- End function
	.def	"测试表达式逻辑";
	.scl	2;
	.type	32;
	.endef
	.globl	"测试表达式逻辑"                # -- Begin function 测试表达式逻辑
	.p2align	4
"测试表达式逻辑":                       # @"\E6\B5\8B\E8\AF\95\E8\A1\A8\E8\BE\BE\E5\BC\8F\E9\80\BB\E8\BE\91"
.seh_proc "测试表达式逻辑"
# %bb.0:                                # %entry
	subq	$136, %rsp
	.seh_stackalloc 136
	.seh_endprologue
	leaq	.L__unnamed_150(%rip), %rax
	movq	%rax, "源代码"(%rip)
	movl	$0, "源位置"(%rip)
	callq	"扫描标记"
	callq	"解析表达式"
	movl	%eax, 128(%rsp)
	movl	$1, 120(%rsp)
	leaq	.L__unnamed_151(%rip), %rax
	movq	%rax, 112(%rsp)
	leaq	128(%rsp), %rcx
	leaq	120(%rsp), %rdx
	leaq	112(%rsp), %r8
	callq	"断言相等"
	leaq	.L__unnamed_152(%rip), %rax
	movq	%rax, "源代码"(%rip)
	movl	$0, "源位置"(%rip)
	callq	"扫描标记"
	callq	"解析表达式"
	movl	%eax, 104(%rsp)
	movl	$0, 96(%rsp)
	leaq	.L__unnamed_153(%rip), %rax
	movq	%rax, 88(%rsp)
	leaq	104(%rsp), %rcx
	leaq	96(%rsp), %rdx
	leaq	88(%rsp), %r8
	callq	"断言相等"
	leaq	.L__unnamed_154(%rip), %rax
	movq	%rax, "源代码"(%rip)
	movl	$0, "源位置"(%rip)
	callq	"扫描标记"
	callq	"解析表达式"
	movl	%eax, 80(%rsp)
	movl	$1, 72(%rsp)
	leaq	.L__unnamed_155(%rip), %rax
	movq	%rax, 64(%rsp)
	leaq	80(%rsp), %rcx
	leaq	72(%rsp), %rdx
	leaq	64(%rsp), %r8
	callq	"断言相等"
	leaq	.L__unnamed_156(%rip), %rax
	movq	%rax, "源代码"(%rip)
	movl	$0, "源位置"(%rip)
	callq	"扫描标记"
	callq	"解析表达式"
	movl	%eax, 56(%rsp)
	movl	$0, 48(%rsp)
	leaq	.L__unnamed_157(%rip), %rax
	movq	%rax, 40(%rsp)
	leaq	56(%rsp), %rcx
	leaq	48(%rsp), %rdx
	leaq	40(%rsp), %r8
	callq	"断言相等"
	nop
	.seh_startepilogue
	addq	$136, %rsp
	.seh_endepilogue
	retq
	.seh_endproc
                                        # -- End function
	.def	"测试表达式负数";
	.scl	2;
	.type	32;
	.endef
	.globl	"测试表达式负数"                # -- Begin function 测试表达式负数
	.p2align	4
"测试表达式负数":                       # @"\E6\B5\8B\E8\AF\95\E8\A1\A8\E8\BE\BE\E5\BC\8F\E8\B4\9F\E6\95\B0"
.seh_proc "测试表达式负数"
# %bb.0:                                # %entry
	subq	$104, %rsp
	.seh_stackalloc 104
	.seh_endprologue
	leaq	.L__unnamed_158(%rip), %rax
	movq	%rax, "源代码"(%rip)
	movl	$0, "源位置"(%rip)
	callq	"扫描标记"
	callq	"解析表达式"
	movl	%eax, 96(%rsp)
	movl	$-5, 88(%rsp)
	leaq	.L__unnamed_159(%rip), %rax
	movq	%rax, 80(%rsp)
	leaq	96(%rsp), %rcx
	leaq	88(%rsp), %rdx
	leaq	80(%rsp), %r8
	callq	"断言相等"
	leaq	.L__unnamed_160(%rip), %rax
	movq	%rax, "源代码"(%rip)
	movl	$0, "源位置"(%rip)
	callq	"扫描标记"
	callq	"解析表达式"
	movl	%eax, 72(%rsp)
	movl	$1, 64(%rsp)
	leaq	.L__unnamed_161(%rip), %rax
	movq	%rax, 56(%rsp)
	leaq	72(%rsp), %rcx
	leaq	64(%rsp), %rdx
	leaq	56(%rsp), %r8
	callq	"断言相等"
	leaq	.L__unnamed_162(%rip), %rax
	movq	%rax, "源代码"(%rip)
	movl	$0, "源位置"(%rip)
	callq	"扫描标记"
	callq	"解析表达式"
	movl	%eax, 48(%rsp)
	movl	$-12, 40(%rsp)
	leaq	.L__unnamed_163(%rip), %rax
	movq	%rax, 32(%rsp)
	leaq	48(%rsp), %rcx
	leaq	40(%rsp), %rdx
	leaq	32(%rsp), %r8
	callq	"断言相等"
	nop
	.seh_startepilogue
	addq	$104, %rsp
	.seh_endepilogue
	retq
	.seh_endproc
                                        # -- End function
	.def	"测试表达式复合";
	.scl	2;
	.type	32;
	.endef
	.globl	"测试表达式复合"                # -- Begin function 测试表达式复合
	.p2align	4
"测试表达式复合":                       # @"\E6\B5\8B\E8\AF\95\E8\A1\A8\E8\BE\BE\E5\BC\8F\E5\A4\8D\E5\90\88"
.seh_proc "测试表达式复合"
# %bb.0:                                # %entry
	subq	$88, %rsp
	.seh_stackalloc 88
	.seh_endprologue
	leaq	.L__unnamed_164(%rip), %rax
	movq	%rax, "源代码"(%rip)
	movl	$0, "源位置"(%rip)
	callq	"扫描标记"
	callq	"解析表达式"
	movl	%eax, 80(%rsp)
	movl	$13, 72(%rsp)
	leaq	.L__unnamed_165(%rip), %rax
	movq	%rax, 64(%rsp)
	leaq	80(%rsp), %rcx
	leaq	72(%rsp), %rdx
	leaq	64(%rsp), %r8
	callq	"断言相等"
	leaq	.L__unnamed_166(%rip), %rax
	movq	%rax, "源代码"(%rip)
	movl	$0, "源位置"(%rip)
	callq	"扫描标记"
	callq	"解析表达式"
	movl	%eax, 56(%rsp)
	movl	$28, 48(%rsp)
	leaq	.L__unnamed_167(%rip), %rax
	movq	%rax, 40(%rsp)
	leaq	56(%rsp), %rcx
	leaq	48(%rsp), %rdx
	leaq	40(%rsp), %r8
	callq	"断言相等"
	nop
	.seh_startepilogue
	addq	$88, %rsp
	.seh_endepilogue
	retq
	.seh_endproc
                                        # -- End function
	.def	"主程序";
	.scl	2;
	.type	32;
	.endef
	.globl	"主程序"                        # -- Begin function 主程序
	.p2align	4
"主程序":                               # @"\E4\B8\BB\E7\A8\8B\E5\BA\8F"
.seh_proc "主程序"
# %bb.0:                                # %entry
	subq	$40, %rsp
	.seh_stackalloc 40
	.seh_endprologue
	leaq	.L__unnamed_168(%rip), %rcx
	leaq	.L__unnamed_169(%rip), %rdx
	callq	printf
	leaq	.L__unnamed_170(%rip), %rcx
	leaq	.L__unnamed_171(%rip), %rdx
	callq	printf
	leaq	.L__unnamed_172(%rip), %rcx
	leaq	.L__unnamed_173(%rip), %rdx
	callq	printf
	callq	"测试词法数字"
	callq	"测试词法运算符"
	callq	"测试词法注释"
	leaq	.L__unnamed_174(%rip), %rcx
	leaq	.L__unnamed_175(%rip), %rdx
	callq	printf
	callq	"测试表达式基本"
	callq	"测试表达式优先级"
	callq	"测试表达式比较"
	callq	"测试表达式逻辑"
	callq	"测试表达式负数"
	callq	"测试表达式复合"
	leaq	.L__unnamed_176(%rip), %rcx
	leaq	.L__unnamed_177(%rip), %rdx
	callq	printf
	leaq	.L__unnamed_178(%rip), %rcx
	leaq	.L__unnamed_179(%rip), %rdx
	callq	printf
	movl	"测试通过"(%rip), %edx
	leaq	.L__unnamed_180(%rip), %rcx
	callq	printf
	leaq	.L__unnamed_181(%rip), %rcx
	leaq	.L__unnamed_182(%rip), %rdx
	callq	printf
	movl	"测试失败"(%rip), %edx
	leaq	.L__unnamed_183(%rip), %rcx
	callq	printf
	cmpl	$0, "测试失败"(%rip)
	jne	.LBB15_2
# %bb.1:                                # %then
	leaq	.L__unnamed_184(%rip), %rcx
	leaq	.L__unnamed_185(%rip), %rdx
	callq	printf
	nop
.LBB15_2:                               # %ifcont
	.seh_startepilogue
	addq	$40, %rsp
	.seh_endepilogue
	retq
	.seh_endproc
                                        # -- End function
	.def	main;
	.scl	2;
	.type	32;
	.endef
	.globl	main                            # -- Begin function main
	.p2align	4
main:                                   # @main
.seh_proc main
# %bb.0:                                # %entry
	pushq	%rbp
	.seh_pushreg %rbp
	subq	$32, %rsp
	.seh_stackalloc 32
	leaq	32(%rsp), %rbp
	.seh_setframe %rbp, 32
	.seh_endprologue
	callq	__main
	callq	"主程序"
	xorl	%eax, %eax
	.seh_startepilogue
	addq	$32, %rsp
	popq	%rbp
	.seh_endepilogue
	retq
	.seh_endproc
                                        # -- End function
	.lcomm	"__文件表",2048,16              # @"__\E6\96\87\E4\BB\B6\E8\A1\A8"
	.lcomm	"__文件句柄计数器",4,4          # @"__\E6\96\87\E4\BB\B6\E5\8F\A5\E6\9F\84\E8\AE\A1\E6\95\B0\E5\99\A8"
	.bss
	.globl	"测试通过"                      # @"\E6\B5\8B\E8\AF\95\E9\80\9A\E8\BF\87"
	.p2align	2, 0x0
"测试通过":
	.long	0                               # 0x0

	.globl	"测试失败"                      # @"\E6\B5\8B\E8\AF\95\E5\A4\B1\E8\B4\A5"
	.p2align	2, 0x0
"测试失败":
	.long	0                               # 0x0

	.section	.rdata,"dr"
".str.源代码":                          # @".str.\E6\BA\90\E4\BB\A3\E7\A0\81"
	.zero	1

	.data
	.globl	"源代码"                        # @"\E6\BA\90\E4\BB\A3\E7\A0\81"
	.p2align	3, 0x0
"源代码":
	.quad	".str.源代码"

	.bss
	.globl	"源位置"                        # @"\E6\BA\90\E4\BD\8D\E7\BD\AE"
	.p2align	2, 0x0
"源位置":
	.long	0                               # 0x0

	.globl	"当前标记类型"                  # @"\E5\BD\93\E5\89\8D\E6\A0\87\E8\AE\B0\E7\B1\BB\E5\9E\8B"
	.p2align	2, 0x0
"当前标记类型":
	.long	0                               # 0x0

	.globl	"当前标记数值"                  # @"\E5\BD\93\E5\89\8D\E6\A0\87\E8\AE\B0\E6\95\B0\E5\80\BC"
	.p2align	2, 0x0
"当前标记数值":
	.long	0                               # 0x0

	.data
	.globl	T_NUM                           # @T_NUM
	.p2align	2, 0x0
T_NUM:
	.long	1                               # 0x1

	.globl	T_IDENT                         # @T_IDENT
	.p2align	2, 0x0
T_IDENT:
	.long	2                               # 0x2

	.globl	T_PLUS                          # @T_PLUS
	.p2align	2, 0x0
T_PLUS:
	.long	3                               # 0x3

	.globl	T_MINUS                         # @T_MINUS
	.p2align	2, 0x0
T_MINUS:
	.long	4                               # 0x4

	.globl	T_STAR                          # @T_STAR
	.p2align	2, 0x0
T_STAR:
	.long	5                               # 0x5

	.globl	T_SLASH                         # @T_SLASH
	.p2align	2, 0x0
T_SLASH:
	.long	6                               # 0x6

	.globl	T_PCT                           # @T_PCT
	.p2align	2, 0x0
T_PCT:
	.long	7                               # 0x7

	.globl	T_LPAREN                        # @T_LPAREN
	.p2align	2, 0x0
T_LPAREN:
	.long	8                               # 0x8

	.globl	T_RPAREN                        # @T_RPAREN
	.p2align	2, 0x0
T_RPAREN:
	.long	9                               # 0x9

	.globl	T_LBRACE                        # @T_LBRACE
	.p2align	2, 0x0
T_LBRACE:
	.long	10                              # 0xa

	.globl	T_RBRACE                        # @T_RBRACE
	.p2align	2, 0x0
T_RBRACE:
	.long	11                              # 0xb

	.globl	T_SEMI                          # @T_SEMI
	.p2align	2, 0x0
T_SEMI:
	.long	12                              # 0xc

	.globl	T_COMMA                         # @T_COMMA
	.p2align	2, 0x0
T_COMMA:
	.long	13                              # 0xd

	.globl	T_ASSIGN                        # @T_ASSIGN
	.p2align	2, 0x0
T_ASSIGN:
	.long	14                              # 0xe

	.globl	T_EQ                            # @T_EQ
	.p2align	2, 0x0
T_EQ:
	.long	15                              # 0xf

	.globl	T_NEQ                           # @T_NEQ
	.p2align	2, 0x0
T_NEQ:
	.long	16                              # 0x10

	.globl	T_LT                            # @T_LT
	.p2align	2, 0x0
T_LT:
	.long	17                              # 0x11

	.globl	T_GT                            # @T_GT
	.p2align	2, 0x0
T_GT:
	.long	18                              # 0x12

	.globl	T_LE                            # @T_LE
	.p2align	2, 0x0
T_LE:
	.long	19                              # 0x13

	.globl	T_GE                            # @T_GE
	.p2align	2, 0x0
T_GE:
	.long	20                              # 0x14

	.globl	T_AND                           # @T_AND
	.p2align	2, 0x0
T_AND:
	.long	21                              # 0x15

	.globl	T_OR                            # @T_OR
	.p2align	2, 0x0
T_OR:
	.long	22                              # 0x16

	.globl	T_NOT                           # @T_NOT
	.p2align	2, 0x0
T_NOT:
	.long	23                              # 0x17

	.globl	T_EOF                           # @T_EOF
	.p2align	2, 0x0
T_EOF:
	.long	99                              # 0x63

	.section	.rdata,"dr"
.L__unnamed_2:                          # @0
	.asciz	"[\345\244\261\350\264\245] "

.L__unnamed_1:                          # @1
	.asciz	"%s\n"

.L__unnamed_3:                          # @2
	.asciz	"%d\n"

.L__unnamed_4:                          # @3
	.asciz	" "

.L__unnamed_6:                          # @4
	.asciz	"\t"

.L__unnamed_5:                          # @5
	.asciz	"\n"

.L__unnamed_7:                          # @6
	.asciz	"\\r"

.L__unnamed_8:                          # @7
	.asciz	"/"

.L__unnamed_9:                          # @8
	.asciz	"/"

.L__unnamed_10:                         # @9
	.asciz	"\n"

.L__unnamed_13:                         # @10
	.asciz	" "

.L__unnamed_11:                         # @11
	.asciz	"\t"

.L__unnamed_12:                         # @12
	.asciz	"\n"

.L__unnamed_14:                         # @13
	.asciz	"\\r"

.L__unnamed_15:                         # @14
	.asciz	"0"

.L__unnamed_16:                         # @15
	.asciz	"1"

.L__unnamed_17:                         # @16
	.asciz	"2"

.L__unnamed_18:                         # @17
	.asciz	"3"

.L__unnamed_19:                         # @18
	.asciz	"4"

.L__unnamed_20:                         # @19
	.asciz	"5"

.L__unnamed_21:                         # @20
	.asciz	"6"

.L__unnamed_22:                         # @21
	.asciz	"7"

.L__unnamed_23:                         # @22
	.asciz	"8"

.L__unnamed_24:                         # @23
	.asciz	"9"

.L__unnamed_29:                         # @24
	.asciz	"0"

.L__unnamed_30:                         # @25
	.asciz	"1"

.L__unnamed_31:                         # @26
	.asciz	"2"

.L__unnamed_32:                         # @27
	.asciz	"3"

.L__unnamed_33:                         # @28
	.asciz	"4"

.L__unnamed_34:                         # @29
	.asciz	"5"

.L__unnamed_35:                         # @30
	.asciz	"6"

.L__unnamed_25:                         # @31
	.asciz	"7"

.L__unnamed_26:                         # @32
	.asciz	"8"

.L__unnamed_27:                         # @33
	.asciz	"9"

.L__unnamed_36:                         # @34
	.asciz	"1"

.L__unnamed_37:                         # @35
	.asciz	"2"

.L__unnamed_38:                         # @36
	.asciz	"3"

.L__unnamed_39:                         # @37
	.asciz	"4"

.L__unnamed_40:                         # @38
	.asciz	"5"

.L__unnamed_41:                         # @39
	.asciz	"6"

.L__unnamed_42:                         # @40
	.asciz	"7"

.L__unnamed_43:                         # @41
	.asciz	"8"

.L__unnamed_28:                         # @42
	.asciz	"9"

.L__unnamed_44:                         # @43
	.asciz	"+"

.L__unnamed_45:                         # @44
	.asciz	"-"

.L__unnamed_46:                         # @45
	.asciz	"*"

.L__unnamed_47:                         # @46
	.asciz	"/"

.L__unnamed_48:                         # @47
	.asciz	"%"

.L__unnamed_49:                         # @48
	.asciz	"("

.L__unnamed_50:                         # @49
	.asciz	")"

.L__unnamed_51:                         # @50
	.asciz	"{"

.L__unnamed_52:                         # @51
	.asciz	"}"

.L__unnamed_53:                         # @52
	.asciz	";"

.L__unnamed_54:                         # @53
	.asciz	","

.L__unnamed_55:                         # @54
	.asciz	"="

.L__unnamed_56:                         # @55
	.asciz	"<"

.L__unnamed_57:                         # @56
	.asciz	">"

.L__unnamed_58:                         # @57
	.asciz	"!"

.L__unnamed_59:                         # @58
	.asciz	"&"

.L__unnamed_60:                         # @59
	.asciz	"|"

.L__unnamed_61:                         # @60
	.asciz	"\""

.L__unnamed_62:                         # @61
	.asciz	"="

.L__unnamed_63:                         # @62
	.asciz	"="

.L__unnamed_64:                         # @63
	.asciz	"!"

.L__unnamed_65:                         # @64
	.asciz	"="

.L__unnamed_66:                         # @65
	.asciz	"<"

.L__unnamed_67:                         # @66
	.asciz	"="

.L__unnamed_68:                         # @67
	.asciz	">"

.L__unnamed_69:                         # @68
	.asciz	"="

.L__unnamed_70:                         # @69
	.asciz	"&"

.L__unnamed_71:                         # @70
	.asciz	"&"

.L__unnamed_72:                         # @71
	.asciz	"|"

.L__unnamed_73:                         # @72
	.asciz	"|"

.L__unnamed_74:                         # @73
	.asciz	"+"

.L__unnamed_75:                         # @74
	.asciz	"-"

.L__unnamed_76:                         # @75
	.asciz	"*"

.L__unnamed_77:                         # @76
	.asciz	"/"

.L__unnamed_78:                         # @77
	.asciz	"%"

.L__unnamed_79:                         # @78
	.asciz	"("

.L__unnamed_80:                         # @79
	.asciz	")"

.L__unnamed_81:                         # @80
	.asciz	"{"

.L__unnamed_82:                         # @81
	.asciz	"}"

.L__unnamed_83:                         # @82
	.asciz	";"

.L__unnamed_84:                         # @83
	.asciz	","

.L__unnamed_85:                         # @84
	.asciz	"123 456"

.L__unnamed_86:                         # @85
	.asciz	"\346\225\260\345\255\227\347\261\273\345\236\213"

.L__unnamed_87:                         # @86
	.asciz	"\346\225\260\345\255\227\345\200\274"

.L__unnamed_88:                         # @87
	.asciz	"\347\254\254\344\272\214\344\270\252\346\225\260\345\255\227"

.L__unnamed_89:                         # @88
	.asciz	"\347\254\254\344\272\214\344\270\252\345\200\274"

.L__unnamed_90:                         # @89
	.asciz	"+ - * / % ( ) { } ; = == != < > <= >= && || !"

.L__unnamed_91:                         # @90
	.asciz	"+"

.L__unnamed_92:                         # @91
	.asciz	"-"

.L__unnamed_93:                         # @92
	.asciz	"*"

.L__unnamed_94:                         # @93
	.asciz	"/"

.L__unnamed_95:                         # @94
	.asciz	"%"

.L__unnamed_96:                         # @95
	.asciz	"("

.L__unnamed_97:                         # @96
	.asciz	")"

.L__unnamed_98:                         # @97
	.asciz	"{"

.L__unnamed_99:                         # @98
	.asciz	"}"

.L__unnamed_100:                        # @99
	.asciz	";"

.L__unnamed_101:                        # @100
	.asciz	"="

.L__unnamed_102:                        # @101
	.asciz	"=="

.L__unnamed_103:                        # @102
	.asciz	"!="

.L__unnamed_104:                        # @103
	.asciz	"<"

.L__unnamed_105:                        # @104
	.asciz	">"

.L__unnamed_106:                        # @105
	.asciz	"<="

.L__unnamed_107:                        # @106
	.asciz	">="

.L__unnamed_108:                        # @107
	.asciz	"&&"

.L__unnamed_109:                        # @108
	.asciz	"||"

.L__unnamed_110:                        # @109
	.asciz	"!"

.L__unnamed_111:                        # @110
	.asciz	"1 // \346\263\250\351\207\212\n2"

.L__unnamed_112:                        # @111
	.asciz	"\346\263\250\351\207\212\345\211\215"

.L__unnamed_113:                        # @112
	.asciz	"\346\263\250\351\207\212\345\211\215\345\200\274"

.L__unnamed_114:                        # @113
	.asciz	"\346\263\250\351\207\212\345\220\216"

.L__unnamed_115:                        # @114
	.asciz	"\346\263\250\351\207\212\345\220\216\345\200\274"

.L__unnamed_116:                        # @115
	.asciz	"2+3"

.L__unnamed_117:                        # @116
	.asciz	"2+3"

.L__unnamed_118:                        # @117
	.asciz	"10-4"

.L__unnamed_119:                        # @118
	.asciz	"10-4"

.L__unnamed_120:                        # @119
	.asciz	"3*7"

.L__unnamed_121:                        # @120
	.asciz	"3*7"

.L__unnamed_122:                        # @121
	.asciz	"20/4"

.L__unnamed_123:                        # @122
	.asciz	"20/4"

.L__unnamed_124:                        # @123
	.asciz	"17%5"

.L__unnamed_125:                        # @124
	.asciz	"17%5"

.L__unnamed_126:                        # @125
	.asciz	"2+3*4"

.L__unnamed_127:                        # @126
	.asciz	"2+3*4"

.L__unnamed_128:                        # @127
	.asciz	"(2+3)*4"

.L__unnamed_129:                        # @128
	.asciz	"(2+3)*4"

.L__unnamed_130:                        # @129
	.asciz	"10-6/2"

.L__unnamed_131:                        # @130
	.asciz	"10-6/2"

.L__unnamed_132:                        # @131
	.asciz	"2*3+4*5"

.L__unnamed_133:                        # @132
	.asciz	"2*3+4*5"

.L__unnamed_134:                        # @133
	.asciz	"100/10/2"

.L__unnamed_135:                        # @134
	.asciz	"100/10/2"

.L__unnamed_136:                        # @135
	.asciz	"(1+2)*(3+4)"

.L__unnamed_137:                        # @136
	.asciz	"(1+2)*(3+4)"

.L__unnamed_138:                        # @137
	.asciz	"10 > 5"

.L__unnamed_139:                        # @138
	.asciz	"10>5"

.L__unnamed_140:                        # @139
	.asciz	"3 < 7"

.L__unnamed_141:                        # @140
	.asciz	"3<7"

.L__unnamed_142:                        # @141
	.asciz	"5 == 5"

.L__unnamed_143:                        # @142
	.asciz	"5==5"

.L__unnamed_144:                        # @143
	.asciz	"5 != 3"

.L__unnamed_145:                        # @144
	.asciz	"5!=3"

.L__unnamed_146:                        # @145
	.asciz	"10 >= 10"

.L__unnamed_147:                        # @146
	.asciz	"10>=10"

.L__unnamed_148:                        # @147
	.asciz	"3 <= 5"

.L__unnamed_149:                        # @148
	.asciz	"3<=5"

.L__unnamed_150:                        # @149
	.asciz	"1 && 1"

.L__unnamed_151:                        # @150
	.asciz	"1&&1"

.L__unnamed_152:                        # @151
	.asciz	"1 && 0"

.L__unnamed_153:                        # @152
	.asciz	"1&&0"

.L__unnamed_154:                        # @153
	.asciz	"0 || 1"

.L__unnamed_155:                        # @154
	.asciz	"0||1"

.L__unnamed_156:                        # @155
	.asciz	"0 || 0"

.L__unnamed_157:                        # @156
	.asciz	"0||0"

.L__unnamed_158:                        # @157
	.asciz	"-5"

.L__unnamed_159:                        # @158
	.asciz	"-5"

.L__unnamed_160:                        # @159
	.asciz	"3+(-2)"

.L__unnamed_161:                        # @160
	.asciz	"3+(-2)"

.L__unnamed_162:                        # @161
	.asciz	"-3*4"

.L__unnamed_163:                        # @162
	.asciz	"-3*4"

.L__unnamed_164:                        # @163
	.asciz	"2+3*4-1"

.L__unnamed_165:                        # @164
	.asciz	"2+3*4-1"

.L__unnamed_166:                        # @165
	.asciz	"(10+5)*2-8/4"

.L__unnamed_167:                        # @166
	.asciz	"(10+5)*2-8/4"

.L__unnamed_169:                        # @167
	.asciz	"=== Stage 1: \345\255\220\351\233\206\347\274\226\350\257\221\345\231\250 ==="

.L__unnamed_168:                        # @168
	.asciz	"%s\n"

.L__unnamed_171:                        # @169
	.zero	1

.L__unnamed_170:                        # @170
	.asciz	"%s\n"

.L__unnamed_173:                        # @171
	.asciz	"[\350\257\215\346\263\225\345\210\206\346\236\220\345\231\250]"

.L__unnamed_172:                        # @172
	.asciz	"%s\n"

.L__unnamed_175:                        # @173
	.asciz	"[\350\241\250\350\276\276\345\274\217\346\261\202\345\200\274\345\231\250]"

.L__unnamed_174:                        # @174
	.asciz	"%s\n"

.L__unnamed_177:                        # @175
	.zero	1

.L__unnamed_176:                        # @176
	.asciz	"%s\n"

.L__unnamed_179:                        # @177
	.asciz	"  \351\200\232\350\277\207: "

.L__unnamed_178:                        # @178
	.asciz	"%s\n"

.L__unnamed_180:                        # @179
	.asciz	"%d\n"

.L__unnamed_182:                        # @180
	.asciz	"  \345\244\261\350\264\245: "

.L__unnamed_181:                        # @181
	.asciz	"%s\n"

.L__unnamed_183:                        # @182
	.asciz	"%d\n"

.L__unnamed_185:                        # @183
	.asciz	"=== Stage 1 \351\200\232\350\277\207 ==="

.L__unnamed_184:                        # @184
	.asciz	"%s\n"

