param($buildNumber = 1,
    $electronVersion = '1.2.5',
    $nodeVersion = '6.2.2',
    [switch]
    $localDotNet)
    
$ATOM_SHELL_URL = 'https://atom.io/download/atom-shell'
$NATIVE_VERSION='1.3.' + $buildNumber

function Force-Copy($source, $destination){
    New-Item -ItemType File -Path $destination -Force
    Copy-Item $source $destination -Force
}

# If specified we will prepend PATH with local dotnet CLI SDK and install node-gyp
if ($localDotNet){
    . .\downloadDotNet.ps1
    Download-DotNet
    & npm install -g node-gyp
    if ($LASTEXITCODE -ne 0){
        Write-Output 'Failed to restore gyp compile dependencies'
        exit 1
    }
}

# Build native and JS part
$nodenativeDir = $PSScriptRoot + '\nodenative'
$outputPath = $nodenativeDir + '\build\Release\ClrLoader.node'
$distributiveDir = $nodenativeDir + '\dist';
$x86binaryTarget = $distributiveDir + '\ClrLoader_x86.node'
$x64binaryTarget = $distributiveDir + '\ClrLoader_x64.node'
Set-Location $nodenativeDir
& npm install
if ($LASTEXITCODE -ne 0){
    Write-Host 'Failed to restore npm dependencies'
    exit 1    
}

& node-gyp rebuild --arch=x64 --target=$nodeVersion
if ($LASTEXITCODE -ne 0){
    Write-Host 'Failed to build for node.js x64'
    exit 1    
}

if (Test-Path $distributiveDir){
    Remove-Item $distributiveDir -Force -Recurse    
}

Force-Copy $outputPath $x64binaryTarget
& node-gyp rebuild --arch=ia32 --target=$nodeVersion
if ($LASTEXITCODE -ne 0){
    Write-Host 'Failed to build for node.js x86'
    exit 1    
}

Force-Copy $outputPath $x86binaryTarget
Get-ChildItem -path $nodenativeDir | Where { $_.Extension -eq '.js' } `
    | Copy-Item -destination {$_.FullName -replace [System.Text.RegularExpressions.Regex]::Escape($nodenativeDir),$distributiveDir} -Force

Force-Copy ($nodenativeDir + '\package_dist.json') ($distributiveDir + '\package.json')
Set-Location $distributiveDir
& npm version $NATIVE_VERSION
if ($LASTEXITCODE -ne 0){
    Write-Host 'Failed to set node.js package version'
}

& npm pack
if ($LASTEXITCODE -ne 0){
    Write-Host 'Failed to create package for node.js'
}

#Build for electron
$electronDistributiveDir = $nodenativeDir + '\dist-electron';
$electronX86binaryTarget = $electronDistributiveDir + '\ClrLoader_x86.node'
$electronX64binaryTarget = $electronDistributiveDir + '\ClrLoader_x64.node'
Set-Location $nodenativeDir
& node-gyp rebuild --arch=x64 --target=$electronVersion --dist-url=$ATOM_SHELL_URL
if ($LASTEXITCODE -ne 0){
    Write-Host 'Failed to build for electron x64'
    exit 1    
}

if (Test-Path $electronDistributiveDir){
    Remove-Item $electronDistributiveDir -Force -Recurse    
}

Force-Copy $outputPath $electronX64binaryTarget
& node-gyp rebuild --arch=ia32 --target=$electronVersion --dist-url=$ATOM_SHELL_URL
if ($LASTEXITCODE -ne 0){
    Write-Host 'Failed to build for electron x86'
    exit 1    
}

Force-Copy $outputPath $electronX86binaryTarget
Get-ChildItem -path $nodenativeDir | Where { $_.Extension -eq '.js' } `
    | Copy-Item -destination {$_.FullName -replace [System.Text.RegularExpressions.Regex]::Escape($nodenativeDir),$electronDistributiveDir} -Force

Force-Copy ($nodenativeDir + '\package_dist.json') ($electronDistributiveDir + '\package.json')
Set-Location $electronDistributiveDir
& npm version $NATIVE_VERSION
if ($LASTEXITCODE -ne 0){
    Write-Host 'Failed to set electron package version'
}

& npm pack
if ($LASTEXITCODE -ne 0){
    Write-Host 'Failed to create package for electron'
}

# Build CLR part
$nodeClrDir = $PSScriptRoot + '\coreclrnode'
Set-Location $nodeClrDir
& dotnet restore
if ($LASTEXITCODE -ne 0){
    Write-Host 'Failed to restore nuget dependencies'
}

& dotnet pack -c Release --version-suffix=$buildNumber
if ($LASTEXITCODE -ne 0){
    Write-Host 'Failed to build nuget package'
}