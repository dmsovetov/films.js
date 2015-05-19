//
//  Histogram.h
//  DataMining
//
//  Created by Советов Дмитрий on 19.12.14.
//  Copyright (c) 2014 data. All rights reserved.
//

#ifndef __DataMining__Histogram__
#define __DataMining__Histogram__

#include "../Common.h"

class Histogram {
public:

    struct Range {
        float   m_min, m_max;
        float   m_frequency;
    };

                        Histogram( int bins = 0, float min = 0.0f, float max = 0.0f );

    const Histogram&    operator += ( float sample );
    const Range&        operator [] ( int index ) const;

    int                 bins( void ) const { return ( int )m_bins.size(); }
    float               total( void ) const;
    float               min( void ) const { return m_min; }
    float               max( void ) const { return m_max; }
    float               frequency( float min, float max ) const;
    Histogram           normalized( void ) const;

private:

    static bool         sampleInRange( float sample, float min, float max, float eps = 0.00001f );

private:

    float               m_min;
    float               m_max;
    float               m_step;
    int                 m_total;
    std::vector<Range>  m_bins;
};

template<typename T>
class Samples {
public:

                        Samples( void )
                            : m_min( std::numeric_limits<T>::max() ), m_max( -std::numeric_limits<T>::max() ) {}

    const Samples<T>&   operator += ( const T& sample );
    Samples<T>&         operator << ( const T& sample );

    int                 size( void ) const { return ( int )m_samples.size(); }
    T                   min( void ) const { return m_min; }
    T                   max( void ) const { return m_max; }
    T                   mean( void ) const;
    T                   median( void ) const;
    T                   quartile( int idx ) const;
    std::vector<T>      quartiles( void ) const;
    T                   dispersion( void ) const;
    int                 lessCount( const T& value ) const;
    int                 greaterCount( const T& value ) const;
    Histogram           histogram( int bins = 10, T min = std::numeric_limits<T>::max(), T max = -std::numeric_limits<T>::max() ) const;
    void                sort( void );

private:

    std::vector<T>      m_samples;
    T                   m_min;
    T                   m_max;
};

template<typename T>
inline const Samples<T>& Samples<T>::operator += ( const T& sample )
{
    m_min = std::min( m_min, sample );
    m_max = std::max( m_max, sample );

    m_samples.push_back( sample );

    return *this;
}

template<typename T>
inline Samples<T>& Samples<T>::operator << ( const T& sample )
{
    *this += sample;
    return *this;
}

template<typename T>
inline void Samples<T>::sort( void )
{
    std::sort( m_samples.begin(), m_samples.end() );
}

template<typename T>
inline int Samples<T>::lessCount( const T& value ) const
{
    int count = 0;

    for( int i = 0, n = size(); i < n; i++ ) {
        count += (m_samples[i] < value) ? 1 : 0;
    }

    return count;
}

template<typename T>
inline int Samples<T>::greaterCount( const T& value ) const
{
    int count = 0;

    for( int i = 0, n = size(); i < n; i++ ) {
        count += (m_samples[i] > value) ? 1 : 0;
    }

    return count;
}


template<typename T>
inline T Samples<T>::dispersion( void ) const
{
    T avg = mean();
    T sum = 0;

    for( int i = 0, n = ( int )m_samples.size(); i < n; i++ ) {
        sum += pow( m_samples[i] - avg, 2 );
    }

    return sum / m_samples.size();
}

template<typename T>
inline T Samples<T>::mean( void ) const
{
    T sum = 0;

    for( int i = 0, n = ( int )m_samples.size(); i < n; i++ ) {
        sum += m_samples[i];
    }

    return sum / m_samples.size();
}

template<typename T>
inline T Samples<T>::median( void ) const
{
    const_cast<Samples<T>*>( this )->sort();

    int sz = size();

    if( sz % 2 ) {
        return (m_samples[sz / 2] + m_samples[sz / 2 + 1]) / 2;
    }

    return m_samples[sz / 2];
}

template<typename T>
inline T Samples<T>::quartile( int idx ) const
{
    const_cast<Samples<T>*>( this )->sort();

    int   sz    = size();
    float q     = idx / 4.0f * (sz + 1);
    int   index = floor( q );

    if( fabs( q - index ) > 0.0f ) {
        return (m_samples[index - 1] + m_samples[index]) / 2;
    }

    return m_samples[index];
}

template<typename T>
inline std::vector<T> Samples<T>::quartiles( void ) const
{
    std::vector<T> result;

    for( int i = 0; i < 3; i++ ) {
        result.push_back( quartile( i + 1 ) );
    }

    return result;
}

template<typename T>
inline Histogram Samples<T>::histogram( int bins, T min, T max ) const
{
    min = min == FLT_MAX ? m_min : min;
    max = max == FLT_MAX ? m_max : max;

    Histogram result( bins, min, max );

    // ** For each samlple place it to a bin
    for( int i = 0; i < ( int )m_samples.size(); i++ ) {
        result += m_samples[i];
    }

//    assert( result.total() == m_samples.size() );

    return result;
}

inline Histogram::Histogram( int bins, float min, float max ) : m_min( min ), m_max( max ), m_total( 0 ), m_bins( bins )
{
    m_step = (max - min) / bins;

    for( int i = 0; i < bins; i++ ) {
        m_bins[i].m_min         = i * m_step;
        m_bins[i].m_max         = (i + 1) * m_step;
        m_bins[i].m_frequency   = 0;
    }
}

