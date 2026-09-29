@echo off
set PATH=D:\src\riyue\llvm\llvm-build\bin;C:\tools\msys64\mingw64\bin;%PATH%
cd /d D:\src\riyue\riyue

set CXXFLAGS=-std=c++17 -g -finput-charset=UTF-8 -fexec-charset=UTF-8 -D_GNU_SOURCE -fexceptions -frtti -ID:/src/riyue/llvm/llvm-project/llvm/include -ID:/src/riyue/llvm/llvm-build/include -D_GLIBCXX_USE_CXX11_ABI=1 -D_FILE_OFFSET_BITS=64 -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS
set CFLAGS=-Wall -g -finput-charset=UTF-8 -fexec-charset=UTF-8
set LDFLAGS=-LD:/src/riyue/llvm/llvm-build/lib -lLLVMX86TargetMCA -lLLVMMCA -lLLVMX86Disassembler -lLLVMX86AsmParser -lLLVMX86CodeGen -lLLVMX86Desc -lLLVMX86Info -lLLVMMCDisassembler -lLLVMAsmPrinter -lLLVMOrcJIT -lLLVMPasses -lLLVMIRPrinter -lLLVMHipStdPar -lLLVMCoroutines -lLLVMipo -lLLVMInstrumentation -lLLVMVectorize -lLLVMSandboxIR -lLLVMLinker -lLLVMFrontendOpenMP -lLLVMFrontendDirective -lLLVMFrontendAtomic -lLLVMFrontendOffloading -lLLVMObjectYAML -lLLVMGlobalISel -lLLVMSelectionDAG -lLLVMCodeGen -lLLVMScalarOpts -lLLVMInstCombine -lLLVMObjCARCOpts -lLLVMCodeGenTypes -lLLVMCGData -lLLVMBitWriter -lLLVMCFGuard -lLLVMAggressiveInstCombine -lLLVMTransformUtils -lLLVMWindowsDriver -lLLVMJITLink -lLLVMOption -lLLVMExecutionEngine -lLLVMTarget -lLLVMAnalysis -lLLVMProfileData -lLLVMSymbolize -lLLVMDebugInfoBTF -lLLVMDebugInfoPDB -lLLVMDebugInfoMSF -lLLVMDebugInfoCodeView -lLLVMDebugInfoGSYM -lLLVMDebugInfoDWARF -lLLVMFrontendHLSL -lLLVMRuntimeDyld -lLLVMOrcTargetProcess -lLLVMOrcShared -lLLVMObject -lLLVMTextAPI -lLLVMMCParser -lLLVMIRReader -lLLVMAsmParser -lLLVMBitReader -lLLVMMC -lLLVMDebugInfoDWARFLowLevel -lLLVMCore -lLLVMRemarks -lLLVMBitstreamReader -lLLVMBinaryFormat -lLLVMTargetParser -lLLVMSupport -lLLVMDemangle -lpsapi -lshell32 -lole32 -luuid -ladvapi32 -lws2_32 -lntdll -lz -lzstd -lshell32 -lws2_32 -lpthread

echo === 编译前端 ===
clang++ %CXXFLAGS% -c "源/前端/词法分析器.cpp" -o "源/前端/词法分析器.o" || exit /b 1
clang++ %CXXFLAGS% -c "源/前端/语法分析器.cpp" -o "源/前端/语法分析器.o" || exit /b 1
clang++ %CXXFLAGS% -c "源/前端/模块合并器.cpp" -o "源/前端/模块合并器.o" || exit /b 1
clang++ %CXXFLAGS% -c "源/前端/诊断.cpp" -o "源/前端/诊断.o" || exit /b 1

echo === 编译后端 ===
clang++ %CXXFLAGS% -c "源/后端/代码生成器.cpp" -o "源/后端/代码生成器.o" || exit /b 1
clang++ %CXXFLAGS% -c "源/后端/代码生成器_表达式.cpp" -o "源/后端/代码生成器_表达式.o" || exit /b 1
clang++ %CXXFLAGS% -c "源/后端/代码生成器_语句.cpp" -o "源/后端/代码生成器_语句.o" || exit /b 1
clang++ %CXXFLAGS% -c "源/后端/LLVM辅助.cpp" -o "源/后端/LLVM辅助.o" || exit /b 1

echo === 编译虚拟机 ===
clang++ %CXXFLAGS% -c "源/虚拟机/字节码.cpp" -o "源/虚拟机/字节码.o" || exit /b 1
clang++ %CXXFLAGS% -c "源/虚拟机/字节码编译器.cpp" -o "源/虚拟机/字节码编译器.o" || exit /b 1
clang++ %CXXFLAGS% -c "源/虚拟机/虚拟机.cpp" -o "源/虚拟机/虚拟机.o" || exit /b 1

echo === 编译主程序 ===
clang++ %CXXFLAGS% -c "源/主程序.cpp" -o "源/主程序.o" || exit /b 1

echo === 编译运行时 ===
clang %CFLAGS% -c "源/运行时/运行时辅助.c" -o "源/运行时/运行时辅助.o" || exit /b 1

echo === 链接 ===
clang++ -o "日月.exe" "源/前端/词法分析器.o" "源/前端/语法分析器.o" "源/前端/模块合并器.o" "源/前端/诊断.o" "源/后端/代码生成器.o" "源/后端/代码生成器_表达式.o" "源/后端/代码生成器_语句.o" "源/后端/LLVM辅助.o" "源/虚拟机/字节码.o" "源/虚拟机/字节码编译器.o" "源/虚拟机/虚拟机.o" "源/主程序.o" "源/运行时/运行时辅助.o" %LDFLAGS% || exit /b 1

echo === 编译成功 ===
dir "日月.exe"