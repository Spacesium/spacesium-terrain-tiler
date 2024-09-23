#ifndef COORDINATE_H_
#define COORDINATE_H_

/**
* @file Coordinate.h
* @brief This declares and defines the `Coordinate` class
*/

namespace stt {
    template <class T> class Coordinate;
}

/// a representation of a point coordinate
template <class T>
class stt::Coordinate
{
public:
    /// create an empty coordinate
    Coordinate(): x(0), y(0) {}

    /// the const copy constructor
    Coordinate(const Coordinate &other): x(other.x), y(other.y) {}

    /// instantiate a coordinate from an x and y value
    Coordinate(T x, T y): x(x), y(y) {}

    /// overload the equality operator
    virtual bool
    operator==(const Coordinate &other) const {
        return x == other.x && y == other.y;
    }

    /// overload the assignment operator
    virtual void
    operator=(const Coordinate &other) {
        x = other.x;
        y = other.y;
    }

    T x;
    T y;
};

#endif /* COORDINATE_H_ */
