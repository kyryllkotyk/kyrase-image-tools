#include "../include/kyrase/sharpener.h"

Image Sharpener::sharpen(
	const SharpenerConfig& config,
	const Image& image,
	SharpenOptimizationLevel optimization_level
) {
	Image return_image = image;

	// As to not duplicate code
	sharpen_modifiable(
		config,
		return_image,
		optimization_level
	);

	return return_image;

}

// For modifiable images
// Modifies the existing image's data
void Sharpener::sharpen_modifiable(
	const SharpenerConfig& config,
	Image& image,
	SharpenOptimizationLevel optimization_level
) {
	// Figure out the sharpening option 
	switch (config.settings.index()) {
	case(0): { // Unsharp mask
		const UnsharpMaskSettings& settings = get<0>(config.settings);

		// Get the blurred image
		Blurrer blurrer;
		Image blurred_image = blurrer.blur(
			settings.blurrer_config, image, settings.blurrer_optimization_level
		);

		// Call the appropriate function based on optimization level requested
		switch (optimization_level) {
		case(SINGLE_NAIVE): {
			high_pass_sharpen_naive(
				image,
				blurred_image,
				settings.sharp_amount,
				settings.threshold
			);

			break;
		}

		default: // Something is wrong
			throw std::invalid_argument(
				"Sharpener: Invalid optimization level requested"
			);
		}

		break;
	}

	case(1): { // High pass
		const HighPassSettings& settings = get<1>(config.settings);

		// Call the appropriate function based on optimization level requested
		switch (optimization_level) {
		case(SINGLE_NAIVE): {
			high_pass_sharpen_naive(
				image, 
				settings.blurred_image, 
				settings.sharp_amount,
				settings.threshold
			);

			break;
		}

		default: // Something is wrong
			throw std::invalid_argument(
				"Sharpener: Invalid optimization level requested"
			);
		}

		break;
	}

	case(2): { // Convolutional 

		// Call the appropriate function based on optimization level requested
		switch (optimization_level) {
		case(SINGLE_NAIVE): {
			convolutional_sharpen_naive(image, config);

			break;
		}

		default: // Something is wrong
			throw std::invalid_argument(
				"Sharpener: Invalid optimization level requested"
			);
		}

		break;
	}
	default: // Something is wrong
		throw std::invalid_argument(
			"Sharpener: Invalid sharpening option requested"
		);
	}
}

void Sharpener::high_pass_sharpen_naive(
	Image& original_image,
	const Image& blurred_image,
	double sharp_amount,
	uint8_t threshold
) {
	int width = original_image.get_width();
	int height = original_image.get_height();

	// Original image sources
	const vector<uint8_t>& src_og_r = original_image.get_r();
	const vector<uint8_t>& src_og_g = original_image.get_g();
	const vector<uint8_t>& src_og_b = original_image.get_b();

	// Blurred image sources
	const vector<uint8_t>& src_b_r = blurred_image.get_r();
	const vector<uint8_t>& src_b_g = blurred_image.get_g();
	const vector<uint8_t>& src_b_b = blurred_image.get_b();

	int pixel_count = width * height;

	// For writing
	vector<uint8_t> out_r(pixel_count), out_g(pixel_count), out_b(pixel_count);

	high_pass_channels_rgb(
		original_image,
		blurred_image,
		out_r,
		out_g,
		out_b,
		sharp_amount,
		threshold
	);

	original_image.set_rgb(out_r, out_g, out_b);
}

void Sharpener::convolutional_sharpen_naive(
	Image& original_image,
	const SharpenerConfig& config
) {
	int width = original_image.get_width();
	int height = original_image.get_height();

	// Original image sources
	const vector<uint8_t>& src_r = original_image.get_r();
	const vector<uint8_t>& src_g = original_image.get_g();
	const vector<uint8_t>& src_b = original_image.get_b();

	int pixel_count = width * height;

	// For writing
	vector<uint8_t> out_r(pixel_count), out_g(pixel_count), out_b(pixel_count);

	const ConvolutionalSettings settings = get<2>(config.settings);
	const SharpenerOptions options = config.options;

	const uint8_t constant_edge_r = options.edge_constant.r;
	const uint8_t constant_edge_g = options.edge_constant.g;
	const uint8_t constant_edge_b = options.edge_constant.b;

	calculate_convolution_with_weights(
		out_r, out_g, out_b,
		height, width,
		src_r, src_g, src_b,
		settings.weight_kernel,
		settings.radius_x, settings.radius_y,
		config.options.edge_mode,
		constant_edge_r, constant_edge_g, constant_edge_b
	);

	original_image.set_rgb(out_r, out_g, out_b);
}

void Sharpener::high_pass_channels_rgb(
	const Image& original_image,
	const Image& blurred_image,
	vector<uint8_t>& out_r,
	vector<uint8_t>& out_g,
	vector<uint8_t>& out_b,
	double sharp_amount,
	uint8_t threshold
) {
	const vector<uint8_t>& src_og_r = original_image.get_r();
	const vector<uint8_t>& src_og_g = original_image.get_g();
	const vector<uint8_t>& src_og_b = original_image.get_b();

	const vector<uint8_t>& src_b_r = blurred_image.get_r();
	const vector<uint8_t>& src_b_g = blurred_image.get_g();
	const vector<uint8_t>& src_b_b = blurred_image.get_b();

	int pixel_count = original_image.get_width() * original_image.get_height();

	for (int idx = 0; idx < pixel_count; idx++) {
		out_r[idx] = high_pass_channel(
			src_og_r[idx],
			src_b_r[idx],
			sharp_amount,
			threshold
		);

		out_g[idx] = high_pass_channel(
			src_og_g[idx],
			src_b_g[idx],
			sharp_amount,
			threshold
		);

		out_b[idx] = high_pass_channel(
			src_og_b[idx],
			src_b_b[idx],
			sharp_amount,
			threshold
		);
	}
}

