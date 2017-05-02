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

        private const int SuccessResult = 0;

        private const int FailResult = 1;
        
        private static string FromBase64(string input)
            => Encoding.UTF8.GetString(Convert.FromBase64String(input));

        public static void CallClrMethod( 
            [In, MarshalAs(UnmanagedType.LPStr)]string config, 
            [In, MarshalAs(UnmanagedType.LPStr)]string input, 
            [Out]out int resultType,
            [Out]out string resultJson)
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
                var result = methodDelegate(inputObj)
                    .ConfigureAwait(false)
                    .GetAwaiter()
                    .GetResult();

                resultType = SuccessResult;
                resultJson = JsonConvert.SerializeObject(result);
            }
            catch (Exception ex)
            {               
                resultType = FailResult;
                resultJson = JsonConvert.SerializeObject(ex);
            }
        }
    }
}
