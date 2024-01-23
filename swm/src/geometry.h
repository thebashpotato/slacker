
#ifndef SWM_GEOMETRY_H
#define SWM_GEOMETRY_H

/// Standard Library
#include <stdint.h>
#include <stdbool.h>

/// @brief Represents a 2D point in space.
///
/// @details Will be used inside a Rect to represent the coordinates of a Client window.
typedef struct Point Point;

/// Member functions
typedef void (*PointLogFunction)(Point *this);
typedef void (*PointMove)(Point *this, int32_t dx, int32_t dy);
typedef bool (*PointEqual)(Point *this, Point *other);

struct Point {
	int32_t x;
	int32_t y;
	// Member functions
	PointLogFunction log;
	PointMove move;
	PointEqual equal;
};

/// @brief Creates a new point based of the given x and y coordinates
///
/// @param `dx` The x coordinate
/// @param `dy` The y coordinate
Point Point__new(int32_t dx, int32_t dy);

/// @brief Creates a new point with the default values of 0, 0
Point Point__default();

#if (UNIT_TESTS)

void Point__equal_test();

#endif // UNIT_TESTS

#endif // SWM_GEOMETRY_H
