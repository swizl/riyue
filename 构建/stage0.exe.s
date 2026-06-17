	.def	@feat.00;
	.scl	3;
	.type	0;
	.endef
	.globl	@feat.00
@feat.00 = 0
	.file	"\347\250\213\345\272\217"
	.def	"断言";
	.scl	2;
	.type	32;
	.endef
	.text
	.globl	"断言"                          # -- Begin function 断言
	.p2align	4
"断言":                                 # @"\E6\96\AD\E8\A8\80"
.seh_proc "断言"
# %bb.0:                                # %entry
	pushq	%rsi
	.seh_pushreg %rsi
	subq	$32, %rsp
	.seh_stackalloc 32
	.seh_endprologue
	cmpl	$0, (%rcx)
	je	.LBB0_1
# %bb.3:                                # %then
	incl	"测试通过"(%rip)
	jmp	.LBB0_2
.LBB0_1:                                # %else
	movq	%rdx, %rsi
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
	.def	"测试整数运算";
	.scl	2;
	.type	32;
	.endef
	.globl	"测试整数运算"                  # -- Begin function 测试整数运算
	.p2align	4
"测试整数运算":                         # @"\E6\B5\8B\E8\AF\95\E6\95\B4\E6\95\B0\E8\BF\90\E7\AE\97"
.seh_proc "测试整数运算"
# %bb.0:                                # %entry
	subq	$120, %rsp
	.seh_stackalloc 120
	.seh_endprologue
	movl	$10, 36(%rsp)
	movl	$3, 32(%rsp)
	movl	$1, 112(%rsp)
	leaq	.L__unnamed_4(%rip), %rax
	movq	%rax, 104(%rsp)
	leaq	112(%rsp), %rcx
	leaq	104(%rsp), %rdx
	callq	"断言"
	movl	36(%rsp), %eax
	subl	32(%rsp), %eax
	xorl	%ecx, %ecx
	cmpl	$7, %eax
	sete	%cl
	movl	%ecx, 96(%rsp)
	leaq	.L__unnamed_5(%rip), %rax
	movq	%rax, 88(%rsp)
	leaq	96(%rsp), %rcx
	leaq	88(%rsp), %rdx
	callq	"断言"
	movl	36(%rsp), %eax
	imull	32(%rsp), %eax
	xorl	%ecx, %ecx
	cmpl	$30, %eax
	sete	%cl
	movl	%ecx, 80(%rsp)
	leaq	.L__unnamed_6(%rip), %rax
	movq	%rax, 72(%rsp)
	leaq	80(%rsp), %rcx
	leaq	72(%rsp), %rdx
	callq	"断言"
	movl	36(%rsp), %eax
	cltd
	idivl	32(%rsp)
	xorl	%ecx, %ecx
	cmpl	$3, %eax
	sete	%cl
	movl	%ecx, 64(%rsp)
	leaq	.L__unnamed_7(%rip), %rax
	movq	%rax, 56(%rsp)
	leaq	64(%rsp), %rcx
	leaq	56(%rsp), %rdx
	callq	"断言"
	movl	36(%rsp), %eax
	cltd
	idivl	32(%rsp)
	xorl	%eax, %eax
	cmpl	$1, %edx
	sete	%al
	movl	%eax, 48(%rsp)
	leaq	.L__unnamed_8(%rip), %rax
	movq	%rax, 40(%rsp)
	leaq	48(%rsp), %rcx
	leaq	40(%rsp), %rdx
	callq	"断言"
	nop
	.seh_startepilogue
	addq	$120, %rsp
	.seh_endepilogue
	retq
	.seh_endproc
                                        # -- End function
	.def	"测试比较运算";
	.scl	2;
	.type	32;
	.endef
	.globl	"测试比较运算"                  # -- Begin function 测试比较运算
	.p2align	4
