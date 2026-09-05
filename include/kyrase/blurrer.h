#ifndef BLURRER_H_
#define BLURRER_H_

#include <string>
#include <vector>
#include <variant>
#include <utility> // std::move
#include <thread>
#include <algorithm>
#include <numeric>
#include <cmath> // std::round

#include "image.h"
#include "utils.h"

//#define MIN_WORK_PER_THREAD 

using std::string;
using std::vector;
using std::pair;
using std::thread;
using std::jthread;

class Blurrer {
public:
	/*
		BOX, // MULTITHREADED
		GAUSSIAN, //NAIVE 
		BINOMIAL, //NAIVE, TODO:: Passes?
		MEDIAN, //NAIVE
		LENS, //TODO
		BILATERAL, //TODO
		KAWASE, //TODO
		MOTION, //TODO
		GUIDED FILTER, //TODO
		ZOOM/RADIAL, //TODO
	*/

	// Box
	struct BoxSettings {
		int radius_x;
		int radius_y;
		int passes = 1;

		BoxSettings(
			int radius_x,
			int radius_y,
			int passes = 1
		)
			: radius_x(radius_x),
			radius_y(radius_y),
			passes(passes) {
		}
	};

	// Gaussian
	struct GaussianSettings {
		int radius_x;
		int radius_y;
		double sigma_x;
		double sigma_y;

		GaussianSettings(
			int radius_x,
			int radius_y,
			double sigma_x,
			double sigma_y
		)
			: radius_x(radius_x),
			radius_y(radius_y),
			sigma_x(sigma_x),
			sigma_y(sigma_y) {
		}

	};

	// Binomial
	struct BinomialSettings {
		int radius_x;
		int radius_y;

		BinomialSettings(
			int radius_x,
			int radius_y
		)
			: radius_x(radius_x),
			radius_y(radius_y) {
		}

	};

	// Median
	struct MedianSettings {
		int radius_x;
		int radius_y;

		MedianSettings(
			int radius_x,
			int radius_y
		)
			: radius_x(radius_x),
			radius_y(radius_y) {
		}

	};

	// Bilateral
	struct BilateralSettings {
		int radius_x;
		int radius_y;
		double sigma_x;
		double sigma_y;
		double sigma_s;

		BilateralSettings(
			int radius_x,
			int radius_y,
			double sigma_x,
			double sigma_y,
			double sigma_s
		)
			: radius_x(radius_x),
			radius_y(radius_y),
			sigma_x(sigma_x),
			sigma_y(sigma_y),
			sigma_s(sigma_s) {
		}

	};

	// Lens
	struct LensSettings {
		int radius;

		LensSettings(
			int radius
		)
			: radius(radius) {
		}

	};

	enum OptimizationLevel {
		SINGLE_NAIVE,
		SINGLE_OPT,
		MULTI_NAIVE,
		MULTI_OPT,
		GPU
	};



	// General
	struct BlurrerOptions {
		EdgeMode edge_mode;
		ConstantColor edge_constant;
		
		uint32_t num_threads;

		BlurrerOptions(
			EdgeMode edge_mode = EdgeMode::REFLECT101,
			ConstantColor edge_constant = {},
			uint32_t threads = 1
		);
	};

	using BlurrerSettings = std::variant<
		BoxSettings,
		GaussianSettings,
		BinomialSettings,
		MedianSettings,
		LensSettings
	>;

	struct BlurrerConfig {
		BlurrerSettings settings;
		BlurrerOptions options;

		BlurrerConfig(
			BlurrerSettings settings,
			BlurrerOptions options = {}
		)
			: settings(std::move(settings)),
			options(std::move(options)) {
		}
	};

	Image blur(
		const BlurrerConfig& config,
		const Image& image,
		OptimizationLevel optimization_level
	);

	// For non-modifiable images
	// Produces an entirely new Image
	// Naive solution, no optimizations
	// Overlapping regions get blurred once 
	Image blur_partial_naive(
		const BlurrerConfig& config,
		const Image& image,
		const vector<pair<pair<int, int>, pair<int, int>>>& blur_regions
	);

