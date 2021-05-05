#include <node.h>
#include <uv.h>
#include <iostream>
#include <thread>

using namespace std;

namespace addon
{
  using v8::Function;
  using v8::FunctionCallbackInfo;
  using v8::Isolate;
  using v8::Local;
  using v8::Object;
  using v8::Persistent;
  using v8::String;
  using v8::Value;
  
  struct Work
  {
    uv_work_t request;
    Persistent<Function> callback;
    string result;
  };

  /**
  * uv_main_work функция которая выполняет основную работу.
  * после её отработки выполняется uv_complete.
  */
  static void uv_main_work(uv_work_t *req)
  {
    this_thread::sleep_for(chrono::milliseconds(5000));
    (static_cast<Work *>(req->data))->result = "work done.";
  }

  /**
  * uv_complete вызывается после выполнения uv_main_work.
  * после здесь мы вызываем callback, переданный нам из index.js
  */
  static void uv_complete(uv_work_t *req, int status)
  {
    Isolate *isolate = Isolate::GetCurrent();

    v8::HandleScope handleScope(isolate);

    Work *work = static_cast<Work *>(req->data);

    const char *result = work->result.c_str();
    v8::MaybeLocal<v8::String> maybeLocal = String::NewFromUtf8(isolate, result);
    v8::Local<v8::Value> local;

    maybeLocal.ToLocal(&local);

    Local<Value> argv[1] = {local};

    Local<Function>::New(isolate, work->callback)->Call(isolate->GetCurrentContext(), isolate->GetCurrentContext()->Global(), 1, argv);

    work->callback.Reset();
    delete work;
  }

  /**
  * __main стартовая функция, вызывается из index.js. Эта функция принимает callback из index.js, после сразу же возвращает
  * в js данные(undefined в нашем случее), также стартует uv таск, который позднее вызывает переданный из index.js callback
  */
  void __main(const FunctionCallbackInfo<Value> &args)
  {
    Isolate *isolate = args.GetIsolate();

    Work *work = new Work();
    work->request.data = work;

    work->callback.Reset(isolate, Local<Function>::Cast(args[0]));

    uv_queue_work(uv_default_loop(), &work->request, uv_main_work, uv_complete);

    args.GetReturnValue().Set(Undefined(isolate));
  }

  /**
  * init
  */
  void init(Local<Object> exports)
  {
    // сетим main функцию для нашего модуля
    NODE_SET_METHOD(exports, "main", __main);
  }

  NODE_MODULE(asyncAddon, init)

}
