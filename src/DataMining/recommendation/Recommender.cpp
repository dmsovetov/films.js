//
//  Recomendation.cpp
//  DataMining
//
//  Created by Советов Дмитрий on 18.12.14.
//  Copyright (c) 2014 data. All rights reserved.
//

#include "Recommender.h"

// ------------------------------------------------------- Recommender ------------------------------------------------------- //

// ** Recommender::Recommender
Recommender::Recommender( const IRecommenderData* data, Similarity* similarity, Similarity* genreRelevance )
    : m_data( data ), m_items( data->items() ), m_similarity( similarity ), m_genreRelevance( genreRelevance )
{
    if( !m_genreRelevance ) m_genreRelevance = new Similarity;
}

// ** Recommender::findRelevantItems
RelevantItems Recommender::findRelevantItems( const RecommenderItem* item, int count ) const
{
    RelevantItems result;
    SimilarItems  similarItems = findSimilarItems( NULL, item );

    for( SimilarItems::const_iterator i = similarItems.begin(), end = similarItems.end(); i != end; ++i ) {
        RecommenderItem    similarItem  = m_data->findById( i->m_item );

        RelevantItem relevant;
        relevant.m_item         = i->m_item;
        relevant.m_similarity   = i->m_similarity;
        relevant.m_relevance    = m_genreRelevance ? m_genreRelevance->calculate( "genre", item, &similarItem ) : 1.0f;
        relevant.m_score        = powf( relevant.m_relevance, 0.3f ) * relevant.m_similarity;

        if( relevant.m_relevance > 0.0f ) {
            result.push_back( relevant );
        }
    }

    // ** Sort & shrink items
    std::sort( result.begin(), result.end(), RelevantItem::sortByScore );

    if( count ) {
        result.resize( std::min( count, ( int )result.size() ) );
    }

    return result;
}

// ** Recommender::findSimilarItems
SimilarItems Recommender::findSimilarItems( IRecommenderItems* items, const RecommenderItem* item, int count ) const
{
    RecommenderItem i;
    SimilarItems    similarItems;
//    float           maxSimilarity = -FLT_MAX;

    if( !items ) {
        items = m_items;
        items->first();
    }

    // ** Build similarity list
    while( items->next( i ) ) {
        if( i.m_id == item->m_id ) {
            continue;
        }

        SimilarItem similar;
        similar.m_item       = i.m_id;
        similar.m_shared     = float(item->features( "votes" ) ^ i.features( "votes" )) / (item->features( "votes" ).size() + i.features( "votes" ).size());
        similar.m_similarity = m_similarity->calculate( "votes", item, &i );

        if( similar.m_similarity > 0.0f ) {
        //    maxSimilarity = std::max( maxSimilarity, similar.m_similarity );
            similarItems.push_back( similar );
        }
    }

    // ** Normalize correlation
//    for( SimilarItems::iterator i = similarItems.begin(), end = similarItems.end(); i != end; ++i ) {
//        i->m_similarity /= maxSimilarity;
//    }

    // ** Sort & shrink similar items
    similarItems.sort( SimilarItem::compare );

    if( count ) {
        similarItems.resize( std::min( count, ( int )similarItems.size() ) );
    }
    
    return similarItems;
}

// ** Recommender::recommend
Recommendations Recommender::recommend( const RecommenderItem* item, int n ) const
{
    NumericFeatures scores = predictScores( item, false );

    Recommendations result;

    for( NumericFeatures::Features::const_iterator i = scores.features().begin(), end = scores.features().end(); i != end; ++i ) {
        result.push_back( Recommendation( i->first, i->second ) );
    }

    std::sort( result.begin(), result.end(), Recommendation::compare );
    result.resize( std::min( ( int )result.size(), n ) );

    return result;
}

// ** Recommender::predictScores
NumericFeatures Recommender::predictScores( const RecommenderItem* item, bool rmseEvaluation ) const
{
    SimilarItems    similar = findSimilarItems( NULL, item );
    NumericFeatures votes;
    NumericFeatures totals;

    for( SimilarItems::const_iterator i = similar.begin(), end = similar.end(); i != end; ++i ) {
        float                            similarity      = i->m_similarity;
        RecommenderItem                  similarItem     = m_data->findById( i->m_item );
        const NumericFeatures::Features& itemFeatures    = item->features( "votes" ).features();
        const NumericFeatures::Features& similarFeatures = similarItem.features( "votes" ).features();

        for( NumericFeatures::Features::const_iterator j = similarFeatures.begin(), end = similarFeatures.end(); j != end; ++j ) {
            bool hasVotedForItem = itemFeatures.count( j->first ) != 0;

            if(  hasVotedForItem && rmseEvaluation == false ) continue;
            if( !hasVotedForItem && rmseEvaluation == true  ) continue;

            votes  += NumericFeatures::Feature( j->first, j->second * similarity );
            totals += NumericFeatures::Feature( j->first, similarity );
        }
    }

    Recommendations result;

    // ** Normalize
    for( NumericFeatures::Features::iterator i = votes.features().begin(), end = votes.features().end(); i != end; ++i ) {
        i->second /= totals.get( i->first );
    }
    
    return votes;
}

// ---------------------------------------------------- ItemBasedRecommender ------------------------------------------------------- //

// ** ItemBasedRecommender::ItemBasedRecommender
ItemBasedRecommender::ItemBasedRecommender( const IRecommenderData* data, Similarity* similarity, Similarity* genreRelevance ) : Recommender( data, similarity, genreRelevance )
{

}

// ** ItemBasedRecommender::recommend
Recommendations ItemBasedRecommender::recommend( const PreferenceItems& preferences, int count ) const
{
    NumericFeatures prediction;
    NumericFeatures totals;
    NumericFeatures counter;
    Recommendations result;

    for( int i = 0, n = ( int )preferences.size(); i < n; i++ ) {
        const PreferenceItem& preference   = preferences[i];
        SimilarItems          similarItems = findSimilarItems( NULL, preference.m_item );

        for( SimilarItems::const_iterator j = similarItems.begin(), end = similarItems.end(); j != end; ++j ) {
            int                 itemId  = j->m_item;

            prediction  += NumericFeatures::Feature( itemId, j->m_similarity * preference.m_score );
            totals      += NumericFeatures::Feature( itemId, j->m_similarity );
            counter     += NumericFeatures::Feature( itemId, 1 );
        }
    }

    for( NumericFeatures::Features::const_iterator i = prediction.features().begin(), end = prediction.features().end(); i != end; ++i ) {
        result.push_back( Recommendation( i->first, i->second / totals.get( i->first ), totals.get( i->first ), counter.get( i->first ) ) );
    }

    std::sort( result.begin(), result.end(), Recommendation::sortByTotal );

    if( count ) {
        result.resize( std::min( count, ( int )result.size() ) );
    }

    return result;
}