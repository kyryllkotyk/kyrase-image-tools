#include "kyrase/blurrer.h" 

Image Blurrer::blur_naive(
	const BlurrerConfig& config,
	const Image& image
) {
	Image return_image = image;

	// As to not duplicate code
	blur_naive_modifiable(
		config,
		return_image
	);

	return return_image;
}

void Blurrer::blur_naive_modifiable(
	const BlurrerConfig& config,
	Image& image
) {
	// Figure out the filter 
	switch (config.settings.index()) {
	case(0): { // Box blur
		box_blur_naive(
			config,
			image
		);

		break;
	}

	default: // Something is wrong
		throw std::invalid_argument(
			"Blurrer: Invalid filter option requested"
		);
	}
}

void Blurrer::box_blur_naive(
	const BlurrerConfig& config,
	Image& image
) {
	// Box filter
	const BlurrerBoxSettings& settings = std::get<0>(config.settings);

	int radius_x = settings.radius_x;
	int radius_y = settings.radius_y;
	int passes = settings.passes;

	int width = image.get_width();
	int height = image.get_height();

	const uint8_t constant_edge_r = config.options.edge_constant.r;
	const uint8_t constant_edge_g = config.options.edge_constant.g;
	const uint8_t constant_edge_b = config.options.edge_constant.b;

	// Ensure the inputs are valid
	verify_blur_input(
		config.options.edge_mode,
		radius_x,
		radius_y,
		passes,
		width,
		height
	);

	BlurrerEdgeMode edge_mode = config.options.edge_mode;

	// For reading
	const vector<uint8_t>& src_r = image.get_r();
	const vector<uint8_t>& src_g = image.get_g();
	const vector<uint8_t>& src_b = image.get_b();

	if (src_r.size() != src_g.size() || src_g.size() != src_b.size()) {
		// r == g, g == b, so b == g!
		throw std::invalid_argument(
			"Blurrer: R,G,B channel dimensions do not match"
		);
	}

	int pixel_count = width * height;

	// For writing
	vector<uint8_t> out_r(pixel_count), out_g(pixel_count), out_b(pixel_count);

	int kernel_size = 4 * radius_x * radius_y + 2 * radius_x + 2 * radius_y + 1;

	int sum_r = 0;
	int sum_g = 0;
	int sum_b = 0;

	for (int i = 0; i < pixel_count; i++) {
		int valid_count = 0;

		// Needs edge handling
		// Conditions:
		// 1) i % width < radius_x
		// 2) i % width > width - radius_x - 1
		// 3) i < radius_y * width
		// 4) i > (height - radius_y) * width - 1
		int x_pos = i % width;

		if (
			x_pos < radius_x
			|| x_pos >(width - radius_x - 1)
			|| i < radius_y * width
			|| i >((height - radius_y) * width - 1)
			) {
			for (int dy = -radius_y; dy <= radius_y; dy++) {
				for (int dx = -radius_x; dx <= radius_x; dx++) {
					if (is_outside(i, dx, dy, height, width)) {
						// Specially handled edge modes
						if (edge_mode == BlurrerEdgeMode::IGNORE) {
							
							continue;
						}
						else if (edge_mode == BlurrerEdgeMode::CONSTANT) {
							add_rgb_sums(
								sum_r,
								sum_g,
								sum_b,
								constant_edge_r,
								constant_edge_g,
								constant_edge_b
							);
						}
						// 'Normal' edge modes
						else {
							size_t mapped_idx = get_mapped_idx_naive(
								edge_mode,
								i,
								dx,
								dy,
								width,
								height
							);

							add_rgb_sums_at_idx(
								sum_r,
								sum_g,
								sum_b,
								src_r,
								src_g,
								src_b,
								mapped_idx
							);
						}
					}
					// Inside. Use the normal values instead of edge-handled ones
					else {
						size_t idx = static_cast<size_t>(
							i + width * dy + dx
							);

						add_rgb_sums_at_idx(
							sum_r,
							sum_g,
							sum_b,
							src_r,
							src_g,
							src_b,
							idx
						);
					}

					valid_count++;
				}
			}
		}
		// Doesn't need edge handling
		else {
			for (int y = -radius_y; y <= radius_y; y++) {
				for (int x = -radius_x; x <= radius_x; x++) {
					size_t idx = static_cast<size_t>(
						i + width * y + x
						);

					// Sum the nearby items
					add_rgb_sums_at_idx(
						sum_r,
						sum_g,
						sum_b,
						src_r,
						src_g,
						src_b,
						idx
					);

					valid_count++;
				}
			}
		}

		int divisor = kernel_size;
		// IGNORE requires us to keep track of how many elements
		// were actually added
		if (edge_mode == BlurrerEdgeMode::IGNORE) {
			divisor = valid_count;
		}

		// Average out
		out_r[i] = static_cast<uint8_t>(sum_r / divisor);
		out_g[i] = static_cast<uint8_t>(sum_g / divisor);
		out_b[i] = static_cast<uint8_t>(sum_b / divisor);

		// Reset sums
		sum_r = 0;
		sum_g = 0;
		sum_b = 0;
	}

	// Done with blurring, assign the R/G/B channel storage to the image
	image.set_rgb(std::move(out_r), std::move(out_g), std::move(out_b));
}

