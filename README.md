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

Installation to electron (v0.37.5 currently used for build):
```
npm install https://github.com/nitridan/nodetocoreclr/releases/download/v1.1/nodetocoreclr-electron-1.1.7.tgz
```

Installation to node.js (v5.10.1 currently used for build):
```
npm install https://github.com/nitridan/nodetocoreclr/releases/download/v1.1/nodetocoreclr-node-1.1.7.tgz
```

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