//
//  FeatureSpace.h
//  DataMining
//
//  Created by Советов Дмитрий on 18.12.14.
//  Copyright (c) 2014 data. All rights reserved.
//

#ifndef __DataMining__FeatureSpace__
#define __DataMining__FeatureSpace__

#include "Common.h"

template<typename T>
class FeatureSpace {
public:

    typedef float Type;
    typedef float (*Similarity)( const FeatureSpace<T>& a, const FeatureSpace<T>& b, int* shared );

    // ** struct IntersectionValue
    struct IntersectionValue {
        Type   a;
        Type   b;

                IntersectionValue( Type _a = 0.0f, Type _b = 0.0f )
                    : a( _a ), b( _b ) {}
    };

    typedef std::pair<T, Type>              Feature;
    typedef std::map<T, Type>               Features;
    typedef std::map<T, IntersectionValue>  Intersection;

public:

    FeatureSpace<T>         operator -  ( const FeatureSpace<T>& other ) const;
    FeatureSpace<T>         operator -  ( Type scalar ) const;
    FeatureSpace<T>         operator +  ( const FeatureSpace<T>& other ) const;
    Type                    operator *  ( const FeatureSpace<T>& other ) const;
    Intersection            operator %  ( const FeatureSpace<T>& other ) const;
    Intersection            operator |  ( const FeatureSpace<T>& other ) const;
    int                     operator ^  ( const FeatureSpace<T>& other ) const;
    const FeatureSpace<T>&  operator += ( const Feature& feature );
    const FeatureSpace<T>&  operator += ( const FeatureSpace<T>& other );
    const FeatureSpace<T>&  operator /= ( const FeatureSpace<T>& other );
    const FeatureSpace<T>&  operator /= ( Type scalar );

    float                   length( void ) const;
    int                     size( void ) const;
    void                    set( const T& feature, Type value );
    Type                    get( const T& feature ) const;
    Type                    average( void ) const;
    const Features&         features( void ) const;
    Features&               features( void );
    float                   normalize( void );
    void                    clean( void );

    static float            tanimoto( const FeatureSpace<T>& a, const FeatureSpace<T>& b, int* intersectionSize = NULL );
    static float            distShared( const FeatureSpace<T>& a, const FeatureSpace<T>& b, int* intersectionSize = NULL );
    static float            dist( const FeatureSpace<T>& a, const FeatureSpace<T>& b, int* intersectionSize = NULL );
    static float            cosine( const FeatureSpace<T>& a, const FeatureSpace<T>& b, int* intersectionSize = NULL );
    static float            cosineAdjusted( const FeatureSpace<T>& a, const FeatureSpace<T>& b, int* intersectionSize = NULL );
    static float            pearson( const FeatureSpace<T>& a, const FeatureSpace<T>& b, int* intersectionSize = NULL );

private:

    Features                m_features;
};

// ** FeatureSpace<T>::size
template<typename T>
inline int FeatureSpace<T>::size( void ) const
{
    return ( int )m_features.size();
}

// ** FeatureSpace<T>::average
template<typename T>
inline typename FeatureSpace<T>::Type FeatureSpace<T>::average( void ) const
{
    Type result = 0;

    for( typename Features::const_iterator i = m_features.begin(), end = m_features.end(); i != end; ++i ) {
        result += i->second;
    }

    result /= m_features.size();

    return result;
}

// ** FeatureSpace<T>::features
template<typename T>
inline const typename FeatureSpace<T>::Features& FeatureSpace<T>::features( void ) const
{
    return m_features;
}

// ** FeatureSpace<T>::features
template<typename T>
inline typename FeatureSpace<T>::Features& FeatureSpace<T>::features( void )
{
    return m_features;
}

// ** FeatureSpace<T>::set
template<typename T>
inline void FeatureSpace<T>::set( const T& feature, Type value )
{
    m_features[feature] = value;
}

// ** FeatureSpace<T>::get
template<typename T>
inline typename FeatureSpace<T>::Type FeatureSpace<T>::get( const T& feature ) const
{
    typename Features::const_iterator i = m_features.find( feature );
    return i != m_features.end() ? i->second : 0;
}

// ** FeatureSpace<T>::normalize
template<typename T>
inline float FeatureSpace<T>::normalize( void )
{
    float len = length();

    if( len == 0.0f ) {
        return 0.0f;
    }

    for( typename Features::iterator i = m_features.begin(), end = m_features.end(); i != end; ++i ) {
        i->second /= len;
    }

    return len;
}

