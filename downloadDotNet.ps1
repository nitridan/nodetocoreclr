function Download-DotNet {
    $DOTNET_SDK_URL = 'https://download.microsoft.com/download/F/D/5/FD52A2F7-65B6-4912-AEDD-4015DF6D8D22/dotnet-dev-win-x64.1.0.1.zip'
    Invoke-WebRequest -Uri $DOTNET_SDK_URL -OutFile dotnet.zip
    Add-Type -assembly 'system.io.compression.filesystem'
    if (Test-Path '.\dist-dotnet'){
        Remove-Item '.\dist-dotnet' -Force -Recurse    
    }
    
    [io.compression.zipfile]::ExtractToDirectory('dotnet.zip', 'dist-dotnet')
    $env:PATH=(Convert-Path .) + '\dist-dotnet;' + $env:PATH
}