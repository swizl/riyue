#如果未定义 C运行时声明_H
#定义 C运行时声明_H

#包含 "llvm/IR/IRBuilder.h"
#包含 "llvm/IR/Module.h"
#包含 "llvm/IR/LLVMContext.h"
#包含 "../共享/LLVM中文.h"

取用 名域 llvm中文;

构型 C运行时函数 {
    静态 虚空型 预声明全部(llvm::LLVMContext& 上下文, llvm::Module& 模块) {
        类型* i8Ptr = 获取指针类型(上下文);
        类型* i32 = 整型32(上下文);
        类型* i64 = 整型64(上下文);
        类型* f64 = 双精度浮点(上下文);

        自动 声明 = [&](恒常 文本& 名称, 类型* 返回类型, 数组向量<类型*> 参数类型, 真假型 变长 = 假值) {
            函数类型* 类型 = 获取函数类型(返回类型, 参数类型, 变长);
            获取或插入函数(模块, 名称, 类型);
        };

        声明("malloc", i8Ptr, {i64});
        声明("strlen", i64, {i8Ptr});
        声明("sprintf", i32, {i8Ptr, i8Ptr}, 真值);
        声明("printf", i32, {i8Ptr}, 真值);
        声明("fopen", i8Ptr, {i8Ptr, i8Ptr});
        声明("fclose", i32, {i8Ptr});
        声明("fprintf", i32, {i8Ptr, i8Ptr}, 真值);
        声明("fscanf", i32, {i8Ptr, i8Ptr}, 真值);
        声明("fgets", i8Ptr, {i8Ptr, i32, i8Ptr});
        声明("strcmp", i32, {i8Ptr, i8Ptr});
        声明("strcpy", i8Ptr, {i8Ptr, i8Ptr});
        声明("strncpy", i8Ptr, {i8Ptr, i8Ptr, i64});
        声明("strstr", i8Ptr, {i8Ptr, i8Ptr});
        声明("atoi", i32, {i8Ptr});
        声明("atof", f64, {i8Ptr});
        声明("sqrt", f64, {f64});
        声明("sin", f64, {f64});
        声明("cos", f64, {f64});
        声明("tan", f64, {f64});
        声明("pow", f64, {f64, f64});
        声明("round", f64, {f64});
        声明("fabs", f64, {f64});
        声明("log", f64, {f64});
        声明("log10", f64, {f64});

        声明("__gxx_personality_seh0", i32, {}, 真值);
        声明("__cxa_allocate_exception", i8Ptr, {i64});
        声明("__cxa_throw", 获取空类型(上下文), {i8Ptr, i8Ptr, i8Ptr}, 假值);
        声明("__cxa_begin_catch", i8Ptr, {i8Ptr});
        声明("__cxa_end_catch", 获取空类型(上下文), {});
        声明("_Unwind_Resume", 获取空类型(上下文), {i8Ptr});

        声明("创建通道函数", i8Ptr, {i32});
        声明("发送到通道函数", i32, {i8Ptr, i32});
        声明("从通道接收函数", i32, {i8Ptr});
        声明("通道是否为空", i32, {i8Ptr});
        声明("通道是否已关闭", i32, {i8Ptr});
        声明("关闭通道函数", 获取空类型(上下文), {i8Ptr});

        // 协程（协作式单线程：Windows 纤程）。创建不执行；首次恢复执行至让出；
        // 让出/恢复值以语言层「整数」（i32）传递；协程体结束后恢复归返 0。
        声明("创建协程对象", i8Ptr, {i8Ptr, i8Ptr});
        声明("协程让出", 获取空类型(上下文), {i32});
        声明("协程恢复", i32, {i8Ptr});

        声明("断点命中", 获取空类型(上下文), {i8Ptr, i32, i8Ptr});
        声明("设置调试器", 获取空类型(上下文), {i32});
        声明("记录行执行", 获取空类型(上下文), {i32});
        声明("输出覆盖率报告", 获取空类型(上下文), {});
        声明("设置异常类型", 获取空类型(上下文), {i8Ptr, i32});
        声明("获取异常类型", i32, {i8Ptr});
        声明("清除异常类型", 获取空类型(上下文), {i8Ptr});
        声明("HTTP获取", i8Ptr, {i8Ptr});
        声明("写入文件", i32, {i8Ptr, i8Ptr});
        声明("读取文件", i8Ptr, {i8Ptr});
        声明("异步写入文件", i32, {i8Ptr, i8Ptr});
        声明("异步读取文件", i32, {i8Ptr});
        声明("IO是否完成", i32, {i32});
        声明("IO获取结果", i32, {i32});
        声明("IO获取读取结果", i8Ptr, {i32});
        声明("异步IO等待", i8Ptr, {i32});
        声明("等待IO完成", i32, {i32});
        声明("创建TCP客户端", i32, {});
        声明("TCP连接", i32, {i32, i8Ptr, i32});
        声明("TCP发送", i32, {i32, i8Ptr});
        声明("TCP接收", i8Ptr, {i32});
        声明("TCP关闭", 获取空类型(上下文), {i32});
        声明("创建TCP服务器", i32, {i32});
        声明("TCP接受连接", i32, {i32});
        声明("创建UDP套接字", i32, {});
        声明("UDP发送到", i32, {i32, i8Ptr, i8Ptr, i32});
        声明("注册GC对象", 获取空类型(上下文), {i8Ptr});
        声明("标记对象", 获取空类型(上下文), {i8Ptr});
        声明("执行GC", 获取空类型(上下文), {});
        声明("启用GC", 获取空类型(上下文), {});
        声明("设置GC阈值", 获取空类型(上下文), {i32});
        声明("获取GC对象数量", i32, {});
        // 阶段3B：ARC 引用计数共享对象（句柄为整数槽号，0 = 无效）
        声明("共享新建", i32, {i32});
        声明("共享保留", i32, {i32});
        声明("共享引用数", i32, {i32});
        声明("共享容量", i32, {i32});
        声明("共享释放", i32, {i32});
        声明("共享取整数", i32, {i32, i32});
        声明("共享存整数", i32, {i32, i32, i32});
        声明("共享存活数", i32, {});
        声明("共享总分配数", i32, {});
        声明("共享总释放数", i32, {});
        // 阶段10 G11：并发原语（互斥锁 / 信号量 / 原子整数 / 并行内核；句柄为整数槽号，0 = 无效）
        声明("创建互斥锁", i32, {});
        声明("互斥锁加锁", i32, {i32});
        声明("互斥锁解锁", i32, {i32});
        声明("互斥锁是否锁定", i32, {i32});
        声明("互斥锁争用次数", i32, {i32});
        声明("销毁互斥锁", i32, {i32});
        声明("创建信号量", i32, {i32});
        声明("信号量等待", i32, {i32});
        声明("信号量释放", i32, {i32});
        声明("信号量当前值", i32, {i32});
        声明("销毁信号量", i32, {i32});
        声明("创建原子整数", i32, {i32});
        声明("原子读", i32, {i32});
        声明("原子写", i32, {i32, i32});
        声明("原子加", i32, {i32, i32});
        声明("原子减", i32, {i32, i32});
        声明("原子比较交换", i32, {i32, i32, i32});
        声明("原子交换", i32, {i32, i32});
        声明("原子与", i32, {i32, i32});
        声明("原子或", i32, {i32, i32});
        声明("原子异或", i32, {i32, i32});
        声明("内存屏障", 获取空类型(上下文), {});
        声明("销毁原子整数", i32, {i32});
        声明("同步存活数", i32, {});
        // 阶段10 G11-D：条件变量（等待-通知）与读写锁（多读共享 / 写独占）
        声明("创建条件变量", i32, {});
        声明("条件变量等待", i32, {i32, i32});
        声明("条件变量唤醒", i32, {i32});
        声明("条件变量唤醒全部", i32, {i32});
        声明("销毁条件变量", i32, {i32});
        声明("创建读写锁", i32, {});
        声明("读锁加锁", i32, {i32});
        声明("读锁解锁", i32, {i32});
        声明("写锁加锁", i32, {i32});
        声明("写锁解锁", i32, {i32});
        声明("销毁读写锁", i32, {i32});
        // 阶段9 G9：标准流 IO（LSP 服务端 stdio 帧收发）
        声明("读取标准输入行", i8Ptr, {});
        声明("读取标准输入字节", i8Ptr, {i32});
        声明("写入标准输出", i32, {i8Ptr});
        声明("字节长度", i32, {i8Ptr});
        声明("并行求和", i32, {i8Ptr, i32, i32});
        声明("并行计数正", i32, {i8Ptr, i32, i32});
        // 阶段10 G11-B：抢占式调度 + 写共享数组的并行
        声明("抢占式调度运行", i32, {i32});
        声明("并行填充", i32, {i8Ptr, i32, i32, i32});
        声明("并行变换加", i32, {i8Ptr, i32, i32, i32});
        // 阶段8 G10：FFI —— 动态库符号调用（库名/符号名为裸字节串，实参/返回均为 i32）
        声明("外部调用0", i32, {i8Ptr, i8Ptr});
        声明("外部调用1", i32, {i8Ptr, i8Ptr, i32});
        声明("外部调用2", i32, {i8Ptr, i8Ptr, i32, i32});
        声明("外部调用3", i32, {i8Ptr, i8Ptr, i32, i32, i32});
        声明("外部调用4", i32, {i8Ptr, i8Ptr, i32, i32, i32, i32});
        // 阶段P4：FFI 浮点增量 —— 实参/返回均为双精度（C ABI 走 XMM 寄存器）。
        声明("外部调用浮点0", f64, {i8Ptr, i8Ptr});
        声明("外部调用浮点1", f64, {i8Ptr, i8Ptr, f64});
        声明("外部调用浮点2", f64, {i8Ptr, i8Ptr, f64, f64});
        声明("外部调用浮点3", f64, {i8Ptr, i8Ptr, f64, f64, f64});
        声明("外部调用浮点4", f64, {i8Ptr, i8Ptr, f64, f64, f64, f64});
        声明("外部调用长整0", i64, {i8Ptr, i8Ptr});
        声明("外部调用长整1", i64, {i8Ptr, i8Ptr, i64});
        声明("外部调用长整2", i64, {i8Ptr, i8Ptr, i64, i64});
        声明("外部调用长整3", i64, {i8Ptr, i8Ptr, i64, i64, i64});
        声明("外部调用长整4", i64, {i8Ptr, i8Ptr, i64, i64, i64, i64});
        // 阶段P4 回调增量：末位为 C 函数指针（i8Ptr），其余为 i32，返回 i32。
        声明("外部调用回调1", i32, {i8Ptr, i8Ptr, i8Ptr});
        声明("外部调用回调2", i32, {i8Ptr, i8Ptr, i32, i8Ptr});
        声明("外部调用回调3", i32, {i8Ptr, i8Ptr, i32, i32, i8Ptr});
        声明("外部调用回调4", i32, {i8Ptr, i8Ptr, i32, i32, i32, i8Ptr});
        // 阶段P4 带参回调增量：末位为 C 函数指针 int (*)(int)，其余为 i32，返回 i32。
        声明("外部调用回调1_1", i32, {i8Ptr, i8Ptr, i8Ptr});
        声明("外部调用回调1_2", i32, {i8Ptr, i8Ptr, i32, i8Ptr});
        声明("外部调用回调1_3", i32, {i8Ptr, i8Ptr, i32, i32, i8Ptr});
        声明("外部调用回调1_4", i32, {i8Ptr, i8Ptr, i32, i32, i32, i8Ptr});
        // 阶段P4 指针增量：字符串/指针（i8Ptr）与整数（i32）混用，入口名 外部调用<模式>_<返回>。
        声明("外部调用_P", i8Ptr, {i8Ptr, i8Ptr});
        声明("外部调用I_P", i8Ptr, {i8Ptr, i8Ptr, i32});
        声明("外部调用P_I", i32, {i8Ptr, i8Ptr, i8Ptr});
        声明("外部调用P_P", i8Ptr, {i8Ptr, i8Ptr, i8Ptr});
        声明("外部调用II_P", i8Ptr, {i8Ptr, i8Ptr, i32, i32});
        声明("外部调用IP_I", i32, {i8Ptr, i8Ptr, i32, i8Ptr});
        声明("外部调用IP_P", i8Ptr, {i8Ptr, i8Ptr, i32, i8Ptr});
        声明("外部调用PI_I", i32, {i8Ptr, i8Ptr, i8Ptr, i32});
        声明("外部调用PI_P", i8Ptr, {i8Ptr, i8Ptr, i8Ptr, i32});
        声明("外部调用PP_I", i32, {i8Ptr, i8Ptr, i8Ptr, i8Ptr});
        声明("外部调用PP_P", i8Ptr, {i8Ptr, i8Ptr, i8Ptr, i8Ptr});
        声明("外部调用III_P", i8Ptr, {i8Ptr, i8Ptr, i32, i32, i32});
        声明("外部调用IIP_I", i32, {i8Ptr, i8Ptr, i32, i32, i8Ptr});
        声明("外部调用IIP_P", i8Ptr, {i8Ptr, i8Ptr, i32, i32, i8Ptr});
        声明("外部调用IPI_I", i32, {i8Ptr, i8Ptr, i32, i8Ptr, i32});
        声明("外部调用IPI_P", i8Ptr, {i8Ptr, i8Ptr, i32, i8Ptr, i32});
        声明("外部调用IPP_I", i32, {i8Ptr, i8Ptr, i32, i8Ptr, i8Ptr});
        声明("外部调用IPP_P", i8Ptr, {i8Ptr, i8Ptr, i32, i8Ptr, i8Ptr});
        声明("外部调用PII_I", i32, {i8Ptr, i8Ptr, i8Ptr, i32, i32});
        声明("外部调用PII_P", i8Ptr, {i8Ptr, i8Ptr, i8Ptr, i32, i32});
        声明("外部调用PIP_I", i32, {i8Ptr, i8Ptr, i8Ptr, i32, i8Ptr});
        声明("外部调用PIP_P", i8Ptr, {i8Ptr, i8Ptr, i8Ptr, i32, i8Ptr});
        声明("外部调用PPI_I", i32, {i8Ptr, i8Ptr, i8Ptr, i8Ptr, i32});
        声明("外部调用PPI_P", i8Ptr, {i8Ptr, i8Ptr, i8Ptr, i8Ptr, i32});
        声明("外部调用PPP_I", i32, {i8Ptr, i8Ptr, i8Ptr, i8Ptr, i8Ptr});
        声明("外部调用PPP_P", i8Ptr, {i8Ptr, i8Ptr, i8Ptr, i8Ptr, i8Ptr});
        声明("外部调用IIII_P", i8Ptr, {i8Ptr, i8Ptr, i32, i32, i32, i32});
        声明("外部调用IIIP_I", i32, {i8Ptr, i8Ptr, i32, i32, i32, i8Ptr});
        声明("外部调用IIIP_P", i8Ptr, {i8Ptr, i8Ptr, i32, i32, i32, i8Ptr});
        声明("外部调用IIPI_I", i32, {i8Ptr, i8Ptr, i32, i32, i8Ptr, i32});
        声明("外部调用IIPI_P", i8Ptr, {i8Ptr, i8Ptr, i32, i32, i8Ptr, i32});
        声明("外部调用IIPP_I", i32, {i8Ptr, i8Ptr, i32, i32, i8Ptr, i8Ptr});
        声明("外部调用IIPP_P", i8Ptr, {i8Ptr, i8Ptr, i32, i32, i8Ptr, i8Ptr});
        声明("外部调用IPII_I", i32, {i8Ptr, i8Ptr, i32, i8Ptr, i32, i32});
        声明("外部调用IPII_P", i8Ptr, {i8Ptr, i8Ptr, i32, i8Ptr, i32, i32});
        声明("外部调用IPIP_I", i32, {i8Ptr, i8Ptr, i32, i8Ptr, i32, i8Ptr});
        声明("外部调用IPIP_P", i8Ptr, {i8Ptr, i8Ptr, i32, i8Ptr, i32, i8Ptr});
        声明("外部调用IPPI_I", i32, {i8Ptr, i8Ptr, i32, i8Ptr, i8Ptr, i32});
        声明("外部调用IPPI_P", i8Ptr, {i8Ptr, i8Ptr, i32, i8Ptr, i8Ptr, i32});
        声明("外部调用IPPP_I", i32, {i8Ptr, i8Ptr, i32, i8Ptr, i8Ptr, i8Ptr});
        声明("外部调用IPPP_P", i8Ptr, {i8Ptr, i8Ptr, i32, i8Ptr, i8Ptr, i8Ptr});
        声明("外部调用PIII_I", i32, {i8Ptr, i8Ptr, i8Ptr, i32, i32, i32});
        声明("外部调用PIII_P", i8Ptr, {i8Ptr, i8Ptr, i8Ptr, i32, i32, i32});
        声明("外部调用PIIP_I", i32, {i8Ptr, i8Ptr, i8Ptr, i32, i32, i8Ptr});
        声明("外部调用PIIP_P", i8Ptr, {i8Ptr, i8Ptr, i8Ptr, i32, i32, i8Ptr});
        声明("外部调用PIPI_I", i32, {i8Ptr, i8Ptr, i8Ptr, i32, i8Ptr, i32});
        声明("外部调用PIPI_P", i8Ptr, {i8Ptr, i8Ptr, i8Ptr, i32, i8Ptr, i32});
        声明("外部调用PIPP_I", i32, {i8Ptr, i8Ptr, i8Ptr, i32, i8Ptr, i8Ptr});
        声明("外部调用PIPP_P", i8Ptr, {i8Ptr, i8Ptr, i8Ptr, i32, i8Ptr, i8Ptr});
        声明("外部调用PPII_I", i32, {i8Ptr, i8Ptr, i8Ptr, i8Ptr, i32, i32});
        声明("外部调用PPII_P", i8Ptr, {i8Ptr, i8Ptr, i8Ptr, i8Ptr, i32, i32});
        声明("外部调用PPIP_I", i32, {i8Ptr, i8Ptr, i8Ptr, i8Ptr, i32, i8Ptr});
        声明("外部调用PPIP_P", i8Ptr, {i8Ptr, i8Ptr, i8Ptr, i8Ptr, i32, i8Ptr});
        声明("外部调用PPPI_I", i32, {i8Ptr, i8Ptr, i8Ptr, i8Ptr, i8Ptr, i32});
        声明("外部调用PPPI_P", i8Ptr, {i8Ptr, i8Ptr, i8Ptr, i8Ptr, i8Ptr, i32});
        声明("外部调用PPPP_I", i32, {i8Ptr, i8Ptr, i8Ptr, i8Ptr, i8Ptr, i8Ptr});
        声明("外部调用PPPP_P", i8Ptr, {i8Ptr, i8Ptr, i8Ptr, i8Ptr, i8Ptr, i8Ptr});
        // 阶段P4 浮点混用增量：模式串 F=双精度（XMM），可与 I（整数）/ P（指针）混用。
        声明("外部调用P_F", f64, {i8Ptr, i8Ptr, i8Ptr});
        声明("外部调用FI_F", f64, {i8Ptr, i8Ptr, f64, i32});
        声明("外部调用PF_I", i32, {i8Ptr, i8Ptr, i8Ptr, f64});
        声明("外部调用PIF_I", i32, {i8Ptr, i8Ptr, i8Ptr, i32, f64});
        声明("外部调用PFF_I", i32, {i8Ptr, i8Ptr, i8Ptr, f64, f64});
        声明("外部调用PPF_I", i32, {i8Ptr, i8Ptr, i8Ptr, i8Ptr, f64});
    }
};

#结束