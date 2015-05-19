/****************************************************************************
 Copyright (c) 2010-2012 cocos2d-x.org
 Copyright (c) 2008-2010 Ricardo Quesada
 Copyright (c) 2011      Zynga Inc.

 http://www.cocos2d-x.org

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/
//

#ifndef __DataMining__Films__
#define __DataMining__Films__

#include "Common.h"
#include "Mongo.h"
#include "FeatureSpace.h"
#include "statistics/Samples.h"
#include "recommendation/Recommender.h"

class Films;

// ** Quality
enum Quality {
    Bad,
    Medium,
    Good
};

// ** struct Film
struct Film {
    mongo::OID          m_id;
    std::string         m_name;
    mongo::IntegerSet   m_genres;
    int                 m_year;
    std::string         m_video;

                        Film( void )
                            : m_id( "" ), m_name( "" ), m_year( 0 ) {}
                        Film( const mongo::OID& oid, const std::string& name, const mongo::IntegerSet& genres, int year )
                            : m_id( oid ), m_name( name ), m_genres( genres ), m_year( year ) {}
};

// ** struct SimilarFilm
struct SimilarFilm {
    Film            m_film;
    float           m_similarity;
    float           m_shared;
    Quality         m_quality;

                    SimilarFilm( void )
                        : m_similarity( 0.0f ), m_shared( 0.0f ), m_quality( Bad ) {}
                    SimilarFilm( const Film& film, float similarity, float shared, Quality quality )
                        : m_film( film ), m_similarity( similarity ), m_shared( shared ), m_quality( quality ) {}

    static bool     sortBySimilarity( const SimilarFilm& a, const SimilarFilm& b ) { return a.m_similarity > b.m_similarity; }
};

typedef std::vector<Film>           FilmsArray;
typedef std::vector<SimilarFilm>    SimilarFilmsArray;

// ** class StreamedFilms
class StreamedFilms : public IRecommenderItems {
public:

                                        StreamedFilms( const Films& films, const mongo::ConnectionPtr& connection );

    // ** IRecommenderItems
    virtual bool                        findById( int itemId, RecommenderItem& item ) const;
    virtual IRecommenderItems*          clone( void );
    virtual bool                        first( void );
    virtual bool                        startFrom( const RecommenderItem& item );
    virtual bool                        next( RecommenderItem& item );
    virtual int                         size( void );

private:

    void                                documentToItem( RecommenderItem& item, const mongo::DocumentPtr& document ) const;

private:

    const Films&                        m_films;
    mongo::ConnectionPtr                m_connection;
    mongo::CursorPtr                    m_cursor;
};

// ** class PreloadedFilms
class PreloadedFilms : public IRecommenderItems {
friend class Films;
public:

                                        PreloadedFilms( const RecommenderItems& items );
                                        PreloadedFilms( const Films& films, const mongo::CursorPtr& cursor );

    // ** IRecommenderItems
    virtual bool                        findById( int itemId, RecommenderItem& item ) const;
    virtual IRecommenderItems*          clone( void );
    virtual bool                        first( void );
    virtual bool                        startFrom( const RecommenderItem& item );
    virtual bool                        next( RecommenderItem& item );
    virtual int                         size( void );

private:

    RecommenderItems::const_iterator    m_iterator;
    RecommenderItems                    m_items;
};

// ** class Films
class Films : public IRecommenderData {
friend class PreloadedFilms;
friend class StreamedFilms;
public:

    // ** IRecommenderData
    virtual IRecommenderItems*          items( void ) const;
    virtual RecommenderItem             findById( int itemId ) const;

    // ** Films
    bool                                connect( const std::string& host, const std::string& source, const std::string& target );
    void                                processFilms( float sharedThreshold );

    void                                updateVotesCount( void );
    void                                updateSharedAndSimilarityRanges( void );
    void                                showStats( void ) const;

    Film                                filmById( const mongo::OID& oid ) const;
    FilmsArray                          find( const std::string& name ) const;
    SimilarFilmsArray                   similarTo( const mongo::StringSet& oids, int count = 0 ) const;
    SimilarFilmsArray                   similarTo( const std::string& oid, int count = 0 ) const;

    mongo::OID                          filmIdToObjectId( int filmId ) const;
    int                                 objectIdToFilmId( const mongo::OID& oid ) const;

    static std::string                  formatGenres( const mongo::IntegerSet& genres );
    static std::string                  qualityToString( Quality quality );
    static Quality                      qualityFromRange( float value, const mongo::FloatArray& quartiles );

public:

    NumericFeatures                     votesForFilm( int filmId ) const;
    Film                                filmFromDocument( const mongo::DocumentPtr& document ) const;

    static unsigned int                 encodeSimilarity( float similarity, float accuracy );
    static void                         decodeSimilarity( unsigned int encoded, float& similarity, float& accuracy );

private:

    mongo::ConnectionPtr                m_source;
    mongo::ConnectionPtr                m_target;
    IRecommenderItems*                  m_items;

    mongo::FloatArray                   m_similarityQuartiles;
    mongo::FloatArray                   m_sharedQuartiles;
};

#endif /* defined(__DataMining__Films__) */
