// Geometric Tools, LLC
// Copyright (c) 1998-2014
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
//
// File Version: 5.0.0 (2010/01/01)

#ifndef WM5AXISALIGNEDBOX3_H
#define WM5AXISALIGNEDBOX3_H

#include "Wm5MathematicsLIB.h"
#include "Wm5Vector3.h"
#include <cereal/archives/binary.hpp>

namespace Wm5
{

template <typename Real>
class AxisAlignedBox3
{
public:
    // Construction and destruction.
    AxisAlignedBox3 ();  // uninitialized
    virtual ~AxisAlignedBox3 ();

    // The caller must ensure that xmin <= xmax, ymin <= ymax, and
    // zmin <= zmax.
    AxisAlignedBox3 (Real xmin, Real xmax, Real ymin,
                     Real ymax, Real zmin, Real zmax);

    AxisAlignedBox3(const Vector3<Real>& minBound, const Vector3<Real>& maxBound);

    AxisAlignedBox3(const Real minBound[3], const Real maxBound[3]);
    
    // Compute the center of the box and the extents (half-lengths)
    // of the box edges.
    Vector3<Real> GetCenter () const;
    Real GetRadius () const;
    Vector3<Real> GetSize () const;
    Vector3<Real> GetHalfSize () const;
    
    void MakeNull();
    bool isNull() const;
    
    void Merge(const AxisAlignedBox3<Real>& rhs);

    // Overlap testing is in the strict sense.  If the two boxes are just
    // touching along a common edge or a common face, the boxes are reported
    // as overlapping.
    bool HasXOverlap (const AxisAlignedBox3& box) const;
    bool HasYOverlap (const AxisAlignedBox3& box) const;
    bool HasZOverlap (const AxisAlignedBox3& box) const;
    bool TestIntersection (const AxisAlignedBox3& box) const;

    // The return value is 'true' if there is overlap.  In this case the
    // intersection is stored in 'intersection'.  If the return value is
    // 'false', there is no overlap.  In this case 'intersection' is
    // undefined.
    bool FindIntersection (const AxisAlignedBox3& box, AxisAlignedBox3& intersection) const;
    
    void SetMinimum(const Wm5::Vector3<Real>& vec);
    void SetMaximum(const Wm5::Vector3<Real>& vec);
    void SetExtents(const Wm5::Vector3<Real>& min, const Wm5::Vector3<Real>& max);
    
    const Wm5::Vector3<Real>& GetMinimum() const;
    const Wm5::Vector3<Real>& GetMaximum() const;
    
    void ComputeVertices (Vector3<Real> vertex[8]) const;
    
    //Serialization
    template<class Archive>
    void save(Archive & archive, std::uint32_t const version) const
    {
        archive(mExtent, Min, Max);
    }
    
    template<class Archive>
    void load(Archive & archive, std::uint32_t const version)
    {
        archive(mExtent, Min, Max);
    }

protected:
    enum Extent
    {
        EXTENT_NULL,
        EXTENT_FINITE,
    };
    Extent mExtent;
    Vector3<Real> Min;
    Vector3<Real> Max;
};

#include "Wm5AxisAlignedBox3.inl"

typedef AxisAlignedBox3<float> AxisAlignedBox3f;
typedef AxisAlignedBox3<double> AxisAlignedBox3d;
    

}

#endif
