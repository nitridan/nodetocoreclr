namespace ClrLoader {

typedef int (coreclrInitializeFunc)(
            const char* exePath,
            const char* appDomainFriendlyName,
            int propertyCount,
            const char** propertyKeys,
            const char** propertyValues,
            void** hostHandle,
            unsigned int* domainId);

typedef int (coreclrShutdownFunc)(
            void* hostHandle,
            unsigned int domainId);

typedef int (coreclrCreateDelegateFunc)(
              void* hostHandle,
              unsigned int domainId,
              const char* entryPointAssemblyName,
              const char* entryPointTypeName,
              const char* entryPointMethodName,
              void** delegate);
}