void Blurrer::verify_blur_input(
	const BlurrerEdgeMode mode,
	const int radius_x,
	const int radius_y,
	const int passes,
	const int width,
	const int height
) const {
	if (radius_x < 0 || radius_y < 0) {
		throw std::invalid_argument(
			"Blurrer: Radii must not be negative"
		);
	}

	if (radius_x >= width || radius_y >= height) {
		throw std::invalid_argument(
			"Blurrer: Radii must not equal or exceed the image dimensions"
		);
	}

	if (passes < 1) {
		throw std::invalid_argument(
			"Blurrer: Passes must be at least 1"
		);
	}

	if (mode == BlurrerEdgeMode::REFLECT_CALLER
		&& (radius_x * 2 > width || radius_y * 2 > height)) {
		throw std::invalid_argument(
			"Blurrer: Caller reflection requires radii no larger than half the image dimensions"
		);
	}
}

bool Blurrer::is_outside(
	const int caller_idx,
	const int dx,
	const int dy,
	const int height,
	const int width
) {
	int caller_x = caller_idx % width;
	int caller_y = caller_idx / width;

	int target_x = caller_x + dx;
	int target_y = caller_y + dy;

	return target_x < 0
		|| target_x >= width
		|| target_y < 0
		|| target_y >= height;
}

void Blurrer::add_rgb_sums(
	int& sum_r,
	int& sum_g,
	int& sum_b,
	const uint8_t r_val,
	const uint8_t g_val,
	const uint8_t b_val
) {
	sum_r += r_val;
	sum_g += g_val;
	sum_b += b_val;
}

void Blurrer::add_rgb_sums_at_idx(
	int& sum_r,
	int& sum_g,
	int& sum_b,
	const vector<uint8_t>& r,
	const vector<uint8_t>& g,
	const vector<uint8_t>& b,
	const size_t idx
) {
	add_rgb_sums(
		sum_r,
		sum_g,
		sum_b,
		r[idx],
		g[idx],
		b[idx]
	);
}

