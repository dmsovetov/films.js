//
//  Db.cpp
//  DataMining
//
//  Created by Советов Дмитрий on 30.12.14.
//  Copyright (c) 2014 data. All rights reserved.
//

#include "Films.h"
#include "recommendation/Recommender.h"

using namespace mongo;

// ** Films::connect
bool Films::connect( const std::string& host, const std::string& source, const std::string& target )
{
    m_source = ConnectionPtr( new Connection( host, source ) );
    m_target = ConnectionPtr( new Connection( host, target ) );
/*
    // ** Ensure indices
    m_source->collection( "items" )->ensureIndex( "idxItemId",      DOCUMENT( "itemId" << 1 ), true  );
    m_source->collection( "users" )->ensureIndex( "idxUserId",      DOCUMENT( "userId" << 1 ), true  );
    m_source->collection( "votes" )->ensureIndex( "idxVoteItemId",  DOCUMENT( "itemId" << 1 ), false );
*/
    // ** Create iterator
    m_items = new StreamedFilms( *this, m_source );

    // ** Get the similarity & shared quartiles
    DocumentPtr info      = m_target->collection( "info" )->findOne( QUERY( "type" << "similarity" ) );
    m_similarityQuartiles = info->numbers( "similarity" );
    m_sharedQuartiles     = info->numbers( "shared" );

    return true;
}

// ** Films::findById
RecommenderItem Films::findById( int itemId ) const
{
    RecommenderItem item;

    if( m_items->findById( itemId, item ) ) {
        return item;
    }

    return item;
}

// ** Films::items
IRecommenderItems* Films::items( void ) const
{
    return m_items->clone();
}

// ** Films::processFilms
void Films::processFilms( float sharedThreshold )
{
    CursorPtr           films       = m_source->collection( "items" )->find();
    CollectionPtr       processed   = m_target->collection( "items" );
    CollectionPtr       similar     = m_target->collection( "similar" );

    IRecommenderItems*  items       = new StreamedFilms( *this, m_target );
    SimilarityPtr       similarity  = SimilarityPtr( new JaccardAccuracySimilarity( NumericFeatures::pearson, 0.0f, 0.0f, sharedThreshold ) );
    Recommender         recommender = Recommender( this, similarity.get() );

    DocumentPtr         document;

    int counter = 0;
    int total   = m_source->collection( "items" )->count();

    while( (document = films->next()) ) {
        // ** Read the item id
        int itemId = document->integer( "itemId" );

        // ** Print progress
        if( (++counter % 10) == 0 ) printf( "Processing films [%d/%d]\n", counter, total );

        // ** Skip processed films
        if( processed->findOne( QUERY( "itemId" << itemId ) ) != NULL ) {
            continue;
        }

        // ** Load first item
        items->first();

        // ** Compute and store similarities
        RecommenderItem item    = findById( itemId );
        SimilarItems    pairs   = recommender.findSimilarItems( items, &item );
        OID             oid     = filmIdToObjectId( itemId );

        for( SimilarItems::const_iterator i = pairs.begin(), end = pairs.end(); i != end; ++i ) {
            similar->insert( DOCUMENT( "first" << oid << "second" << filmIdToObjectId( i->m_item ) << "value" << ( int )encodeSimilarity( i->m_similarity, i->m_shared ) ) );
        }

        // ** Store processed film
        processed->insert( DOCUMENT( "itemId" << itemId ) );
    }

    // ** Build indices
    similar->ensureIndex( "idxFirst",  DOCUMENT( "first"  << 1 ) );
    similar->ensureIndex( "idxSecond", DOCUMENT( "second" << 1 ) );

    // ** Delete iterator
    delete items;
}

// ** Films::filmById
Film Films::filmById( const OID& oid ) const
{
    DocumentPtr document = m_source->collection( "items" )->findOne( QUERY( "_id" << oid ) );
    assert( document != NULL );

    return filmFromDocument( document );
}

// ** Films::find
FilmsArray Films::find( const std::string& name ) const
{
    FilmsArray result;

    std::string query;
    std::transform( name.begin(), name.end(), std::back_inserter( query ), toupper );

    BSON          regex  = DOCUMENT( "$regex" << query );
    CollectionPtr items  = m_source->collection( "items" );
    CursorPtr     cursor = items->find( QUERY( "$or" << ARRAY( DOCUMENT( "search.en" << regex ) << DOCUMENT( "search.ru" << regex ) ) ) );
    DocumentPtr   document;

    while( (document = cursor->next()) ) {
        result.push_back( filmFromDocument( document ) );
    }

    return result;
}

