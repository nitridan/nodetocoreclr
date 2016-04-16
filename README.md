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

### Build requirements:
- Python 2.7+
- VS Express 2013+ (haven't tested with earlier versions)
- Powershell 4.0+

### Build steps
1. Open build.ps1
2. Set your versions of node.js/electron(you have to change 2 variables: _$ELECTRON_VERSION_, _$NODE_VERSION_)
3. Run _"powershell -file build.ps1 -localDotNet"_
4. Go to _"nodenative\dist"_ for node.js build
5. Go to _"nodenative\dist-electron"_ for electron build
6. Nuget package for core CLR build can be found under: _"coreclrnode\bin\Release"_

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
const addon = require('nodenative');
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