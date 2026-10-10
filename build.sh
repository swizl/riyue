#!/bin/bash
set -e
# 中文路径必须禁用 MSYS 参数转换，否则 源/前端/*.cpp 被改写导致打不开。
export MSYS2_ARG_CONV_EXCL='*'
export PATH=/d/src/riyue/llvm/llvm-build/bin:/c/tools/msys64/mingw64/bin:/usr/bin:/bin
cd /d/src/riyue/riyue

# 说明：引导.exe 仅作自举种子，无需调试信息（-g 会使单文件编译慢 3.6x），
#       故此处不加 -g；如需调试编译器本身，另用带 -g 的调试构建。
CXXFLAGS="-std=c++17 -finput-charset=UTF-8 -fexec-charset=UTF-8 -D_GNU_SOURCE -fexceptions -frtti -ID:/src/riyue/llvm/llvm-project/llvm/include -ID:/src/riyue/llvm/llvm-build/include -D_GLIBCXX_USE_CXX11_ABI=1 -D_FILE_OFFSET_BITS=64 -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS"
CFLAGS="-Wall -finput-charset=UTF-8 -fexec-charset=UTF-8"
LDFLAGS="-LD:/src/riyue/llvm/llvm-build/lib -lLLVMX86TargetMCA -lLLVMMCA -lLLVMX86Disassembler -lLLVMX86AsmParser -lLLVMX86CodeGen -lLLVMX86Desc -lLLVMX86Info -lLLVMMCDisassembler -lLLVMAsmPrinter -lLLVMOrcJIT -lLLVMPasses -lLLVMIRPrinter -lLLVMHipStdPar -lLLVMCoroutines -lLLVMipo -lLLVMInstrumentation -lLLVMVectorize -lLLVMSandboxIR -lLLVMLinker -lLLVMFrontendOpenMP -lLLVMFrontendDirective -lLLVMFrontendAtomic -lLLVMFrontendOffloading -lLLVMObjectYAML -lLLVMGlobalISel -lLLVMSelectionDAG -lLLVMCodeGen -lLLVMScalarOpts -lLLVMInstCombine -lLLVMObjCARCOpts -lLLVMCodeGenTypes -lLLVMCGData -lLLVMBitWriter -lLLVMCFGuard -lLLVMAggressiveInstCombine -lLLVMTransformUtils -lLLVMWindowsDriver -lLLVMJITLink -lLLVMOption -lLLVMExecutionEngine -lLLVMTarget -lLLVMAnalysis -lLLVMProfileData -lLLVMSymbolize -lLLVMDebugInfoBTF -lLLVMDebugInfoPDB -lLLVMDebugInfoMSF -lLLVMDebugInfoCodeView -lLLVMDebugInfoGSYM -lLLVMDebugInfoDWARF -lLLVMFrontendHLSL -lLLVMRuntimeDyld -lLLVMOrcTargetProcess -lLLVMOrcShared -lLLVMObject -lLLVMTextAPI -lLLVMMCParser -lLLVMIRReader -lLLVMAsmParser -lLLVMBitReader -lLLVMMC -lLLVMDebugInfoDWARFLowLevel -lLLVMCore -lLLVMRemarks -lLLVMBitstreamReader -lLLVMBinaryFormat -lLLVMTargetParser -lLLVMSupport -lLLVMDemangle -lpsapi -lshell32 -lole32 -luuid -ladvapi32 -lws2_32 -lntdll -lz -lzstd -lshell32 -lws2_32 -lpthread"

echo "=== 编译前端/后端/虚拟机/主程序（并行） ==="
clang++ $CXXFLAGS -c 源/前端/词法分析器.cpp       -o 源/前端/词法分析器.o &
clang++ $CXXFLAGS -c 源/前端/语法分析器.cpp       -o 源/前端/语法分析器.o &
clang++ $CXXFLAGS -c 源/前端/模块合并器.cpp       -o 源/前端/模块合并器.o &
clang++ $CXXFLAGS -c 源/前端/诊断.cpp             -o 源/前端/诊断.o &
clang++ $CXXFLAGS -c 源/后端/代码生成器.cpp       -o 源/后端/代码生成器.o &
clang++ $CXXFLAGS -c 源/后端/代码生成器_表达式.cpp -o 源/后端/代码生成器_表达式.o &
clang++ $CXXFLAGS -c 源/后端/代码生成器_语句.cpp   -o 源/后端/代码生成器_语句.o &
clang++ $CXXFLAGS -c 源/后端/LLVM辅助.cpp         -o 源/后端/LLVM辅助.o &
clang++ $CXXFLAGS -c 源/虚拟机/字节码.cpp         -o 源/虚拟机/字节码.o &
clang++ $CXXFLAGS -c 源/虚拟机/字节码编译器.cpp   -o 源/虚拟机/字节码编译器.o &
clang++ $CXXFLAGS -c 源/虚拟机/虚拟机.cpp         -o 源/虚拟机/虚拟机.o &
clang++ $CXXFLAGS -c 源/主程序.cpp                -o 源/主程序.o &
wait

echo "=== 编译运行时 ==="
clang $CFLAGS -c 源/运行时/运行时辅助.c -o 源/运行时/运行时辅助.o

echo "=== 链接（lld） ==="
clang++ -fuse-ld=lld -o 引导.exe \
  源/前端/词法分析器.o 源/前端/语法分析器.o 源/前端/模块合并器.o 源/前端/诊断.o \
  源/后端/代码生成器.o 源/后端/代码生成器_表达式.o 源/后端/代码生成器_语句.o 源/后端/LLVM辅助.o \
  源/虚拟机/字节码.o 源/虚拟机/字节码编译器.o 源/虚拟机/虚拟机.o \
  源/主程序.o 源/运行时/运行时辅助.o \
  $LDFLAGS

echo "=== 编译成功 ==="
ls -la 引导.exe