// ** Films::filmFromDocument
Film Films::filmFromDocument( const DocumentPtr& document ) const
{
    Film film( document->_id(), document->string( "name.ru" ), document->integerSet( "genres" ), document->integer( "year" ) );
    film.m_video = document->string( "video" );

    return film;
}

// ** Films::qualityFromRange
Quality Films::qualityFromRange( float value, const FloatArray& quartiles )
{
    Quality qualities[] = { Bad, Bad, Medium, Good };
    for( int i = 0; i < 3; i++ ) {
        if( value < quartiles[i] ) return qualities[i];
    }

    return Good;
}

// ** Films::qualityToString
std::string Films::qualityToString( Quality quality )
{
    const char* str[] = { "Плохое", "Хорошее", "Отличное" };
    return str[quality];
}

// ** Films::formatGenres
std::string Films::formatGenres( const IntegerSet& genres )
{
    const char* genreName[] = {
        "",
        "Экшн",
        "Приключения",
        "Мультфильм",
        "Детский",
        "Комедия",
        "Криминал",
        "Документальный",
        "Драма",
        "Фентези",
        "Нуар",
        "Ужасы",
        "Музыкальный",
        "Таинственный",
        "Романтический",
        "Научная фантастика",
        "Триллер",
        "Военный",
        "Вестерн",
        "IMAX",
    };

    std::string result;
    for( IntegerSet::const_iterator i = genres.begin(), end = genres.end(); i != end; ++i ) {
        if( result != "" ) {
            result += ", ";
        }
        
        result += genreName[*i];
    }

    return result;
}

// ** Films::encodeSimilarity
unsigned int Films::encodeSimilarity( float similarity, float accuracy )
{
    unsigned int s = similarity * USHRT_MAX;
    unsigned int a = accuracy   * USHRT_MAX;
    unsigned int r = s << 16 | a;

    float _s, _a;
    decodeSimilarity( r, _s, _a );
    assert( fabs( _s - similarity ) <= 0.001f && fabs( _a - accuracy ) <= 0.001f );

    return r;
}

// ** Films::decodeSimilarity
void Films::decodeSimilarity( unsigned int encoded, float& similarity, float& accuracy )
{
    unsigned int s = encoded >> 16;
    unsigned int a = encoded & 0xFFFF;

    similarity = float( s ) / USHRT_MAX;
    accuracy   = float( a ) / USHRT_MAX;
}

// ** Films::filmIdToObjectId
OID Films::filmIdToObjectId( int filmId ) const
{
    DocumentPtr document = m_source->collection( "items" )->findOne( QUERY( "itemId" << filmId ) );
    assert( document != NULL );

    return document->_id();
}

// ** Films::similarTo
SimilarFilmsArray Films::similarTo( const StringSet& oids, int count ) const
{
    struct SimilarFilmIntersection {
        SimilarFilm     m_film;
        float           m_similarity;
        int             m_counter;

                        SimilarFilmIntersection( const SimilarFilm& film = SimilarFilm() )
                            : m_film( film ), m_similarity( 0.0f ), m_counter( 0 ) {}
    };

    std::map<std::string, SimilarFilmIntersection>  filmIntersection;
    SimilarFilmsArray           result;

    for( StringSet::const_iterator i = oids.begin(), end = oids.end(); i != end; ++i ) {
        SimilarFilmsArray items = similarTo( *i );

        for( SimilarFilmsArray::const_iterator j = items.begin(), jend = items.end(); j != jend; ++j ) {
            const SimilarFilm&  similar = *j;
            std::string         key     = j->m_film.m_id.toString();

            if( filmIntersection.count( key ) == 0 ) {
                filmIntersection[key] = similar;
            }

            filmIntersection[key].m_similarity += j->m_similarity;
            filmIntersection[key].m_counter++;
        }
    }

    for( auto i = filmIntersection.begin(), end = filmIntersection.end(); i != end; ++i ) {
        if( i->second.m_counter == ( int )oids.size() ) {
            float similarity = i->second.m_similarity / i->second.m_counter;
            result.push_back( SimilarFilm( i->second.m_film.m_film, similarity, 0.0f, qualityFromRange( similarity, m_similarityQuartiles ) ) );
        }
    }

    std::sort( result.begin(), result.end(), SimilarFilm::sortBySimilarity );
    if( count ) {
        result.resize( std::min( count, ( int )result.size() ) );
    }

    return result;
}

