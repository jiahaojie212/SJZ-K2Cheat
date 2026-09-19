$ErrorActionPreference = 'Stop'
[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12
$proxy = 'http://127.0.0.1:7890'
$dest = 'D:\OLLVM\lib\clang\17\include'

# wipe clang22 headers
Remove-Item "$dest\*" -Recurse -Force -ErrorAction SilentlyContinue

# list headers from llvm-project source tag 17.0.6
$items = Invoke-RestMethod -Uri 'https://api.github.com/repos/llvm/llvm-project/contents/clang/lib/Headers?ref=llvmorg-17.0.6' -Proxy $proxy -TimeoutSec 120
$files = $items | Where-Object { $_.type -eq 'file' -and ($_.name -match '\.(h|inc)$') }
Write-Host "FOUND: $($files.Count) header files"

$ok = 0; $fail = 0
foreach ($f in $files) {
  $out = Join-Path $dest $f.name
  & curl.exe -sSk --proxy $proxy --connect-timeout 15 --max-time 60 -o $out $f.download_url
  if ((Test-Path $out) -and ((Get-Item $out).Length -gt 0)) { $ok++ } else { $fail++; Write-Host "FAIL: $($f.name)" }
}
Write-Host "DOWNLOADED: $ok OK, $fail FAIL"
Write-Host ("stddef.h: " + (Test-Path "$dest\stddef.h") + "  stdarg.h: " + (Test-Path "$dest\stdarg.h") + "  immintrin.h: " + (Test-Path "$dest\immintrin.h"))
