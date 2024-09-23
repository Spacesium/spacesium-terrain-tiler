#ifndef BOUNDS_H_
#define BOUNDS_H_

/**
* @file Bounds.h
* @brief this declares and defines the `Bounds` class
*/

#include "Coordinate.h"
#include "STTException.h"

namespace stt {
    template <class T> class Bounds;
}

/// a representation of an extent
template <class T>
class stt::Bounds
{
public:
    /// create an empty bounds
    Bounds() {
        bounds[0] = bounds[1] = bounds[2] = bounds[3] = 0;
    }

    /// create bounds from individual extents
    Bounds(T minx, T miny, T maxx, T maxy) {
        setBounds(minx, miny, maxx, maxy);
    }

    /// create bounds represented by lower left and upper right coordinates
    Bounds(const Coordinate<T> &lowerLeft, const Coordinate<T> &upperRight) {
        setBounds(lowerLeft, upperRight);
    }

    /// overload the equality operator
    virtual bool
    operator==(const Bounds<T> &other) const {
        return bounds[0] == other.bounds[0]
            && bounds[1] == other.bounds[1]
            && bounds[2] == other.bounds[2]
            && bounds[3] == other.bounds[3];
    }

    /// set the bounds from extents
    inline void
    setBounds(T minx, T miny, T maxx, T maxy) {
        if (minx > maxx) {
            throw STTException("The minimum X value is greater than the maximum X value");
        } else if (miny > maxy) {
            throw STTException("The minimum Y value is greater than the maximum Y value");
        }

        bounds[0] = minx;
        bounds[1] = miny;
        bounds[2] = maxx;
        bounds[3] = maxy;
    }

    /// set the bounds from lower left adn upper right coordinates
    inline void
    setBounds(const Coordinate<T> &lowerLeft, const Coordinate<T> &upperRight) {
        setBounds(lowerLeft.x, lowerLeft.y, upperRight.x, upperRight.y);
    }

    /// get the minimum X value
    inline T
    getMinX() const {
        return bounds[0];
    }

    /// get the minimum Y value
    inline T
    getMinY() const {
        return bounds[1];
    }

    /// get the maximum X value
    inline T
    getMaxX() const {
        return bounds[2];
    }

    /// get the maximum Y value
    inline T
    getMaxY() const {
        return bounds[3];
    }

    /// set the minimum X value
    inline void
    setMinX(T newValue) {
        if (newValue > getMaxX())
            throw STTException("The value is greater than the maximum X value");

        bounds[0] = newValue;
    }

    /// set the minimum Y value
    inline void
    setMinY(T newValue) {
        if (newValue > getMaxY())
            throw STTException("The value is greater than the maximum Y value");

        bounds[1] = newValue;
    }

    /// set the maximum X value
    inline void
    setMaxX(T newValue) {
        if (newValue < getMinX())
            throw STTException("The value is less than the minimum X value");

        bounds[2] = newValue;
    }

    /// set the maximum Y value
    inline void
    setMaxY(T newValue) {
        if (newValue < getMinY())
            throw STTException("The value is less than the minimum Y value");

        bounds[3] = newValue;
    }

    /// get the lower left corner
    inline Coordinate<T>
    getLowerLeft() const {
        return Coordinate<T>(getMinX(), getMinY());
    }

    /// get the lower right corner
    inline Coordinate<T>
    getLowerRight() const {
        return Coordinate<T>(getMaxX(), getMinY());
    }

    /// get the upper right corner
    inline Coordinate<T>
    getUpperRight() const {
        return Coordinate<T>(getMaxX(), getMaxY());
    }

    /// get the upper left corner
    inline Coordinate<T>
    getUpperLeft() const {
        return Coordinate<T>(getMinX(), getMaxY());
    }

    /// get the width
    inline T
    getWidth() const {
        return getMaxX() - getMinX();
    }

    /// get the height
    inline T
    getHeight() const {
        return getMaxY() - getMinY();
    }

    /// get the lower left quarter of the extents
    inline Bounds<T>
    getSW() const {
        return Bounds<T>(
            getMinX(),
            getMinY(),
            getMinX() + (getWidth() / 2),
            getMinY() + (getHeight() / 2)
        );
    }

    /// get the upper left quarter of the extents
    inline Bounds<T>
    getNW() const {
        return Bounds<T>(
            getMinX(),
            getMaxY() - (getHeight() / 2),
            getMinX() + (getWidth() / 2),
            getMaxY()
        );
    }

    /// get the upper right quarter of the extents
    inline Bounds<T>
    getNE() const {
        return Bounds<T>(
            getMaxX() - (getWidth() / 2),
            getMaxY() - (getHeight() / 2),
            getMaxX(),
            getMaxY()
        );
    }

    /// get the lower right quarter of the extents
    inline Bounds<T>
    getSE() const {
        return Bounds<T>(
            getMaxX() - (getWidth() / 2),
            getMinY(),
            getMaxX(),
            getMinY() + (getHeight() / 2)
        );
    }

    /// do these bounds overlap with another
    inline bool
    overlaps(const Bounds<T> *other) const {
        return overlaps(*other);
    }

    /// do these bounds overlap with another
    inline bool
    overlaps(const Bounds<T> &other) const {
        return getMinX() < other.getMaxX() && other.getMinX() < getMaxX() &&
            getMinY() < other.getMaxY() && other.getMinY() < getMaxY();
    }

private:
    /// the extents themselves as { minx, miny, maxx, maxy }
    T bounds[4];
};

#endif /* BOUNDS_H_ */