"测试比较运算":                         # @"\E6\B5\8B\E8\AF\95\E6\AF\94\E8\BE\83\E8\BF\90\E7\AE\97"
.seh_proc "测试比较运算"
# %bb.0:                                # %entry
	subq	$136, %rsp
	.seh_stackalloc 136
	.seh_endprologue
	movl	$10, 36(%rsp)
	movl	$20, 32(%rsp)
	movl	$1, 128(%rsp)
	leaq	.L__unnamed_9(%rip), %rax
	movq	%rax, 120(%rsp)
	leaq	128(%rsp), %rcx
	leaq	120(%rsp), %rdx
	callq	"断言"
	movl	32(%rsp), %eax
	xorl	%ecx, %ecx
	cmpl	36(%rsp), %eax
	setg	%cl
	movl	%ecx, 112(%rsp)
	leaq	.L__unnamed_10(%rip), %rax
	movq	%rax, 104(%rsp)
	leaq	112(%rsp), %rcx
	leaq	104(%rsp), %rdx
	callq	"断言"
	movl	$1, 96(%rsp)
	leaq	.L__unnamed_11(%rip), %rax
	movq	%rax, 88(%rsp)
	leaq	96(%rsp), %rcx
	leaq	88(%rsp), %rdx
	callq	"断言"
	movl	$1, 80(%rsp)
	leaq	.L__unnamed_12(%rip), %rax
	movq	%rax, 72(%rsp)
	leaq	80(%rsp), %rcx
	leaq	72(%rsp), %rdx
	callq	"断言"
	movl	$1, 64(%rsp)
	leaq	.L__unnamed_13(%rip), %rax
	movq	%rax, 56(%rsp)
	leaq	64(%rsp), %rcx
	leaq	56(%rsp), %rdx
	callq	"断言"
	movl	36(%rsp), %eax
	xorl	%ecx, %ecx
	cmpl	32(%rsp), %eax
	setne	%cl
	movl	%ecx, 48(%rsp)
	leaq	.L__unnamed_14(%rip), %rax
	movq	%rax, 40(%rsp)
	leaq	48(%rsp), %rcx
	leaq	40(%rsp), %rdx
	callq	"断言"
	nop
	.seh_startepilogue
	addq	$136, %rsp
	.seh_endepilogue
	retq
	.seh_endproc
                                        # -- End function
	.def	"测试逻辑运算";
	.scl	2;
	.type	32;
	.endef
	.globl	"测试逻辑运算"                  # -- Begin function 测试逻辑运算
	.p2align	4
"测试逻辑运算":                         # @"\E6\B5\8B\E8\AF\95\E9\80\BB\E8\BE\91\E8\BF\90\E7\AE\97"
.seh_proc "测试逻辑运算"
# %bb.0:                                # %entry
	subq	$88, %rsp
	.seh_stackalloc 88
	.seh_endprologue
	movl	$1, 80(%rsp)
	leaq	.L__unnamed_15(%rip), %rax
	movq	%rax, 72(%rsp)
	leaq	80(%rsp), %rcx
	leaq	72(%rsp), %rdx
	callq	"断言"
	movl	$1, 64(%rsp)
	leaq	.L__unnamed_16(%rip), %rax
	movq	%rax, 56(%rsp)
	leaq	64(%rsp), %rcx
	leaq	56(%rsp), %rdx
	callq	"断言"
	movl	$1, 48(%rsp)
	leaq	.L__unnamed_17(%rip), %rax
	movq	%rax, 40(%rsp)
	leaq	48(%rsp), %rcx
	leaq	40(%rsp), %rdx
	callq	"断言"
	nop
	.seh_startepilogue
	addq	$88, %rsp
	.seh_endepilogue
	retq
	.seh_endproc
                                        # -- End function
	.def	"测试控制流";
	.scl	2;
	.type	32;
	.endef
	.globl	"测试控制流"                    # -- Begin function 测试控制流
	.p2align	4