uint8_t Sharpener::high_pass_channel(
	uint8_t original, uint8_t blurred,
	double sharp_amount,
	uint8_t threshold
) {
	if (abs(original - blurred) < threshold) {
		return original;
	}

	double og_mult = sharp_amount + 1.0;

	int out_buf = std::round(
		og_mult * original - sharp_amount * blurred
	);

	if (out_buf < 0) {
		return 0;
	}
	else if (out_buf > 255) {
		return 255;
	}

	return out_buf;
}

void Sharpener::calculate_convolution_with_weights(
	vector<uint8_t>& out_r, vector<uint8_t>& out_g, vector<uint8_t>& out_b,

	int height, int width,

	const vector<uint8_t>& src_r,
	const vector<uint8_t>& src_g,
	const vector<uint8_t>& src_b,

	const vector<double>& weight_kernel,

	int radius_x, int radius_y,

	EdgeMode edge_mode,

	uint8_t constant_edge_r,
	uint8_t constant_edge_g,
	uint8_t constant_edge_b 
) {

	int image_size = height * width;
	
	// Go through each pixel
	for (int idx = 0; idx < image_size; idx++) {
		// Calculate the 
		calculate_convolution_with_weights_pixel(
			out_r[idx], out_g[idx], out_b[idx],
			idx,
			height, width,
			src_r, src_g, src_b,
			weight_kernel,
			radius_x, radius_y,
			edge_mode,
			constant_edge_r, constant_edge_g, constant_edge_b
		);
	}
}


void Sharpener::calculate_convolution_with_weights_pixel(
	uint8_t& out_r, uint8_t& out_g, uint8_t& out_b,

	int caller_idx,
	int height, int width,

	const vector<uint8_t>& src_r,
	const vector<uint8_t>& src_g,
	const vector<uint8_t>& src_b,

	const vector<double>& weight_kernel,

	int radius_x, int radius_y,

	EdgeMode edge_mode,

	uint8_t constant_edge_r,
	uint8_t constant_edge_g,
	uint8_t constant_edge_b
) {
	
	int kernel_width = radius_x * 2 + 1;

	double accum_r = 0, accum_g = 0, accum_b = 0;
	// No need to do outside checks if it's for sure all inside
	if (is_kernel_all_inside(caller_idx, radius_x, radius_y, height, width)) {
		for (int dy = -radius_y; dy <= radius_y; dy++) {
			for (int dx = -radius_x; dx <= radius_x; dx++) {
				int curr_idx = get_curr_idx(caller_idx, dy, dx, width);

				accumulate_rgb(
					accum_r, accum_g, accum_b,
					src_r[curr_idx], src_g[curr_idx], src_b[curr_idx],
					// Current index - offset
					weight_kernel[(dy + radius_y) * (2 * radius_x + 1) + dx + radius_x]
				);
			}
		}
	}
	else {
		int mapped_idx;
		int kernel_width = 2 * radius_x + 1;

		for (int dy = -radius_y; dy <= radius_y; dy++) {
			for (int dx = -radius_x; dx <= radius_x; dx++) {
				int curr_idx = get_curr_idx(
					caller_idx,
					dy,
					dx,
					width
				);

				double weight = weight_kernel[
					(dy + radius_y) * kernel_width + dx + radius_x
				];

				uint8_t r_to_add;
				uint8_t g_to_add;
				uint8_t b_to_add;

				if (!is_outside(caller_idx, dx, dy, height, width)) {
					r_to_add = src_r[curr_idx];
					g_to_add = src_g[curr_idx];
					b_to_add = src_b[curr_idx];
				}
				else if (edge_mode == EdgeMode::IGNORE) {
					continue;
				}
				else if (edge_mode == EdgeMode::CONSTANT) {
					r_to_add = constant_edge_r;
					g_to_add = constant_edge_g;
					b_to_add = constant_edge_b;
				}
				else {
					mapped_idx = get_mapped_idx_naive(
						edge_mode,
						caller_idx,
						dx,
						dy,
						width,
						height
					);

					r_to_add = src_r[mapped_idx];
					g_to_add = src_g[mapped_idx];
					b_to_add = src_b[mapped_idx];
				}

				accumulate_rgb(
					accum_r,
					accum_g,
					accum_b,
					r_to_add,
					g_to_add,
					b_to_add,
					weight
				);
			}
		}
	}

	// RGB accumulated at this point, convert to output
	out_r = convert_accumulator_out_1d(accum_r);
	out_g = convert_accumulator_out_1d(accum_g);
	out_b = convert_accumulator_out_1d(accum_b);
}

uint8_t Sharpener::convert_accumulator_out_1d(double accum) {
	// Too large -> cap at 255
	if (accum > 255) {
		return 255;
	}
	// Too small -> cap at 0
	if (accum < 0) {
		return 0;
	}
	// Normal -> round first
	return std::round(accum);
}


void Sharpener::accumulate_rgb(
	double& accum_r, double& accum_g, double& accum_b,
	uint8_t pixel_r, uint8_t pixel_g, uint8_t pixel_b,
	double weight
) {
	accum_r += pixel_r * weight;
	accum_g += pixel_g * weight;
	accum_b += pixel_b * weight;
}