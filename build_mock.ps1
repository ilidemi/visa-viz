$ErrorActionPreference = "Stop"

Remove-Item -Recurse out\*
if (-not (Test-Path out)) {
    New-Item out -ItemType Directory | Out-Null
}
Copy-Item res\* out\
Copy-Item src\shell\index.html out\
New-Item out\media -ItemType Directory | Out-Null
Copy-Item -Recurse media_mock\* out\media\
Write-Output "Initialized out"

.\scripts\build.ps1 -emccArgs @(`
    "--preload-file", "data_mock@data", "-O0", "-g", "-DDATA_MOCK", "-o", "out\index.js" `
)

Write-Output "All done"
