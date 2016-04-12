#ifdef _WIN32
// Important!!! Windows.h must be declared before mscoree2.h
#define _WIN32_WINNT _WIN32_WINNT_WIN7
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "mscoree2.h"
typedef HINSTANCE p_independent_lib_t;
typedef DWORD clr_domain_id;
#else
#include "CoreClrLinux.h"
#include <dlfcn.h>
typedef void* p_independent_lib_t;
typedef unsigned int clr_domain_id;
#endif

#include <stdlib.h>

namespace ClrLoader {

const LPCWSTR MANAGED_ASSEMBLY_NAME = L"Nitridan.CoreClrNode";

const LPCWSTR MANAGED_CLASS_NAME = L"Nitridan.CoreClrNode.CoreClrExecutor";

const LPCWSTR MANAGED_METHOD_NAME = L"CallClrMethod";

enum ErrorType {
	Success,
	CantLoadLibrary,
	CantFindRuntimeHostFactory,
	CantStartClrHost,
	CantSetStartupFlags,
	CantAuthenticateHost,
	CantCreateClrHost,
	CantCreateAppDomain,
	CantFindClrDelegate
};

wchar_t* charToWChar(const char* text)
{
    auto size = strlen(text) + 1;
    wchar_t* wa = new wchar_t[size];
    mbstowcs(wa,text,size);
    return wa;
}

p_independent_lib_t LoadLibPlatformIndependent(const char* libPath){
# ifdef _WIN32
  return ::LoadLibraryExW(charToWChar(libPath), NULL, 0);
# else
  return ::dlopen(libPath, RTLD_LAZY);
# endif
}

void* LoadProcPlatformIndependent(p_independent_lib_t lib, const char* funcName){
# ifdef _WIN32
  return ::GetProcAddress(lib, funcName);
# else
  return ::dlsym(lib, funcName);
# endif
}

ErrorType InitializeCoreClr(const char* clrLibPath,
		const char* appBasePath,
		const char* trustedPlatformAssemblies,
		const char* appPaths,
		void* clrHost,
		void* delegate,
		clr_domain_id* domainId){
  auto hCoreCLRModule = LoadLibPlatformIndependent(clrLibPath);
  if (hCoreCLRModule == NULL){
	return CantLoadLibrary;
  }

  auto pfnGetCLRRuntimeHost = (FnGetCLRRuntimeHost)LoadProcPlatformIndependent(hCoreCLRModule, "GetCLRRuntimeHost");
  if (pfnGetCLRRuntimeHost == NULL){
	return CantFindRuntimeHostFactory;
  }

  ICLRRuntimeHost2* pCLRRuntimeHost = nullptr;

  // Call the factory function to create a new instance of a runtime host
  auto initRuntimeResult = pfnGetCLRRuntimeHost(IID_ICLRRuntimeHost2, (IUnknown**)&pCLRRuntimeHost);
  if (!SUCCEEDED(initRuntimeResult)){
	  return CantCreateClrHost;
  }

  clrHost = pCLRRuntimeHost;
  const STARTUP_FLAGS dwStartupFlags = (STARTUP_FLAGS)(
	  STARTUP_FLAGS::STARTUP_LOADER_OPTIMIZATION_SINGLE_DOMAIN |
	  STARTUP_FLAGS::STARTUP_SINGLE_APPDOMAIN |
	  STARTUP_FLAGS::STARTUP_SERVER_GC);

  auto setStartupFlagsResult = pCLRRuntimeHost->SetStartupFlags(dwStartupFlags);
  if (!SUCCEEDED(setStartupFlagsResult)){
	return CantSetStartupFlags;
  }

  auto authenticateResult = pCLRRuntimeHost->Authenticate(CORECLR_HOST_AUTHENTICATION_KEY);
  if (!SUCCEEDED(authenticateResult)){
	return CantAuthenticateHost;
  }

  auto startResult = pCLRRuntimeHost->Start();
  if (!SUCCEEDED(startResult)){
	return CantStartClrHost;
  }

  const wchar_t* property_keys[] =
  {
	L"APPBASE",
	L"TRUSTED_PLATFORM_ASSEMBLIES",
	L"APP_PATHS",
  };

  const wchar_t* property_values[] = {
	// APPBASE
	charToWChar(appBasePath),
	// TRUSTED_PLATFORM_ASSEMBLIES
	charToWChar(trustedPlatformAssemblies),
	// APP_PATHS
	charToWChar(appPaths)
  };

  int nprops = sizeof(property_keys) / sizeof(wchar_t*);

  auto appDomainFlags = APPDOMAIN_ENABLE_PLATFORM_SPECIFIC_APPS
		  | APPDOMAIN_ENABLE_PINVOKE_AND_CLASSIC_COMINTEROP
		  | APPDOMAIN_ENABLE_ASSEMBLY_LOADFILE;

  auto appDomainResult = pCLRRuntimeHost->CreateAppDomainWithManager(
	 MANAGED_ASSEMBLY_NAME,
	 appDomainFlags,
	 NULL,
	 NULL,
	 nprops,
	 property_keys,
	 property_values,
	 (DWORD*)&domainId);
  if (!SUCCEEDED(appDomainResult)){
	return CantCreateAppDomain;
  }

  // Create a delegate to the managed entry point
  auto createDelegateResult = pCLRRuntimeHost->CreateDelegate(
     (DWORD)domainId,
	 MANAGED_ASSEMBLY_NAME,
	 MANAGED_CLASS_NAME,
	 MANAGED_METHOD_NAME,
	 (INT_PTR*)delegate);
  if (!SUCCEEDED(createDelegateResult)){
	return CantFindClrDelegate;
  }

  return Success;
}

}
