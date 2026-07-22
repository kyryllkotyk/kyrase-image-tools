#ifndef BLURRER_H_
#define BLURRER_H_

#include <string>
#include <vector>
#include <variant>
#include <utility> // std::move

#include "image.h"

using std::string;
using std::vector;

class Blurrer {
public:
	/*
		BOX,
		GAUSSIAN, //TODO
		BINOMIAL, //TODO
		MEDIAN, //TODO
		MOTION, //TODO
		KAWASE //TODO
	*/


	// Box
	struct BlurrerBoxSettings {
		int radius_x;
		int radius_y;
		int passes = 1;

		BlurrerBoxSettings(
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
	struct BlurrerGaussianSettings {

	};

	// Binomial
	struct BlurrerBinomialSettings {

	};

	enum class BlurrerEdgeMode {
		REFLECT101,
		REFLECT,
		CLAMP,
		WRAP,
		CONSTANT,
		IGNORE,
		REFLECT_CALLER
	};

	struct BlurrerConstantColor {
		uint8_t r = 0;
		uint8_t g = 0;
		uint8_t b = 0;

		BlurrerConstantColor(
			uint8_t r = 0,
			uint8_t g = 0,
			uint8_t b = 0
		)
			: r(r),
			g(g),
			b(b) {
		}
	};

	// General
	struct BlurrerOptions {
		BlurrerEdgeMode edge_mode;
		BlurrerConstantColor edge_constant;

		BlurrerOptions(
			BlurrerEdgeMode edge_mode = BlurrerEdgeMode::REFLECT101,
			BlurrerConstantColor edge_constant = {}
		)
			: edge_mode(edge_mode),
			edge_constant(edge_constant) {
		}
	};

	using BlurrerSettings = std::variant<
		BlurrerBoxSettings,
		BlurrerGaussianSettings,
		BlurrerBinomialSettings
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

	// For non-modifiable images
	// Produces an entirely new Image
	// Naive solution, no optimizations
	Image blur_naive(const BlurrerConfig& config, const Image& image);

	// For modifiable images
	// Modifies the existing image's data
	void blur_naive_modifiable(const BlurrerConfig& config, Image& image);

private:
	//TODO:: Handle passes. Currently it's validated but unused
	void box_blur_naive(
		const BlurrerConfig& config,
		Image& image
	);

	void verify_blur_input(
		const BlurrerEdgeMode mode,
		const int radius_x,
		const int radius_y,
		const int passes,
		const int width,
		const int height
	) const;

	bool is_outside(
		const int caller_idx,
		const int dx,
		const int dy,
		const int height,
		const int width
	);

	void add_rgb_sums(
		int& sum_r,
		int& sum_g,
		int& sum_b,
		const uint8_t r_val,
		const uint8_t g_val,
		const uint8_t b_val
	);

	void add_rgb_sums_at_idx(
		int& sum_r,
		int& sum_g,
		int& sum_b,
		const vector<uint8_t>& r,
		const vector<uint8_t>& g,
		const vector<uint8_t>& b,
		const size_t idx
	);

	size_t get_mapped_idx_naive(
		const BlurrerEdgeMode edge_mode,
		const int caller_idx,
		const int dx,
		const int dy,
		const int width,
		const int height
	) const;

	int calculate_reflection101_per_d(
		const int caller_i,
		const int delta,
		const int dimension_size
	) const;

	int calculate_reflection_per_d(
		const int caller_i,
		const int delta,
		const int dimension_size
	) const;

	int calculate_caller_reflection_per_d(
		const int caller_i,
		const int delta,
		const int dimension_size
	) const;

	int calculate_clamp_per_d(
		const int caller_i,
		const int delta,
		const int dimension_size
	) const;

	int calculate_wrap_per_d(
		const int caller_i,
		const int delta,
		const int dimension_size
	) const;

	size_t convert_yx_to_idx(
		const int y,
		const int x,
		const int width
	) const;
};



#endif

// Box:
// N = horizontal radius
// M = vertical radius



// Gaussian:
// Constants: N radius, o = Multiplier
// 
// Buffered updating (Use original for read, and second for write, swap at the end)
// 
// Go through each pixel PER CHANNEL:
//	The NxN window is centered on this pixel
//	For all the neighbors: 
//		x = column offset from this pixel
//		y = row offset from this pixel
// 
//		Gx(x) = e^(-x^2 / (2o^2))
//		Gy(y) = e^(-y^2 / (2o^2))
//		G(x, y) = Gx(x) * Gy(y)
//		This G is the weight
//		new_board's this pixel channel += (G * this pixel's channel's value)
// 
//	So new pixel = sum of (neighbor's values * their weight (G))

// Mirroring edges:
// -1 -> 1
// Size 8, Desired Index = 10, New Index = 4
// (because final index is size - 1)
// Effectively, size - 2 - (N - size) = 2 * size - 2 - N

// So:
// >= Size -> 2 * size - 2 - N
// < 0 -> * -1 
/*
period = 2 * size - 2
m = index mod period

if m < 0:
	m += period

if m < size:
	reflected = m
else:
	reflected = period - m
*/