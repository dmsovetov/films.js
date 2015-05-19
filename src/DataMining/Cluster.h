//
//  Cluster.h
//  DataMining
//
//  Created by Советов Дмитрий on 18.12.14.
//  Copyright (c) 2014 data. All rights reserved.
//

#ifndef __DataMining__Cluster__
#define __DataMining__Cluster__

#include "statistics/Samples.h"
#include "FeatureSpace.h"

typedef FeatureSpace<int>                       ClusterItemFeatures;
typedef ClusterItemFeatures::Similarity         ClusterItemSimilarity;
typedef std::vector<const struct ClusterItem*>  ClusterItems;

typedef std::shared_ptr<class Cluster>          ClusterPtr;
typedef std::vector<ClusterPtr>                 Clusters;

class Cluster;

struct ClusterItem {
                            ClusterItem( ClusterItemFeatures* features ) : m_cluster( NULL ), m_features( features ) {}

    mutable const Cluster*  m_cluster;
    ClusterItemFeatures*    m_features;
};

struct DistanceMeasure {
    virtual float           operator()( const ClusterItem* a, const ClusterItem* b ) { return 0.0f; }
};

struct EuclideanDistance : public DistanceMeasure {
    virtual float           operator()( const ClusterItem* a, const ClusterItem* b ) { return (*a->m_features - *b->m_features).length(); }
};

struct SimilarityDistance : public DistanceMeasure {
                            SimilarityDistance( ClusterItemSimilarity similarity ) : m_similarity( similarity ) {}
    virtual float           operator()( const ClusterItem* a, const ClusterItem* b ) { return 1.0f - m_similarity( *a->m_features, *b->m_features, NULL ); }
    ClusterItemSimilarity   m_similarity;
};

class Cluster {
friend class Clusterizer;
public:

                                Cluster( DistanceMeasure* distance = NULL );

    void                        addItem( const ClusterItem* item );
    void                        addItems( const ClusterItems& items );
    void                        removeItems( const ClusterItems& items );
    void                        clear( void );
    const ClusterItems&         items( void ) const { return m_items; }
    ClusterItems&               items( void ) { return m_items; }

protected:

    ClusterItems                m_items;
    DistanceMeasure*            m_distance;
};

class KMedoidsCluster : public Cluster {
public:

                                KMedoidsCluster( const ClusterItem* medoid = NULL, DistanceMeasure* distance = NULL );

    const ClusterItem*          medoid( void ) const;
    bool                        selectMedoid( void );

private:

    const ClusterItem*          m_medoid;
};

class Clusterizer {
public:

                                Clusterizer( const ClusterItems& items, DistanceMeasure* distance = NULL );

    virtual Clusters            clusterize( void ) = 0;
    const Cluster&              orphaned( void ) const;

    static PercentageSamples    itemSimilarity( const ClusterItems& items, ClusterItemSimilarity similarity );

protected:

    ClusterItems                m_items;
    DistanceMeasure*            m_distance;
    Cluster                     m_orphaned;
};

class KMedoids : public Clusterizer {
public:

                                KMedoids( int nClusters, const ClusterItems& items, DistanceMeasure* distance = NULL );

    // ** Clusterizer
    virtual Clusters            clusterize( void );

private:

    ClusterItems                selectInitialMedoids( int count ) const;

private:

    int                         m_nClusters;
};

class AgglomerativeClusterizer : public Clusterizer {
public:

                                AgglomerativeClusterizer( float distanceThreshold, const ClusterItems& items, DistanceMeasure* distance = NULL );

    // ** Clusterizer
    virtual Clusters            clusterize( void );

private:

    bool                        mergeClusters( Clusters& clusters );
    float                       maximalDistance( const ClusterItems& a, const ClusterItems& b, float bestDistance ) const;

private:

    // ** struct BestClusterPair
    struct BestClusterPair {
        int             m_a;
        int             m_b;
        float           m_distance;

                        BestClusterPair( int a = -1, int b = -1, float distance = FLT_MAX )
                            : m_a( a ), m_b( b ), m_distance( distance ) {}

        operator bool() { return m_a >= 0 && m_b >= 0; }
    };

    float                       m_distanceThreshold;
};

#endif /* defined(__DataMining__Cluster__) */
