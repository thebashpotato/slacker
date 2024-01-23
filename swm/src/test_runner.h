#ifndef SWM_TEST_RUNNER_H
#define SWM_TEST_RUNNER_H

#include "geometry.h"

#if (UNIT_TESTS)

/// @brief Runs all unit tests
void test_runner()
{
	Point__equal_test();
}

#endif // UNIT_TESTS

#endif // SWM_TEST_RUNNER_H
