// utils.h

#ifndef UTILS_H_
#define UTILS_H_

#include <cstdint>
#include <cstddef>
#include <stdexcept>

enum class EdgeMode {
	REFLECT101,
	REFLECT,
	CLAMP,
	WRAP,
	CONSTANT,
	IGNORE
};

struct ConstantColor {
	uint8_t r = 0;
	uint8_t g = 0;
	uint8_t b = 0;

	ConstantColor(
		uint8_t r = 0,
		uint8_t g = 0,
		uint8_t b = 0
	)
		: r(r),
		g(g),
		b(b) {
	}
};


// Bounds ------------------------------------------------------------

/**
 * @brief Checks whether the target (offset pixel) is out of bounds
 *
 * @param caller_idx Caller's 1D index (caller = pixel from which to offset)
 * @param[in] dx Offset from the caller in x dimension
 * @param[in] dy Offset from the caller in y dimension
 * @param[in] height Height of the whole image
 * @param[in] width Width of the whole image
 *
 * @return true if target pixel is out of bounds, false otherwise
 */
bool is_outside(
	int caller_idx,
	int dx, int dy,
	int height, int width
);

/**
 * @brief Checks whether all pixels in the kernel's radius are in bounds
 *
 * @param caller_idx Caller's 1D index (caller = pixel from which to offset)
 * @param[in] radius_x How many pixels to the left and right of the current
 * pixel to use in calculations
 * @param[in] radius_y How many pixels above or below the current pixel to use
 * in calculations
 * @param[in] height Height of the whole image
 * @param[in] width Width of the whole image
 *
 * @return true if all kernel pixels are in bounds, false otherwise
 */
bool is_kernel_all_inside(
	int caller_idx,
	int radius_x, int radius_y,
	int height, int width
);


// Edge mapping ------------------------------------------------------

/**
 * @brief Finds the mapped in-bounds 1D index of the target position
 * based on the offsets in both dimensions from the caller pixel
 *
 * @param[in] edge_mode How to handle image boundaries during the blur operation
 * @param[in] caller_idx Caller's 1D index (pixel offset is calculated from)
 * @param[in] dx Offset from the caller in x dimension
 * @param[in] dy Offset from the caller in y dimension
 * @param[in] width Width of the whole image
 * @param[in] height Height of the whole image
 *
 * @return Mapped in-bounds 1D index of the target position
 */
size_t get_mapped_idx_naive(
	EdgeMode edge_mode,
	int caller_idx,
	int dx, int dy,
	int width, int height
);

/**
 * @brief Calculates mapped index in the given dimension for reflection101 edge mode
 *
 * @param[in] caller_i Caller's index in the given dimension
 * @param[in] delta Offset from caller in the given dimension
 * @param[in] dimension_size Size of the image in the given dimension
 *
 * @return Mapped in-bounds index of the target
 */
int calculate_reflection101_per_d(
	int caller_i,
	int delta,
	int dimension_size
);

/**
 * @brief Calculates mapped index in the given dimension for reflection edge mode
 *
 * @param[in] caller_i Caller's index in the given dimension
 * @param[in] delta Offset from caller in the given dimension
 * @param[in] dimension_size Size of the image in the given dimension
 *
 * @return Mapped in-bounds index of the target
 */
int calculate_reflection_per_d(
	int caller_i,
	int delta,
	int dimension_size
);

/**
 * @brief Calculates mapped index in the given dimension for clamp edge mode
 *
 * @param[in] caller_i Caller's index in the given dimension
 * @param[in] delta Offset from caller in the given dimension
 * @param[in] dimension_size Size of the image in the given dimension
 *
 * @return Mapped in-bounds index of the target
 */
int calculate_clamp_per_d(
	int caller_i,
	int delta,
	int dimension_size
);

/**
 * @brief Calculates mapped index in the given dimension for wrap edge mode
 *
 * @param[in] caller_i Caller's index in the given dimension
 * @param[in] delta Offset from caller in the given dimension
 * @param[in] dimension_size Size of the image in the given dimension
 *
 * @return Mapped in-bounds index of the target
 */
int calculate_wrap_per_d(
	int caller_i,
	int delta,
	int dimension_size
);


// General image indexing -------------------------------------------

/**
 * @brief Converts 2D coordinates to a 1D index using image's width
 * @param y Pixel's y index (row)
 * @param x Pixel's x index (column)
 * @param width Full width of the image
 * @return Corresponding 1D index of the caller
 *
 * @pre y and x must be >= 0 and within the image's dimensions.
 * width must match the image's width
 */
size_t convert_yx_to_idx(int y, int x, int width);

int get_curr_idx(int caller_idx, int dy, int dx, int width);

#endif