"测试控制流":                           # @"\E6\B5\8B\E8\AF\95\E6\8E\A7\E5\88\B6\E6\B5\81"
.seh_proc "测试控制流"
# %bb.0:                                # %entry
	pushq	%rbp
	.seh_pushreg %rbp
	subq	$16, %rsp
	.seh_stackalloc 16
	leaq	16(%rsp), %rbp
	.seh_setframe %rbp, 16
	.seh_endprologue
	movl	$5, -8(%rbp)
	movl	$0, -4(%rbp)
	movb	$1, %al
	testb	%al, %al
	je	.LBB4_1
# %bb.8:                                # %then
	movl	$1, -4(%rbp)
	jmp	.LBB4_2
.LBB4_1:                                # %else
	movl	$2, -4(%rbp)
.LBB4_2:                                # %ifcont
	xorl	%edx, %edx
	cmpl	$1, -4(%rbp)
	sete	%dl
	movl	$16, %eax
	callq	___chkstk_ms
	subq	%rax, %rsp
	movq	%rsp, %rcx
	movl	%edx, (%rcx)
	movl	$16, %eax
	callq	___chkstk_ms
	subq	%rax, %rsp
	movq	%rsp, %rdx
	leaq	.L__unnamed_18(%rip), %rax
	movq	%rax, (%rdx)
	subq	$32, %rsp
	callq	"断言"
	addq	$32, %rsp
	movl	$0, -4(%rbp)
	cmpl	$10, -8(%rbp)
	jle	.LBB4_3
# %bb.6:                                # %then9
	movl	$3, -4(%rbp)
	jmp	.LBB4_5
.LBB4_3:                                # %elif0
	cmpl	$3, -8(%rbp)
	jle	.LBB4_4
# %bb.7:                                # %elifbody
	movl	$4, -4(%rbp)
	jmp	.LBB4_5
.LBB4_4:                                # %else7
	movl	$5, -4(%rbp)
.LBB4_5:                                # %ifcont8
	xorl	%edx, %edx
	cmpl	$4, -4(%rbp)
	sete	%dl
	movl	$16, %eax
	callq	___chkstk_ms
	subq	%rax, %rsp
	movq	%rsp, %rcx
	movl	%edx, (%rcx)
	movl	$16, %eax
	callq	___chkstk_ms
	subq	%rax, %rsp
	movq	%rsp, %rdx
	leaq	.L__unnamed_19(%rip), %rax
	movq	%rax, (%rdx)
	subq	$32, %rsp
	callq	"断言"
	nop
	.seh_startepilogue
	movq	%rbp, %rsp
	popq	%rbp
	.seh_endepilogue
	retq
	.seh_endproc
                                        # -- End function
	.def	"测试循环";
	.scl	2;
	.type	32;
	.endef
	.globl	"测试循环"                      # -- Begin function 测试循环
	.p2align	4
"测试循环":                             # @"\E6\B5\8B\E8\AF\95\E5\BE\AA\E7\8E\AF"
.seh_proc "测试循环"
# %bb.0:                                # %entry
	pushq	%rbp
	.seh_pushreg %rbp
	subq	$16, %rsp
	.seh_stackalloc 16
	leaq	16(%rsp), %rbp
	.seh_setframe %rbp, 16
	.seh_endprologue
	movl	$0, -4(%rbp)
	movl	$1, -8(%rbp)
	cmpl	$10, -8(%rbp)
	jg	.LBB5_3
	.p2align	4
.LBB5_2:                                # %whilebody
                                        # =>This Inner Loop Header: Depth=1
	movl	-8(%rbp), %eax
	addl	%eax, -4(%rbp)
	incl	%eax
	movl	%eax, -8(%rbp)
	cmpl	$10, -8(%rbp)
	jle	.LBB5_2
.LBB5_3:                                # %whileend
	xorl	%edx, %edx
	cmpl	$55, -4(%rbp)
	sete	%dl
	movl	$16, %eax
	callq	___chkstk_ms
	subq	%rax, %rsp
	movq	%rsp, %rcx
	movl	%edx, (%rcx)
	movl	$16, %eax
	callq	___chkstk_ms
	subq	%rax, %rsp
	movq	%rsp, %rdx
	leaq	.L__unnamed_20(%rip), %rax
	movq	%rax, (%rdx)
	subq	$32, %rsp
	callq	"断言"
	addq	$32, %rsp
	movl	$0, -4(%rbp)
	movl	$16, %eax
	callq	___chkstk_ms
	subq	%rax, %rsp
	movq	%rsp, %rax
	movl	$1, (%rax)
	cmpl	$10, (%rax)
	jg	.LBB5_6
	.p2align	4
