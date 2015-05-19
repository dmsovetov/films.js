//
//  Cluster.cpp
//  DataMining
//
//  Created by Советов Дмитрий on 18.12.14.
//  Copyright (c) 2014 data. All rights reserved.
//

#include "Cluster.h"
#include "statistics/Samples.h"

Cluster::Cluster( DistanceMeasure* distance ) : m_distance( distance )
{

}

void Cluster::clear( void )
{
    m_items.clear();
}

void Cluster::addItem( const ClusterItem* item )
{
    assert( item != NULL );

    item->m_cluster = this;
    m_items.push_back( item );
}

void Cluster::addItems( const ClusterItems& items )
{
    m_items.reserve( m_items.size() + items.size() );
    m_items.insert( m_items.end(), items.begin(), items.end() );
}

void Cluster::removeItems( const ClusterItems& items )
{
    for( int i = 0, n = ( int )items.size(); i < n; i++ ) {
        ClusterItems::iterator it = std::find( m_items.begin(), m_items.end(), items[i] );
    //    assert( it != m_items.end() );
        m_items.erase( it );
    }
}

KMedoidsCluster::KMedoidsCluster( const ClusterItem* medoid, DistanceMeasure* distance ) : Cluster( distance ), m_medoid( medoid )
{

}

const ClusterItem* KMedoidsCluster::medoid( void ) const
{
    return m_medoid;
}

bool KMedoidsCluster::selectMedoid( void )
{
    if( m_items.empty() ) {
        return false;
    }

    const ClusterItem*  bestMedoid  = NULL;
    float               bestSum     = FLT_MAX;

    for( ClusterItems::const_iterator i = m_items.begin(), iend = m_items.end(); i != iend; ++i ) {
        float sum = 0.0f;

        for( ClusterItems::const_iterator j = m_items.begin(), jend = m_items.end(); j != jend; ++j ) {
            if( *i == *j ) {
                continue;
            }

            sum += (*m_distance)( *i, *j );
        }

        if( sum < bestSum ) {
            bestSum     = sum;
            bestMedoid  = *i;
        }
    }

    assert( bestMedoid != NULL );

    bool changed = (bestMedoid != m_medoid);
    m_medoid = bestMedoid;

    return changed;
}

PercentageSamples Clusterizer::itemSimilarity( const ClusterItems& items, ClusterItemSimilarity similarity )
{
    PercentageSamples samples;

    for( int i = 0, n = ( int )items.size(); i < n; i++ ) {
        for( int j = i; j < n; j++ ) {
            samples += similarity( *items[i]->m_features, *items[j]->m_features, NULL );
        }
    }

    return samples;
}

Clusterizer::Clusterizer( const ClusterItems& items, DistanceMeasure* distance ) : m_items( items ), m_distance( distance )
{

}

const Cluster& Clusterizer::orphaned( void ) const
{
    return m_orphaned;
}

KMedoids::KMedoids( int nClusters, const ClusterItems& items, DistanceMeasure* distance ) : Clusterizer( items, distance ), m_nClusters( nClusters )
{
    
}

Clusters KMedoids::clusterize( void )
{
    Clusters     clusters;
    ClusterItems medoids = selectInitialMedoids( m_nClusters );
    assert( ( int )medoids.size() == m_nClusters );

    for( int i = 0; i < ( int )medoids.size(); i++ ) {
        clusters.push_back( ClusterPtr( new KMedoidsCluster( medoids[i], m_distance ) ) );
    }

    bool wasChanged = false;

    do {
        for( int i = 0; i < m_nClusters; i++ ) {
            clusters[i]->clear();
        }

        for( int i = 0, n = ( int )m_items.size(); i < n; i++ ) {
            const ClusterItem*  item            = m_items[i];
            float               bestDistance    = FLT_MAX;
            Cluster*            bestCluster     = &m_orphaned;

            for( int j = 0; j < m_nClusters; j++ ) {
                KMedoidsCluster* cluster  = reinterpret_cast<KMedoidsCluster*>( clusters[j].get() );
                float            distance = (*m_distance)( item, cluster->medoid() );

                if( distance < bestDistance ) {
                    bestCluster   = cluster;
                    bestDistance  = distance;
                }
            }

            bestCluster->addItem( item );
        }

        wasChanged = false;
        for( int i = 0; i < m_nClusters; i++ ) {
            bool medoidChanged = reinterpret_cast<KMedoidsCluster*>( clusters[i].get() )->selectMedoid();
            wasChanged = wasChanged || medoidChanged;
        }
    } while( wasChanged );

    return clusters;
}