// ** Films::similarTo
SimilarFilmsArray Films::similarTo( const std::string& oid, int count ) const
{
    SimilarFilmsArray result;

    OID           objectId = OID( oid );
    CollectionPtr similars = m_target->collection( "similar" );
    CursorPtr     cursor   = similars->find( QUERY( "$or" << ARRAY( DOCUMENT( "first" << oid ) << DOCUMENT( "second" << objectId ) ) ) );
    DocumentPtr   document;

    while( (document = cursor->next()) ) {
        // ** Read data from document
        OID first  = document->objectId( "first" );
        OID second = document->objectId( "second" );
        int value  = document->integer( "value" );

        // ** Decode similarity & accuracy
        float similarity, accuracy;
        decodeSimilarity( value, similarity, accuracy );

        // ** Push similar film
        Film    film    = filmById( first == objectId ? second : first );
        Quality quality = qualityFromRange( similarity, m_similarityQuartiles );
        result.push_back( SimilarFilm( film, similarity, accuracy, quality ) );
    }

    std::sort( result.begin(), result.end(), SimilarFilm::sortBySimilarity );
    if( count ) {
        result.resize( std::min( count, ( int )result.size() ) );
    }

    return result;
}

// ** Films::votesForFilm
NumericFeatures Films::votesForFilm( int filmId ) const
{
    NumericFeatures result;

    CollectionPtr votes  = m_source->collection( "votes" );
    CursorPtr     cursor = votes->find( QUERY( "itemId" << filmId ) );
    DocumentPtr   document;

    while( (document = cursor->next()) ) {
        printf( "get\n" );
        result.set( document->integer( "userId" ), document->number( "rating" ) );
        printf( "v %d\n", result.size() );
    //    sleep(1);
        printf( "next\n" );
    }

    printf( "done! %d\n", result.size() );

    return result;
}

// ** Films::updateSharedAndSimilarityRanges
void Films::updateSharedAndSimilarityRanges( void )
{
    CollectionPtr similar   = m_target->collection( "similar" );
    CursorPtr     cursor    = similar->find();
    int           count     = similar->count();
    int           progress  = 0;
    DocumentPtr   document;

    FloatSamples  similaritySamples, sharedSamples;

    while( (document = cursor->next()) ) {
        float similarity, shared;
        decodeSimilarity( document->integer( "value" ), similarity, shared );

        similaritySamples += similarity;
        sharedSamples     += shared;

        if( (++progress % 1000) == 0 ) printf( "Updating ranges [%d/%d]\n", progress, count );
    }

    m_target->collection( "info" )->upsert( QUERY( "type" << "similarity" ), DOCUMENT( "type" << "similarity" << "shared" << sharedSamples.quartiles() << "similarity" << similaritySamples.quartiles() ) );
}

// ** Films::updateVotesCount
void Films::updateVotesCount( void )
{
    CollectionPtr  items  = m_source->collection( "items" );
    CollectionPtr  votes  = m_source->collection( "votes" );
    CursorPtr      cursor = items->find();
    DocumentPtr    document;
    IntegerSamples samples;

    int progress = 0;
    int total    = items->count();

    while( (document = cursor->next()) ) {
        int itemId = document->integer( "itemId" );
        int count  = votes->count( QUERY( "itemId" << itemId ) );

        items->update( QUERY( "itemId" << itemId ), DOCUMENT( "$set" << DOCUMENT( "votesCount" << count ) ) );
        printf( "Updating votes count [%d/%d]...\n", ++progress, total );
    }
}

// ** Films::showStats
void Films::showStats( void ) const
{
    CollectionPtr  items  = m_source->collection( "items" );
    CursorPtr      cursor = items->find();
    DocumentPtr    document;
    IntegerSamples countSamples;

    while( (document = cursor->next()) ) {
        countSamples += document->integer( "votesCount" );
    }

    IntegerArray quartiles = countSamples.quartiles();

    printf( "Votes: min %d, max %d, average amount %d, medean %d, quartiles %d/%d/%d\n", countSamples.min(), countSamples.max(), countSamples.mean(), countSamples.median(), quartiles[0], quartiles[1], quartiles[2] );

    for( int i = 2; i <= 7; i++ ) {
        int amount = pow( 10, i );
        printf( "%d items has greater than %d votes\n", countSamples.greaterCount( amount ), amount );
    }
}