	// For modifiable images
	// Modifies the existing image's data
	void blur_modifiable(
		const BlurrerConfig& config, 
		Image& image,
		OptimizationLevel optimization_level
	);

	void blur_partial_modifiable_naive(
		const BlurrerConfig& config,
		Image& image,
		const vector<pair<pair<int, int>, pair<int, int>>>& blur_regions
	);

private:
	/*=========================================================================
	===========================================================================
	!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!MASTER HELPERS!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	===========================================================================
	=========================================================================*/
	
	// REGULAR!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	void box_single_naive(
		const BlurrerConfig& config,
		Image& image
	);

	void box_single_optimized(
		const BlurrerConfig& config,
		Image& image
	);

	void box_multi_naive(
		const BlurrerConfig& config,
		Image& image
	);

	void gaussian_single_naive(
		const BlurrerConfig& config,
		Image& image
	);

	void binomial_single_naive(
		const BlurrerConfig& config,
		Image& image
	);

	void median_single_naive(
		const BlurrerConfig& config,
		Image& image
	);

	void lens_single_naive(
		const BlurrerConfig& config,
		Image& image
	);

	// PARTIAL!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	
	// vector<<Row Starting, Row Ending>, <Column Starting, Column Ending>>
	// Add error handling for blur_regions
	// End exclusive <-- VERIFY THIS!!

	void box_partial_naive_single(
		const BlurrerConfig& config,
		Image& image,
		const vector<pair<pair<int, int>, pair<int, int>>>& blur_regions
	);

	/*=========================================================================
	===========================================================================
	!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!CORE HELPERS!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	===========================================================================
	=========================================================================*/

	/**
	 * @brief Called by each thread to calculate the output for its assigned range
	 * 
	 * @param[in] src_r Source red channel to read from
	 * @param[in] src_g Source green channel to read from
	 * @param[in] src_b Source blue channel to read from
	 * @param[in] start_col Start of the column range
	 * @param[in] end_col End of the column range (inclusive)
	 * @param[in] rows Width of the whole image
	 * @param[in] img_cols Height of the whole image
	 * @param[in] radius_x How many pixels to the left and right of the current
	 * pixel to use in calculations
	 * @param[in] radius_y How many pixels above or below the current pixel to use
	 * in calculations
	 * @param[in] kernel_size Total number of pixels the kernel uses
	 * @param[in] edge_mode How to handle image boundaries during the blur operation
	 * @param sums_r Working accumulator used internally
	 * @param sums_g Working accumulator used internally 
	 * @param sums_b Working accumulator used internally 
	 * @param[out] out_r Averaged red channel output
	 * @param[out] out_g Averaged red channel output
	 * @param[out] out_b Averaged red channel output
	 * @param[in] constant_edge_r Constant red value to use for constant edge mode
	 * @param[in] constant_edge_g Constant green value to use for constant edge mode
	 * @param[in] constant_edge_b Constant blue value to use for constant edge mode
	 * 
	 * @pre sums_r/g/b size == end_col - start_col + 1
	 */
	void box_multi_per_thread(
		// Sources
		const vector<uint8_t>& src_r,
		const vector<uint8_t>& src_g,
		const vector<uint8_t>& src_b, 
		
		// Columns we can touch
		uint32_t start_col, uint32_t end_col, 
		
		int rows, int img_cols, 
		int radius_x, int radius_y, 
		int kernel_size, 
		EdgeMode edge_mode,

		// Sum buffers
		vector<uint64_t>& sums_r, vector<uint64_t>& sums_g, vector<uint64_t>& sums_b,

		// Output writing
		vector<uint8_t>& out_r,	vector<uint8_t>& out_g, vector<uint8_t>& out_b,
		
		uint8_t constant_edge_r = 0, 
		uint8_t constant_edge_g = 0, 
		uint8_t constant_edge_b = 0
	);

	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// ROW-LEVEL ABSTRACTIONS!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	/**
	 * @brief Row-level abstraction of summation that selects the right pixel-level function
	 * 
	 * @param[out] sums_r Sum accumulator for each pixel in row for the red channel
	 * @param[out] sums_g Sum accumulator for each pixel in row for the green channel
	 * @param[out] sums_b Sum accumulator for each pixel in row for the blue channel
	 * @param[in] curr_row Row to calculate
	 * @param[in] radius_x How many pixels to the left and right of the current
	 * pixel to use in calculations
	 * @param[in] radius_y How many pixels above or below the current pixel to use
	 * in calculations
	 * @param[in] edge_mode How to handle image boundaries during the blur operation
	 * @param[in] src_r Source red channel to read from
	 * @param[in] src_g Source green channel to read from
	 * @param[in] src_b Source blue channel to read from
	 * @param[in] width Width of the whole image
	 * @param[in] height Height of the whole image
	 * @param[in] start_col Start of the column range
	 * @param[in] end_col End of the column range (inclusive)
	 * @param[in] constant_edge_r Constant red value to use for constant edge mode
	 * @param[in] constant_edge_g Constant green value to use for constant edge mode
	 * @param[in] constant_edge_b Constant blue value to use for constant edge mode
	 */
	void calculate_sum_row(
		vector<uint64_t>& sums_r, vector<uint64_t>& sums_g, vector<uint64_t>& sums_b,
		int curr_row, 
		int radius_x, int radius_y, 
		EdgeMode edge_mode,
		const vector<uint8_t>& src_r,
		const vector<uint8_t>& src_g,
		const vector<uint8_t>& src_b, 
		int height, int width, 
		int start_col, int end_col, 
		uint8_t constant_edge_r = 0, 
		uint8_t constant_edge_g = 0,
		uint8_t constant_edge_b = 0
	);

	/**
	 * @brief Calculates the kernel row sum when edge handling may be needed
	 * 
	 * @param[out] sum_r Red channel sum output accumulator (Reset at start)
	 * @param[out] sum_g Green channel sum output accumulator (Reset at start)
	 * @param[out] sum_b Blue channel sum output accumulator (Reset at start)
	 * @param[in] caller_idx Caller's 1D index
	 * @param[in] dx_begin Starting offset from the caller in x dimension
	 * @param[in] dx_end Ending offset from the caller in x dimension (inclusive)
	 * @param[in] dy Offset from the caller in y dimension
	 * @param[in] height Height of the whole image
	 * @param[in] width Width of the whole image
	 * @param[in] edge_mode How to handle image boundaries during the blur operation
	 * @param[in] src_r Source red channel to read from
	 * @param[in] src_g Source green channel to read from
	 * @param[in] src_b Source blue channel to read from
	 * @param[in] constant_edge_r Constant red value to use for constant edge mode
	 * @param[in] constant_edge_g Constant green value to use for constant edge mode
	 * @param[in] constant_edge_b Constant blue value to use for constant edge mode
	 */
	void calculate_kernel_row_sum(
		uint64_t& sum_r, uint64_t& sum_g, uint64_t& sum_b,
		int caller_idx,
		int dx_begin, int dx_end,
		int dy,
		int height, int width,
		EdgeMode edge_mode,
		const vector<uint8_t>& src_r,
		const vector<uint8_t>& src_g,
		const vector<uint8_t>& src_b,
		uint8_t constant_edge_r = 0, 
		uint8_t constant_edge_g = 0, 
		uint8_t constant_edge_b = 0
	);

	/**
	 * @brief Calculates the kernel row sum when all its pixels are known to be in range
	 * 
	 * @param[out] sum_r Red channel sum output accumulator (Reset at start)
	 * @param[out] sum_g Green channel sum output accumulator (Reset at start)
	 * @param[out] sum_b Blue channel sum output accumulator (Reset at start)
	 * @param[in] caller_idx Caller's 1D index
	 * @param[in] dx_begin Starting offset from the caller in x dimension 
	 * @param[in] dx_end Ending offset from the caller in x dimension (inclusive)
	 * @param[in] dy Offset from the caller in y dimension
	 * @param[in] width Width of the whole image
	 * @param[in] src_r Source red channel to read from
	 * @param[in] src_g Source green channel to read from
	 * @param[in] src_b Source blue channel to read from
	 */
	void calculate_kernel_row_sum_in_bounds(
		uint64_t& sum_r, uint64_t& sum_g, uint64_t& sum_b,
		int caller_idx,
		int dx_begin, int dx_end,
		int dy,
		int width,
		const vector<uint8_t>& src_r,
		const vector<uint8_t>& src_g,
		const vector<uint8_t>& src_b
	);

	/**
	 * @brief Row-level abstraction of setting output based on sum
	 *
	 * @param[out] output_r Output storage for the red channel
	 * @param[out] output_g Output storage for the green channel
	 * @param[out] output_b Output storage for the blue channel
	 * @param[in] sum_r Accumulated sums for the red channel
	 * @param[in] sum_g Accumulated sums for the green channel
	 * @param[in] sum_b Accumulated sums for the blue channel
	 * @param[in] divisor How many pixels were used to calculate per-pixel sums
	 * @param[in] row Row to work with
	 * @param[in] width Width of the whole image
	 * @param[in] start_col Start of the column range
	 * @param[in] end_col End of the column range (inclusive)
	 */
	void set_output_row(
		vector<uint8_t>& output_r,
		vector<uint8_t>& output_g,
		vector<uint8_t>& output_b,

		const vector<uint64_t>& sum_r,
		const vector<uint64_t>& sum_g,
		const vector<uint64_t>& sum_b, 
		
		uint32_t divisor, 
		int row, int width, 
		int start_col, int end_col
	);

	/**
	 * @brief Row-level abstraction of setting output based on sum
	 * Additionally finds divisor (for IGNORE edge mode)
	 * 
	 * @param[out] output_r Output storage for the red channel
	 * @param[out] output_g Output storage for the green channel
	 * @param[out] output_b Output storage for the blue channel
	 * @param[in] sum_r Accumulated sums for the red channel
	 * @param[in] sum_g Accumulated sums for the green channel
	 * @param[in] sum_b Accumulated sums for the blue channel
	 * @param[in] row Row to work with
	 * @param[in] radius_x How many pixels to the left and right of the current
	 * pixel to use in calculations
	 * @param[in] radius_y How many pixels above or below the current pixel to use
	 * in calculations
	 * @param[in] height Height of the whole image
	 * @param[in] width Width of the whole image
	 * @param[in] start_col Start of the column range
	 * @param[in] end_col End of the column range (inclusive)
	 */
	void set_output_row_find_divisor(
		// Set output info
		vector<uint8_t>& output_r,
		vector<uint8_t>& output_g,
		vector<uint8_t>& output_b,
		const vector<uint64_t>& sum_r,
		const vector<uint64_t>& sum_g,
		const vector<uint64_t>& sum_b, 
		// Divisor ignore info
		int row, 
		int radius_x, int radius_y, 
		int height, int width, 
		int start_col, int end_col
	);

	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// PIXEL-LEVEL ABSTRACTIONS!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	
	/**
	 * @brief Calculates unnormalized RGB kernel sum for one pixel by reusing
	 * above pixel's RGB kernel sum
	 *
	 * @param[out] sum_r Red channel sum output accumulator (Reset at start)
	 * @param[out] sum_g Green channel sum output accumulator (Reset at start)
	 * @param[out] sum_b Blue channel sum output accumulator (Reset at start)
	 * @param[in] row This pixel's row
	 * @param[in] col This pixel's column
	 * @param[in] radius_x How many pixels to the left and right of the current
	 * pixel to use in calculations
	 * @param[in] radius_y How many pixels above or below the current pixel to use
	 * in calculations
	 * @param[in] edge_mode How to handle image boundaries during the blur operation
	 * @param[in] src_r Source red channel to read from
	 * @param[in] src_g Source green channel to read from
	 * @param[in] src_b Source blue channel to read from
	 * @param[in] height Height of the whole image
	 * @param[in] width Width of the whole image
	 * @param[in] row_col_already_verified Set to false to verify row & col are valid
	 * @param[in] constant_edge_r Constant red value to use for constant edge mode
	 * @param[in] constant_edge_g Constant green value to use for constant edge mode
	 * @param[in] constant_edge_b Constant blue value to use for constant edge mode
	 *
	 */
	void calculate_sum_pixel_using_above(
		uint64_t& sum_r, uint64_t& sum_g, uint64_t& sum_b, 
		int row, int col, 
		int radius_x, int radius_y, 
		EdgeMode edge_mode,
		const vector<uint8_t>& src_r,
		const vector<uint8_t>& src_g,
		const vector<uint8_t>& src_b, 
		int height, int width,
		bool row_col_already_verified = false, 
		uint8_t constant_edge_r = 0,
		uint8_t constant_edge_g = 0, 
		uint8_t constant_edge_b = 0
	);

	/**
	 * @brief Calculates unnormalized RGB kernel sum for one pixel from scratch
	 * 
	 * @param[out] sum_r Red channel sum output accumulator (Reset at start)
	 * @param[out] sum_g Green channel sum output accumulator (Reset at start)
	 * @param[out] sum_b Blue channel sum output accumulator (Reset at start)
	 * @param[in] row This pixel's row
	 * @param[in] col This pixel's column
	 * @param[in] radius_x How many pixels to the left and right of the current
	 * pixel to use in calculations
	 * @param[in] radius_y How many pixels above or below the current pixel to use
	 * in calculations
	 * @param[in] edge_mode How to handle image boundaries during the blur operation
	 * @param[in] src_r Source red channel to read from
	 * @param[in] src_g Source green channel to read from
	 * @param[in] src_b Source blue channel to read from
	 * @param[in] height Height of the whole image
	 * @param[in] width Width of the whole image
	 * @param[in] row_col_already_verified Set to false to verify row & col are valid
	 * @param[in] constant_edge_r Constant red value to use for constant edge mode
	 * @param[in] constant_edge_g Constant green value to use for constant edge mode
	 * @param[in] constant_edge_b Constant blue value to use for constant edge mode
	 * 
	 */
	void calculate_sum_pixel(
		uint64_t& sum_r, uint64_t& sum_g, uint64_t& sum_b,
		int row, int col,
		int radius_x, int radius_y,
		EdgeMode edge_mode,
		const vector<uint8_t>& src_r,
		const vector<uint8_t>& src_g,
		const vector<uint8_t>& src_b,
		int height, int width,
		bool row_col_already_verified = false, 
		uint8_t constant_edge_r = 0, 
		uint8_t constant_edge_g = 0, 
		uint8_t constant_edge_b = 0
	);

	/**
	 * @brief Converts and stores an unnormalized sum into the final output for one pixel
	 * 
	 * @param[out] output_r Output storage for red channel
	 * @param[out] output_g Output storage for green channel
	 * @param[out] output_b Output storage for blue channel
	 * @param[in] sum_r Accumulated kernel sum of red channel
	 * @param[in] sum_g Accumulated kernel sum of green channel
	 * @param[in] sum_b Accumulated kernel sum of blue channel
	 * @param divisor How many pixels contributed to the sum accumulation
	 * 
 	 * @pre divisor > 0
	 * @pre sum_r, sum_g, and sum_b must each be <= UINT64_MAX - divisor / 2
	 * @pre (Each sum + divisor / 2) / divisor) <= UINT8_MAX
	 * 
	 */
	void set_output_pixel(
		uint8_t& output_r, uint8_t& output_g, uint8_t& output_b,
		uint64_t sum_r, uint64_t sum_g, uint64_t sum_b,
		uint32_t divisor
	);

	/**
	 * @brief Abstraction that combines get_divisor_ignore + set_output_pixel.
	 * Meant only to be used for IGNORE edge mode
	 * 
	 * @param[out] output_r Output storage for red channel 
	 * @param[out] output_g Output storage for green channel 
	 * @param[out] output_b Output storage for blue channel 
	 * @param[in] sum_r Accumulated kernel sum of red channel
	 * @param[in] sum_g Accumulated kernel sum of green channel
	 * @param[in] sum_b Accumulated kernel sum of blue channel
	 * @param[in] row This pixel's row
	 * @param[in] col This pixel's column
	 * @param[in] radius_x How many pixels to the left and right of the current
	 * pixel to use in calculations
	 * @param[in] radius_y How many pixels above or below the current pixel to use
	 * in calculations
	 * @param[in] height Height of the whole image
	 * @param[in] width Width of the whole image
	 * 
	 * @pre width > 0 and height > 0
	 * @pre 0 <= col < width and 0 <= row < height
	 * @pre radius_x >= 0 and radius_y >= 0
	 * 
	 * @see get_divisor_ignore
	 * @see set_output_pixel
	 */
	void set_output_pixel_find_divisor(
		// Set output info
		uint8_t& output_r, uint8_t& output_g, uint8_t& output_b,
		uint64_t sum_r, uint64_t sum_g, uint64_t sum_b,
		// Divisor ignore info
		int row, int col,
		int radius_x, int radius_y, 
		int height, int width
	);

	/*=========================================================================
	===========================================================================
	!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!SMALLER HELPERS!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	===========================================================================
	=========================================================================*/

	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// EDGE / BOUNDS HELPERS!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	/**
	* @brief Calculates divisor for one output pixel
	*
	 * @param[in] row Row index of this pixel
	 * @param[in] col Column index of this pixel
	 * @param[in] radius_x How many pixels to the left and right of the current
	 * pixel to use in calculations
	 * @param[in] radius_y How many pixels above or below the current pixel to use
	 * in calculations
 	 * @param[in] height Height of the whole image
	 * @param[in] width Width of the whole image
	 * 
	 * @return How many pixels contributed towards the sum (were not out of bounds)
	 * 
	 * @pre width > 0 and height > 0
	 * @pre 0 <= col < width and 0 <= row < height
	 * @pre radius_x >= 0 and radius_y >= 0
	 * 
	 * @note This function is intended to be used for IGNORE edge mode
	 */
	uint32_t get_divisor_ignore(
		int row, int col, 
		int radius_x, int radius_y, 
		int height, int width
	);

	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// SUPPORTING HELPERS!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	/**
	* @brief Ensures the inputs passed to the blurring operation are valid
	 * 
	 * @param[in] mode How to handle image boundaries during the blur operation
	 * @param[in] radius_x How many pixels to the left and right of the current
	 * pixel to use in calculations
	 * @param[in] radius_y How many pixels above or below the current pixel to use
	 * in calculations
	 * @param[in] passes How many times to repeat the operation
	 * @param[in] width Width of the whole image
	 * @param[in] height Height of the whole image
	 * 
	 * @throws std::invalid_argument If radius_x or radius_y is negative
	 * @throws std::invalid_argument If passes is less than 1
	 * @throws std::invalid_argument if the radii exceed respective image dimensions
	 * 
	 */
	void verify_blur_input(
		EdgeMode mode, 
		int radius_x, int radius_y, 
		int passes, 
		int width, int height
	) const;

	/**
	* @brief Finds the first and last columns owned by the given thread
	*
	* @param[out] col_begin_buf Output destination for the x index of the first
	column this thread owns
	* @param[out] col_end_buf Output destination for the x index of the last
	 column this thread owns
	 * @param[in] thread_num Zero-based index of the current thread
	 * @param[in] total_threads Total number of threads used in
	 the blurring operation
	 * @param[in] total_cols Total amount of columns in
	 the picture (width of the image)
	 * 
	 * @pre total_cols > 0
	 * @pre thread_num < total_threads
	 * @pre total_threads <= total_cols
	 * 
	 * @post col_begin_buf <= col_end_buf
	 * 
	 * @warning col_end_buf is inclusive. Using it as exclusive will cause off-by-one errors
	 */
	void get_col_range_thread(
		uint32_t& col_begin_buf, uint32_t& col_end_buf, 
		uint32_t thread_num, uint32_t total_threads, 
		int total_cols
	);

	/**
	* @brief Extracts values from source vectors and adds these values
	* to the sum accumulators of the respective channels
	*
	 * @param[in,out] sum_r Sum accumulator for the red channel
	 * @param[in,out] sum_g Sum accumulator for the green channel
	 * @param[in,out] sum_b Sum accumulator for the blue channel
	 * @param[in] r Source vector for the red channel
	 * @param[in] g Source vector for the blue channel 
	 * @param[in] b Source vector for the green channel 
	 * @param[in] idx 1D index of the target value
	 * 
	 * @pre all sum accumulators <= UINT64_MAX - respective val
	 * @pre idx must be a valid index for r, g, and b
	 * 
	 * @post Each sum accumulator is increased by its corresponding value
	 * 
 	 * @warning This function does not do validation. 
	 * 
	 * @see add_rgb_sums 
	 */
	void add_rgb_sums_at_idx(
		uint64_t& sum_r,
		uint64_t& sum_g,
		uint64_t& sum_b,
		const vector<uint8_t>& r,
		const vector<uint8_t>& g,
		const vector<uint8_t>& b, 
		size_t idx
	);

	/**
	* @brief Adds provided values to the sum accumulators of
	 the respective channels
	 * 
	 * @param[in,out] sum_r Sum accumulator for the red channel
	 * @param[in,out] sum_g Sum accumulator for the green channel
	 * @param[in,out] sum_b Sum accumulator for the blue channel
	 * @param[in] r_val Value to add to the red channel
	 * @param[in] g_val Value to add to the blue channel
	 * @param[in] b_val Value to add to the green channel
	 * 
	 * @post Each sum accumulator is increased by its corresponding value
	 * 
  	 * @warning This function does not do validation.
	 */
	void add_rgb_sums(
		uint64_t& sum_r, uint64_t& sum_g, uint64_t& sum_b, 
		uint8_t r_val, uint8_t g_val, uint8_t b_val
	);



