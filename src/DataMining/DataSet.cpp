//
//  DataSet.cpp
//  DataMining
//
//  Created by Советов Дмитрий on 18.12.14.
//  Copyright (c) 2014 data. All rights reserved.
//

#include "DataSet.h"
#include "Films.h"

#include "csv/csv_parser.hpp"
/*
ClusterItems DataSet::itemList( void ) const
{
    ClusterItems items;

    assert( false );
    return items;
}

ClusterItems DataSet::userList( void ) const
{
    ClusterItems items;

    for( Users::const_iterator i = m_users.begin(), end = m_users.end(); i != end; ++i ) {
        items.push_back( i->second );
    }

    return items;
}
*/

Item* DataSet::addItem( int itemId, const char* title )
{
    m_items[itemId] = new Item;
    m_items[itemId]->m_name   = title;
    m_items[itemId]->m_itemId = itemId;
    return m_items[itemId];
}

User* DataSet::addUser( int userId, const char* name )
{
    m_users[userId] = new User;
    m_users[userId]->m_name   = name;
    m_users[userId]->m_userId = userId;
    return m_users[userId];
}

void DataSet::addVote( int userId, int itemId, UserVote value )
{
    itemById( itemId );
    const User* user = userById( userId );
    const Item* item = itemById( itemId );

    const_cast<User*>( user )->m_votes.set( itemId, value );
    const_cast<Item*>( item )->m_votes.set( userId, value );
//    const_cast<User*>( user )->m_votes[itemId] = value;
}

RecommenderItems DataSet::recommenderUsers( void ) const
{
    RecommenderItems users;

    for( Users::const_iterator i = m_users.begin(), end = m_users.end(); i != end; ++i ) {
        RecommenderItem* user = new RecommenderItem;
        user->m_id                      = i->first;
        user->m_name                    = i->second->m_name;
        user->m_featureSpaces["votes"]  = i->second->m_votes;

        users[i->first] = user;
    }

    return users;
}

RecommenderItems DataSet::recommenderFilms( void ) const
{
    RecommenderItems films;

    for( Items::const_iterator i = m_items.begin(), end = m_items.end(); i != end; ++i ) {
        RecommenderItem* film = new RecommenderItem;
        film->m_id                      = i->first;
        film->m_name                    = i->second->m_name;
        film->m_featureSpaces["votes"]  = i->second->m_votes;
        film->m_featureSpaces["genre"]  = i->second->m_genres;

        films[i->first] = film;
    }

    return films;
}

void DataSet::subtractUserAverage( void )
{
    printf( "Subtracting user average...\n" );
    
    for( Items::iterator i = m_items.begin(), end = m_items.end(); i != end; ++i ) {
        for( NumericFeatures::Features::iterator j = i->second->m_votes.features().begin(), jend = i->second->m_votes.features().end(); j != jend; ++j ) {
            j->second -= userById( j->first )->m_averageVote;
        }
    }
}

void DataSet::convertToBinaryRatings( void )
{
    printf( "Converting to binary ratings...\n" );

    for( Items::iterator i = m_items.begin(), end = m_items.end(); i != end; ++i ) {
        for( NumericFeatures::Features::iterator j = i->second->m_votes.features().begin(), jend = i->second->m_votes.features().end(); j != jend; ++j ) {
            j->second = j->second >= userById( j->first )->m_averageVote ? 1 : -1;
        }
    }
}

void DataSet::normalizeUserRatings( void )
{
    for( Users::iterator i = m_users.begin(), end = m_users.end(); i != end; ++i ) {
        i->second->m_votes.normalize();
    }
}

void DataSet::convertToBinaryRatings( UserVote threshold )
{
    for( Users::iterator i = m_users.begin(), end = m_users.end(); i != end; ++i ) {
        for( NumericFeatures::Features::iterator j = i->second->m_votes.features().begin(), jend = i->second->m_votes.features().end(); j != jend; ++j ) {
            j->second = j->second > threshold ? 1 : 0;
        }
    }
}

