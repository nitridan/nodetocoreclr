# Node to Core CLR

This project was inspired by edge.js
But it's much simplified version.
It only allows to call Core CLR assemblies from node.js

## Requires
- Windows 7 SP1 x86/x64 or newer (Windows 8 not supported by Microsoft so issues may happen)
- Ubuntu Linux 14.04 x64

### Build status
Windows x86/x64

[![Build status](https://ci.appveyor.com/api/projects/status/i9mulv9q8f789y4i?svg=true)](https://ci.appveyor.com/project/nitridan/nodetocoreclr)

Ubuntu 14.04 x64

[![Build Status](https://travis-ci.org/nitridan/nodetocoreclr.svg?branch=master)](https://travis-ci.org/nitridan/nodetocoreclr)

## Installation

### Installation to electron (v1.2.5 currently used for build)

- Linux
```
npm install https://github.com/nitridan/nodetocoreclr/releases/download/v1.2/nodetocoreclr-electron-linux-1.2.12.tgz
```
- Windows
```
npm install https://github.com/nitridan/nodetocoreclr/releases/download/v1.2/nodetocoreclr-electron-win-1.2.27.tgz
```

### Installation to node.js (v6.2.2 currently used for build):

- Linux
```
npm install https://github.com/nitridan/nodetocoreclr/releases/download/v1.2/nodetocoreclr-node-linux-1.2.12.tgz
```
- Windows
```
npm install https://github.com/nitridan/nodetocoreclr/releases/download/v1.2/nodetocoreclr-node-win-1.2.27.tgz
```

## Latest nuget package

Nuget package completely OS independent. You can put it into your nuget repository and use.

https://github.com/nitridan/nodetocoreclr/releases/download/v1.2/Nitridan.CoreClrNode.1.2-release-27.nupkg

## Build

If you have to support different version of node.js/electron custom build can be performed.

Currently build with scripts supported only on x64 platforms.
Code changes required for x86 platforms.

### Build requirements Windows
- Python 2.7+
- VS Express 2013+ (haven't tested with earlier versions)
- Powershell 4.0+

### Build steps Windows (x64)
1. Run _"powershell -file build.ps1 -localDotNet"_ 
   - to change your node.js version provide command line argument _"-nodeVersion 5.10.1"_
   - to change your electron version provide command line argument _"-electronVersion 0.37.6"_
2. Go to _"nodenative\dist"_ for node.js build
3. Go to _"nodenative\dist-electron"_ for electron build
4. Nuget package for core CLR build can be found under: _"coreclrnode\bin\Release"_

#### Note
If you experience issues with VS 2015 Update 3:
Install VS 2013
Run:
```
npm config set msvs_version 2013 --global
```

### Build requirements Linux (x64)
- dotnet cli SDK
- Python 2.7+
- node.js build tools
- node-gyp 3.3.1 (build-essential package on Ubuntu)
- gcc-multilib
- g++-multilib

### Build steps Linux
1. Run _"python buildlinux.py"_ 
   - to change your node.js version provide command line argument _"-nodeVersion=5.10.1"_
   - to change your electron version provide command line argument _"-electronVersion=0.37.6"_
2. Go to _"nodenative/dist"_ for node.js build
3. Go to _"nodenative/dist-electron"_ for electron build
4. To build managed part go to _"coreclrnode"_
5. Run _"dotnet restore"_
6. Run _"dotnet pack"_
7. Now nuget package for core CLR build can be found under: _"coreclrnode/bin/Release"_


Sample C# code for Core CLR assembly:

```csharp
using System.Threading.Tasks;

namespace TestAssembly
{
    public class TestClass
    {
        public static Task<object> TestMethod(dynamic input)
        {
            object result = input.Param;
            return Task.FromResult(result);
        }
    }
}

```

Sample project.json

```json
{
    "version": "1.0.0-*",
    "compilationOptions": {
        "emitEntryPoint": false
    },

    "dependencies": {
        "NETStandard.Library": "1.0.0-rc3-*",
        "Microsoft.CSharp": "4.0.1-rc3-*",
        "System.Dynamic.Runtime": "4.0.11-rc3-*"
    },

    "frameworks": {
        "dnxcore50": { }
    }
}
```

Sample JS code which invokes this sample assembly

```javascript
const addon = require('nodetocoreclr');
const config = {
    AssemblyName: 'testlib',
    TypeName: 'TestAssembly.TestClass',
    MethodName: 'TestMethod'
};

const input = {
    Param: 'lol',
};
  
addon.configureRuntime({
    // pth to base core CLR directory
    clrAppBasePath: 'C:\\git\\coreclr',
    // path to core CLR library itself
    clrLibPath: 'C:\\git\\coreclr\\coreclr.dll',
    // List of coma separated fully trusted assemblies
    clrTrustedPlatformAssemblies: '',
    // list of coma separated folders to search for assemblies    
    clrAppPaths: 'C:\\git\\coreclr'
});

const clrRuntime = addon.getClrRuntime();
const clrPromise = clrRuntime.callClrMethod(config, input);
clrPromise.then((result) => {
    console.log('This should be eight: ' + result);
});

const stdin = process.openStdin();
stdin.on('data', (chunk) => { console.log("Got chunk: " + chunk); });
```