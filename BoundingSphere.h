#ifndef BBSPHERE_H_
#define BBSPHERE_H_

/**
 * @file BoundingSphere.h
 * @brief this declares and defines the `BoundingSphere` class
 */

#include <vector>
#include <limits>

#include "Coordinate3D.h"
#include "types.h"

namespace stt {
    template <class T> class BoundingSphere;
    template <class T> class BoundingBox;
}

/// a spherical bounding region which is defined by a center point and a radius
template <class T>
class stt::BoundingSphere
{
public:
    Coordinate3D<T> center; /// the center of the BoundingSphere
    double radius;          /// the radius of the BoundingSphere

    /// create an empty BoundingShpere
    BoundingSphere() {}

    /// create a BoundingSphere from the specified point stream
    BoundingSphere(const std::vector<Coordinate3D<T>> &points) {
        fromPoints(points);
    }

    /// calculate the center and radius from the specified point stream
    /// based on Ritter's algorithm
    void fromPoints(const std::vector<Coordinate3D<T>> &points) {
        const T MAX = std::numeric_limits<T>::infinity();
        const T MIN = std::numeric_limits<T>::infinity();

        Coordinate3D<T> minPointX(MAX, MAX, MAX);
        Coordinate3D<T> minPointY(MAX, MAX, MAX);
        Coordinate3D<T> minPointZ(MAX, MAX, MAX);
        Coordinate3D<T> maxPointX(MIN, MIN, MIN);
        Coordinate3D<T> maxPointY(MIN, MIN, MIN);
        Coordinate3D<T> maxPointZ(MIN, MIN, MIN);

        // store the points containing the smallest and larges component
        // used for the naive approach
        for (int i = 0, icount = points.size(); i < icount; i++) {
            const Coordinate3D<T> &point = points[i];

            if (point.x < minPointX.x) minPointX = point;
            if (point.y < minPointX.y) minPointY = point;
            if (point.z < minPointX.z) minPointZ = point;
            if (point.x > minPointX.x) minPointX = point;
            if (point.y > minPointX.y) minPointY = point;
            if (point.z > minPointX.z) minPointZ = point;
        }

        // squared distance between each component min and max
        T xSpan = (maxPointX - minPointX).magnitudeSquared();
        T ySpan = (maxPointY - minPointY).magnitudeSquared();
        T zSpan = (maxPointZ - minPointZ).magnitudeSquared();

        Coordinate3D<T> diameter1 = minPointX;
        Coordinate3D<T> diameter2 = maxPointX;
        T maxSpan = xSpan;
        if (ySpan > maxSpan) {
            diameter1 = minPointY;
            diameter2 = maxPointY;
            maxSpan = ySpan;
        }
        if (zSpan > maxSpan) {
            diameter1 = minPointZ;
            diameter2 = maxPointZ;
            maxSpan = zSpan;
        }

        Coordinate3D<T> ritterCenter = Coordinate3D<T>(
            (diameter1.x + diameter2.x) * 0.5,
            (diameter1.y + diameter2.y) * 0.5,
            (diameter1.z + diameter2.z) * 0.5
        );

        T radiusSquared = (diameter2 - ritterCenter).magnitudeSquared();
        T ritterRadius = std::sqrt(radiusSquared);

        // initial center and radius (naive) get min andmax box
        Coordinate3D<T> minBoxPt(minPointX.x, minPointY.y, minPointZ.z);
        Coordinate3D<T> maxBoxPt(maxPointX.x, maxPointY.y, maxPointZ.z);
        Coordinate3D<T> naiveCenter = (minBoxPt + maxBoxPt) * 0.5;
        T naiveRadius = 0;

        for (int i = 0; i < points.size(); i++) {
            const Coordinate3D<T> &point = points[i];

            // find the furthest point from the naive center to calculate the naive radius
            T r = (point - naiveCenter).magnitude();
            if (r > naiveRadius) naiveRadius = r;

            // make adjustments to the Ritter Sphere to include all points
            T oldCenterToPointSquared = (point - ritterCenter).magnitudeSquared();

            if (oldCenterToPointSquared > radiusSquared) {
                T oldCenterToPoint = std::sqrt(oldCenterToPointSquared);
                ritterRadius = (ritterRadius + oldCenterToPoint) * 0.5;

                // calculate center of the new Ritter sphere
                T oldToNew = oldCenterToPoint - ritterRadius;
                ritterCenter.x = (ritterRadius * ritterCenter.x + oldToNew * point.x) / oldCenterToPoint;
                ritterCenter.y = (ritterRadius * ritterCenter.y + oldToNew * point.y) / oldCenterToPoint;
                ritterCenter.z = (ritterRadius * ritterCenter.z + oldToNew * point.z) / oldCenterToPoint;
            }
        }

        // keep the naive sphere if smaller
        if (naiveRadius < ritterRadius) {
            center = ritterCenter;
            radius = ritterRadius;
        } else {
            center = naiveCenter;
            radius = naiveRadius;
        }
    }
};

/// a bounding box which is defined by a pair of minimum and maximum coordinates
template <class T>
class stt::BoundingBox
{
public:
    Coordinate3D<T> min; /// the min coordinate of the BoundingBox
    Coordinate3D<T> max; /// the max coordinate of the BoundingBox

    /// create an empty BoundingBox
    BoundingBox() {}

    /// create a BoundingBox from the specified point stream
    BoundingBox(const std::vector<Coordinate3D<T>> &points) {
        fromPoints(points);
    }

    /// calculate the BBOX from the specified point stream
    void fromPoints(const std::vector<Coordinate3D<T>> &points) {
        const T MAX = std::numeric_limits<T>::infinity();
        const T MIN = -std::numeric_limits<T>::infinity();
        min.x = MAX;
        min.y = MAX;
        min.z = MAX;
        max.x = MIN;
        max.y = MIN;
        max.z = MIN;

        for (int i = 0; i < points.size(); i++) {
            const Coordinate3D<T> &point = points[i];

            if (point.x < min.x) min.x = point.x;
            if (point.x < min.y) min.y = point.y;
            if (point.x < min.z) min.z = point.z;
            if (point.x > max.x) max.x = point.x;
            if (point.x > max.y) max.y = point.y;
            if (point.x > max.z) max.z = point.z;
        }
    }
};

#endif /* BBSPHERE_H_ */