// ** Films::objectIdToFilmId
int Films::objectIdToFilmId( const OID& oid ) const
{
    int result = -1;

    CollectionPtr items = m_source->collection( "items" );
    DocumentPtr   item  = items->findOne( QUERY( "_id" << oid ) );

    assert( item != NULL );
    if( item != NULL ) {
        result = item->integer( "itemId" );
    }

    return result;
}

// ----------------------------------------------------- FilmIterator ------------------------------------------------------ //

// ** PreloadedFilms::PreloadedFilms
PreloadedFilms::PreloadedFilms( const Films& films, const CursorPtr& cursor )
{
    do {
        DocumentPtr document = cursor->next();
        if( document == NULL ) {
            break;
        }

        RecommenderItem* item = new RecommenderItem;
        item->m_userData                = new OID( *document->_id().value() );
        item->m_id                      = document->integer( "itemId" );
        item->m_name                    = document->string( "name.ru" );
        item->m_featureSpaces["votes"]  = films.votesForFilm( item->m_id );
        m_items[item->m_id]             = item;
    } while( true );
}

PreloadedFilms::PreloadedFilms( const RecommenderItems& items ) : m_items( items )
{
    
}

// ** PreloadedFilms::findById
bool PreloadedFilms::findById( int itemId, RecommenderItem& item ) const
{
    RecommenderItems::const_iterator i = m_items.find( itemId );
    if( i != m_items.end() ) {
        item = *i->second;
        return true;
    }

    return false;
}

// ** PreloadedFilms::clone
IRecommenderItems* PreloadedFilms::clone( void )
{
    return new PreloadedFilms( m_items );
}

// ** PreloadedFilms::startFrom
bool PreloadedFilms::startFrom( const RecommenderItem& item )
{
    m_iterator = m_items.find( item.m_id );
    return m_iterator != m_items.end();
}

// ** PreloadedFilms::first
bool PreloadedFilms::first( void )
{
    m_iterator = m_items.begin();
    return true;
}

// ** PreloadedFilms::next
bool PreloadedFilms::next( RecommenderItem& item )
{
    if( m_iterator == m_items.end() ) {
        return false;
    }

    item = *m_iterator->second;
    ++m_iterator;

    return true;
}

// ** PreloadedFilms::size
int PreloadedFilms::size( void )
{
    return ( int )m_items.size();
}

// --------------------------------------------------- StreamedFilms ------------------------------------------------- //

// ** StreamedFilms::StreamedFilms
StreamedFilms::StreamedFilms( const Films& films, const ConnectionPtr& connection ) : m_films( films ), m_connection( connection )
{

}

// ** StreamedFilms::findById
bool StreamedFilms::findById( int itemId, RecommenderItem& item ) const
{
    DocumentPtr document = m_connection->collection( "items" )->find( QUERY( "itemId" << itemId ) )->next();
    if( document != NULL ) {
        documentToItem( item, document );
        return true;
    }

    return false;
}

// ** StreamedFilms::clone
IRecommenderItems* StreamedFilms::clone( void )
{
    return new StreamedFilms( m_films, m_connection );
}

// ** StreamedFilms::size
int StreamedFilms::size( void )
{
    return m_connection->collection( "items" )->count();
}

// ** StreamedFilms::startFrom
bool StreamedFilms::startFrom( const RecommenderItem& item )
{
    assert( false );
    return false;
}

// ** StreamedFilms::first
bool StreamedFilms::first( void )
{
    m_cursor = m_connection->collection( "items" )->find();
    return true;
}

// ** StreamedFilms::next
bool StreamedFilms::next( RecommenderItem& item )
{
    assert( m_cursor != NULL );

    DocumentPtr document = m_cursor->next();

    if( document == NULL ) {
        return false;
    }

    documentToItem( item, document );

    return true;
}

// ** StreamedFilms::documentToItem
void StreamedFilms::documentToItem( RecommenderItem& item, const DocumentPtr& document ) const
{
    item.m_id                      = document->integer( "itemId" );
    item.m_featureSpaces["votes"]  = m_films.votesForFilm( item.m_id );
}