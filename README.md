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
process.env.CLR_APPBASE_PATH = "C:\\git\\coreclr";
// List of coma separated fully trusted assemblies
process.env.CLR_TRUSTED_PLATFORM_ASSEMBLIES = "";
// list of coma separated folders to search for assemblies
process.env.CLR_APP_PATHS = "C:\\git\\coreclr";
// path to core CLR library itself
process.env.CLR_LIB_PATH = 'C:\\git\\coreclr\\coreclr.dll';

console.log('Starting');
const addon = require('./build/Release/ClrLoader');
const config = JSON.stringify({
        AssemblyName: 'testlib',
        TypeName: 'TestAssembly.TestClass',
        MethodName: 'TestMethod'
    });

const input = JSON.stringify({
        Param: 'lol',
    });
    

addon.ClrExecute(config, input, (val) => {
    console.log('This should be eight: ' + val);    
});

const stdin = process.openStdin();
stdin.on('data', (chunk) => { console.log("Got chunk: " + chunk); });
```