.LBB5_5:                                # %forbody
                                        # =>This Inner Loop Header: Depth=1
	movl	(%rax), %ecx
	addl	%ecx, -4(%rbp)
	incl	(%rax)
	cmpl	$10, (%rax)
	jle	.LBB5_5
.LBB5_6:                                # %forend
	xorl	%edx, %edx
	cmpl	$55, -4(%rbp)
	sete	%dl
	movl	$16, %eax
	callq	___chkstk_ms
	subq	%rax, %rsp
	movq	%rsp, %rcx
	movl	%edx, (%rcx)
	movl	$16, %eax
	callq	___chkstk_ms
	subq	%rax, %rsp
	movq	%rsp, %rdx
	leaq	.L__unnamed_21(%rip), %rax
	movq	%rax, (%rdx)
	subq	$32, %rsp
	callq	"断言"
	nop
	.seh_startepilogue
	movq	%rbp, %rsp
	popq	%rbp
	.seh_endepilogue
	retq
	.seh_endproc
                                        # -- End function
	.def	"加法";
	.scl	2;
	.type	32;
	.endef
	.globl	"加法"                          # -- Begin function 加法
	.p2align	4
"加法":                                 # @"\E5\8A\A0\E6\B3\95"
# %bb.0:                                # %entry
	movl	(%rcx), %eax
	addl	(%rdx), %eax
	retq
                                        # -- End function
	.def	"阶乘";
	.scl	2;
	.type	32;
	.endef
	.globl	"阶乘"                          # -- Begin function 阶乘
	.p2align	4
"阶乘":                                 # @"\E9\98\B6\E4\B9\98"
.seh_proc "阶乘"
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
	cmpl	$2, (%rcx)
	jge	.LBB7_1
# %bb.2:                                # %then
	movl	$1, %eax
	jmp	.LBB7_3
.LBB7_1:                                # %else
	movl	(%rcx), %esi
	leal	-1(%rsi), %edx
	movl	$16, %eax
	callq	___chkstk_ms
	subq	%rax, %rsp
	movq	%rsp, %rcx
	movl	%edx, (%rcx)
	subq	$32, %rsp
	callq	"阶乘"
	addq	$32, %rsp
	imull	%esi, %eax
.LBB7_3:                                # %then
	.seh_startepilogue
	leaq	8(%rbp), %rsp
	popq	%rsi
	popq	%rbp
	.seh_endepilogue
	retq
	.seh_endproc
                                        # -- End function
	.def	"测试函数";
	.scl	2;
	.type	32;
	.endef
	.globl	"测试函数"                      # -- Begin function 测试函数
	.p2align	4
"测试函数":                             # @"\E6\B5\8B\E8\AF\95\E5\87\BD\E6\95\B0"
.seh_proc "测试函数"
# %bb.0:                                # %entry
	subq	$88, %rsp
	.seh_stackalloc 88
	.seh_endprologue
	movl	$3, 80(%rsp)
	movl	$4, 72(%rsp)
	leaq	80(%rsp), %rcx
	leaq	72(%rsp), %rdx
	callq	"加法"
	xorl	%ecx, %ecx
	cmpl	$7, %eax
	sete	%cl
	movl	%ecx, 64(%rsp)
	leaq	.L__unnamed_22(%rip), %rax
	movq	%rax, 56(%rsp)
	leaq	64(%rsp), %rcx
	leaq	56(%rsp), %rdx
	callq	"断言"
	movl	$5, 48(%rsp)
	leaq	48(%rsp), %rcx
	callq	"阶乘"
	xorl	%ecx, %ecx
	cmpl	$120, %eax
	sete	%cl
	movl	%ecx, 40(%rsp)
	leaq	.L__unnamed_23(%rip), %rax
	movq	%rax, 32(%rsp)
	leaq	40(%rsp), %rcx
	leaq	32(%rsp), %rdx
	callq	"断言"
	nop
	.seh_startepilogue
	addq	$88, %rsp
	.seh_endepilogue
	retq
	.seh_endproc
                                        # -- End function
	.def	"测试数组";
	.scl	2;
	.type	32;
	.endef
	.globl	"测试数组"                      # -- Begin function 测试数组
	.p2align	4
