Push-Location out

$Listener = New-Object Net.HttpListener
$Listener.Prefixes.Add("http://127.0.0.1:8000/")
$Listener.Start()
Write-Output "HTTP server started at http://localhost:8000"

try {
    while ($Listener.IsListening) {
        $ContextTask = $Listener.GetContextAsync()

        # Spin wait in 10ms increments to allow Ctrl+C to come through
        while (-not $ContextTask.AsyncWaitHandle.WaitOne(10)) { }
        $Context = $ContextTask.GetAwaiter().GetResult()

        $Path = ($Context.Request).RawUrl
        Write-Output "Request ${Path}"

        if ($Path -eq "/") {
            $Path = "/index.html"
        }

        $Response = $Context.Response
        if ($Path.EndsWith(".html")) {
            $Response.Headers.Add("Content-Type","text/html");
        }
        elseif ($Path.EndsWith(".js")) {
            $Response.Headers.Add("Content-Type","application/javascript");
        }
        elseif ($Path.EndsWith(".wasm")) {
            $Response.Headers.Add("Content-Type","application/wasm");
        }
        elseif ($Path.EndsWith(".png")) {
            $Response.Headers.Add("Content-Type","image/png");
        }
        elseif ($Path.EndsWith(".jpg")) {
            $Response.Headers.Add("Content-Type","image/jpeg");
        }
        else {
            $Response.Headers.Add("Content-Type","application/octet-stream");
        }

        $FullPath = Join-Path $Pwd $Path
        if (Test-Path $FullPath) {
            $Buffer = [System.IO.File]::ReadAllBytes($FullPath)
            $Response.ContentLength64 = $Buffer.Length
            $Response.OutputStream.Write($Buffer, 0, $Buffer.Length)
        }
        else {
            $Response.StatusCode = 404;
        }
        $Response.Close()
    }
}
finally {
    $Listener.Stop()
    Pop-Location
}