/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!UNSORTED!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/


// GAUSSIAN--------------------------------------------------------------------


	void calculate_out_pixel_weighted(
		uint8_t& out_r, uint8_t& out_g, uint8_t& out_b,
		int row, int col,
		int radius_x, int radius_y,
		const vector<double>& weights_x, const vector<double>& weights_y,
		EdgeMode edge_mode,
		const vector<uint8_t>& src_r,
		const vector<uint8_t>& src_g,
		const vector<uint8_t>& src_b,
		int height, int width,
		bool row_col_already_verified = false,
		uint8_t constant_edge_r = 0,
		uint8_t constant_edge_g = 0,
		uint8_t constant_edge_b = 0
	);

	double get_sum_participating_weights_ignore(
		int caller_idx, 
		int radius_x, int radius_y,
		int height, int width,
		const vector<double>& weights_x, const vector<double>& weights_y
	);

	void calculate_gaussian_weights_normalized(
		// Storage
		vector<double>& x_weights_store, vector<double>& y_weights_store,
		double sigma_x, double sigma_y,
		int radius_x, int radius_y
	);

	void calculate_gaussian_weights_raw(
		// Storage
		vector<double>& x_weights_store, vector<double>& y_weights_store,
		double sigma_x, double sigma_y,
		int radius_x, int radius_y
	);

	void calculate_gaussian_weights_1d(
		vector<double>& i_weights_store,
		double sigma_i,
		int radius_i
	);

	// Works for both Gaussian and Binomial
	// Binomial doesn't need it though, but it works.
	void normalize_weights(
		vector<double>& x_weights_store, vector<double>& y_weights_store,
		size_t size_x, size_t size_y
	);

	void calculate_binomial_normalized_weights(
		vector<double>& x_weights_store, vector<double>& y_weights_store,
		int radius_x, int radius_y
	);

	// Uses multiplicative recurrence for binomial coefficients to get 
	// weights that are already normalized
	void calculate_binomial_normalized_weights_1d(
		vector<double>& i_weights_store,
		int radius_i
	);

	void fill_kernel_temps(
		vector<uint8_t>& kernel_pixels_r,
		vector<uint8_t>& kernel_pixels_g,
		vector<uint8_t>& kernel_pixels_b,
		int caller_idx,
		int radius_x, int radius_y,
		int height, int width,
		const vector<uint8_t>& src_r,
		const vector<uint8_t>& src_g,
		const vector<uint8_t>& src_b,
		EdgeMode edge_mode,
		uint8_t constant_edge_r = 0,
		uint8_t constant_edge_g = 0,
		uint8_t constant_edge_b = 0
	);

	// MUST NOT PASS CHUNKS OF ACTUAL KERNEL, USE BUFFER
	// inclusive end
	void find_kernel_median_rgb(
		vector<uint8_t>& kernel_pixels_r,
		vector<uint8_t>& kernel_pixels_g,
		vector<uint8_t>& kernel_pixels_b,
		int start_idx, int end_idx,
		uint8_t& median_r, uint8_t& median_g, uint8_t& median_b
	);

	uint8_t find_kernel_median_1d(
		vector<uint8_t>& kernel_pixels,
		int start_idx, int end_idx
	);

	void calculate_out_pixel_lens(
		uint8_t& out_r, uint8_t& out_g, uint8_t& out_b,
		int radius,
		int caller_idx,
		int height, int width,
		int total_weights,
		const vector<uint8_t>& src_r,
		const vector<uint8_t>& src_g,
		const vector<uint8_t>& src_b,
		EdgeMode edge_mode,
		uint8_t constant_edge_r = 0,
		uint8_t constant_edge_g = 0,
		uint8_t constant_edge_b = 0
	);

	int calculate_total_weights_lens(int radius);

	void add_from_src_all_dir(
		uint64_t& sum_r, uint64_t& sum_g, uint64_t& sum_b,
		int caller_idx,
		int dx, int dy,
		int width,
		const vector<uint8_t>& src_r,
		const vector<uint8_t>& src_g,
		const vector<uint8_t>& src_b
	);

	int add_from_src_all_dir_figure_oob (
		uint64_t& sum_r, uint64_t& sum_g, uint64_t& sum_b,
		int caller_idx,
		int dx, int dy,
		EdgeMode edge_mode,
		int height, int width,
		const vector<uint8_t>& src_r,
		const vector<uint8_t>& src_g,
		const vector<uint8_t>& src_b,
		uint8_t constant_edge_r,
		uint8_t constant_edge_g,
		uint8_t constant_edge_b
	);

	void add_from_src(
		uint64_t& r, uint64_t& g, uint64_t& b,
		uint8_t rta, uint8_t gta, uint8_t bta
	);

	int add_from_src_figure_oob(
		uint64_t& sum_r, uint64_t& sum_g, uint64_t& sum_b,
		int caller_idx,
		int dx, int dy,
		EdgeMode edge_mode,
		int height, int width,
		const vector<uint8_t>& src_r,
		const vector<uint8_t>& src_g,
		const vector<uint8_t>& src_b,
		uint8_t constant_edge_r,
		uint8_t constant_edge_g,
		uint8_t constant_edge_b
	);

	void calculate_sigma_from_radius_gaussian_xy(
		double& sigma_x, double& sigma_y,
		int radius_x, int radius_y
	);

	double calculate_sigma_from_radius_gaussian_1d(int radius);
};


#endif
