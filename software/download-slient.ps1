$ErrorActionPreference = "Stop"
$ProgressPreference = "SilentlyContinue"
Invoke-WebRequest -Uri $args[0] -OutFile $args[1]