void DataSet::loadSimple( void )
{
    enum {
        Lady_in_the_Water = 1,
        Snakes_on_a_Plane,
        Just_My_Luck,
        Home_Alone,
        Home_Alone_2,
        Home_Alone_3
    };

    enum {
        Toby = 1,
        Lisa_Rose,
        Gene_Seymour,
        Michael_Phillips,
        Claudia_Puig,
        Mick_LaSalle,
        Jack_Matthews
    };

    addItem( Lady_in_the_Water, "Lady in the Water" );
    addItem( Snakes_on_a_Plane, "Snakes on a Plane" );
    addItem( Just_My_Luck, "Just My Luck" );
    addItem( Home_Alone, "Home Alone" );
    addItem( Home_Alone_2, "Home Alone 2" );
    addItem( Home_Alone_3, "Home Alone 3" );

    addUser( Toby, "Toby" );
    addVote( Toby, Snakes_on_a_Plane, 4.5 );
    addVote( Toby, Home_Alone, 5.0 );
    addVote( Toby, Home_Alone_2,  5.0 );

    addUser( Lisa_Rose, "Lisa Rose" );
    addVote( Lisa_Rose, Lady_in_the_Water, 2.5 );
    addVote( Lisa_Rose, Snakes_on_a_Plane, 3.5 );
    addVote( Lisa_Rose, Just_My_Luck, 3.0 );
    addVote( Lisa_Rose, Home_Alone, 5 );
    addVote( Lisa_Rose, Home_Alone_2, 5 );
    addVote( Lisa_Rose, Home_Alone_3, 1 );

    addUser( Gene_Seymour, "Gene Seymour" );
    addVote( Gene_Seymour, Lady_in_the_Water, 3.0 );
    addVote( Gene_Seymour, Snakes_on_a_Plane, 3.5 );
    addVote( Gene_Seymour, Just_My_Luck, 1.5 );
    addVote( Gene_Seymour, Home_Alone, 5.0 );
    addVote( Gene_Seymour, Home_Alone_2, 4.0 );
    addVote( Gene_Seymour, Home_Alone_3, 1 );

    addUser( Michael_Phillips, "Michael Phillips" );
    addVote( Michael_Phillips, Lady_in_the_Water, 2.5 );
    addVote( Michael_Phillips, Snakes_on_a_Plane, 3.0 );
    addVote( Michael_Phillips, Home_Alone_2, 4.5 );
    addVote( Michael_Phillips, Home_Alone, 5 );

    addUser( Claudia_Puig, "Claudia Puig" );
    addVote( Claudia_Puig, Snakes_on_a_Plane, 3.5 );
    addVote( Claudia_Puig, Just_My_Luck, 3.0 );
    addVote( Claudia_Puig, Home_Alone, 4.5 );
    addVote( Claudia_Puig, Home_Alone_2, 4.0 );
    addVote( Claudia_Puig, Home_Alone_3, 1 );

    addUser( Mick_LaSalle, "Mick LaSalle" );
    addVote( Mick_LaSalle, Lady_in_the_Water, 3.0 );
    addVote( Mick_LaSalle, Snakes_on_a_Plane, 4.0 );
    addVote( Mick_LaSalle, Just_My_Luck, 2.0 );
    addVote( Mick_LaSalle, Home_Alone, 3.0 );
    addVote( Mick_LaSalle, Home_Alone_2, 3.0 );
    addVote( Mick_LaSalle, Home_Alone_3, 1.0 );

    addUser( Jack_Matthews, "Jack Matthews" );
    addVote( Jack_Matthews, Lady_in_the_Water, 3.0 );
    addVote( Jack_Matthews, Snakes_on_a_Plane, 4.0 );
    addVote( Jack_Matthews, Home_Alone, 5.0 );
    addVote( Jack_Matthews, Home_Alone_3, 3.0 );
    addVote( Jack_Matthews, Home_Alone_2, 3.5 );
}

