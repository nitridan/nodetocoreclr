# Node to Core CLR

This project was inspired by edge.js
But it's much simplified version.
It only allows to call Core CLR assemblies from node.js

__Works only on Windows system__

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
const addon = require('./nodenative.js');
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