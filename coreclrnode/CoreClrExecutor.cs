using System;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;

namespace Nitridan.CoreClrNode
{    
    public static class CoreClrExecutor
    {
        public delegate Task<object> ClrMethodDelegate(object input);
        
        public delegate void UnmanagedCallback(IntPtr ptr, string result);
        
        private static string FromBase64(string input)
            => Encoding.UTF8.GetString(Convert.FromBase64String(input));

        public static async void CallClrMethod(IntPtr ptr, 
            [In, MarshalAs(UnmanagedType.LPStr)]string config, 
            [In, MarshalAs(UnmanagedType.LPStr)]string input, 
            UnmanagedCallback cb)
        {
            try 
            {
                var inputObj = JObject.Parse(FromBase64(input));
                var asmConfig = JsonConvert.DeserializeObject<AssemblyConfig>(FromBase64(config));
                var assemblyName = new AssemblyName(asmConfig.AssemblyName);
                var assembly = Assembly.Load(assemblyName);
                var typeInfo = assembly.GetType(asmConfig.TypeName).GetTypeInfo();
                var methodInfo = typeInfo.GetDeclaredMethod(asmConfig.MethodName);
                var methodDelegate = (ClrMethodDelegate)methodInfo.CreateDelegate(typeof(ClrMethodDelegate));
                var result = await methodDelegate(inputObj);
                var jsonResult = new JsonResult 
                {
                    ResultType = ResultType.Success,
                    Result = result
                };
                
                cb(ptr, JsonConvert.SerializeObject(jsonResult));
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