"测试数组":                             # @"\E6\B5\8B\E8\AF\95\E6\95\B0\E7\BB\84"
.seh_proc "测试数组"
# %bb.0:                                # %entry
	subq	$104, %rsp
	.seh_stackalloc 104
	.seh_endprologue
	movabsq	$85899345930, %rax              # imm = 0x140000000A
	movq	%rax, 84(%rsp)
	movabsq	$171798691870, %rax             # imm = 0x280000001E
	movq	%rax, 92(%rsp)
	movl	$50, 100(%rsp)
	movl	$1, 72(%rsp)
	leaq	.L__unnamed_24(%rip), %rax
	movq	%rax, 64(%rsp)
	leaq	72(%rsp), %rcx
	leaq	64(%rsp), %rdx
	callq	"断言"
	xorl	%eax, %eax
	cmpl	$30, 92(%rsp)
	sete	%al
	movl	%eax, 56(%rsp)
	leaq	.L__unnamed_25(%rip), %rax
	movq	%rax, 48(%rsp)
	leaq	56(%rsp), %rcx
	leaq	48(%rsp), %rdx
	callq	"断言"
	movl	$99, 88(%rsp)
	movl	$1, 40(%rsp)
	leaq	.L__unnamed_26(%rip), %rax
	movq	%rax, 32(%rsp)
	leaq	40(%rsp), %rcx
	leaq	32(%rsp), %rdx
	callq	"断言"
	nop
	.seh_startepilogue
	addq	$104, %rsp
	.seh_endepilogue
	retq
	.seh_endproc
                                        # -- End function
	.def	"测试字符串";
	.scl	2;
	.type	32;
	.endef
	.globl	"测试字符串"                    # -- Begin function 测试字符串
	.p2align	4
"测试字符串":                           # @"\E6\B5\8B\E8\AF\95\E5\AD\97\E7\AC\A6\E4\B8\B2"
.seh_proc "测试字符串"
# %bb.0:                                # %entry
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$80, %rsp
	.seh_stackalloc 80
	.seh_endprologue
	leaq	.L__unnamed_27(%rip), %rcx
	movq	%rcx, 72(%rsp)
	callq	"获取字符数"
	xorl	%ecx, %ecx
	cmpl	$4, %eax
	sete	%cl
	movl	%ecx, 56(%rsp)
	leaq	.L__unnamed_28(%rip), %rax
	movq	%rax, 48(%rsp)
	leaq	56(%rsp), %rcx
	leaq	48(%rsp), %rdx
	callq	"断言"
	leaq	.L__unnamed_29(%rip), %rsi
	movq	%rsi, %rcx
	callq	strlen
	movq	%rax, %rdi
	leaq	.L__unnamed_30(%rip), %rbx
	movq	%rbx, %rcx
	callq	strlen
	leaq	1(%rdi,%rax), %rcx
	callq	malloc
	movq	%rax, %rdi
	leaq	.L__unnamed_31(%rip), %rdx
	movq	%rax, %rcx
	movq	%rsi, %r8
	movq	%rbx, %r9
	callq	sprintf
	movq	%rdi, 64(%rsp)
	movq	%rdi, %rcx
	callq	"获取字符数"
	xorl	%ecx, %ecx
	cmpl	$4, %eax
	sete	%cl
	movl	%ecx, 40(%rsp)
	leaq	.L__unnamed_32(%rip), %rax
	movq	%rax, 32(%rsp)
	leaq	40(%rsp), %rcx
	leaq	32(%rsp), %rdx
	callq	"断言"
	nop
	.seh_startepilogue
	addq	$80, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	.seh_endepilogue
	retq
	.seh_endproc
                                        # -- End function
	.def	"测试中断继续";
	.scl	2;
	.type	32;
	.endef
	.globl	"测试中断继续"                  # -- Begin function 测试中断继续
	.p2align	4