size_t Blurrer::get_mapped_idx_naive(
	const BlurrerEdgeMode edge_mode,
	const int caller_idx,
	const int dx,
	const int dy,
	const int width,
	const int height
) const {
	
	int caller_x = caller_idx % width;
	int caller_y = caller_idx / width;

	int mapped_x = 0, mapped_y = 0;
	// Figure out which edge type the user selected
	switch (edge_mode) {
	case(BlurrerEdgeMode::REFLECT101): {

		mapped_x = calculate_reflection101_per_d(
			caller_x,
			dx,
			width
		);

		mapped_y = calculate_reflection101_per_d(
			caller_y,
			dy,
			height
		);

		break;
	}
		
	case(BlurrerEdgeMode::REFLECT_CALLER): {
		mapped_x = calculate_caller_reflection_per_d(
			caller_x,
			dx,
			width
		);

		mapped_y = calculate_caller_reflection_per_d(
			caller_y,
			dy,
			height
		);

		break;
	}

	case(BlurrerEdgeMode::REFLECT): {
		mapped_x = calculate_reflection_per_d(
			caller_x,
			dx,
			width
		);

		mapped_y = calculate_reflection_per_d(
			caller_y,
			dy,
			height
		);

		break;
	}

	case(BlurrerEdgeMode::CLAMP): {
		mapped_x = calculate_clamp_per_d(
			caller_x,
			dx,
			width
		);

		mapped_y = calculate_clamp_per_d(
			caller_y,
			dy,
			height
		);

		break;
	}

	case(BlurrerEdgeMode::WRAP): {
		mapped_x = calculate_wrap_per_d(
			caller_x,
			dx,
			width
		);

		mapped_y = calculate_wrap_per_d(
			caller_y,
			dy,
			height
		);

		break;
	}

	case(BlurrerEdgeMode::CONSTANT): {
		throw std::logic_error(
			"Blurrer: Constant edge mode must not utilize index mapping"
		);
	}

	case(BlurrerEdgeMode::IGNORE): {
		throw std::logic_error(
			"Blurrer: Ignore edge mode must not utilize index mapping"
		);
	}

	default:
		throw std::invalid_argument(
			"Blurrer: Unsupported edge mode"
		);
	}

	return convert_yx_to_idx(mapped_y, mapped_x, width);

}

int Blurrer::calculate_reflection101_per_d(
	const int caller_i,
	const int delta,
	const int dimension_size
) const {
	// Reflect101:
	// ti = caller i + di
	int target_i = caller_i + delta;

	// if ti < 0: mi = -ti
	if (target_i < 0) {
		return -target_i;
	}
	// if ti >= dimension size: mi = 2 * dimension size - 2 - ti
	else if (target_i >= dimension_size) {
		return 2 * dimension_size - 2 - target_i;
	}
	// if 0 <= ti < dimension size : mi = ti
	else {
		return target_i;
	}
}

int Blurrer::calculate_reflection_per_d(
	const int caller_i,
	const int delta,
	const int dimension_size
) const {
	int target_i = caller_i + delta;

	// Rely on the existing 101 and just adjust the inputs to avoid rewriting

	if (target_i < 0) {
		return calculate_reflection101_per_d(
			caller_i,
			delta + 1,
			dimension_size
		);
	}
	else if (target_i >= dimension_size) {
		return calculate_reflection101_per_d(
			caller_i,
			delta - 1,
			dimension_size
		);
	}
	else {
		return target_i;
	}
}

int Blurrer::calculate_caller_reflection_per_d(
	const int caller_i,
	const int delta,
	const int dimension_size
) const {
	// Reflect caller:
	// ti = caller i + di
	int target_i = caller_i + delta;

	// if ti < 0 : mi = caller_i - delta
	// if ti >= dimension size : mi = caller_i - delta
	if (target_i < 0 || target_i >= dimension_size) {
		return caller_i - delta;
	}
	// if 0 <= ti < dimension size : mi = target_i
	else {
		return target_i;
	}
}

int Blurrer::calculate_clamp_per_d(
	const int caller_i, 
	const int delta, 
	const int dimension_size
) const {
	int target_i = caller_i + delta;

	if (target_i < 0) {
		return 0;
	}
	else if (target_i >= dimension_size) {
		return dimension_size - 1;
	}
	else {
		return target_i;
	}
}

int Blurrer::calculate_wrap_per_d(
	const int caller_i,
	const int delta,
	const int dimension_size
) const {
	int target_i = caller_i + delta;

	if (target_i < 0) {
		return dimension_size + target_i;
	}
	else if (target_i >= dimension_size) {
		return target_i - dimension_size;
	}
	else {
		return target_i;
	}
}

size_t Blurrer::convert_yx_to_idx(
	const int y,
	const int x,
	const int width
) const {
	return static_cast<size_t>(y * width + x);
}
}