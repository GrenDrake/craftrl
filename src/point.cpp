#include <cmath>
#include <sstream>
#include <string>
#include "world.h"

const Point nowhere(-1, -1);

std::string Point::toString() const {
    std::stringstream s;
    s << '(' << x << ',' << y << ')';
    return s.str();
}

Point Point::shift(Dir dir, int amnt) const {
    switch (dir) {
        case Dir::North:        return Point(x,         y - amnt);
        case Dir::Northeast:    return Point(x + amnt,  y - amnt);
        case Dir::East:         return Point(x + amnt,  y       );
        case Dir::Southeast:    return Point(x + amnt,  y + amnt);
        case Dir::South:        return Point(x,         y + amnt);
        case Dir::Southwest:    return Point(x - amnt,  y + amnt);
        case Dir::West:         return Point(x - amnt,  y       );
        case Dir::Northwest:    return Point(x - amnt,  y - amnt);
        default:                return *this;
    }
}

Dir Point::directionTo(const Point &rhs) const {
    if (rhs.x < x) {
        if (rhs.y < y)      return Dir::Northwest;
        else if (rhs.y > y) return Dir::Southwest;
        else                return Dir::West;
    } else if (rhs.x > x) {
        if (rhs.y < y)      return Dir::Northeast;
        else if (rhs.y > y) return Dir::Southeast;
        else                return Dir::East;
    } else {
        if (rhs.y < y)      return Dir::North;
        else if (rhs.y > y) return Dir::South;
        else                return Dir::None;
    }
}

double Point::distance(const Point &rhs) const {
    return sqrt((rhs.x - x) * (rhs.x - x) + (rhs.y - y) * (rhs.y - y));
}

bool Point::operator==(const Point &rhs) const {
    return rhs.x == x && rhs.y == y;
}

Dir rotate45(Dir d) {
    switch(d) {
        case Dir::North:        return Dir::Northeast;
        case Dir::Northeast:    return Dir::East;
        case Dir::East:         return Dir::Southeast;
        case Dir::Southeast:    return Dir::South;
        case Dir::South:        return Dir::Southwest;
        case Dir::Southwest:    return Dir::West;
        case Dir::West:         return Dir::Northwest;
        case Dir::Northwest:    return Dir::North;
        default:                return Dir::None;
    }
    return Dir::None;
}

Dir unrotate45(Dir d) {
    switch(d) {
        case Dir::North:        return Dir::Northwest;
        case Dir::Northeast:    return Dir::North;
        case Dir::East:         return Dir::Northeast;
        case Dir::Southeast:    return Dir::East;
        case Dir::South:        return Dir::Southeast;
        case Dir::Southwest:    return Dir::South;
        case Dir::West:         return Dir::Southwest;
        case Dir::Northwest:    return Dir::West;
        default:                return Dir::None;
    }
    return Dir::None;
}