void DataSet::loadItems( const char* fileName )
{
    enum { kItemId, kTitle, kGenre };

    csv_parser parser;

    parser.init( fileName );

    while( parser.has_more_rows() ) {
        csv_row row = parser.get_row();

        ItemId      itemId = atoi( row[kItemId].c_str() );
        std::string title  = row[kTitle];

        m_items[itemId]                 = new Item;
        m_items[itemId]->m_itemId       = itemId;
        m_items[itemId]->m_name         = title;
        m_items[itemId]->m_genreString  = row[kGenre];
        m_items[itemId]->m_genres       = parseGenres( row[kGenre] );
    }
}

NumericFeatures DataSet::genreFeatures( const GenreArray& genres ) const
{
    NumericFeatures features;

    float score = 1.0f;
    for( int i = 0; i < ( int )genres.size(); i++ ) {
        features.set( genres[i], score );
        score *= 0.25f;
    }

    return features;
}

Genres DataSet::genres( void ) const
{
    Genres result;

    for( Items::const_iterator i = m_items.begin(), end = m_items.end(); i != end; ++i ) {
        const std::string& genreString = i->second->m_genreString;

        if( result.find( genreString ) != result.end() ) {
            continue;
        }

        result[i->second->m_genreString] = i->second->m_genres;
    }

    return result;
}

void DataSet::save( Films& films, bool saveVotes ) const
{
    using namespace mongo;

    printf( "Saving users...\n" );

    ConnectionPtr connection = ConnectionPtr( new Connection( "mongodb://localhost:27017/", "movielens" ) );
    CollectionPtr users      = connection->collection( "users" );
    CollectionPtr items      = connection->collection( "items" );
    CollectionPtr votes      = connection->collection( "votes" );

    CursorPtr nulled = items->find( QUERY( "name" << NIL ) );
    DocumentPtr document;

    while( (document = nulled->next()) ) {
        std::string name = m_items.find( document->integer( "itemId" ) )->second->m_name;
        items->update( QUERY( "_id" << document->_id() ), DOCUMENT( "$set" << DOCUMENT( "name" << DOCUMENT( "en" << name ) ) ) );
    }
/*
    // ** Save users
    for( Users::const_iterator i = m_users.begin(), end = m_users.end(); i != end; ++i ) {
        users->insert( DOCUMENT( "userId" << i->first ) );
    }

    if( saveVotes ) {
        printf( "Saving votes...\n" );

        // ** Save votes
        for( Users::const_iterator i = m_users.begin(), end = m_users.end(); i != end; ++i ) {
            for( NumericFeatures::Features::const_iterator j = i->second->m_votes.features().begin(), jend = i->second->m_votes.features().end(); j != jend; ++j ) {
                votes->insert( DOCUMENT( "itemId" << j->first << "userId" << i->first << "rating" << j->second ) );
            }
        }
    }

    printf( "Saving items...\n" );

    // ** Save items
    int counter = 0;
    int total   = ( int )m_items.size();

    for( Items::const_iterator i = m_items.begin(), end = m_items.end(); i != end; ++i ) {
        std::vector<int> genres;
        for( NumericFeatures::Features::const_iterator j = i->second->m_genres.features().begin(), jend = i->second->m_genres.features().end(); j != jend; ++j ) {
            genres.push_back( j->first );
        }

        items->insert( DOCUMENT( "itemId" << i->first << "name" << i->second->m_name << "genres" << genres ) );

        if( (++counter % 10) == 0 ) printf( "Adding films %d%%...\n", int( float( counter ) / total * 100 ) );
    }
*/
    printf( "Dataset is saved to a db!\n" );
}

