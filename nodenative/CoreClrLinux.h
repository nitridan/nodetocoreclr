#include <stdint.h>

namespace ClrLoader {

typedef int32_t (coreclrInitializeFunc)(
            const char* exePath,
            const char* appDomainFriendlyName,
            int32_t propertyCount,
            const char** propertyKeys,
            const char** propertyValues,
            intptr_t* hostHandle,
            uint32_t* domainId);

typedef int32_t (coreclrShutdownFunc)(
            intptr_t hostHandle,
            uint32_t domainId);

typedef int32_t (coreclrCreateDelegateFunc)(
              intptr_t hostHandle,
              uint32_t domainId,
              const char* entryPointAssemblyName,
              const char* entryPointTypeName,
              const char* entryPointMethodName,
              intptr_t* delegate);
}
