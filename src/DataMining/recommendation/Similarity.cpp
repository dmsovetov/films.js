//
//  Similarity.cpp
//  DataMining
//
//  Created by Советов Дмитрий on 26.12.14.
//  Copyright (c) 2014 data. All rights reserved.
//

#include "Similarity.h"
#include "Recommender.h"

// -------------------------------------------------------- Similarity -------------------------------------------------------- //

// ** Similarity::Similarity
Similarity::Similarity( void )
{

}

// ** Similarity::calculate
float Similarity::calculate( const std::string& space, const RecommenderItem* a, const RecommenderItem* b )
{
    return 1.0f;
}

// ------------------------------------------------------ FeatureSimilarity ----------------------------------------------------- //

// ** FeatureSimilarity::FeatureSimilarity
FeatureSimilarity::FeatureSimilarity( NumericFeaturesSimilarity function ) : m_function( function )
{

}

// ** FeatureSimilarity::calculate
float FeatureSimilarity::calculate( const std::string& space, const RecommenderItem* a, const RecommenderItem* b )
{
    return m_function( a->features( space ), b->features( space ), NULL );
}

// ----------------------------------------------------- AccuracySimilarity ----------------------------------------------------- //

// ** AccuracySimilarity::AccuracySimilarity
AccuracySimilarity::AccuracySimilarity( NumericFeaturesSimilarity function, float threshold, float influence, float accuracyThreshold )
    : FeatureSimilarity( function ), m_threshold( threshold ), m_influence( influence ), m_accuracyThreshold( accuracyThreshold )
{
    
}

// ** AccuracySimilarity::calculate
float AccuracySimilarity::calculate( const std::string& space, const RecommenderItem* a, const RecommenderItem* b )
{
    const NumericFeatures& featuresA = a->features( space );
    const NumericFeatures& featuresB = b->features( space );

    int   shared     = 0;
    float similarity = m_function( featuresA, featuresB, &shared );
    float acc        = accuracy( shared, featuresA, featuresB );

    if( acc < m_accuracyThreshold ) {
        return 0.0f;
    }

    similarity = similarity * powf( acc, m_influence );

    return similarity >= m_threshold ? similarity : 0.0f;
}

// ** AccuracySimilarity::accuracy
float AccuracySimilarity::accuracy( int shared, const NumericFeatures& a, const NumericFeatures& b ) const
{
    return 1.0f;
}

// -------------------------------------------------- JaccardAccuracySimilarity -------------------------------------------------- //

// ** JaccardAccuracySimilarity::JaccardAccuracySimilarity
JaccardAccuracySimilarity::JaccardAccuracySimilarity( NumericFeaturesSimilarity function, float threshold, float influence, float accuracyThreshold ) : AccuracySimilarity( function, threshold, influence, accuracyThreshold )
{

}

// ** JaccardAccuracySimilarity::accuracy
float JaccardAccuracySimilarity::accuracy( int shared, const NumericFeatures& a, const NumericFeatures& b ) const
{
    return float( shared ) / (a.size() + b.size() - shared);
}