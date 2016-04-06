using System;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Threading.Tasks;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;

namespace Nitridan.CoreClrNode
{    
    public static class CoreClrExecutor
    {
        public delegate Task<object> ClrMethodDelegate(object input);
        
        public delegate void UnmanagedCallback(IntPtr ptr, string result);
        
        public static void CallClrMethod(IntPtr ptr, 
            [In, MarshalAs(UnmanagedType.LPStr)]string config, 
            [In, MarshalAs(UnmanagedType.LPStr)]string input, 
            UnmanagedCallback cb)
        {
            try 
            {
                var inputObj = JObject.Parse(input);
                var asmConfig = JsonConvert.DeserializeObject<AssemblyConfig>(config);
                var assemblyName = new AssemblyName(asmConfig.AssemblyName);
                var assembly = Assembly.Load(assemblyName);
                var typeInfo = assembly.GetType(asmConfig.TypeName).GetTypeInfo();
                var methodInfo = typeInfo.GetDeclaredMethod(asmConfig.MethodName);
                var methodDelegate = (ClrMethodDelegate)methodInfo.CreateDelegate(typeof(ClrMethodDelegate));
                var resultTask = methodDelegate(inputObj);
                resultTask.ContinueWith(t => 
                { 
                    var result = new JsonResult 
                    {
                        ResultType = t.IsCompleted ? ResultType.Success : ResultType.Error,
                        Result = t.IsCompleted ? t.Result : t.Exception
                    };
                    
                    cb(ptr, JsonConvert.SerializeObject(result));
                });
            }
            catch (Exception ex)
            {
                var resultEx = new JsonResult 
                {
                    ResultType = ResultType.Error,
                    Result = ex
                };
                
                cb(ptr, JsonConvert.SerializeObject(resultEx));
            }
        }
    }
}
