const nodeNative = (function(){
    var _runtime;
    
    const DEFAULT_DOTNET_DIR = '\\dotnet\\bin';
    var _environmentConfig = {
        clrAppBasePath: process.env.ProgramFiles + DEFAULT_DOTNET_DIR,
        clrLibPath: process.env.ProgramFiles + DEFAULT_DOTNET_DIR + '\\coreclr.dll',
        clrTrustedPlatformAssemblies: '',
        clrAppPaths: process.env.ProgramFiles + DEFAULT_DOTNET_DIR + ';' + require('path').dirname(require.main.filename)
    };
    
    var _configureRuntime = function(config){
        if (!config){
            throw new Error("Missing configuration parameter");
        }
        
        if (config.clrAppBasePath){
            _environmentConfig.clrAppBasePath = config.clrAppBasePath;
        }
        
        if (config.clrLibPath){
            _environmentConfig.clrLibPath = config.clrLibPath;
        }
        
        if (config.clrTrustedPlatformAssemblies){
            _environmentConfig.clrTrustedPlatformAssemblies = config.clrTrustedPlatformAssemblies;
        }
        
        if (config.clrAppPaths){
            _environmentConfig.clrAppPaths = config.clrAppPaths;
        }
    };
    
    var _getClrRuntime = function(){
        if (!_runtime){
            process.env.CLR_APPBASE_PATH = _environmentConfig.clrAppBasePath;
            process.env.CLR_TRUSTED_PLATFORM_ASSEMBLIES = _environmentConfig.clrTrustedPlatformAssemblies;
            process.env.CLR_APP_PATHS = _environmentConfig.clrAppPaths;
            process.env.CLR_LIB_PATH = _environmentConfig.clrLibPath;
            _runtime = require('./clrhost.js');
        }
        
        return _runtime;
    };
    
    return {
        configureRuntime: _configureRuntime,
        getClrRuntime: _getClrRuntime
    };
})();

module.exports = nodeNative;