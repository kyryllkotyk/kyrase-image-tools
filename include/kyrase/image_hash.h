#ifndef IMAGE_HASH_H_
#define IMAGE_HASH_H_

#include <cstdint>
#include <vector>

#include "image.h"

using std::vector;

constexpr uint64_t FNV_OFFSET = 14695981039346656037ull;
constexpr uint64_t FNV_PRIME = 1099511628211ull;

uint64_t hash_image_fnv1a(const Image& image);

#endif