"测试中断继续":                         # @"\E6\B5\8B\E8\AF\95\E4\B8\AD\E6\96\AD\E7\BB\A7\E7\BB\AD"
.seh_proc "测试中断继续"
# %bb.0:                                # %entry
	pushq	%rbp
	.seh_pushreg %rbp
	subq	$16, %rsp
	.seh_stackalloc 16
	leaq	16(%rsp), %rbp
	.seh_setframe %rbp, 16
	.seh_endprologue
	movl	$0, -8(%rbp)
	movl	$0, -4(%rbp)
	.p2align	4
.LBB11_1:                               # %whilecond
                                        # =>This Inner Loop Header: Depth=1
	movl	-4(%rbp), %eax
	incl	%eax
	movl	%eax, -4(%rbp)
	cmpl	$11, %eax
	jge	.LBB11_4
# %bb.2:                                # %else
                                        #   in Loop: Header=BB11_1 Depth=1
	movl	-4(%rbp), %eax
	movl	%eax, %ecx
	shrl	$31, %ecx
	addl	%eax, %ecx
	andl	$-2, %ecx
	cmpl	%ecx, %eax
	je	.LBB11_1
# %bb.3:                                # %else5
                                        #   in Loop: Header=BB11_1 Depth=1
	movl	-4(%rbp), %eax
	addl	%eax, -8(%rbp)
	jmp	.LBB11_1
.LBB11_4:                               # %whileend
	xorl	%edx, %edx
	cmpl	$25, -8(%rbp)
	sete	%dl
	movl	$16, %eax
	callq	___chkstk_ms
	subq	%rax, %rsp
	movq	%rsp, %rcx
	movl	%edx, (%rcx)
	movl	$16, %eax
	callq	___chkstk_ms
	subq	%rax, %rsp
	movq	%rsp, %rdx
	leaq	.L__unnamed_33(%rip), %rax
	movq	%rax, (%rdx)
	subq	$32, %rsp
	callq	"断言"
	nop
	.seh_startepilogue
	movq	%rbp, %rsp
	popq	%rbp
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
	leaq	.L__unnamed_34(%rip), %rcx
	leaq	.L__unnamed_35(%rip), %rdx
	callq	printf
	callq	"测试整数运算"
	callq	"测试比较运算"
	callq	"测试逻辑运算"
	callq	"测试控制流"
	callq	"测试循环"
	callq	"测试函数"
	callq	"测试数组"
	callq	"测试字符串"
	callq	"测试中断继续"
	leaq	.L__unnamed_36(%rip), %rcx
	leaq	.L__unnamed_37(%rip), %rdx
	callq	printf
	movl	"测试通过"(%rip), %edx
	leaq	.L__unnamed_38(%rip), %rcx
	callq	printf
	leaq	.L__unnamed_39(%rip), %rcx
	leaq	.L__unnamed_40(%rip), %rdx
	callq	printf
	movl	"测试失败"(%rip), %edx
	leaq	.L__unnamed_41(%rip), %rcx
	callq	printf
	cmpl	$0, "测试失败"(%rip)
	jne	.LBB12_2
# %bb.1:                                # %then
	leaq	.L__unnamed_42(%rip), %rcx
	leaq	.L__unnamed_43(%rip), %rdx
	callq	printf
	nop
.LBB12_2:                               # %ifcont
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
.L__unnamed_2:                          # @0
	.asciz	"[\345\244\261\350\264\245] "

.L__unnamed_1:                          # @1
	.asciz	"%s\n"

.L__unnamed_3:                          # @2
	.asciz	"%d\n"

.L__unnamed_4:                          # @3
	.asciz	"\345\212\240\346\263\225"

