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