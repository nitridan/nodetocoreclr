var addon;

if (process.arch == 'ia32'){
    addon = require('./ClrLoader_x86');
} else if (process.arch == 'x64'){
    addon = require('./ClrLoader_x64');
} else {
    throw new Error('Unsupported processor architecture');
}

module.exports.callClrMethod = function(config, args){
    const configStr = Buffer.from(JSON.stringify(config)).toString('base64');
    const argsStr = Buffer.from(JSON.stringify(args)).toString('base64');
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