inline const Histogram::Range& Histogram::operator []( int index ) const
{
    return m_bins[index];
}

inline const Histogram& Histogram::operator += ( float sample )
{
//    const float kEpsilon = 0.00001f;

    m_total++;

    if( !sampleInRange( sample, m_min, m_max ) ) {
        return *this;
    }

//    if( (sample + kEpsilon) < m_min || (sample - kEpsilon) > m_max ) {
//        return *this;
//    }

    sample = std::min( m_max, std::max( m_min, sample ) );
    
    int idx = 0;

    for( idx = 0; idx < ( int )m_bins.size(); idx++ ) {
        Range& bin = m_bins[idx];

    //    if( sample >= bin.m_min && sample < bin.m_max ) {
        if( sampleInRange( sample, bin.m_min, bin.m_max ) ) {
            idx++;
            break;
        }
    }

    assert( idx > 0 && idx <= ( int )m_bins.size() );

    m_bins[idx - 1].m_frequency++;

    return *this;
}

inline float Histogram::total( void ) const
{
    float result = 0.0f;

    for( int i = 0, n = ( int )m_bins.size(); i < n; i++ ) {
        result += m_bins[i].m_frequency;
    }

    return result;
}


inline Histogram Histogram::normalized( void ) const
{
    Histogram result( bins(), m_min, m_max );

    result.m_bins = m_bins;

    float total = 0.0f;

    for( int i = 0; i < ( int )m_bins.size(); i++ ) {
        result.m_bins[i].m_frequency = result.m_bins[i].m_frequency / m_total * 100;
        total += result.m_bins[i].m_frequency;
    }

    return result;
}

inline float Histogram::frequency( float min, float max ) const
{
    float result = 0.0f;

    for( int i = 0, n = ( int )m_bins.size(); i < n; i++ ) {
        const Range& range = m_bins[i];

        if( sampleInRange( (range.m_min + range.m_max) * 0.5f, min, max ) ) {
            result += range.m_frequency;
        }
    }

    return result;
}

inline bool Histogram::sampleInRange( float sample, float min, float max, float eps )
{
    return (sample + eps) >= min && (sample - eps) < max;
}

typedef Samples<float>                              PercentageSamples;
typedef std::map<std::string, PercentageSamples>    PercentageSamplesMap;
typedef std::map<std::string, Histogram>            Histograms;

class HistogramSerializer {
public:

    static void             serialize( const char* fileName, const Histograms& histograms, int ranges );
    static void             serialize( const char* fileName, const Histogram& histogram );
    static void             serialize( const char* fileName, const PercentageSamplesMap& samples, int ranges, float min = FLT_MAX, float max = FLT_MAX );
};

inline void HistogramSerializer::serialize( const char* fileName, const Histogram& histogram )
{
    FILE* fp = fopen( fileName, "wt" );

    // ** Output fields
    fprintf( fp, "Range, Total\n" );

    for( int i = 0; i < histogram.bins(); i++ ) {
        fprintf( fp, "[%2.2f %2.2f],%d\n", histogram[i].m_min, histogram[i].m_max, ( int )histogram[i].m_frequency );
    }

    fclose( fp );
}

inline void HistogramSerializer::serialize( const char *fileName, const PercentageSamplesMap& samples, int ranges, float rangeMin, float rangeMax )
{
    float       min =  FLT_MAX;
    float       max = -FLT_MAX;
    Histograms  histograms;

    for( PercentageSamplesMap::const_iterator i = samples.begin(), end = samples.end(); i != end; ++i ) {
        min = std::min( min, i->second.min() );
        max = std::max( max, i->second.max() );
    }

    min = rangeMin == FLT_MAX ? min : rangeMin;
    max = rangeMax == FLT_MAX ? max : rangeMax;

    for( PercentageSamplesMap::const_iterator i = samples.begin(), end = samples.end(); i != end; ++i ) {
        histograms[i->first] = i->second.histogram( ranges, min, max ).normalized();
    }

    float step = (max - min) / ranges;
    FILE* fp   = fopen( fileName, "wt" );

    // ** Output fields
    fprintf( fp, "Range" );
    for( Histograms::const_iterator i = histograms.begin(), end = histograms.end(); i != end; ++i ) {
        fprintf( fp, "," );
        fprintf( fp, "%s", i->first.c_str() );
    }
    fprintf( fp, "\n" );

    // ** Output data
    for( int i = 0; i < ranges; i++ ) {
        if( step > 1.0f ) {
            fprintf( fp, "%d-%d", int( i * step ), int( (i + 1) * step ) );
        } else {
            fprintf( fp, "%2.2f-%2.2f", i * step, (i + 1) * step );
        }

        for( Histograms::const_iterator j = histograms.begin(), end = histograms.end(); j != end; ++j ) {
            fprintf( fp, "," );
            fprintf( fp, "%2.2f", j->second.frequency( i * step, (i + 1) * step ) );
        }
        fprintf( fp, "\n" );
    }
    
    fclose( fp );
}

typedef Samples<int>    IntegerSamples;
typedef Samples<float>  FloatSamples;

#endif /* defined(__DataMining__Histogram__) */
