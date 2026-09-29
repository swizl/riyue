$ErrorActionPreference = "Continue"
[Console]::OutputEncoding = [System.Text.Encoding]::UTF8
$root = "D:\src\riyue\riyue"
Set-Location $root
$env:PATH = 'D:\src\riyue\llvm\llvm-build\bin;C:\tools\msys64\mingw64\bin;C:\tools\msys64\usr\bin;' + $env:PATH
$pass = 0; $fail = 0
function 断言($名称, $条件, $详情) {
    if ($条件) { Write-Host "  OK   $名称"; $script:pass++ }
    else { Write-Host "  FAIL $名称 -- $详情"; $script:fail++ }
}

# 1) 嵌套导入应成功且输出 6
& ".\日月.exe" "测试/导入/嵌套主.心" "stage/t5_nested.exe" *> $null
断言 "嵌套导入 编译" ($LASTEXITCODE -eq 0) "exit=$LASTEXITCODE"
$out = & ".\stage\t5_nested.exe" 2>&1 | Out-String
断言 "嵌套导入 输出6" ($out -match "6") "out=$out"

# 2) 重复导入应去重且输出 11
& ".\日月.exe" "测试/导入/去重主.心" "stage/t5_dedup.exe" *> $null
断言 "重复导入 编译" ($LASTEXITCODE -eq 0) "exit=$LASTEXITCODE"
$out = & ".\stage\t5_dedup.exe" 2>&1 | Out-String
断言 "重复导入 输出11" ($out -match "11") "out=$out"

# 3) 循环导入应报错（非 0 退出）
$err = & ".\日月.exe" "测试/导入/环主.心" "stage/t5_cycle.exe" 2>&1 | Out-String
断言 "循环导入 报错" (($LASTEXITCODE -ne 0) -and ($err -match "循环导入")) "exit=$LASTEXITCODE err=$err"

# 4) 同名函数冲突应报错
$err = & ".\日月.exe" "测试/导入/冲突主.心" "stage/t5_conflict.exe" 2>&1 | Out-String
断言 "同名冲突 报错" (($LASTEXITCODE -ne 0) -and ($err -match "同名函数冲突")) "exit=$LASTEXITCODE err=$err"

# 5) 入口非末尾应输出 42
& ".\日月.exe" "测试/导入/入口非末尾.心" "stage/t5_entry.exe" *> $null
$out = & ".\stage\t5_entry.exe" 2>&1 | Out-String
断言 "入口非末尾 输出42" ($out -match "42") "out=$out"

# 6) 计划外追加：选择性导入应有明确报错（关闭 Task 2 deferred minor）
$err = & ".\日月.exe" "测试/导入/选择性应报错.心" "stage/t5_selective.exe" 2>&1 | Out-String
断言 "选择性导入 报错" (($LASTEXITCODE -ne 0) -and ($err -match "暂不支持选择性导入")) "exit=$LASTEXITCODE err=$err"

Write-Host ""
Write-Host "=== 导入测试 Pass: $pass  Fail: $fail ==="
if ($fail -gt 0) { exit 1 }
