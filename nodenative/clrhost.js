var addon;

if (process.arch == 'ia32'){
    addon = require('./ClrLoader_x86');
} else if (process.arch == 'x64'){
    addon = require('./ClrLoader_x64');
} else {
    throw new Error('Unsupported processor architecture');
}

module.exports.callClrMethod = function(config, args){
    const configStr = JSON.stringify(config);
    const argsStr = JSON.stringify(args);
    const clrPromise = new Promise(function(resolve, reject){
        addon.ClrExecute(configStr, argsStr, (result) => {
		    const resultObj = JSON.parse(result);
            if (resultObj.ResultType == 1){
                resolve(resultObj.Result);
            } else {
                reject(resultObj.Result);
            }
	    });
    });
    
    return clrPromise;
};