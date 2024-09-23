#ifndef COORDINATE3D_H_
#define COORDINATE3D_H_

#include <vector>
#include <cmath>

/**
* @file Coordinate3D.h
* @brief This declares and defines the `Coordinate3D` class
*/

namespace stt {
    template <class T> class Coordinate3D;
}

/// a representation of a 3-dimensional point coordinate
template <class T>
class stt::Coordinate3D
{
public:
    T x; /// the x, y, and z coordinate members
    T y;
    T z;

    /// create an empty coordinate
    Coordinate3D(): x(0), y(0), z(0) {}

    /// the const copy constructor
    Coordinate3D(const Coordinate3D &other): x(other.x), y(other.y), z(other.z) {}

    /// instantiate a coordinate from an x, y, and z value
    Coordinate3D(T x, T y, T z): x(x), y(y), z(z) {}

    /// overload the equality operator
    virtual bool
    operator==(const Coordinate3D &other) const {
        return x == other.x
            && y == other.y
            && z == other.z;
    }

    /// overload the assignment operator
    virtual void
    operator=(const Coordinate3D &other) {
        x = other.x;
        y = other.y;
        z = other.z;
    }

    /// gets a read-only index-ordinate of the coordinate
    inline virtual T operator[](const int index) const {
        return (index == 0) ? x : (index == 1 ? y : z);
    }
};

#endif /* COORDINATE3D_H_ */
