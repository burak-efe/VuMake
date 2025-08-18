# Check if dotnet-script is installed
$dotnetScriptPath = Get-Command dotnet-script -ErrorAction SilentlyContinue

if (-not $dotnetScriptPath) {
    Write-Host "dotnet-script not found, installing..."
    dotnet tool install -g dotnet-script
} else {
    Write-Host "dotnet-script is already installed."
}

dotnet script main.csx