#include <stdlib.h>
#include "PlatformAbstractions.h"
#include <nan.h>
#include <stdint.h>

namespace ClrLoader {

using namespace v8;

typedef void (STDMETHODCALLTYPE *PManagedEntryPoint)(const char*, const char*, int32_t*, char **);

// Declare a variable pointing to our managed method.
PManagedEntryPoint pManagedEntryPoint;

class ClrWorker : public Nan::AsyncWorker {
    public:
        ClrWorker(Nan::Callback *callback, std::string c, std::string args)
        : AsyncWorker(callback){
          config = c;
          arguments = args;
        }

        ~ClrWorker() {}

        void Execute () {
          char* resultChar;
          int32_t resultCode;
          pManagedEntryPoint(
            config.c_str(),
            arguments.c_str(),
            &resultCode,
            &resultChar);

          results = std::string(resultChar);
          delete[] resultChar;

          if (resultCode > 0){
            SetErrorMessage(results.c_str());
            return;
          }
        }

        // We have the results, and we're back in the event loop.
        void HandleOKCallback () {
            Nan::HandleScope scope;

            Local<Value> argv[] = {
                Nan::Null(),
                Nan::New<String>(results).ToLocalChecked()
            };

            callback->Call(2, argv);
        }

    private:
        std::string config;
        std::string arguments;
        std::string results;
};

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
  auto config = std::string(*utfStringConfig);

  String::Utf8Value utfStringData(args[1].As<String>());
  auto data = std::string(*utfStringData);

  // Perform the operation
  auto callback = new Nan::Callback(args[2].As<Function>());
  AsyncQueueWorker(new ClrWorker(callback, config, data));
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
