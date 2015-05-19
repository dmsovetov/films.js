//
//  Similarity.h
//  DataMining
//
//  Created by Советов Дмитрий on 26.12.14.
//  Copyright (c) 2014 data. All rights reserved.
//

#ifndef __DataMining__Similarity__
#define __DataMining__Similarity__

#include "../FeatureSpace.h"

struct RecommenderItem;

typedef std::map<std::string, NumericFeatures>  RecommenderItemFeatures;
typedef std::shared_ptr<class Similarity>       SimilarityPtr;

// ** class Similarity
class Similarity {
public:

                                Similarity( void );
    virtual                     ~Similarity( void ) {}

    virtual float               calculate( const std::string& space, const RecommenderItem* a, const RecommenderItem* b );
};

// ** class FeatureSimilarity
class FeatureSimilarity : public Similarity {
public:

                                FeatureSimilarity( NumericFeaturesSimilarity function );

    virtual float               calculate( const std::string& space, const RecommenderItem* a, const RecommenderItem* b );

protected:

    NumericFeaturesSimilarity   m_function;
};

// ** class AccuracySimilarity
class AccuracySimilarity : public FeatureSimilarity {
public:

                                AccuracySimilarity( NumericFeaturesSimilarity function, float threshold, float influence, float accuracyThreshold = 0.0f );

    // ** Similarity
    virtual float               calculate( const std::string& space, const RecommenderItem* a, const RecommenderItem* b );

protected:

    // ** AccuracySimilarity
    virtual float               accuracy( int shared, const NumericFeatures& a, const NumericFeatures& b ) const;

protected:

    float                       m_threshold;
    float                       m_influence;
    float                       m_accuracyThreshold;
};

// ** class JaccardAccuracySimilarity
class JaccardAccuracySimilarity : public AccuracySimilarity {
public:

                                JaccardAccuracySimilarity( NumericFeaturesSimilarity function, float threshold, float influence, float accuracyThreshold );

protected:

    // ** AccuracySimilarity
    virtual float               accuracy( int shared, const NumericFeatures& a, const NumericFeatures& b ) const;
};

#endif /* defined(__DataMining__Similarity__) */
