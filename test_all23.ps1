$ErrorActionPreference = 'Continue'
[Console]::OutputEncoding = [System.Text.Encoding]::UTF8
Set-Location D:\src\riyue\riyue
$env:PATH = 'D:\src\riyue\llvm\llvm-build\bin;C:\tools\msys64\mingw64\bin;C:\tools\msys64\usr\bin;' + $env:PATH
$pass = 0; $fail = 0; $fails = @()
$heart = [char]0x5fc3
$ri = [char]0x65e5; $yue = [char]0x6708
$exe = ".\$ri$yue.exe"
$pattern = [char]0x793a + [char]0x4f8b + "\*$heart"
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
  if (Wait-Job $j -Timeout 30) { $rc = Receive-Job $j } else { Stop-Job $j; $rc = 124 }
  Remove-Job $j -Force
  if ($rc -eq 124) { Write-Host ("TIMEOUT: " + $name + " (compile)"); $fail++; $fails += ($name + "(cmp_timeout)"); return }
  if ($rc -ne 0) { Write-Host ("FAIL: " + $name + " (compile rc=" + $rc + ")"); $fail++; $fails += ($name + "(cmp" + $rc + ")"); return }
  $expectPath = Join-Path '示例' ($name + [char]0x671f + [char]0x671b)
  $hasExpect = Test-Path $expectPath
  $j2 = Start-Job -ScriptBlock {
    param($d, $capture)
    Set-Location $d
    if ($capture) {
      $out = & .\out_t.exe 2>$null | Out-String
      @{ rc = $LASTEXITCODE; out = $out }
    } else {
      & .\out_t.exe *> $null
      @{ rc = $LASTEXITCODE; out = '' }
    }
  } -ArgumentList (Get-Location).Path, $hasExpect
  if (Wait-Job $j2 -Timeout 10) { $r = Receive-Job $j2 } else { Stop-Job $j2; $r = @{ rc = 124; out = '' } }
  Remove-Job $j2 -Force
  $rc2 = $r.rc
  if ($rc2 -eq 124) { Write-Host ("TIMEOUT: " + $name + " (run)"); $fail++; $fails += ($name + "(run_timeout)") }
  elseif ($hasExpect) {
    $actual = ($r.out -replace "`r", "").TrimEnd("`n")
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
