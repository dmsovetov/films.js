#include <v8.h>
#include <node.h>
#include <node_buffer.h>
#include <assert.h>

#define SIZE 8

using namespace v8;

// ** struct
struct js_work {
    uv_work_t req;
    Persistent<Function> callback;
    char* data;
    size_t len;
};

void run_work( uv_work_t* req )
{
    js_work* work = static_cast<js_work*>(req->data);
    char* data = new char[SIZE];
    for (int i = 0; i < SIZE; i++)
        data[i] = 97;
    work->data = data;
    work->len = SIZE;
}

void run_callback( uv_work_t* req, int status )
{
    HandleScope scope;

    js_work* work = static_cast<js_work*>(req->data);
    char* data = work->data;
    node::Buffer* buf = node::Buffer::New(data, work->len);

    Handle<Value> argv[1] = { buf->handle_ };

    // proper way to reenter the js world
    node::MakeCallback( Context::GetCurrent()->Global(), work->callback, 1, argv );

    // properly cleanup, or death by millions of tiny leaks
    work->callback.Dispose();
    work->callback.Clear();

    // unfortunately in v0.10 Buffer::New(char*, size_t) makes a copy
    // and we don't have the Buffer::Use() api yet
    delete[] data;
    delete work;
}