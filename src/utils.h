#ifndef SLACKER_UTILS_H
#define SLACKER_UTILS_H

#include <stdlib.h>

/// @brief Generic purpose macros and helper functions

#define MAX(A, B) ((A) > (B) ? (A) : (B))
#define MIN(A, B) ((A) < (B) ? (A) : (B))
#define BETWEEN(X, A, B) ((A) <= (X) && (X) <= (B))
#define LENGTH(X) (sizeof X / sizeof X[0])


/// @brief Print error message and exit the window manager
void die(const char *fmt, ...);

/// @brief Allocate memory and check for errors
void *ecalloc(size_t nmemb, size_t size);

#endif// SLACKER_UTILS_H
