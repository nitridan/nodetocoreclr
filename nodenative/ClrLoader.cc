#include <stdlib.h>
#include "PlatformAbstractions.h"
#include <nan.h>
#include <uv.h>
#include "CallbackData.h"

namespace ClrLoader {

using namespace v8;

typedef void (STDMETHODCALLTYPE *PUnmanagedCallback)(CallbackData*, const char*);

typedef void (STDMETHODCALLTYPE *PManagedEntryPoint)(const CallbackData*, const char*, const char*, const intptr_t);

// Declare a variable pointing to our managed method.
PManagedEntryPoint pManagedEntryPoint;

void ClrCallback(CallbackData* callbackData, const char* value){
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
  // All libuv sucessfull codes greater than 0
  if (initResult < 0){
      Nan::ThrowError(uv_strerror(initResult));
      return;
  }

  pManagedEntryPoint(nanCb,
    stdCStringConfig.c_str(),
    stdCStringData.c_str(),
    (intptr_t)ClrCallback);
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

  intptr_t pCLRRuntimeHost;
  intptr_t delegatePointer;
  clr_domain_id domainId;
  auto initClrResult = InitializeCoreClr(libPath,
		  appBasePath,
		  trustedPlatformAssemblies,
		  appPaths,
		  &pCLRRuntimeHost,
		  &delegatePointer,
		  &domainId);
  switch(initClrResult){
    case Success:
      break;
    case CantLoadLibrary:
      Nan::ThrowError("Can't load coreclr.dll");
      return;
    case CantFindRuntimeHostFactory:
      Nan::ThrowError("Can't get CLR Runtime Host factory");
      return;
    case CantStartClrHost:
      Nan::ThrowError("Error starting CLR host");
      return;
    case CantSetStartupFlags:
      Nan::ThrowError("Error while setting CLR Runtime startup flags");
      return;
    case CantAuthenticateHost:
      Nan::ThrowError("Error while authenticating CLR host");
      return;
    case CantCreateClrHost:
      Nan::ThrowError("Error while initializing CLR Runtime host");
      return;
    case CantCreateAppDomain:
      Nan::ThrowError("Failed to create default AppDomain");
      return;
    case CantFindClrDelegate:
      Nan::ThrowError("Failed to find managed entry point");
      return;
    default:
      Nan::ThrowError("Unknown error");
      return;
  }

  pManagedEntryPoint = (PManagedEntryPoint)delegatePointer;
  Nan::SetMethod(exports, "ClrExecute", ClrExecute);
}

NODE_MODULE(addon, Init)
}