ClusterItems KMedoids::selectInitialMedoids( int count ) const
{
    ClusterItems result;

    // ** Push a random medoid
    int idx = rand() % m_items.size();
    assert( idx >= 0 && idx < ( int )m_items.size() );

    result.push_back( m_items[idx] );

    // ** Choose the next one
    while( ( int )result.size() < count ) {
        float               largestDistance = -FLT_MAX;
        const ClusterItem*  selectedMedoid  = NULL;

        for( int i = 0, n = ( int )m_items.size(); i < n; i++ ) {
            const ClusterItem*  item = m_items[i];
            float               sum  = 0.0f;

            // ** This user is already selected as a medoid
            if( std::find( result.begin(), result.end(), item ) != result.end() ) {
                continue;
            }

            // ** Calculate a sum of distances with all selected medoids
            for( int j = 0, n = ( int )result.size(); j < n; j++ ) {
                if( result[j] == item ) {
                    continue;
                }

                sum += (*m_distance)( result[j], item );
            }

            // ** Choose the lowest similarity sum
            if( sum > largestDistance ) {
                largestDistance = sum;
                selectedMedoid  = item;
            }
        }

        assert( selectedMedoid != NULL );

        result.push_back( selectedMedoid );
    }

    return result;
}

AgglomerativeClusterizer::AgglomerativeClusterizer( float distanceThreshold, const ClusterItems& items, DistanceMeasure* distance )
    : Clusterizer( items, distance ), m_distanceThreshold( distanceThreshold )
{

}

Clusters AgglomerativeClusterizer::clusterize( void )
{
    Clusters clusters;

    for( int i = 0, n = ( int )m_items.size(); i < n; i++ ) {
        Cluster* c = new Cluster( m_distance );
        c->addItem( m_items[i] );
        clusters.push_back( ClusterPtr( c ) );
    }

    while( mergeClusters( clusters ) ) {
        printf( "Merging clusters, %d left\n", ( int )clusters.size() );
    }

    printf( "Produced %d clusters\n", ( int )clusters.size() );

    return clusters;
}

bool AgglomerativeClusterizer::mergeClusters( Clusters& clusters )
{
    BestClusterPair pair;

    for( int i = 0, n = ( int )clusters.size(); i < n; i++ ) {
        ClusterPtr& cluster = clusters[i];

        for( int j = i + 1; j < n; j++ ) {
            ClusterPtr& other = clusters[j];
            float distance    = maximalDistance( cluster->items(), other->items(), pair.m_distance );

            if( distance > m_distanceThreshold ) {
                continue;
            }

            if( distance == 0.0f ) {
                cluster->addItems( other->items() );
                clusters.erase( clusters.begin() + j );
                return true;
            }

            if( distance < pair.m_distance ) {
                pair = BestClusterPair( i, j, distance );
            }
        }
    }

    if( pair ) {
        clusters[pair.m_a]->addItems( clusters[pair.m_b]->items() );
        clusters.erase( clusters.begin() + pair.m_b );
    }

    return pair;
}

float AgglomerativeClusterizer::maximalDistance( const ClusterItems& a, const ClusterItems& b, float bestDistance ) const
{
    float distance = -FLT_MAX;

    for( int i = 0, na = ( int )a.size(); i < na; i++ ) {
        for( int j = 0, nb = ( int )b.size(); j < nb; j++ ) {
            distance = std::max( distance, (*m_distance)( a[i], b[j] ) );

            if( distance >= bestDistance ) {
                return FLT_MAX;
            }
        }
    }

    return distance;
}
/*
void ClusterChecker::check( const ClusterItems& items, Stats& stats, DistanceMeasure* distance, float threshold )
{
    stats.truePositives = stats.falsePositives = 0;

    for( int i = 0, n = ( int )items.size(); i < n; i++ ) {
        for( int j = i + 1; j < n; j++ ) {
            float d = (*distance)( items[i], items[j] );

            if( d <= threshold ) {
                stats.truePositives++;
            } else {
                stats.falsePositives++;
            }
        }
    }
}

void ClusterChecker::check( int index, const Clusters& clusters, Stats& stats, DistanceMeasure* distance, float threshold )
{
    stats.falseNegatives = stats.trueNegatives = 0;

    const ClusterItems& items = clusters[index]->items();

    for( int i = 0, n = ( int )clusters.size(); i < n; i++ ) {
        if( i == index ) {
            continue;
        }

        const ClusterItems& other = clusters[i]->items();

        for( int j = 0; j < items.size(); j++ ) {
            for( int k = 0; k < other.size(); k++ ) {
                float d = (*distance)( items[j], other[k] );

                if( d <= threshold ) {
                    stats.falseNegatives++;
                } else {
                    stats.trueNegatives++;
                }
            }
        }
    }
}*/