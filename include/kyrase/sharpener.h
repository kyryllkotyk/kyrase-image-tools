#ifndef SHARPENER_H_
#define SHARPENER_H_

#include <string>
#include <vector>
#include <variant> // std::get

#include "blurrer.h"
#include "utils.h"

using std::string;
using std::vector;

using BlurrerConfig = Blurrer::BlurrerConfig;

class Sharpener {
public:
	struct UnsharpMaskSettings {
		BlurrerConfig& blurrer_config;
		Blurrer::OptimizationLevel blurrer_optimization_level;
		double sharp_amount;
		uint8_t threshold;

		UnsharpMaskSettings(
			BlurrerConfig& blurrer_config,
			Blurrer::OptimizationLevel blurrer_optimization_level,
			double sharp_amount,
			uint8_t threshold
		)
			: blurrer_config(blurrer_config),
			blurrer_optimization_level(blurrer_optimization_level),
			sharp_amount(sharp_amount),
			threshold(threshold) {
		}
	};

	struct HighPassSettings {
		Image blurred_image;
		double sharp_amount;
		uint8_t threshold;

		HighPassSettings(Image blurred_image, double sharp_amount, uint8_t threshold)
			: blurred_image(blurred_image), 
			sharp_amount(sharp_amount), threshold(threshold) {
		};
	};

	struct ConvolutionalSettings {
		vector<double> weight_kernel;
		int radius_x, radius_y;

		ConvolutionalSettings(vector<double> weight_kernel, int radius_x, int radius_y)
			: weight_kernel(weight_kernel), radius_x(radius_x), radius_y(radius_y) {
		};
	};


	struct SharpenerOptions {
		EdgeMode edge_mode;
		ConstantColor edge_constant;

		uint32_t num_threads;

		SharpenerOptions(
			EdgeMode edge_mode = EdgeMode::REFLECT101,
			ConstantColor edge_constant = {},
			uint32_t threads = 1
		) : edge_mode(edge_mode), edge_constant(edge_constant), num_threads(threads) {
		};
	};

	using SharpenerSettings = std::variant<
		UnsharpMaskSettings,
		HighPassSettings,
		ConvolutionalSettings
	>;

	struct SharpenerConfig {
		SharpenerSettings settings;
		SharpenerOptions options;

		SharpenerConfig(
			SharpenerSettings settings,
			SharpenerOptions options = {}
		)
			: settings(std::move(settings)),
			options(std::move(options)) {
		}
	};

	enum SharpenOptimizationLevel {
		SINGLE_NAIVE,
		SINGLE_OPT,
		MULTI_NAIVE,
		MULTI_OPT,
		GPU
	};

	//TODO:: Create simple mode (creates the blurring config for the user)
	//Image sharpen(
	//	const SharpenerConfig& config,
	//	const Image& image,
	//	SharpenOptimizationLevel optimization_level
	//);

	Image sharpen(
		const SharpenerConfig& config,
		const Image& image,
		SharpenOptimizationLevel optimization_level
	);

	// For modifiable images
	// Modifies the existing image's data
	void sharpen_modifiable(
		const SharpenerConfig& config,
		Image& image,
		SharpenOptimizationLevel optimization_level
	);


private:
	void high_pass_sharpen_naive(
		Image& original_image,
		const Image& blurred_image,
		double sharp_amount,
		uint8_t threshold
	);

	void convolutional_sharpen_naive(
		Image& original_image,
		const SharpenerConfig& config
	);


	void high_pass_channels_rgb(
		const Image& original_image,
		const Image& blurred_image,
		vector<uint8_t>& out_r,
		vector<uint8_t>& out_g,
		vector<uint8_t>& out_b,
		double sharp_amount,
		uint8_t threshold
	);

	uint8_t high_pass_channel(
		uint8_t original, uint8_t blurred,
		double sharp_amount,
		uint8_t threshold
	);



	void calculate_convolution_with_weights(
		vector<uint8_t>& out_r, vector<uint8_t>& out_g, vector<uint8_t>& out_b,
		
		int height, int width,

		const vector<uint8_t>& src_r,
		const vector<uint8_t>& src_g,
		const vector<uint8_t>& src_b,

		const vector<double>& weight_kernel,

		int radius_x, int radius_y,

		EdgeMode edge_mode,

		uint8_t constant_edge_r = 0,
		uint8_t constant_edge_g = 0,
		uint8_t constant_edge_b = 0
	);

	// Go through kernel for one pixel and get the weighted sum
	// radii must match weight kernel 
	// Clamps to [0, 255]
	void calculate_convolution_with_weights_pixel(
		uint8_t& out_r, uint8_t& out_g, uint8_t& out_b,

		int caller_idx,
		int height, int width,

		const vector<uint8_t>& src_r,
		const vector<uint8_t>& src_g,
		const vector<uint8_t>& src_b,

		const vector<double>& weight_kernel,

		int radius_x, int radius_y,

		EdgeMode edge_mode,

		uint8_t constant_edge_r = 0,
		uint8_t constant_edge_g = 0,
		uint8_t constant_edge_b = 0
	);

	uint8_t convert_accumulator_out_1d(double accum);

	void accumulate_rgb(
		double& accum_r, double& accum_g, double& accum_b,
		uint8_t pixel_r, uint8_t pixel_g, uint8_t pixel_b,
		double weight
	);
};

/*
Helper that goes across the image/pixels.
Pixel helper goes through neighboring pixels.
Multiply neighboring values by their respective kernel weights.
Accumulate.
Apply edge handling.
Round and clamp result to [0,255].
*/

#endif
