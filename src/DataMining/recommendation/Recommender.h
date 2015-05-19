//
//  Recomendation.h
//  DataMining
//
//  Created by Советов Дмитрий on 18.12.14.
//  Copyright (c) 2014 data. All rights reserved.
//

#ifndef __DataMining__Recomendation__
#define __DataMining__Recomendation__

#include "Similarity.h"

struct RecommenderItem;
typedef std::list<struct SimilarItem>           SimilarItems;
typedef std::map<int, struct RecommenderItem*>  RecommenderItems;
typedef std::vector<struct PreferenceItem>      PreferenceItems;
typedef std::vector<struct Recommendation>      Recommendations;
typedef std::vector<struct RelevantItem>        RelevantItems;

// ** class IRecommenderItems
class IRecommenderItems {
public:

    virtual                         ~IRecommenderItems( void ) {}

    virtual bool                    findById( int itemId, RecommenderItem& item ) const     = 0;
    virtual IRecommenderItems*      clone( void )                                           = 0;
    virtual bool                    first( void )                                           = 0;
    virtual bool                    startFrom( const RecommenderItem& item )                = 0;
    virtual bool                    next( RecommenderItem& item )                           = 0;
    virtual int                     size( void )                                            = 0;
};

// ** IRecommenderData
class IRecommenderData {
public:

    virtual                         ~IRecommenderData( void ) {}

    virtual IRecommenderItems*      items( void ) const             = 0;
    virtual RecommenderItem         findById( int itemId ) const    = 0;
};

// ** struct SimilarItem
struct SimilarItem {
//    RecommenderItem*        m_item;
    int                     m_item;
    float                   m_similarity;
    float                   m_shared;

    static bool             compare( const SimilarItem& a, const SimilarItem& b ) { return a.m_similarity > b.m_similarity; }
};

// ** struct RelevantItem
struct RelevantItem {
//    RecommenderItem*        m_item;
    int                     m_item;
    float                   m_similarity;
    float                   m_relevance;
    float                   m_score;

    static bool             sortByScore( const RelevantItem& a, const RelevantItem& b ) { return fabs(a.m_score - b.m_score) > 0.0001f ? a.m_score > b.m_score : sortByRelevance( a, b ); }
    static bool             sortByRelevance( const RelevantItem& a, const RelevantItem& b ) { return fabs(a.m_relevance - b.m_relevance) > 0.0001f ? a.m_relevance > b.m_relevance : sortBySimilarity( a, b ); }
    static bool             sortBySimilarity( const RelevantItem& a, const RelevantItem& b ) { return a.m_similarity > b.m_similarity; }
};

// ** struct RecommenderItem
struct RecommenderItem {
    int                             m_id;
    std::string                     m_name;
    RecommenderItemFeatures         m_featureSpaces;
    SimilarItems                    m_similar;
    void*                           m_userData;

    virtual const NumericFeatures&  features( const std::string& name ) const { assert( m_featureSpaces.find( name ) != m_featureSpaces.end() ); return m_featureSpaces.find( name )->second; }
};

// ** struct PreferenceItem
struct PreferenceItem {
    const RecommenderItem*  m_item;
    float                   m_score;

                            PreferenceItem( const RecommenderItem* item = NULL, float score = 0.0f )
                                : m_item( item ), m_score( score ) {}
};

// ** struct Recommendation
struct Recommendation {
    int                     m_id;
    float                   m_score;
    float                   m_total;
    int                     m_counter;

                            Recommendation( void ) {}
                            Recommendation( int id, float score, float total = 0.0f, int counter = 0 ) : m_id( id ), m_score( score ), m_total( total ), m_counter( counter ) {}

    static bool             compare( const Recommendation& a, const Recommendation& b ) { return a.m_score != b.m_score ? a.m_score > b.m_score : a.m_total > b.m_total; }
    static bool             sortByTotal( const Recommendation& a, const Recommendation& b ) { return a.m_total > b.m_total; }
};

// ** Recommender
class Recommender {
public:

                                Recommender( const IRecommenderData* data, Similarity* similarity, Similarity* genreRelevance = NULL );

    SimilarItems                findSimilarItems( IRecommenderItems* items, const RecommenderItem* item, int count = 0 ) const;
    RelevantItems               findRelevantItems( const RecommenderItem* item, int count = 0 ) const;
    Recommendations             recommend( const RecommenderItem* item, int count ) const;

private:

    NumericFeatures             predictScores( const RecommenderItem* item, bool rmseEvaluation = false ) const;

private:

    const IRecommenderData*     m_data;
    IRecommenderItems*          m_items;
    Similarity*                 m_similarity;
    Similarity*                 m_genreRelevance;
};

// ** class ItemBasedRecommender
class ItemBasedRecommender : public Recommender {
public:

                                ItemBasedRecommender( const IRecommenderData* data, Similarity* similarity, Similarity* genreRelevance = NULL );

    Recommendations             recommend( const PreferenceItems& preferences, int count ) const;
};

#endif /* defined(__DataMining__Recomendation__) */
