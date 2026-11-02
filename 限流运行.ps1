# 限流运行.ps1 —— 用 Windows Job Object 给测试进程树加硬上限
#
# 背景：执行沙箱只限制文件读写与网络，不提供内存/进程数配额；MSYS2 的 ulimit
# 在 Windows 上不生效。run_tests.sh 里的「剩余提交内存 < 800MB 即中止」是事后
# 软守卫；本脚本在内核层硬限，超限的进程直接分配失败，不会把整机提交额度打穿
# （此前并发编译曾触发 MSYS2 fork 半残 + bugcheck 0xEF 重启）。
#
# 用法（仓库根，单独串行，勿与其他编译/回归并发）：
#   powershell -NoProfile -ExecutionPolicy Bypass -File .\限流运行.ps1
#   powershell ... -File .\限流运行.ps1 -脚本 "run_tests.sh --分片 1/8"
#   powershell ... -File .\限流运行.ps1 -脚本 不动点验证.sh -内存上限MB 4096
#
# 说明：
#   - 作业开启 KILL_ON_JOB_CLOSE：本脚本退出时，作业内残留子进程一并被杀。
#   - 内存上限统计的是「提交内存」（committed），不是虚拟地址保留。
#   - 触顶时的表现是子进程分配失败（用例判失败），而不是整机死机。
#   - 与 run_tests.sh 的 --分片 配合：一片的进程树独占一个作业，片内用例串行执行，
#     因此作业上限≈单用例上限；坏用例只会死在片内。

param(
    [int]$内存上限MB = 4096,
    [int]$单进程上限MB = 2048,
    [int]$最大进程数 = 24,
    [string]$脚本 = 'run_tests.sh',
    [string]$Bash = 'C:\tools\msys64\usr\bin\bash.exe'
)

$ErrorActionPreference = 'Stop'

Add-Type -TypeDefinition @'
using System;
using System.Runtime.InteropServices;

[StructLayout(LayoutKind.Sequential)]
public struct IO_COUNTERS
{
    public ulong ReadOperationCount;
    public ulong WriteOperationCount;
    public ulong OtherOperationCount;
    public ulong ReadTransferCount;
    public ulong WriteTransferCount;
    public ulong OtherTransferCount;
}

[StructLayout(LayoutKind.Sequential)]
public struct JOBOBJECT_BASIC_LIMIT_INFORMATION
{
    public long PerProcessUserTimeLimit;
    public long PerJobUserTimeLimit;
    public uint LimitFlags;
    public UIntPtr MinimumWorkingSetSize;
    public UIntPtr MaximumWorkingSetSize;
    public uint ActiveProcessLimit;
    public UIntPtr Affinity;
    public uint PriorityClass;
    public uint SchedulingClass;
}

[StructLayout(LayoutKind.Sequential)]
public struct JOBOBJECT_EXTENDED_LIMIT_INFORMATION
{
    public JOBOBJECT_BASIC_LIMIT_INFORMATION BasicLimitInformation;
    public IO_COUNTERS IoInfo;
    public UIntPtr ProcessMemoryLimit;
    public UIntPtr JobMemoryLimit;
    public UIntPtr PeakProcessMemoryUsed;
    public UIntPtr PeakJobMemoryUsed;
}

public static class JobHardCap
{
    [DllImport("kernel32.dll", CharSet = CharSet.Unicode, SetLastError = true)]
    public static extern IntPtr CreateJobObject(IntPtr attrs, string name);

    [DllImport("kernel32.dll", SetLastError = true)]
    public static extern bool SetInformationJobObject(IntPtr job, int infoClass, IntPtr info, uint len);

    [DllImport("kernel32.dll", SetLastError = true)]
    public static extern bool AssignProcessToJobObject(IntPtr job, IntPtr process);

    [DllImport("kernel32.dll")]
    public static extern IntPtr GetCurrentProcess();
}
'@

