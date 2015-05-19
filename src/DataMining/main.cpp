//
//  main.cpp
//  DataMining
//
//  Created by Советов Дмитрий on 18.12.14.
//  Copyright (c) 2014 data. All rights reserved.
//

#include <assert.h>

#include "DataSet.h"
#include "recommendation/Recommender.h"
#include "Cluster.h"
#include "statistics/LinearRegression.h"
#include "Films.h"

#define USE_SIMPLE_DATASET      (0)
#define DATASET_TO_DB           (0)
/*
struct TestUserVote {
    int     m_filmId;
    float   m_vote;
};

void testUserRecommendations( const DataSet& data, int userId, Similarity* votesSimilarity )
{
    RecommenderItems users   = data.recommenderUsers();
    Recommender      rec     = Recommender( users, votesSimilarity );
    RecommenderItem* user    = users.find( userId )->second;
    SimilarItems     similar = rec.findSimilarItems( user );

    printf( "%d similar users to %s\n", ( int )similar.size(), user->m_name.c_str() );
    for( int i = 0; i < ( int )similar.size(); i++ ) {
        printf( "\t%s:%2.4f \n", similar[i].m_item->m_name.c_str(), similar[i].m_similarity );
    }
    printf( "\n" );

    Recommendations films = rec.recommend( user, 30 );
    printf( "%d recommendations for %s\n", ( int )films.size(), user->m_name.c_str() );
    for( int i = 0; i < ( int )films.size(); i++ ) {
        const Item* item = data.itemById( films[i].m_id );
        printf( "\t%d: %s (~%2.4f)\n", item->m_itemId, item->m_name.c_str(), films[i].m_score );
    }
    printf( "\n" );
}

void testFilmRecommendations( const DataSet& data, const PreferenceItems& preferences, Similarity* votesSimilarity, Similarity* genreRelevance )
{
    RecommenderItems     films       = data.recommenderFilms();
    ItemBasedRecommender rec         = ItemBasedRecommender( films, votesSimilarity, genreRelevance );
    Recommendations      recommended = rec.recommend( preferences, 25 );

    printf( "Recommended films:\n" );
    for( int i = 0; i < ( int )recommended.size(); i++ ) {
        const Item* film = data.itemById( recommended[i].m_id );
        printf( "\t%s - %s [%2.2f with total similarity %2.4f and counter %d]\n", film->m_name.c_str(), film->m_genreString.c_str(), recommended[i].m_score, recommended[i].m_total, recommended[i].m_counter );
    }
    printf( "\n" );
}

void testRecommedationByFilm( const DataSet& data, int filmId, Similarity* votesSimilarity, Similarity* genreRelevance )
{
    RecommenderItems films    = data.recommenderFilms();
    Recommender      rec      = Recommender( films, votesSimilarity, genreRelevance );
    RecommenderItem* film     = films.find( filmId )->second;
    RelevantItems    relevant = rec.findRelevantItems( film, 10 );

    // **
    PreferenceItems preferences;

    preferences.push_back( PreferenceItem( films.find( filmId )->second, 5 ) );

    for( int i = 0; i < ( int )relevant.size(); i++ ) {
        preferences.push_back( PreferenceItem( relevant[i].m_item, 5 ) );
    }

    testFilmRecommendations( data, preferences, votesSimilarity, genreRelevance );
}

void testFilmSimilarity( const DataSet& data, int filmId, Similarity* votesSimilarity )
{
    RecommenderItems films    = data.recommenderFilms();
    Recommender      rec      = Recommender( films, votesSimilarity );
    RecommenderItem* film     = films.find( filmId )->second;
    SimilarItems     similar  = rec.findSimilarItems( film, 100 );

    printf( "%d total votes for %s\n", film->features( "votes" ).size(), film->m_name.c_str() );
    printf( "%d similar films to %s - %s\n", ( int )similar.size(), film->m_name.c_str(), data.itemById( filmId )->m_genreString.c_str() );
    for( int i = 0; i < ( int )similar.size(); i++ ) {
        printf( "\t%s - %s [%2.4f with %2.2f%% shared votes]\n", similar[i].m_item->m_name.c_str(), data.itemById( similar[i].m_item->m_id )->m_genreString.c_str(), similar[i].m_similarity, similar[i].m_shared * 100 );
    }
    printf( "\n" );
}

void testFilmRelevance( const DataSet& data, int filmId, Similarity* votesSimilarity, Similarity* genreRelevance )
{
    RecommenderItems films    = data.recommenderFilms();
    Recommender      rec      = Recommender( films, votesSimilarity, genreRelevance );
    RecommenderItem* film     = films.find( filmId )->second;
    RelevantItems    relevant = rec.findRelevantItems( film, 50 );

    printf( "%d relevant films to %s - %s\n", ( int )relevant.size(), film->m_name.c_str(), data.itemById( filmId )->m_genreString.c_str() );
    for( int i = 0; i < ( int )relevant.size(); i++ ) {
        const Item* film = data.itemById( relevant[i].m_item->m_id );
        printf( "\t%s - %s [similarity %2.4f, genre relevance %2.4f, score %2.4f]\n", film->m_name.c_str(), film->m_genreString.c_str(), relevant[i].m_similarity, relevant[i].m_relevance, relevant[i].m_score );
    }
    printf( "\n" );
}

void testGenreRelevance( const DataSet& data, NumericFeaturesSimilarity similarity )
{
    Genres genres = data.genres();

    struct TestGenreRelevance {
        std::string a, b;
        float       relevance;
        int         shared;

        static bool sort( const TestGenreRelevance& a, const TestGenreRelevance& b ) { return a.relevance > b.relevance; }
    };

    std::vector<TestGenreRelevance> genreRelevance;

    for( Genres::const_iterator i = genres.begin(), end = genres.end(); i != end; ++i ) {
        for( Genres::const_iterator j = genres.find( i->first ); j != end; ++j ) {
            TestGenreRelevance r;
            r.a         = i->first;
            r.b         = j->first;
            r.relevance = similarity( i->second, j->second, &r.shared );

            if( r.a == r.b ) {
                if( fabs( r.relevance - 1.0f ) > 0.0001f ) printf( "Invalid relevance %2.2f for %s and %s\n", r.relevance, r.a.c_str(), r.b.c_str() );
                continue;
            }

            if( r.relevance > 0 ) {
                genreRelevance.push_back( r );
            }
        }
    }

    std::sort( genreRelevance.begin(), genreRelevance.end(), TestGenreRelevance::sort );

    printf( "Genre relevance:\n" );
    for( int i = 0; i < genreRelevance.size(); i++ ) {
        printf( "%s to %s is %2.5f (%d shared)\n", genreRelevance[i].a.c_str(), genreRelevance[i].b.c_str(), genreRelevance[i].relevance, genreRelevance[i].shared );
    }
}

#include <fstream>
#include <iostream>

void replaceStringInFile( const char* sourceFileName, const char* destFileName, const char* str, const char* repl )
{
    using namespace std;

    ifstream    in( sourceFileName );
    ofstream    out( destFileName );
    string      wordToReplace( str );
    string      wordToReplaceWith( repl );

    if( !in ) {
        printf( "failed to open %s\n", sourceFileName );
        return;
    }
    if( !out ) {
        printf( "failed to open %s\n", destFileName );
        return;
    }

    string line;
    size_t len = wordToReplace.length();

    while( getline( in, line ) ) {
        while( true )
        {
            size_t pos = line.find( wordToReplace );
            if( pos != string::npos ) {
                line.replace( pos, len, wordToReplaceWith );
            } else {
                break;
            }
        }

        out << line << '\n';
    }
}
*/

