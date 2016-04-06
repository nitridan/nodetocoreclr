#include <nan.h>
#include <uv.h>

namespace ClrLoader {
using namespace v8;

class CallbackData
{
    public:
        CallbackData(Persistent<Function>* callback){
            this->_callback = callback;
            this->_loopData.data = this;
        }

        ~CallbackData(){
            this->_callback->Reset();
            delete this->_callback;
        }

        uv_async_t* getLoopData(){
            return &this->_loopData;
        }

        Persistent<Function>* getCallback(){
            return this->_callback;
        }

        void setResult(const char* result){
            this->_result = std::string(result);
        }

        const char* getResult(){
            return this->_result.c_str();
        }

    private:
        uv_async_t _loopData;
        std::string _result;
        Persistent<Function>* _callback;
};
}