# 0x100 PROCESS_MEMORY | 0x200 JOB_MEMORY | 0x8 ACTIVE_PROCESS | 0x2000 KILL_ON_JOB_CLOSE
$LIMIT_FLAGS = 0x00000100 -bor 0x00000200 -bor 0x00000008 -bor 0x00002000
$JobObjectExtendedLimitInformation = 9

$job = [JobHardCap]::CreateJobObject([IntPtr]::Zero, $null)
if ($job -eq [IntPtr]::Zero) {
    throw "CreateJobObject 失败（Win32 $([Runtime.InteropServices.Marshal]::GetLastWin32Error())）"
}

$basic = New-Object JOBOBJECT_BASIC_LIMIT_INFORMATION
$basic.LimitFlags = [uint32]$LIMIT_FLAGS
$basic.ActiveProcessLimit = [uint32]$最大进程数

$info = New-Object JOBOBJECT_EXTENDED_LIMIT_INFORMATION
$info.BasicLimitInformation = $basic
$info.ProcessMemoryLimit = [UIntPtr]::new([uint64]$单进程上限MB * 1MB)
$info.JobMemoryLimit = [UIntPtr]::new([uint64]$内存上限MB * 1MB)

$size = [Runtime.InteropServices.Marshal]::SizeOf([type]'JOBOBJECT_EXTENDED_LIMIT_INFORMATION')
$buf = [Runtime.InteropServices.Marshal]::AllocHGlobal($size)
try {
    [Runtime.InteropServices.Marshal]::StructureToPtr($info, $buf, $false)
    $ok = [JobHardCap]::SetInformationJobObject($job, $JobObjectExtendedLimitInformation, $buf, [uint32]$size)
} finally {
    [Runtime.InteropServices.Marshal]::FreeHGlobal($buf)
}
if (-not $ok) {
    throw "SetInformationJobObject 失败（Win32 $([Runtime.InteropServices.Marshal]::GetLastWin32Error())），可能不支持该限制项"
}

# 先把本进程加入作业，子进程自动继承，避免 Start 与 Assign 之间的竞态窗口。
$selfAssigned = [JobHardCap]::AssignProcessToJobObject($job, [JobHardCap]::GetCurrentProcess())
if (-not $selfAssigned) {
    Write-Warning "本进程无法加入作业（Win32 $([Runtime.InteropServices.Marshal]::GetLastWin32Error())），改为只指派子进程（存在极小竞态窗口）"
}

Write-Host ("作业硬限：总提交 {0} MB / 单进程 {1} MB / 最大进程 {2}；脚本 {3}" -f $内存上限MB, $单进程上限MB, $最大进程数, $脚本)

# 登录 shell 的 profile 可能把 cwd 切到 HOME，故显式 cd 回仓库根（POSIX 形式路径）。
$root = (Get-Location).Path
$msysRoot = ($root -replace '\\', '/') -replace '^([A-Za-z]):', '/$1'

$psi = New-Object System.Diagnostics.ProcessStartInfo
$psi.FileName = $Bash
$psi.Arguments = '-lc "cd ' + $msysRoot + ' && bash ' + $脚本 + '"'
$psi.UseShellExecute = $false
$psi.WorkingDirectory = $root
$psi.EnvironmentVariables['MSYSTEM'] = 'MINGW64'
$psi.EnvironmentVariables['MSYS_NO_PATHCONV'] = '1'
$psi.EnvironmentVariables['MSYS2_ARG_CONV_EXCL'] = '*'

$p = [System.Diagnostics.Process]::Start($psi)
if (-not $selfAssigned) {
    if (-not [JobHardCap]::AssignProcessToJobObject($job, $p.Handle)) {
        Write-Warning "子进程指派失败（Win32 $([Runtime.InteropServices.Marshal]::GetLastWin32Error())），本次运行**没有**硬上限"
    }
}

$p.WaitForExit()
$code = $p.ExitCode
# 不显式 CloseHandle：本进程退出时句柄关闭 → 作业关闭 → 残留子进程被杀。
Write-Host ("子进程退出码：{0}" -f $code)
exit $code
