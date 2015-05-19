//
//  DataSet.h
//  DataMining
//
//  Created by Советов Дмитрий on 18.12.14.
//  Copyright (c) 2014 data. All rights reserved.
//

#ifndef __DataMining__DataSet__
#define __DataMining__DataSet__

#include "Common.h"
#include "statistics/Samples.h"
#include "Cluster.h"
#include "recommendation/Recommender.h"

#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <float.h>
#include <map>
#include <vector>
#include <string>
#include <set>

#define USE_1m  (0)
#define USE_10m (1)

struct User;
struct Item;

class Films;

enum Genre {
    Unknown     = 0,
    Action      = 1,
    Adventure   = 2,
    Animation   = 3,
    Childrens   = 4,
    Comedy      = 5,
    Crime       = 6,
    Documentary = 7,
    Drama       = 8,
    Fantasy     = 9,
    FilmNoir    = 10,
    Horror      = 11,
    Musical     = 12,
    Mystery     = 13,
    Romance     = 14,
    SciFi       = 15,
    Thriller    = 16,
    War         = 17,
    Western     = 18,
    IMAX        = 19,
};

typedef float                               UserVote;
typedef int                                 UserId;
typedef int                                 ItemId;
typedef std::vector<const User*>            UserArray;
typedef std::map<ItemId, Item*>             Items;
typedef std::map<UserId, User*>             Users;
typedef std::vector<Genre>                  GenreArray;

typedef std::map<std::string, NumericFeatures>    Genres;

struct User : public ClusterItem {
                    User() : ClusterItem( &m_votes ) {}
    
    UserId          m_userId;
    NumericFeatures m_votes;
    float           m_averageVote;
    std::string     m_name;
};

struct Item : public ClusterItem {
                    Item( void ) : ClusterItem( NULL ) {}
    ItemId          m_itemId;
    NumericFeatures m_votes;
    NumericFeatures m_genres;
    std::string     m_name;
    std::string     m_genreString;
};

class DataSet {
public:

    enum eFilms {
    #if USE_1m
        Braveheart          = 110,
        ShawshankRedemption = 318,
        StarWars            = 260,
        JurassicPark        = 480,
        ForrestGump         = 356,
        HomeAlone           = 586,
        HomeAlone2          = 2953,
        HomeAlone3          = 1707,
        DieHard             = 1036,
    #elif USE_10m
        RentAKid            = 310,
        MiracleOn34thStreet = 277,
        Scrooge             = 8492,
        Braveheart          = 110,
        ShawshankRedemption = 318,
        StarWars            = 260,
        JurassicPark        = 480,
        ForrestGump         = 356,
        HomeAlone           = 586,
        SomeLikeItHot       = 910,
        HomeAlone2          = 2953,
        HomeAlone3          = 1707,
        DieHard             = 1036,
        HudsonHawk          = 7000,
    #else
        Braveheart          = 22,
        ShawshankRedemption = 64,
        StarWars            = 50,
        JurassicPark        = 82,
        ForrestGump         = 69,
    #endif
    };

    void                save( Films& films, bool saveVotes ) const;

    Item*               addItem( int itemId, const char* title );
    User*               addUser( int userId, const char* name );
    void                addVote( int userId, int itemId, UserVote value );

    void                loadSimple( void );
    void                loadRatings( const char* fileName, int votesThreshold = 1 );
    void                loadItems( const char* fileName );

    void                serialize( const char* fileName );
    void                deserialize( const char* fileName );

    void                dumpVotesLineChart( const char* fileName, int itemA, int itemB ) const;
    void                dumpGenres( const char* fileName, int userA, int userB ) const;
    void                dumpSharedVotes( const char* fileName, int itemA, int itemB ) const;
    float               averageVotesPerItem( void ) const;

    NumericFeatures     userGenrePreferences( int userId ) const;
    NumericFeatures     globalGenrePreferences( void ) const;

    void                normalizeUserRatings( void );
    void                subtractUserAverage( void );
    void                convertToBinaryRatings( void );
    void                convertToBinaryRatings( UserVote threshold );

    RecommenderItems    recommenderUsers( void ) const;
    RecommenderItems    recommenderFilms( void ) const;
    Genres              genres( void ) const;

//    ClusterItems        userList( void ) const;
//    ClusterItems        itemList( void ) const;
    Users&              users( void ) { return m_users; }
    const Users&        users( void ) const { return m_users; }
    Items&              items( void ) { return m_items; }
    const User*         userById( UserId userId ) const;
    const Item*         itemById( ItemId itemId ) const;

    NumericFeatures     parseGenres( const std::string& str ) const;
    NumericFeatures     genreFeatures( const GenreArray& genres ) const;

private:

    Users               m_users;
    Items               m_items;
};

#endif /* defined(__DataMining__DataSet__) */
