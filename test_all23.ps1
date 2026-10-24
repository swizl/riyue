$ErrorActionPreference = 'Continue'
[Console]::OutputEncoding = [System.Text.Encoding]::UTF8
Set-Location D:\src\riyue\riyue
$env:PATH = 'D:\src\riyue\llvm\llvm-build\bin;C:\tools\msys64\mingw64\bin;C:\tools\msys64\usr\bin;' + $env:PATH
$pass = 0; $fail = 0; $fails = @()
$minPass = 203
$heart = [char]0x5fc3
$ri = [char]0x65e5; $yue = [char]0x6708
$exe = ".\$ri$yue.exe"
$pattern = [char]0x793a + [char]0x4f8b + "\*$heart"
$dir = [char]0x793a + [char]0x4f8b
$suffixOut = [char]0x671f + [char]0x671b
$suffixErr = [char]0x671f + [char]0x671b + [char]0x9519
Get-ChildItem $pattern | ForEach-Object {
  $src = $_.FullName
  $name = $_.BaseName
  Remove-Item out_t.exe,out_t.s,out_t.ll,out_t.riue -ErrorAction SilentlyContinue
  $j = Start-Job -ScriptBlock {
    param($d, $s, $exe)
    Set-Location $d
    $env:PATH = 'D:\src\riyue\llvm\llvm-build\bin;C:\tools\msys64\mingw64\bin;C:\tools\msys64\usr\bin;' + $env:PATH
    & $exe $s out_t *> $null
    $LASTEXITCODE
  } -ArgumentList (Get-Location).Path, $src, $exe
  if (Wait-Job $j -Timeout 60) { $rc = Receive-Job $j } else { Stop-Job $j; $rc = 124 }
  Remove-Job $j -Force
  if ($rc -eq 124) { Write-Host ("TIMEOUT: " + $name + " (compile)"); $fail++; $fails += ($name + "(cmp_timeout)"); return }
  if ($rc -ne 0) { Write-Host ("FAIL: " + $name + " (compile rc=" + $rc + ")"); $fail++; $fails += ($name + "(cmp" + $rc + ")"); return }
  $expectPath = Join-Path $dir ($name + "." + $suffixOut)
  $expectErrPath = Join-Path $dir ($name + "." + $suffixErr)
  $hasExpect = Test-Path $expectPath
  $hasExpectErr = Test-Path $expectErrPath
  # 采集模式：none = 只判退出码；out = 比对 stdout；err = 比对 stderr 且要求非零退出
  $mode = 'none'
  if ($hasExpectErr) { $mode = 'err' } elseif ($hasExpect) { $mode = 'out' }
  $j2 = Start-Job -ScriptBlock {
    param($d, $mode)
    Set-Location $d
    $o = Join-Path $d '_ps_out.txt'
    $e = Join-Path $d '_ps_err.txt'
    Remove-Item $o, $e -ErrorAction SilentlyContinue
    if ($mode -eq 'none') {
      & .\out_t.exe *> $null
      return @{ rc = $LASTEXITCODE; text = '' }
    }
    # 用 Start-Process 重定向到文件再按 UTF-8 读回：Start-Job 是新进程，
    # 不继承父进程的 [Console]::OutputEncoding，直接管道读原生输出会按 GBK 解码成乱码
    $p = Start-Process -FilePath (Join-Path $d 'out_t.exe') -NoNewWindow -Wait -PassThru `
      -RedirectStandardOutput $o -RedirectStandardError $e
    $text = ''
    $src = $null
    if ($mode -eq 'out') { $src = $o } elseif ($mode -eq 'err') { $src = $e }
    if ($src -and (Test-Path $src)) { $text = Get-Content -Raw -Encoding UTF8 $src }
    Remove-Item $o, $e -ErrorAction SilentlyContinue
    @{ rc = $p.ExitCode; text = $text }
  } -ArgumentList (Get-Location).Path, $mode
  if (Wait-Job $j2 -Timeout 30) { $r = Receive-Job $j2 } else { Stop-Job $j2; $r = @{ rc = 124; text = '' } }
  Remove-Job $j2 -Force
  $rc2 = $r.rc
  if ($rc2 -eq 124) { Write-Host ("TIMEOUT: " + $name + " (run)"); $fail++; $fails += ($name + "(run_timeout)") }
  elseif ($mode -eq 'err') {
    # 期望运行期「被拦截并中止」：退出码须非零，且 stderr（去 CR / 去空行）须等于 .期望错
    $actual = (($r.text -replace "`r", "") -split "`n" | Where-Object { $_.Trim() -ne '' }) -join "`n"
    $expected = ((Get-Content -Raw -Encoding UTF8 $expectErrPath) -replace "`r", "") -split "`n" | Where-Object { $_.Trim() -ne '' }
    $expected = $expected -join "`n"
    if ($rc2 -eq 0) { Write-Host ("FAIL: " + $name + " (期望中止但正常退出)"); $fail++; $fails += ($name + "(rt_ok)") }
    elseif ($actual -ne $expected) { Write-Host ("FAIL: " + $name + " (运行时诊断不匹配)"); $fail++; $fails += ($name + "(rt_err)") }
    else { Write-Host ("OK:   " + $name + " (运行期越界被拦截)"); $pass++ }
  }
  elseif ($mode -eq 'out') {
    $actual = ($r.text -replace "`r", "").TrimEnd("`n")
    $expected = ((Get-Content -Raw -Encoding UTF8 $expectPath) -replace "`r", "").TrimEnd("`n")
    if ($actual -eq $expected) { Write-Host ("OK:   " + $name + " (output match)"); $pass++ }
    else { Write-Host ("FAIL: " + $name + " (output mismatch)"); $fail++; $fails += ($name + "(out)") }
  }
  elseif ($rc2 -eq 0) { Write-Host ("OK:   " + $name); $pass++ }
  else { Write-Host ("FAIL: " + $name + " (run rc=" + $rc2 + ")"); $fail++; $fails += ($name + "(rt" + $rc2 + ")") }
}
Remove-Item out_t.* -ErrorAction SilentlyContinue
Write-Host ''
Write-Host ("=== Pass: " + $pass + "  Fail: " + $fail + " ===")
if ($fails.Count -gt 0) { Write-Host ('Failed: ' + ($fails -join ' ')) }
if ($fail -eq 0 -and $pass -ge $minPass) {
  Write-Host ("门禁: 通过 (Pass=" + $pass + " >= " + $minPass + ", Fail=0)")
  exit 0
}
Write-Host ("门禁: 未通过 (Pass=" + $pass + ", Fail=" + $fail + "；要求 Pass>=" + $minPass + " 且 Fail=0)")
exit 1
