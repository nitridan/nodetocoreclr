#define _WIN32_WINNT _WIN32_WINNT_WIN7
#include <nan.h>
#include <Windows.h>
#include "mscoree2.h"
#include <stdlib.h>
#include <uv.h>
#include "CallbackData.h"

namespace ClrLoader {

using namespace v8;

const LPCWSTR MANAGED_ASSEMBLY_NAME = L"Nitridan.CoreClrNode";

const LPCWSTR MANAGED_CLASS_NAME = L"Nitridan.CoreClrNode.CoreClrExecutor";

const LPCWSTR MANAGED_METHOD_NAME = L"CallClrMethod";

typedef void (STDMETHODCALLTYPE *PUnmanagedCallback)(const INT_PTR, const char*);

typedef void (STDMETHODCALLTYPE *PManagedEntryPoint)(const INT_PTR, const char*, const char*, const void (*PUnmanagedCallback));

// Declare a variable pointing to our managed method.
PManagedEntryPoint pManagedEntryPoint;

void ClrCallback(const INT_PTR returnPtr, const char* value){
    auto callbackData = (CallbackData*)returnPtr;
    callbackData->setResult(value);
    uv_async_send(callbackData->getLoopData());
}

void FinalizeUvCallback(uv_async_t* handle){
    Nan::HandleScope handleScope;
    auto data = (CallbackData*)handle->data;
    const unsigned argc = 1;
    Local<Value> argv[argc] = {  Nan::New<String>(data->getResult()).ToLocalChecked() };
    Local<Function> origFunc = Nan::New(*data->getCallback());
    Nan::Callback cb(origFunc);
    cb.Call(argc, argv);
    delete data;
}

void ClrExecute(const Nan::FunctionCallbackInfo<Value>& args) {
  if (args.Length() < 3) {
    Nan::ThrowTypeError("Wrong number of arguments");
    return;
  }

  if (!args[0]->IsString()
          || !args[1]->IsString()
          || !args[2]->IsFunction()) {
    Nan::ThrowError("Wrong arguments");
    return;
  }

  String::Utf8Value utfStringConfig(args[0].As<String>());
  auto stdCStringConfig = std::string(*utfStringConfig);

  String::Utf8Value utfStringData(args[1].As<String>());
  auto stdCStringData = std::string(*utfStringData);

  // Perform the operation
  auto cb = args[2].As<Function>();
  auto nanCb = new CallbackData(new Nan::Persistent<Function>(cb));
  auto initResult = uv_async_init(uv_default_loop(), nanCb->getLoopData(), FinalizeUvCallback);
  if (!SUCCEEDED(initResult)){
      Nan::ThrowError(uv_strerror(initResult));
      return;
  }

  pManagedEntryPoint((INT_PTR)nanCb, stdCStringConfig.c_str(), stdCStringData.c_str(), ClrCallback);
}

wchar_t* charToWChar(const char* text)
{
    auto size = strlen(text) + 1;
    wchar_t* wa = new wchar_t[size];
    mbstowcs(wa,text,size);
    return wa;
}

void Init(Local<Object> exports) {
  auto libPath = std::getenv("CLR_LIB_PATH");
  if (libPath == NULL){
    Nan::ThrowError("CLR_LIB_PATH not set");
    return;
  }

  auto appBasePath = std::getenv("CLR_APPBASE_PATH");
  if (appBasePath == NULL){
      Nan::ThrowError("CLR_APPBASE_PATH not set");
      return;
  }

  auto trustedPlatformAssemblies = std::getenv("CLR_TRUSTED_PLATFORM_ASSEMBLIES");
  if (trustedPlatformAssemblies == NULL){
    Nan::ThrowError("CLR_TRUSTED_PLATFORM_ASSEMBLIES not set");
    return;
  }

  auto appPaths = std::getenv("CLR_APP_PATHS");
  if (appPaths == NULL){
    Nan::ThrowError("CLR_APP_PATHS not set");
    return;
  }

  auto hCoreCLRModule = ::LoadLibraryExW(charToWChar(libPath), NULL, 0);
  if (hCoreCLRModule == NULL){
    Nan::ThrowError("Can't load coreclr.dll");
    return;
  }

  auto pfnGetCLRRuntimeHost = (FnGetCLRRuntimeHost)::GetProcAddress(hCoreCLRModule, "GetCLRRuntimeHost");
  if (pfnGetCLRRuntimeHost == NULL){
    Nan::ThrowError("Can't get CLR Runtime Host factory");
    return;
  }

  ICLRRuntimeHost2* pCLRRuntimeHost = nullptr;

  // Call the factory function to create a new instance of a runtime host
  auto initRuntimeResult = pfnGetCLRRuntimeHost(IID_ICLRRuntimeHost2, (IUnknown**)&pCLRRuntimeHost);
  if (!SUCCEEDED(initRuntimeResult)){
    Nan::ThrowError("Error while initializing CLR Runtime host");
    return;
  }

  const STARTUP_FLAGS dwStartupFlags = (STARTUP_FLAGS)(
      STARTUP_FLAGS::STARTUP_LOADER_OPTIMIZATION_SINGLE_DOMAIN |
      STARTUP_FLAGS::STARTUP_SINGLE_APPDOMAIN |
      STARTUP_FLAGS::STARTUP_SERVER_GC);

  auto setStartupFlagsResult = pCLRRuntimeHost->SetStartupFlags(dwStartupFlags);
  if (!SUCCEEDED(setStartupFlagsResult)){
    Nan::ThrowError("Error while setting CLR Runtime startup flags");
    return;
  }

  auto authenticateResult = pCLRRuntimeHost->Authenticate(CORECLR_HOST_AUTHENTICATION_KEY);
  if (!SUCCEEDED(authenticateResult)){
    Nan::ThrowError("Error while authenticating CLR host");
    return;
  }

  auto startResult = pCLRRuntimeHost->Start();
  if (!SUCCEEDED(startResult)){
    Nan::ThrowError("Error starting CLR host");
    return;
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

  DWORD domainId;
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
     &domainId);
  if (!SUCCEEDED(appDomainResult)){
    Nan::ThrowError("Failed to create default AppDomain");
    return;
  }

  // Create a delegate to the managed entry point
  auto createDelegateResult = pCLRRuntimeHost->CreateDelegate(
     domainId,
     MANAGED_ASSEMBLY_NAME,
     MANAGED_CLASS_NAME,
     MANAGED_METHOD_NAME,
     (INT_PTR*)&pManagedEntryPoint);
  if (!SUCCEEDED(createDelegateResult)){
    Nan::ThrowError("Failed to find managed entry point");
    return;
  }

  Nan::SetMethod(exports, "ClrExecute", ClrExecute);
}

NODE_MODULE(addon, Init)
}
