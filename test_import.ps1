$ErrorActionPreference = "Continue"
[Console]::OutputEncoding = [System.Text.Encoding]::UTF8
$root = "D:\src\riyue\riyue"
Set-Location $root
$env:PATH = 'D:\src\riyue\llvm\llvm-build\bin;C:\tools\msys64\mingw64\bin;C:\tools\msys64\usr\bin;' + $env:PATH
# 已知限制：中文输出路径暂不支持（PowerShell 向原生 exe 传中文 argv 会被 ANSI 重编码，
# 导致 clang/ld 打不开输出文件），故本脚本一律使用 ASCII 输出路径 阶段/t5_*.exe。
New-Item -ItemType Directory -Force -Path (Join-Path $root '阶段') | Out-Null
$pass = 0; $fail = 0
function 断言($名称, $条件, $详情) {
    if ($条件) { Write-Host "  OK   $名称"; $script:pass++ }
    else { Write-Host "  FAIL $名称 -- $详情"; $script:fail++ }
}

# ---- 超时保护：照抄 test_all23.ps1 的 Start-Job + Wait-Job + Stop/Remove-Job 模式 ----
# 编译：30s 超时；返回退出码（124 表示超时）
function 运行编译($输入, $输出) {
    $j = Start-Job -ScriptBlock {
        param($d, $i, $o)
        Set-Location $d
        $env:PATH = 'D:\src\riyue\llvm\llvm-build\bin;C:\tools\msys64\mingw64\bin;C:\tools\msys64\usr\bin;' + $env:PATH
        & ".\日月.exe" $i $o *> $null
        $LASTEXITCODE
    } -ArgumentList $root, $输入, $输出
    if (Wait-Job $j -Timeout 30) { $rc = Receive-Job $j } else { Stop-Job $j; $rc = 124 }
    Remove-Job $j -Force
    return $rc
}

# 编译并捕获合并输出：30s 超时；返回对象 { rc; out }（rc=124 表示超时）
function 运行捕获($输入, $输出) {
    $j = Start-Job -ScriptBlock {
        param($d, $i, $o)
        Set-Location $d
        $env:PATH = 'D:\src\riyue\llvm\llvm-build\bin;C:\tools\msys64\mingw64\bin;C:\tools\msys64\usr\bin;' + $env:PATH
        try { [Console]::OutputEncoding = [System.Text.Encoding]::UTF8 } catch {}
        $t = & ".\日月.exe" $i $o 2>&1 | Out-String
        [pscustomobject]@{ rc = $LASTEXITCODE; out = $t }
    } -ArgumentList $root, $输入, $输出
    if (Wait-Job $j -Timeout 30) { $r = Receive-Job $j } else { Stop-Job $j; $r = [pscustomobject]@{ rc = 124; out = 'TIMEOUT' } }
    Remove-Job $j -Force
    return $r
}

# 运行被测 exe：10s 超时；返回对象 { rc; out }（rc=124 表示超时）
function 运行程序($路径) {
    $j = Start-Job -ScriptBlock {
        param($d, $p)
        Set-Location $d
        try { [Console]::OutputEncoding = [System.Text.Encoding]::UTF8 } catch {}
        if (-not (Test-Path -LiteralPath $p)) { [pscustomobject]@{ rc = 127; out = '' }; return }
        $t = & $p 2>&1 | Out-String
        [pscustomobject]@{ rc = $LASTEXITCODE; out = $t }
    } -ArgumentList $root, $路径
    if (Wait-Job $j -Timeout 10) { $r = Receive-Job $j } else { Stop-Job $j; $r = [pscustomobject]@{ rc = 124; out = 'TIMEOUT' } }
    Remove-Job $j -Force
    return $r
}

# 1) 嵌套导入应成功且输出 6
$rc = 运行编译 "测试/导入/嵌套主.心" "阶段/t5_nested.exe"
if ($rc -eq 124) { 断言 "嵌套导入 编译" $false "TIMEOUT" } else { 断言 "嵌套导入 编译" ($rc -eq 0) "exit=$rc" }
$r = 运行程序 ".\阶段\t5_nested.exe"
if ($r.rc -eq 124) { 断言 "嵌套导入 输出6" $false "TIMEOUT" } else { 断言 "嵌套导入 输出6" ($r.out.Trim() -eq "6") "out=$($r.out)" }

# 2) 重复导入应去重且输出 11
$rc = 运行编译 "测试/导入/去重主.心" "阶段/t5_dedup.exe"
if ($rc -eq 124) { 断言 "重复导入 编译" $false "TIMEOUT" } else { 断言 "重复导入 编译" ($rc -eq 0) "exit=$rc" }
$r = 运行程序 ".\阶段\t5_dedup.exe"
if ($r.rc -eq 124) { 断言 "重复导入 输出11" $false "TIMEOUT" } else { 断言 "重复导入 输出11" ($r.out.Trim() -eq "11") "out=$($r.out)" }

# 3) 循环导入应报错（非 0 退出）
$r = 运行捕获 "测试/导入/环主.心" "阶段/t5_cycle.exe"
if ($r.rc -eq 124) { 断言 "循环导入 报错" $false "TIMEOUT" } else { 断言 "循环导入 报错" (($r.rc -ne 0) -and ($r.out -match "循环导入")) "exit=$($r.rc) err=$($r.out)" }

# 4) 同名函数冲突应报错
$r = 运行捕获 "测试/导入/冲突主.心" "阶段/t5_conflict.exe"
if ($r.rc -eq 124) { 断言 "同名冲突 报错" $false "TIMEOUT" } else { 断言 "同名冲突 报错" (($r.rc -ne 0) -and ($r.out -match "同名函数冲突")) "exit=$($r.rc) err=$($r.out)" }

# 5) 入口非末尾应输出 42
$rc = 运行编译 "测试/导入/入口非末尾.心" "阶段/t5_entry.exe"
$r = 运行程序 ".\阶段\t5_entry.exe"
if ($rc -eq 124 -or $r.rc -eq 124) { 断言 "入口非末尾 输出42" $false "TIMEOUT" } else { 断言 "入口非末尾 输出42" ($r.out.Trim() -eq "42") "out=$($r.out)" }

# 6) 计划外追加：选择性导入应有明确报错（关闭 Task 2 deferred minor）
$r = 运行捕获 "测试/导入/选择性应报错.心" "阶段/t5_selective.exe"
if ($r.rc -eq 124) { 断言 "选择性导入 报错" $false "TIMEOUT" } else { 断言 "选择性导入 报错" (($r.rc -ne 0) -and ($r.out -match "暂不支持选择性导入")) "exit=$($r.rc) err=$($r.out)" }

Remove-Item (Join-Path $root '阶段\t5_*.exe'), (Join-Path $root '阶段\t5_*.o') -ErrorAction SilentlyContinue
Write-Host ""
Write-Host "=== 导入测试 Pass: $pass  Fail: $fail ==="
if ($fail -gt 0) { exit 1 }
