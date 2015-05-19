var express = require( 'express' )
  , morgan  = require( 'morgan' )
  , async   = require( 'async' )
  , films   = require( './build/Release/hello' )
  , app     = express()

// ** launch
module.exports.launch = function( port ) {
    console.log( 'Working directory', process.cwd() )

    app.set( 'view engine', 'jade' )
    app.set( 'views', './views')
    app.use( morgan( 'dev' ) )
    app.use( express.static( __dirname + '/public' ) )

    // ** Connect to films database
    films.connect( "mongodb://localhost:27017/", "movielens", "recommender" )

    // ** Index
    app.get( '/',
        function( req, res ) {
            res.render( 'index', { title: 'Hey', caption: 'Поиск фильмов', description: 'Приложение находится на стадии разработки', initial: 'asdad' } )
        } )

    // ** Items
    app.get( '/items',
        function( req, res ) {
            var text  = req.param( "query" )
            var items = films.find( text.toUpperCase() )
            res.json( items )
        } )

    // ** Similar
    app.get( '/similar',
        function( req, res ) {
            try {
                var items = JSON.parse( req.param( 'query' ) )
                if( items.length == 0 ) {
                    return res.render( 'info', { message: 'Строка поиска пуста', description: 'Для того, чтобы начать пользоваться приложением, необходимо ввести в строку поиска один или несколько понравившихся Вам фильмов.' } )
                }

                var similar = films.findSimilar( items, 25 )
                if( similar.length == 0 ) {
                    return res.render( 'alert', { message: 'К сожалению ничего не найдено', description: 'Попробуйте изменить Ваш запрос.' } )
                }

                res.render( 'similar', { items: similar } )
            } catch( e ) {
                res.send( e )
            }
        } )

    var server = app.listen( port,
        function() {
            console.log( 'Listening...' )
/*
            films.connect( "mongodb://localhost:27017/", "movielens", "recommender" )
            var items   = films.find( 'Die Hard \\(1988\\)' )
            var similar = films.findSimilar( items[0].id )

            console.log( similar.length, 'similar items for', items[0].name )
            similar.forEach(
                function( item ) {
                    console.log( '\t', item.name )
                } )
*/
        } )
}