NumericFeatures DataSet::parseGenres( const std::string& input ) const
{
    GenreArray genreArray;

    char* str   = strdup( input.c_str() );
    char* token =  strtok( str, "|" );

    std::map<std::string, Genre> stringToGenre;
    stringToGenre["Action"] = Action;
    stringToGenre["Adventure"] = Adventure;
    stringToGenre["Animation"] = Animation;
    stringToGenre["Children's"] = Childrens;
    stringToGenre["Children"] = Childrens;
    stringToGenre["Comedy"] = Comedy;
    stringToGenre["Crime"] = Crime;
    stringToGenre["Documentary"] = Documentary;
    stringToGenre["Drama"] = Drama;
    stringToGenre["Fantasy"] = Fantasy;
    stringToGenre["Film-Noir"] = FilmNoir;
    stringToGenre["Horror"] = Horror;
    stringToGenre["Musical"] = Musical;
    stringToGenre["Mystery"] = Mystery;
    stringToGenre["Romance"] = Romance;
    stringToGenre["Sci-Fi"] = SciFi;
    stringToGenre["Thriller"] = Thriller;
    stringToGenre["War"] = War;
    stringToGenre["Western"] = Western;
    stringToGenre["IMAX"] = IMAX;
    stringToGenre["(no genres listed)"] = Unknown;

    while( token != NULL ) {
        assert( stringToGenre.find( token ) != stringToGenre.end() );
        genreArray.push_back( stringToGenre[token] );
        token = strtok( NULL, "|" );
    }

    free( str );

    return genreFeatures( genreArray );
}

NumericFeatures DataSet::globalGenrePreferences( void ) const
{
    NumericFeatures preferences;
    float           max = -FLT_MAX;

    for( Users::const_iterator i = m_users.begin(), end = m_users.end(); i != end; ++i ) {
        for( NumericFeatures::Features::const_iterator j = i->second->m_votes.features().begin(), jend = i->second->m_votes.features().end(); j != jend; ++j ) {
            preferences += itemById( j->first )->m_genres;
        }
    }

    for( NumericFeatures::Features::const_iterator i = preferences.features().begin(), end = preferences.features().end(); i != end; i++ ) {
        max = std::max( max, i->second );
    }

    preferences /= max;

    return preferences;
}

NumericFeatures DataSet::userGenrePreferences( int userId ) const
{
    const User* user = userById( userId );

    NumericFeatures preferences;
    float           max = -FLT_MAX;

    for( NumericFeatures::Features::const_iterator i = user->m_votes.features().begin(), end = user->m_votes.features().end(); i != end; ++i ) {
        preferences += itemById( i->first )->m_genres;
    }

    for( NumericFeatures::Features::const_iterator i = preferences.features().begin(), end = preferences.features().end(); i != end; i++ ) {
        max = std::max( max, i->second );
    }

    preferences /= max;

    return preferences;
}

void DataSet::dumpGenres( const char *fileName, int userA, int userB ) const
{
    const User* a = userById( userA );
    const User* b = userById( userB );

    NumericFeatures::Intersection votes  = userGenrePreferences( userA ) | userGenrePreferences( userB );
    NumericFeatures               global = globalGenrePreferences();

    FILE* fp = fopen( fileName, "wt" );

    // ** Header
    fprintf( fp, "User, %s, %s, Global\n", a->m_name.c_str(), b->m_name.c_str() );

    for( NumericFeatures::Intersection::const_iterator i = votes.begin(), end = votes.end(); i != end; ++i ) {
        fprintf( fp, "%d, %2.6f, %2.6f, %2.6f\n", i->first, i->second.a * 100, i->second.b * 100, global.get( i->first ) * 100 );
    }

    fclose( fp );
}

void DataSet::dumpVotesLineChart( const char* fileName, int itemA, int itemB ) const
{
    const Item* a = itemById( itemA );
    const Item* b = itemById( itemB );

    NumericFeatures::Intersection votes = a->m_votes % b->m_votes;

    FILE* fp = fopen( fileName, "wt" );

    // ** Header
    fprintf( fp, "User, %s, %s\n", a->m_name.c_str(), b->m_name.c_str() );

    for( NumericFeatures::Intersection::const_iterator i = votes.begin(), end = votes.end(); i != end; ++i ) {
        fprintf( fp, "%d, %2.2f, %2.2f\n", i->first, i->second.a, i->second.b );
    }

    fclose( fp );
}

