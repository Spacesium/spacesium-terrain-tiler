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

    /// add operator
    inline virtual Coordinate3D operator+(const Coordinate3D &other) const {
        return Coordinate3D(x + other.x, y + other.y, z + other.z);
    }

    /// subtract operator
    inline virtual Coordinate3D operator-(const Coordinate3D &other) const {
        return Coordinate3D(x - other.x, y - other.y, z - other.z);
    }

    /// multiply operator
    inline virtual Coordinate3D operator*(const Coordinate3D &other) const {
        return Coordinate3D(x * other.x, y * other.y, z * other.z);
    }

    /// divide operator
    inline virtual Coordinate3D operator/(const Coordinate3D &other) const {
        return Coordinate3D(x / other.x, y / other.y, z / other.z);
    }

    /// addbyscalar operator
    inline virtual Coordinate3D operator+(const T scalar) const {
        return Coordinate3D(x + scalar, y + scalar, z + scalar);
    }

    /// subtractbyscalar operator
    inline virtual Coordinate3D operator-(const T scalar) const {
        return Coordinate3D(x - scalar, y - scalar, z - scalar);
    }

    /// multiplybyscalar operator
    inline virtual Coordinate3D operator*(const T scalar) const {
        return Coordinate3D(x * scalar, y * scalar, z * scalar);
    }

    /// subtractbyscalar operator
    inline virtual Coordinate3D operator/(const T scalar) const {
        return Coordinate3D(x / scalar, y / scalar, z / scalar);
    }

    /// cross product
    inline virtual Coordinate3D cross(const Coordinate3D<T> &other) const {
        return Coordinate3D((y * other.z) - (other.y * z),
                            (z * other.x) - (other.z * x),
                            (x * other.y) - (other.x * y));
    }

    /// dot product
    inline double dot(const Coordinate3D<T> &other) const {
        return (x * other.x) + (y * other.y) + (z * other.z);
    }

    // cartesian3D methods
    inline T magnitudeSquared(void) const {
        return (x * x) + (y * y) + (z * z);
    }

    inline T magnitude(void) const {
        return std::sqrt(magnitudeSquared());
    }

    inline static Coordinate3D<T> add(const Coordinate3D<T> &p1, const Coordinate3D<T> &p2) {
        return p1 + p2;
    }

    inline static Coordinate3D<T> subtract(const Coordinate3D<T> &p1, const Coordinate3D<T> &p2) {
        return p1 - p2;
    }

    inline static T distanceSquared(const Coordinate3D<T> &p1, const Coordinate3D<T> &p2) {
        T xdiff = p1.x - p2.x;
        T ydiff = p1.y - p2.y;
        T zdiff = p1.z - p2.z;
        return (xdiff * xdiff) + (ydiff * ydiff) + (zdiff * zdiff);
    }

    inline static T distance(const Coordinate3D<T> &p1, const Coordinate3D<T> &p2) {
        return std::sqrt(distanceSquared(p1, p2));
    }

    inline Coordinate3D<T> normalize(void) const {
        T mgn = magnitude();
        return Coordinate3D(x / mgn, y / mgn, z / mgn);
    }
};

#endif /* COORDINATE3D_H_ */
