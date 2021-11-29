param(
    [array]$emccArgs
)

$ErrorActionPreference = "Stop"

# Update these when (re)installing Emscripten
$EmsdkPath = "C:\Portable\emsdk"
$EmsdkPythonPath = "$EmsdkPath\python\3.7.4-pywin32_64bit"
$EmsdkNodePath = "$EmsdkPath\node\12.18.1_64bit\bin"
$EmsdkJavaPath = "$EmsdkPath\java\8.152_64bit"

$Env:EMSDK_PY = "$EmsdkPythonPath/python.exe"
$Env:PYTHONHOME = ""
$Env:PYTHONPATH = ""
$Env:EMSDK = "$EmsdkPath"
$Env:EMSDK_NODE = "$EmsdkNodePath/node.exe"
$Env:EMSDK_PYTHON = "$EmsdkPythonPath/python.exe"
$Env:EM_CACHE = "$EmsdkPath/upstream/emscripten/cache"
$Env:EM_CONFIG = "$EmsdkPath/.emscripten"
$Env:EM_PY = "$EmsdkPythonPath/python.exe"
$Env:JAVA_HOME = $EmsdkJavaPath
$Env:PATH = "$EmsdkPath;$EmsdkPath/upstream/emscripten;$EmsdkNodePath;$EmsdkPythonPath;$EmsdkJavaPath/bin;$Env:PATH"

Write-Output "emcc start"
emcc src\main.cpp --js-library "src\shell\library.js" "-lwebgl.js" -s TEXTDECODER=2 -s MINIMAL_RUNTIME=0 `
    -s MIN_WEBGL_VERSION=1 -s MAX_WEBGL_VERSION=2 -s ENVIRONMENT=web -s MIN_FIREFOX_VERSION=70 `
    -s MIN_SAFARI_VERSION=130000 -s MIN_IE_VERSION=0x7FFFFFFF -s MIN_EDGE_VERSION=79 `
    -s MIN_CHROME_VERSION=80 -s ABORTING_MALLOC=0 -s GL_SUPPORT_AUTOMATIC_ENABLE_EXTENSIONS=0 `
    -s GL_EXTENSIONS_IN_PREFIXED_FORMAT=0 -s GL_EMULATE_GLES_VERSION_STRING_FORMAT=0 `
    -s GL_SUPPORT_SIMPLE_ENABLE_EXTENSIONS=0 -s SUPPORT_ERRNO=0 -s ALLOW_MEMORY_GROWTH=1 `
    -s "EXPORTED_FUNCTIONS=[`"_main`", `"_locationHashChangeCallback`", `"_imageLoadedCallback`"]" `
    -s EXPORTED_RUNTIME_METHODS=["ccall"] -Wall -Werror `
    @emccArgs

if ($LASTEXITCODE -eq 0) {
    Write-Output "emcc finish"
}
else {
    Write-Error "emcc failed (return code $LASTEXITCODE)"
}