void DataSet::dumpSharedVotes( const char* fileName, int itemA, int itemB ) const
{
    const Item* a = itemById( itemA );
    const Item* b = itemById( itemB );

    NumericFeatures::Intersection shared = a->m_votes % b->m_votes;

    FILE* fp = fopen( fileName, "wt" );

    // ** Header
    fprintf( fp, "Item" );
    for( NumericFeatures::Intersection::const_iterator i = shared.begin(), end = shared.end(); i != end; ++i ) {
        fprintf( fp, ", %d", i->first );
    }
    fprintf( fp, "\n" );

    // ** Votes for a
    fprintf( fp, "%s", a->m_name.c_str() );
    for( NumericFeatures::Intersection::const_iterator i = shared.begin(), end = shared.end(); i != end; ++i ) {
        fprintf( fp, ", %2.2f", i->second.a );
    }
    fprintf( fp, "\n" );

    // ** Votes for b
    fprintf( fp, "%s", b->m_name.c_str() );
    for( NumericFeatures::Intersection::const_iterator i = shared.begin(), end = shared.end(); i != end; ++i ) {
        fprintf( fp, ", %2.2f", i->second.b );
    }
    fprintf( fp, "\n" );

    // ** Average vote
    fprintf( fp, "Average user vote" );
    for( NumericFeatures::Intersection::const_iterator i = shared.begin(), end = shared.end(); i != end; ++i ) {
        fprintf( fp, ", %2.2f", userById( i->first )->m_votes.average() );
    }
    fprintf( fp, "\n" );

    // ** Average vote
    fprintf( fp, "Total user votes" );
    for( NumericFeatures::Intersection::const_iterator i = shared.begin(), end = shared.end(); i != end; ++i ) {
        fprintf( fp, ", %d", userById( i->first )->m_votes.size() );
    }
    fprintf( fp, "\n" );

    fclose( fp );
}

float DataSet::averageVotesPerItem( void ) const
{
    float sum = 0.0f;

    for( Items::const_iterator i = m_items.begin(), end = m_items.end(); i != end; ++i ) {
        sum += i->second->m_votes.size();
    }

    return sum / m_items.size();
}

void DataSet::loadRatings( const char* fileName, int votesThreshold )
{
    enum { kUserId, kItemId, kRating, kTimestamp };

    csv_parser parser;

    parser.init( fileName );

    while( parser.has_more_rows() ) {
        csv_row row = parser.get_row();

        UserId   userId = atoi( row[kUserId].c_str() );
        ItemId   itemId = atoi( row[kItemId].c_str() );
        UserVote rating = atof( row[kRating].c_str() );

        assert( rating > 0 && rating <= 5 );

        itemById( itemId );

        if( m_users.count( userId ) == 0 ) {
            char name[64];
            sprintf( name, "User%d", userId );

            m_users[userId] = new User;
            m_users[userId]->m_userId = userId;
            m_users[userId]->m_name   = name;
        }

        addVote( userId, itemId, rating );
    }

    for( Users::iterator i = m_users.begin(), end = m_users.end(); i != end;  ) {
        if( i->second->m_votes.size() < votesThreshold ) {
            i = m_users.erase( i );
        } else {
            i->second->m_averageVote = i->second->m_votes.average();
            ++i;
        }
    }
}

void DataSet::serialize( const char* fileName )
{
    FILE* fp = fopen( fileName, "wb" );

    for( Users::const_iterator i = m_users.begin(), end = m_users.end(); i != end; ++i ) {

    }

    fclose( fp );
}

const Item* DataSet::itemById( ItemId id ) const
{
    Items::const_iterator i = m_items.find( id );
    assert( i != m_items.end() );
    assert( i->second->m_itemId == id );
    assert( i->second->m_itemId == i->first );

    return i->second;
}

const User* DataSet::userById( UserId id ) const
{
    Users::const_iterator i = m_users.find( id );
    assert( i != m_users.end() );
    assert( i->second->m_userId == id );
    assert( i->second->m_userId == i->first );

    return i->second;
}