// ** FeatureSpace<T>::length
template<typename T>
inline float FeatureSpace<T>::length( void ) const
{
    Type sum = 0.0f;

    for( typename Features::const_iterator i = m_features.begin(), end = m_features.end(); i != end; ++i ) {
        sum += i->second * i->second;
    }

    return sqrtf( sum );
}

// ** FeatureSpace<T>::clean
template<typename T>
inline void FeatureSpace<T>::clean( void )
{
    for( typename Features::iterator i = m_features.begin(), end = m_features.end(); i != end; ++i ) {
        if( fabs( i->second ) < 0.001f ) {
            m_features.erase( i );
        }
    }
}


// ** FeatureSpace<T>::operator +=
template<typename T>
inline const FeatureSpace<T>& FeatureSpace<T>::operator += ( const Feature& feature )
{
    typename Features::iterator i = m_features.find( feature.first );
    m_features[feature.first] = (i != m_features.end() ? i->second : 0) + feature.second;

    return *this;
}

// ** FeatureSpace<T>::operator +=
template<typename T>
inline const FeatureSpace<T>& FeatureSpace<T>::operator += ( const FeatureSpace<T>& other )
{
    for( typename Features::const_iterator i = other.m_features.begin(), end = other.m_features.end(); i != end; ++i ) {
        m_features[i->first] = get( i->first ) + i->second;
    }

    return *this;
}

// ** FeatureSpace<T>::operator +
template<typename T>
inline FeatureSpace<T> FeatureSpace<T>::operator + ( const FeatureSpace<T>& other ) const
{
    FeatureSpace<T> result = other;

    for( typename Features::const_iterator i = m_features.begin(), end = m_features.end(); i != end; ++i ) {
        result.set( i->first, other.get( i->first ) + i->second );
    }

    return result;
}

// ** FeatureSpace<T>::operator /=
template<typename T>
inline const FeatureSpace<T>& FeatureSpace<T>::operator /= ( const FeatureSpace<T>& other )
{
    for( typename Features::iterator i = m_features.begin(), end = m_features.end(); i != end; ++i ) {
        i->second /= other.get( i->first );
    }

    return *this;
}

// ** FeatureSpace<T>::operator /=
template<typename T>
inline const FeatureSpace<T>& FeatureSpace<T>::operator /= ( Type scalar )
{
    for( typename Features::iterator i = m_features.begin(), end = m_features.end(); i != end; ++i ) {
        i->second /= scalar;
    }

    return *this;
}

// ** FeatureSpace<T>::operator -
template<typename T>
inline FeatureSpace<T> FeatureSpace<T>::operator - ( const FeatureSpace<T>& other ) const
{
    FeatureSpace<T> result = (*this);

    Features& features = result.m_features;

    for( typename Features::const_iterator i = other.m_features.begin(), end = other.m_features.end(); i != end; ++i ) {
        typename Features::iterator j = features.find( i->first );
        features[i->first] = (j == features.end() ? 0 : j->second) - i->second;
    }

    return result;
}

// ** FeatureSpace<T>::operator %
template<typename T>
inline typename FeatureSpace<T>::Intersection FeatureSpace<T>::operator % ( const FeatureSpace<T>& other ) const
{
    Intersection result;

    for( typename Features::const_iterator i = other.m_features.begin(), end = other.m_features.end(); i != end; ++i ) {
        typename Features::const_iterator j = m_features.find( i->first );
        if( j != m_features.end() ) {
            result[i->first] = IntersectionValue( j->second, i->second );
        }
    }

    return result;
}

// ** FeatureSpace<T>::operator |
template<typename T>
inline typename FeatureSpace<T>::Intersection FeatureSpace<T>::operator | ( const FeatureSpace<T>& other ) const
{
    Intersection result;

    for( typename Features::const_iterator i = m_features.begin(), end = m_features.end(); i != end; ++i ) {
        result[i->first] = IntersectionValue( i->second, other.get( i->first ) );
    }

    for( typename Features::const_iterator i = other.m_features.begin(), end = other.m_features.end(); i != end; ++i ) {
        result[i->first] = IntersectionValue( get( i->first ), i->second );
    }

    return result;
}

// ** FeatureSpace<T>::operator ^
template<typename T>
inline int FeatureSpace<T>::operator ^ ( const FeatureSpace<T>& other ) const
{
    int result = 0;

    for( typename Features::const_iterator i = other.m_features.begin(), end = other.m_features.end(); i != end; ++i ) {
        result += m_features.count( i->first ) ? 1 : 0;
    }

    return result;
}

