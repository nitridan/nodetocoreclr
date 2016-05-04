function Download-DotNet {
    $DOTNET_SDK_URL = 'https://dotnetcli.blob.core.windows.net/dotnet/beta/Binaries/Latest/dotnet-dev-win-x64.latest.zip'
    Invoke-WebRequest -Uri $DOTNET_SDK_URL -OutFile dotnet.zip
    Add-Type -assembly 'system.io.compression.filesystem'
    if (Test-Path '.\dist-dotnet'){
        Remove-Item '.\dist-dotnet' -Force -Recurse    
    }
    
    [io.compression.zipfile]::ExtractToDirectory('dotnet.zip', 'dist-dotnet')
    $env:PATH=(Convert-Path .) + '\dist-dotnet;' + $env:PATH
}