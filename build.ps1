param($buildNumber)

$ELECTRON_VERSION=0.37.5

if (!$buildNumber){
    $buildNumber = 1
}

function Force-Copy($source, $destination){
    New-Item -ItemType File -Path $destination -Force
    Copy-Item $source $destination -Force
}

# Build native and JS part
$nodenativeDir = $PSScriptRoot + '\nodenative'
$outputPath = $nodenativeDir + '\build\Release\ClrLoader.node'
$distributiveDir = $nodenativeDir + '\dist';
$x86binaryTarget = $distributiveDir + '\ClrLoader_x86.node'
$x64binaryTarget = $distributiveDir + '\ClrLoader_x64.node'
Set-Location $nodenativeDir
& npm install
& node-gyp rebuild --arch=x64
Force-Copy $outputPath $x64binaryTarget
& node-gyp rebuild --arch=ia32
Force-Copy $outputPath $x86binaryTarget
Get-ChildItem -path $nodenativeDir | Where { $_.Extension -eq '.js' } `
    | Copy-Item -destination {$_.FullName -replace [System.Text.RegularExpressions.Regex]::Escape($nodenativeDir),$distributiveDir} -Force

#Build for electron
$electronDistributiveDir = $nodenativeDir + '\dist-electron';
$electronX86binaryTarget = $electronDistributiveDir + '\ClrLoader_x86.node'
$electronX64binaryTarget = $electronDistributiveDir + '\ClrLoader_x64.node'
Set-Location $nodenativeDir
& npm install
& node-gyp rebuild --arch=x64 --target=$ELECTRON_VERSION --dist-url=https://atom.io/download/atom-shell
Force-Copy $outputPath $electronX86binaryTarget
& node-gyp rebuild --arch=ia32 --target=$ELECTRON_VERSION --dist-url=https://atom.io/download/atom-shell
Force-Copy $outputPath $electronX64binaryTarget
Get-ChildItem -path $nodenativeDir | Where { $_.Extension -eq '.js' } `
    | Copy-Item -destination {$_.FullName -replace [System.Text.RegularExpressions.Regex]::Escape($nodenativeDir),$electronDistributiveDir} -Force


# Build CLR part
$nodeClrDir = $PSScriptRoot + '\coreclrnode'
Set-Location $nodeClrDir
& dotnet restore
& dotnet pack -c Release --version-suffix=$buildNumber