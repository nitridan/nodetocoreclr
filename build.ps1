function Force-Copy($source, $destination){
    New-Item -ItemType File -Path $destination -Force
    Copy-Item $source $destination -Force
}

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