.L__unnamed_5:                          # @4
	.asciz	"\345\207\217\346\263\225"

.L__unnamed_6:                          # @5
	.asciz	"\344\271\230\346\263\225"

.L__unnamed_7:                          # @6
	.asciz	"\351\231\244\346\263\225"

.L__unnamed_8:                          # @7
	.asciz	"\345\217\226\346\250\241"

.L__unnamed_9:                          # @8
	.asciz	"\345\260\217\344\272\216"

.L__unnamed_10:                         # @9
	.asciz	"\345\244\247\344\272\216"

.L__unnamed_11:                         # @10
	.asciz	"\345\260\217\344\272\216\347\255\211\344\272\216"

.L__unnamed_12:                         # @11
	.asciz	"\345\244\247\344\272\216\347\255\211\344\272\216"

.L__unnamed_13:                         # @12
	.asciz	"\347\255\211\344\272\216"

.L__unnamed_14:                         # @13
	.asciz	"\344\270\215\347\255\211\344\272\216"

.L__unnamed_15:                         # @14
	.asciz	"\344\270\216"

.L__unnamed_16:                         # @15
	.asciz	"\346\210\226"

.L__unnamed_17:                         # @16
	.asciz	"\351\235\236"

.L__unnamed_18:                         # @17
	.asciz	"\345\246\202\346\236\234"

.L__unnamed_19:                         # @18
	.asciz	"\345\220\246\345\210\231\345\246\202\346\236\234"

.L__unnamed_20:                         # @19
	.asciz	"\345\275\223\345\276\252\347\216\257"

.L__unnamed_21:                         # @20
	.asciz	"\345\276\252\347\216\257"

.L__unnamed_22:                         # @21
	.asciz	"\345\207\275\346\225\260\350\260\203\347\224\250"

.L__unnamed_23:                         # @22
	.asciz	"\351\200\222\345\275\222"

.L__unnamed_24:                         # @23
	.asciz	"\346\225\260\347\273\204\350\256\277\351\227\256"

.L__unnamed_25:                         # @24
	.asciz	"\346\225\260\347\273\204\350\256\277\351\227\2562"

.L__unnamed_26:                         # @25
	.asciz	"\346\225\260\347\273\204\350\265\213\345\200\274"

.L__unnamed_27:                         # @26
	.asciz	"\344\275\240\345\245\275\344\270\226\347\225\214"

.L__unnamed_28:                         # @27
	.asciz	"\345\255\227\347\254\246\344\270\262\351\225\277\345\272\246"

.L__unnamed_29:                         # @28
	.asciz	"\344\275\240\345\245\275"

.L__unnamed_30:                         # @29
	.asciz	"\344\270\226\347\225\214"

.L__unnamed_31:                         # @30
	.asciz	"%s%s"

.L__unnamed_32:                         # @31
	.asciz	"\346\213\274\346\216\245\351\225\277\345\272\246"

.L__unnamed_33:                         # @32
	.asciz	"\344\270\255\346\226\255\347\273\247\347\273\255"

.L__unnamed_35:                         # @33
	.asciz	"=== Stage 0: \347\247\215\345\255\220\347\274\226\350\257\221\345\231\250\351\252\214\350\257\201 ==="

.L__unnamed_34:                         # @34
	.asciz	"%s\n"

.L__unnamed_37:                         # @35
	.asciz	"  \351\200\232\350\277\207: "

.L__unnamed_36:                         # @36
	.asciz	"%s\n"

.L__unnamed_38:                         # @37
	.asciz	"%d\n"

.L__unnamed_40:                         # @38
	.asciz	"  \345\244\261\350\264\245: "

.L__unnamed_39:                         # @39
	.asciz	"%s\n"

.L__unnamed_41:                         # @40
	.asciz	"%d\n"

.L__unnamed_43:                         # @41
	.asciz	"=== Stage 0 \351\200\232\350\277\207 ==="

.L__unnamed_42:                         # @42
	.asciz	"%s\n"

