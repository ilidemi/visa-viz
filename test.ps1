$ErrorActionPreference = "Stop"
$Env:PATH = "C:\Portable\wasi-sdk-12.0\bin;C:\Program Files\Wasmtime\bin;$Env:PATH"

Remove-Item -Recurse out_test\*
if (-not (Test-Path out_test)) {
    New-Item out_test -ItemType Directory | Out-Null
}

clang test\test.cpp --sysroot C:\Portable\wasi-sdk-12.0\share\wasi-sysroot -Wall -Werror -O0 `
    -g -o out_test\test.wasm

wasmtime out_test\test.wasm
