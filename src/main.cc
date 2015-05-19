#include <node.h>
#include <v8.h>

#include "DataMining/Films.h"

#define ARG_NUMBER( idx, defaultValue ) args.Length() > idx ? args[idx]->NumberValue()                      : defaultValue
#define ARG_STRING( idx, defaultValue ) args.Length() > idx ? *String::Utf8Value( args[idx]->ToString() )   : defaultValue

using namespace v8;

Films gFilms;

// ** struct LoadingRequest
struct LoadingRequest {
    Persistent<Function>    m_callback;
    bool                    m_error;
    std::string             m_message;
    int32_t                 m_result;
};

void AsyncLoadAfter( uv_work_t* req, int status )
{
    HandleScope     scope;
    LoadingRequest* loading = static_cast<LoadingRequest*>( req->data );

    if( loading->m_error ) {
        Local<Value> err = Exception::Error( String::New( loading->m_message.c_str() ) );

        // Prepare the parameters for the callback function.
        const unsigned argc = 1;
        Local<Value> argv[argc] = { err };

        // Wrap the callback function call in a TryCatch so that we can call
        // node's FatalException afterwards. This makes it possible to catch
        // the exception from JavaScript land using the
        // process.on('uncaughtException') event.
        TryCatch try_catch;
        loading->m_callback->Call( Context::GetCurrent()->Global(), argc, argv );
        if( try_catch.HasCaught() ) {
            node::FatalException( try_catch );
        }
    } else {
        // In case the operation succeeded, convention is to pass null as the
        // first argument before the result arguments.
        // In case you produced more complex data, this is the place to convert
        // your plain C++ data structures into JavaScript/V8 data structures.
        const unsigned argc = 2;
        Local<Value> argv[argc] = {
            Local<Value>::New( Null() ),
            Local<Value>::New( Boolean::New( true ) )
        };

        // Wrap the callback function call in a TryCatch so that we can call
        // node's FatalException afterwards. This makes it possible to catch
        // the exception from JavaScript land using the
        // process.on('uncaughtException') event.
        TryCatch try_catch;
        loading->m_callback->Call( Context::GetCurrent()->Global(), argc, argv );
        if( try_catch.HasCaught() ) {
            node::FatalException( try_catch );
        }
    }

    // The callback is a permanent handle, so we have to dispose of it manually.
    loading->m_callback.Dispose();

    // We also created the baton and the work_req struct with new, so we have to
    // manually delete both.
    delete loading;
    delete req;
}

void AsyncLoad( uv_work_t* req )
{
//    LoadingRequest* loading = static_cast<LoadingRequest*>( req->data );

//    gData.loadItems( "data/10m/movies.csv" );
//    gData.loadRatings( "data/10m/ratings.csv" );

//    gData.subtractUserAverage();
}

// ** load
Handle<Value> load( const Arguments& args )
{
    HandleScope scope;
/*
    if( !args[0]->IsFunction() ) {
        return ThrowException( Exception::TypeError( String::New( "First argument must be a callback function" ) ) );
    }

    Local<Function> callback = Local<Function>::Cast( args[0] );

    // The baton holds our custom status information for this asynchronous call,
    // like the callback function we want to call when returning to the main
    // thread and the status information.
    LoadingRequest* loading = new LoadingRequest();
    loading->m_error        = false;
    loading->m_callback     = Persistent<Function>::New( callback );

    // This creates the work request struct.
    uv_work_t* req = new uv_work_t();
    req->data = loading;

    // Schedule our work request with libuv. Here you can specify the functions
    // that should be executed in the threadpool and back in the main thread
    // after the threadpool function completed.
    int status = uv_queue_work( uv_default_loop(), req, AsyncLoad, AsyncLoadAfter );
    assert( status == 0 );
*/
    return Undefined();
}

// ** connect
Handle<Value> connect( const Arguments& args )
{
    HandleScope scope;

    std::string host    = ARG_STRING( 0, "mongodb://localhost:27017/" );
    std::string source  = ARG_STRING( 1, "movielens" );
    std::string target  = ARG_STRING( 2, "recommender" );

    gFilms.connect( host, source, target );

    return Undefined();
}

// ** find
// ** films.find( str )
Handle<Value> find( const Arguments& args )
{
    HandleScope scope;

    std::string str     = ARG_STRING( 0, "" );
    FilmsArray  items   = gFilms.find( str );

    Local<Array> result = Array::New();
    for( int i = 0; i < ( int )items.size(); i++ ) {
        Local<Object> item = Object::New();
        item->Set( String::NewSymbol( "id" ),       String::NewSymbol( items[i].m_id.toString().c_str() ) );
        item->Set( String::NewSymbol( "name" ),     String::NewSymbol( items[i].m_name.c_str() ) );
        item->Set( String::NewSymbol( "genres" ),   String::NewSymbol( Films::formatGenres( items[i].m_genres ).c_str() ) );
        item->Set( String::NewSymbol( "year" ),     Number::New( items[i].m_year ) );

        result->Set( i, item );
    }

    return scope.Close( result );
}

// ** findSimilar
// ** films.findSimilar( id, similarityThreshold, accuracyInfluence, accuracyThreshold, count )
Handle<Value> findSimilar( const Arguments& args )
{
    HandleScope scope;

    mongo::StringSet oids;
    Local<Array> input = Local<Array>::Cast( args[0] );
    for( int i = 0; i < ( int )input->Length(); i++ ) {
        oids.insert( *String::Utf8Value( input->Get( i )->ToString() ) );
    }

    int count = ARG_NUMBER( 1, 0  );

    SimilarFilmsArray similar = gFilms.similarTo( oids, count );

    Local<Array> result = Array::New();
    for( int i = 0; i < ( int )similar.size(); i++ ) {
        Local<Object> item = Object::New();
        item->Set( String::NewSymbol( "id" ),           String::NewSymbol( similar[i].m_film.m_id.toString().c_str() ) );
        item->Set( String::NewSymbol( "name" ),         String::NewSymbol( similar[i].m_film.m_name.c_str() ) );
        item->Set( String::NewSymbol( "genres" ),       String::NewSymbol( Films::formatGenres( similar[i].m_film.m_genres ).c_str() ) );
        item->Set( String::NewSymbol( "year" ),         Number::New( similar[i].m_film.m_year ) );
        item->Set( String::NewSymbol( "video" ),        String::NewSymbol( similar[i].m_film.m_video.c_str() ) );
        item->Set( String::NewSymbol( "similarity" ),   Number::New( similar[i].m_similarity ) );
        item->Set( String::NewSymbol( "shared" ),       Number::New( similar[i].m_shared ) );
        item->Set( String::NewSymbol( "quality" ),      Number::New( similar[i].m_quality ) );
        item->Set( String::NewSymbol( "qualityText" ),  String::NewSymbol( Films::qualityToString( similar[i].m_quality ).c_str() ) );

        result->Set( i, item );
    }

    return scope.Close( result );
}

void init( Handle<Object> exports )
{
    HandleScope scope;

    exports->Set( String::NewSymbol( "connect" ),       FunctionTemplate::New( connect      )->GetFunction() );
    exports->Set( String::NewSymbol( "find" ),          FunctionTemplate::New( find         )->GetFunction() );
    exports->Set( String::NewSymbol( "findSimilar" ),   FunctionTemplate::New( findSimilar  )->GetFunction() );
}

NODE_MODULE( hello, init )