// ** FeatureSpace<T>::operator *
template<typename T>
inline typename FeatureSpace<T>::Type FeatureSpace<T>::operator * ( const FeatureSpace<T>& other ) const
{
    Type result = 0;

    for( typename Features::const_iterator i = other.m_features.begin(), end = other.m_features.end(); i != end; ++i ) {
        typename Features::const_iterator j = m_features.find( i->first );
        result += i->second * (j != m_features.end() ? j->second : 0);
    }

    return result;
}

// ** FeatureSpace<T>::operator -
template<typename T>
inline FeatureSpace<T> FeatureSpace<T>::operator - ( Type scalar ) const
{
    FeatureSpace<T> result = (*this);

    for( typename Features::iterator i = result.m_features.begin(), end = result.m_features.end(); i != end; ++i ) {
        i->second -= scalar;
    }

    return result;
}

// ** FeatureSpace<T>::tanimoto
template<typename T>
inline float FeatureSpace<T>::tanimoto( const FeatureSpace<T>& a, const FeatureSpace<T>& b, int* intersectionSize )
{
    int   shared = a ^ b;
    float result = float( shared ) / (a.size() + b.size() - shared);

    if( intersectionSize ) *intersectionSize = shared;

    return result;
}

// ** FeatureSpace<T>::distShared
template<typename T>
inline float FeatureSpace<T>::distShared( const FeatureSpace<T>& a, const FeatureSpace<T>& b, int* intersectionSize )
{
    float sum    = 0;
    int   shared = 0;

    for( typename Features::const_iterator i = a.m_features.begin(), end = a.m_features.end(); i != end; ++i ) {
        typename Features::const_iterator j = b.m_features.find( i->first );
        if( j == b.m_features.end() ) {
            continue;
        }

        float diff = i->second - j->second;
        sum += diff * diff;
        shared++;
    }

    if( intersectionSize ) *intersectionSize = shared;

    if( shared == 0 ) {
        return 0.0f;
    }

    return 1.0f / (1.0f + sqrtf( sum ) );
}

// ** FeatureSpace<T>::dist
template<typename T>
inline float FeatureSpace<T>::dist( const FeatureSpace<T>& a, const FeatureSpace<T>& b, int* intersectionSize )
{
    if( intersectionSize ) *intersectionSize = a ^ b;

    float distance = (a - b).length();
    return 1.0f / (1.0f + distance );
}

// ** FeatureSpace<T>::cosine
template<typename T>
inline float FeatureSpace<T>::cosine( const FeatureSpace<T>& a, const FeatureSpace<T>& b, int* intersectionSize )
{
    if( intersectionSize ) *intersectionSize = a ^ b;
    
    float d = a.length() * b.length();
    if( d == 0.0f ) {
        return 0.0f;
    }

    return std::max( a * b / d, 0.0f );
}

// ** FeatureSpace<T>::cosineAdjusted
template<typename T>
inline float FeatureSpace<T>::cosineAdjusted( const FeatureSpace<T>& a, const FeatureSpace<T>& b, int* intersectionSize )
{
    if( intersectionSize ) *intersectionSize = a ^ b;

    FeatureSpace<T> aa = a - a.average();
    FeatureSpace<T> ab = b - b.average();

    float d = aa.length() * ab.length();
    if( d == 0.0f ) {
        return 0.0f;
    }

    return std::max( aa * ab / d, 0.0f );
}

// ** FeatureSpace<T>::pearson
template<typename T>
inline float FeatureSpace<T>::pearson( const FeatureSpace<T>& a, const FeatureSpace<T>& b, int* intersectionSize )
{
    Intersection intersection = a % b;

    if( intersectionSize ) *intersectionSize = ( int )intersection.size();

    if( intersection.empty() ) {
        return 0.0f;
    }

    float sum1 = 0.0f, sum2 = 0.0f, sum1sq = 0.0f, sum2sq = 0.0f, psum = 0.0f;

    for( typename Intersection::const_iterator i = intersection.begin(), end = intersection.end(); i != end; ++i ) {
        Type a = i->second.a;
        Type b = i->second.b;

        sum1 += a;
        sum2 += b;

        sum1sq += a * a;
        sum2sq += b * b;

        psum += a * b;
    }

    float n   = intersection.size();
    float num = psum - (sum1 * sum2 / n);
    float den = sqrtf( (sum1sq - pow(sum1, 2) / n) * (sum2sq - pow(sum2, 2) / n) );

    if( den == 0.0f ) {
        return 0.0f;
    }

    return std::max( num / den, 0.0f );
}

typedef FeatureSpace<int>           NumericFeatures;
typedef NumericFeatures::Similarity NumericFeaturesSimilarity;

#endif /* defined(__DataMining__FeatureSpace__) */
