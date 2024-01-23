/// Standard Library
#include <stdio.h>

/// Slacker Headers
#include "geometry.h"

/////////////////////////////////////////////////////////////
/// 				Private Functions
/////////////////////////////////////////////////////////////

/// @brief Allows a client to log itself to stdout
///
/// @param `this` The point to log: point->log(&point)
static void Point__log(Point *this)
{
#if (DEBUG)
	fprintf(stdout, "\nPoint: x%d\nPoint y: %d\n", this->x, this->y);
#endif
}

/// @brief Moves a point to a new location on the screen.
///
/// @param `this` The point to move: point->move(&point, 10, 10)
/// @param `dx` The new x coordinate.
/// @param `dy` The new y coordinate.
static void Point__move(Point *this, int32_t dx, int32_t dy)
{
	this->x = dx;
	this->y = dy;
}

/// @brief Compares two points for equality.
///
/// @param `this` The first point to compare.
/// @param `other` The second point to compare.
static bool Point__equal(Point *this, Point *other)
{
	return (this->x == other->x && this->y == other->y);
}

/////////////////////////////////////////////////////////////
/// 				Public Functions
/////////////////////////////////////////////////////////////

Point Point__new(int32_t dx, int32_t dy)
{
	Point p = { .x = dx, .y = dy };
	p.log = Point__log;
	p.move = Point__move;
	p.equal = Point__equal;
	return p;
}

Point Point__default()
{
	Point p = { .x = 0, .y = 0 };
	p.log = Point__log;
	p.move = Point__move;
	p.equal = Point__equal;
	return p;
}

#if (UNIT_TESTS)

#include <assert.h>

void Point__equal_test()
{
	Point p1 = Point__new(1, 1);
	Point p2 = Point__new(1, 1);
	Point p3 = Point__new(2, 2);
	assert(p1.equal(&p1, &p2));
	assert(!p1.equal(&p1, &p3));

	fprintf(stdout, "\n%s passed...\n", __PRETTY_FUNCTION__);
}

#endif // UNIT_TESTS
