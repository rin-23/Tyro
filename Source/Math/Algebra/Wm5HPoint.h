// Geometric Tools, LLC
// Copyright (c) 1998-2014
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
//
// File Version: 5.0.2 (2011/03/27)

#ifndef WM5HPOINT_H
#define WM5HPOINT_H

#include "Wm5MathematicsLIB.h"
#include <cereal/archives/binary.hpp>

namespace Wm5
{

class WM5_MATHEMATICS_ITEM HPoint
{
public:
    // Construction and destruction.  HPoint represents a homogeneous point
    // of the form (x,y,z,w).  Affine points are characterized by w = 1
    // (see class APoint) and affine vectors are characterized by w = 0
    // (see class AVector).
    HPoint ();  // uninitialized
    HPoint (const HPoint& pnt);
    HPoint (HPoint&& pnt);
    HPoint (float x, float y, float z, float w);
    ~HPoint ();

    // Coordinate access.
    inline operator const float* () const;
    inline operator float* ();
    inline const float& operator[] (int i) const;
    inline float& operator[] (int i);
    inline float X () const;
    inline float& X ();
    inline float Y () const;
    inline float& Y ();
    inline float Z () const;
    inline float& Z ();
    inline float W () const;
    inline float& W ();

    // Assignment.
    HPoint& operator= (const HPoint& pnt);
    HPoint& operator= (HPoint&& pnt);


    // Comparison (for use by STL containers).
    bool operator== (const HPoint& pnt) const;
    bool operator!= (const HPoint& pnt) const;
    bool operator<  (const HPoint& pnt) const;
    bool operator<= (const HPoint& pnt) const;
    bool operator>  (const HPoint& pnt) const;
    bool operator>= (const HPoint& pnt) const;
    
    template<class Archive>
    void save(Archive & archive, std::uint32_t const version) const
    {
        archive(cereal::binary_data(&mTuple[0], sizeof(float)*4));
    }
    
    template<class Archive>
    void load(Archive & archive, std::uint32_t const version)
    {
        archive(cereal::binary_data(&mTuple[0], sizeof(float)*4));
    }


protected:
    float mTuple[4];
};

#include "Wm5HPoint.inl"

}

#endif