int main(int argc, const char * argv[])
{
#if DATASET_TO_DB
    DataSet data;

    data.loadItems( "data/10m/movies.csv" );
    data.loadRatings( "data/10m/ratings.csv" );

    Films films;
    if( films.connect( "mongodb://localhost:27017/", "movielens", "recommender" ) ) {
        data.save( films, false );
    }
#else
/*
    mongo::ConnectionPtr connection( new mongo::Connection( "mongodb://localhost:27017/", "kinopoisk" ) );
    mongo::CollectionPtr discovered = connection->collection( "discovered" );
    mongo::CursorPtr     cursor     = discovered->find();
    mongo::CollectionPtr items      = connection->collection( "items" );
    mongo::CollectionPtr users      = connection->collection( "users" );
    mongo::DocumentPtr   document;

    while( (document = cursor->next()) ) {
        std::string type    = document->string( "type" );
        int         itemId  = document->integer( "itemId" );

        mongo::DocumentPtr item = type == "user" ? users->findOne( QUERY( "userId" << itemId ) ) : items->findOne( QUERY( "itemId" << itemId ) );
        if( item != NULL ) {
            discovered->remove( QUERY( "_id" << document->_id() ) );
        }
    }
*/

    Films films;
//    films.connect( "mongodb://localhost:27017/", "movielens", "recommender" );

//    films.updateSharedAndSimilarityRanges();

    films.connect( "mongodb://localhost:27017/", "kinopoisk", "recommender" );
//    films.updateVotesCount();
    films.showStats();

//    films.processFilms( 0.04f );
/*
    // ** Find
    FilmsArray items = films.find( "Die Hard" );
    for( int i = 0; i < items.size(); i++ ) {
        printf( "%s\n", items[i].m_name.c_str() );
    }

    // ** Find similar

    TimeMeasure* t1 = new TimeMeasure( "similarTo" );
    SimilarFilmsArray similar = films.similarTo( films.filmIdToObjectId( DataSet::HomeAlone2 ).toString() );
    delete t1;

    printf( "\n%d similar items for %s\n", ( int )similar.size(), films.filmById( films.filmIdToObjectId( DataSet::SomeLikeItHot ) ).m_name.c_str() );
    for( int i = 0; i < similar.size(); i++ ) {
        printf( "%s : %s : %2.2f (%s)\n", Films::qualityToString( similar[i].m_quality ).c_str(), similar[i].m_film.m_name.c_str(), similar[i].m_similarity, Films::formatGenres( similar[i].m_film.m_genres ).c_str() );
    }
*/
/*
    mongo::StringSet oids;
    oids.insert( films.filmIdToObjectId( DataSet::HomeAlone ).toString() );
    oids.insert( films.filmIdToObjectId( DataSet::SomeLikeItHot ).toString() );
    similar = films.similarTo( oids );

    printf( "\n%d similar items for %s\n", ( int )similar.size(), films.filmById( films.filmIdToObjectId( DataSet::SomeLikeItHot ) ).m_name.c_str() );
    for( int i = 0; i < similar.size(); i++ ) {
        printf( "%s : %s : %2.2f\n", Films::qualityToString( similar[i].m_quality ).c_str(), similar[i].m_film.m_name.c_str(), similar[i].m_similarity );
    }
*/
    // ** Build similars
//    films.calculateSimilarities( 0.01f, 0.01f, 1.0f, 50, 100 );

    /*
    Db db;

    if( db.connect( "mongodb://localhost:27017/", "movielens" ) ) {
        FilmsArray films = db.findFilms( "Die Hard" );   // "Die Hard \\(1988\\)"
        for( int i = 0; i < films.size(); i++ ) {
            printf( "%s: %s\n", films[i].m_id.toString().c_str(), films[i].m_name.c_str() );
        }*/
/*
        SimilarFilmsArray similars;
        similars.push_back( SimilarFilm( films[0].m_id, 1.10f ) );
        similars.push_back( SimilarFilm( films[0].m_id, 30.90f ) );
        similars.push_back( SimilarFilm( films[0].m_id, 0.67f ) );
        similars.push_back( SimilarFilm( films[0].m_id, 0.12f ) );
        db.addSimilarFilms( films[0].m_id, similars );
*/
/*
        SimilarFilmsArray similar = db.similarTo( films[0].m_id );

        for( int i = 0; i < similar.size(); i++ ) {
            printf( "%s : %2.2f\n", similar[i].m_id.toString().c_str(), similar[i].m_similarity );
        }

        VotesArray votes = db.votesForFilm( films[0].m_id );
        for( int i = 0; i < votes.size(); i++ ) {
            printf( "%d : %2.2f\n", votes[i].m_userId, votes[i].m_value );
        }
    }*/
#endif
/*
    DataSet data;

#if USE_SIMPLE_DATASET
    UserId testUserId           = 1;
    ItemId testFilmId           = 4;
    float  similarityThreshold  = 0.1f;
    int    sharedThreshold      = 10;
    data.loadSimple();

    testFilmRecomendations( data, 4, similarityThreshold, 0.0f );
#else
//    float  similarityThreshold  = 0.01f;
//    float  sharedInfluence      = 2;
    #if USE_1m
        data.loadItems( "data/1m/movies.csv" );
        data.loadRatings( "data/1m/ratings.csv" );
    #elif USE_10m
        data.loadItems( "data/10m/movies.csv" );
        data.loadRatings( "data/10m/ratings.csv" );
    #else
        data.loadItems( "data/100k/movies.csv" );
        data.loadRatings( "data/100k/ratings.csv" );
    #endif

    Db db;
    if( db.connect( "localhost", "movielens" ) ) {
        data.save( db );
    }

    Similarity* votesSimilarity     = new JaccardAccuracySimilarity( NumericFeatures::pearson, 0.01f, 0.00f, 0.15f );
    Similarity* votesSimilarity2_0  = new JaccardAccuracySimilarity( NumericFeatures::pearson, 0.01f, 2.00f, 0.15f );
    Similarity* votesSimilarity1_25 = new JaccardAccuracySimilarity( NumericFeatures::pearson, 0.01f, 1.25f, 0.0f );
    Similarity* genreRelevance2_0   = new JaccardAccuracySimilarity( NumericFeatures::cosine,  0.00f, 2.00f, 0.0f );

    printf( "%d users loaded, %d items loaded\n", ( int )data.users().size(), ( int )data.items().size() );
    printf( "%2.4f votes per item\n", data.averageVotesPerItem() );

    data.subtractUserAverage();
//    data.convertToBinaryRatings();

//    data.dumpGenres( "output/preferences.csv", 253, 4389 );

//    data.dumpVotesLineChart( "output/homeAlone.csv", DataSet::HomeAlone, DataSet::HomeAlone2 );
//    data.dumpVotesLineChart( "output/homeAlone3.csv", DataSet::HomeAlone, DataSet::HomeAlone3 );
//    data.dumpVotesLineChart( "output/forrestGump.csv", DataSet::HomeAlone, DataSet::ForrestGump );

    // ** DieHard
    testFilmSimilarity( data, DataSet::DieHard, votesSimilarity );
    testFilmRelevance( data, DataSet::DieHard, votesSimilarity, genreRelevance2_0 );
    testRecommedationByFilm( data, DataSet::DieHard, votesSimilarity, genreRelevance2_0 );


    // ** Home Alone
    testFilmSimilarity( data, DataSet::HomeAlone, votesSimilarity2_0 );
    testFilmRelevance( data, DataSet::HomeAlone, votesSimilarity2_0, genreRelevance2_0 );
    testRecommedationByFilm( data, DataSet::HomeAlone, votesSimilarity2_0, genreRelevance2_0 );

    // ** Miracle on 34th street
    testFilmSimilarity( data, DataSet::MiracleOn34thStreet, votesSimilarity1_25 );
    testFilmRelevance( data, DataSet::MiracleOn34thStreet, votesSimilarity1_25, genreRelevance2_0 );
    testRecommedationByFilm( data, DataSet::MiracleOn34thStreet, votesSimilarity1_25, genreRelevance2_0 );

    // ** Scrooge
    testFilmSimilarity( data, DataSet::Scrooge, votesSimilarity1_25 );
    testFilmRelevance( data, DataSet::Scrooge, votesSimilarity1_25, genreRelevance2_0 );
    testRecommedationByFilm( data, DataSet::Scrooge, votesSimilarity1_25, genreRelevance2_0 );

    // ** Rent a kid
    testFilmSimilarity( data, DataSet::RentAKid, votesSimilarity1_25 );
    testFilmRelevance( data, DataSet::RentAKid, votesSimilarity1_25, genreRelevance2_0 );
    testRecommedationByFilm( data, DataSet::RentAKid, votesSimilarity1_25, genreRelevance2_0 );
#endif

    LinearRegression::selfTest();
*/
    return 0;
}
