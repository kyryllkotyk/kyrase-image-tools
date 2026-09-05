#include "kyrase/blurrer.h" 

Blurrer::BlurrerOptions::BlurrerOptions(
	EdgeMode edge_mode,
	ConstantColor edge_constant,
	uint32_t threads
) : edge_mode(edge_mode),
edge_constant(edge_constant),
num_threads(threads) {

}

Image Blurrer::blur(
	const BlurrerConfig& config,
	const Image& image,
	OptimizationLevel optimization_level
) {
	Image return_image = image;

	// As to not duplicate code
	blur_modifiable(
		config,
		return_image,
		optimization_level
	);

	return return_image;
}

Image Blurrer::blur_partial_naive(
	const BlurrerConfig& config,
	const Image& image,
	const vector<pair<pair<int, int>, pair<int, int>>>& blur_regions
) {
	Image return_image = image;

	// As to not duplicate code
	blur_partial_modifiable_naive(
		config,
		return_image,
		blur_regions
	);

	return return_image;
}

void Blurrer::blur_modifiable(
	const BlurrerConfig& config,
	Image& image,
	OptimizationLevel optimization_level
) {
	// Figure out the filter 
	switch (config.settings.index()) {
	case(0): { // Box blur
		// Call the appropriate function based on optimization level requested
		switch (optimization_level) {
		case(SINGLE_NAIVE): {
			box_single_naive(
				config,
				image
			);

			break;
		}

		case(SINGLE_OPT): {
			box_single_optimized(
				config,
				image
			);

			break;
		}

		case(MULTI_NAIVE): {
			box_multi_naive(
				config,
				image
			);

			break;
		}

						 // TODO:: fill out after box_optimized_multi is implemented
		case(MULTI_OPT): {
			break;
		}

		default: // Something is wrong
			throw std::invalid_argument(
				"Blurrer: Invalid optimization level requested"
			);
		}

		break;
	}

	case(1): { // Gaussian blur
		// Call the appropriate function based on optimization level requested
		switch (optimization_level) {
		case(SINGLE_NAIVE): {
			gaussian_single_naive(
				config,
				image
			);

			break;
		}

		default: // Something is wrong
			throw std::invalid_argument(
				"Blurrer: Invalid optimization level requested"
			);
		}

		break;
	}

	case(2): { // Binomial blur
		// Call the appropriate function based on optimization level requested
		switch (optimization_level) {
		case(SINGLE_NAIVE): {
			binomial_single_naive(
				config,
				image
			);

			break;
		}

		default: // Something is wrong
			throw std::invalid_argument(
				"Blurrer: Invalid optimization level requested"
			);
		}

		break;
	}

	case(3): { // Median blur
		// Call the appropriate function based on optimization level requested
		switch (optimization_level) {
		case(SINGLE_NAIVE): {
			median_single_naive(
				config,
				image
			);

			break;
		}

		default: // Something is wrong
			throw std::invalid_argument(
				"Blurrer: Invalid optimization level requested"
			);
		}

		break;
	}

	case(4): { // Lens blur
		// Call the appropriate function based on optimization level requested
		switch (optimization_level) {
		case(SINGLE_NAIVE): {
			lens_single_naive(
				config,
				image
			);

			break;
		}

		default: // Something is wrong
			throw std::invalid_argument(
				"Blurrer: Invalid optimization level requested"
			);
		}

		break;
	}
	default: // Something is wrong
		throw std::invalid_argument(
			"Blurrer: Invalid filter option requested"
		);
	}
}

void Blurrer::blur_partial_modifiable_naive(
	const BlurrerConfig& config,
	Image& image,
	const vector<pair<pair<int, int>, pair<int, int>>>& blur_regions
) {
	// Figure out the filter 
	switch (config.settings.index()) {
	case(0): { // Box blur
		box_partial_naive_single(
			config,
			image,
			blur_regions
		);

		break;
	}

	default: // Something is wrong
		throw std::invalid_argument(
			"Blurrer: Invalid filter option requested"
		);
	}
}

void Blurrer::box_single_naive(
	const BlurrerConfig& config,
	Image& image
) {
	// Box filter
	const BoxSettings& settings = std::get<0>(config.settings);

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
		height);

	EdgeMode edge_mode = config.options.edge_mode;


	int pixel_count = width * height;

	// For writing
	vector<uint8_t> out_r(pixel_count), out_g(pixel_count), out_b(pixel_count);

	int kernel_size = 4 * radius_x * radius_y + 2 * radius_x + 2 * radius_y + 1;

	uint64_t sum_r = 0;
	uint64_t sum_g = 0;
	uint64_t sum_b = 0;

	for (int pass = 0; pass < passes; pass++) {
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

		int idx;
		// Go through each row
		for (int pixel_y = 0; pixel_y < height; pixel_y++) {
			for (int pixel_x = 0; pixel_x < width; pixel_x++) {

				idx = convert_yx_to_idx(pixel_y, pixel_x, width);

				// Get current sum
				calculate_sum_pixel(
					sum_r,
					sum_g,
					sum_b,
					pixel_y,
					pixel_x,
					radius_x,
					radius_y,
					edge_mode,
					src_r,
					src_g,
					src_b,
					height,
					width,
					true,
					constant_edge_r,
					constant_edge_g,
					constant_edge_b
				);

				if (edge_mode != EdgeMode::IGNORE) {
					// Set output
					set_output_pixel(
						out_r[idx],
						out_g[idx],
						out_b[idx],
						sum_r,
						sum_g,
						sum_b,
						kernel_size);
				}
				else {
					set_output_pixel_find_divisor(
						out_r[idx],
						out_g[idx],
						out_b[idx],
						sum_r,
						sum_g,
						sum_b,
						pixel_y,
						pixel_x,
						radius_x,
						radius_y,
						height,
						width);
				}
			}
		}

		// Done with blurring, assign the R/G/B channel storage to the image
		image.swap_rgb(out_r, out_g, out_b);
	}
}

void Blurrer::box_partial_naive_single(
	const BlurrerConfig& config,
	Image& image,
	const vector<pair<pair<int, int>, pair<int, int>>>& blur_regions
) {
	// Box filter
	const BoxSettings& settings = std::get<0>(config.settings);

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
		height);

	EdgeMode edge_mode = config.options.edge_mode;

	int pixel_count = width * height;

	// For writing
	vector<uint8_t> out_r(pixel_count), out_g(pixel_count), out_b(pixel_count);

	int kernel_size = 4 * radius_x * radius_y + 2 * radius_x + 2 * radius_y + 1;

	uint64_t sum_r = 0;
	uint64_t sum_g = 0;
	uint64_t sum_b = 0;

	int i;
	vector<bool> blurred(pixel_count);

	for (int pass = 0; pass < passes; pass++) {
		// For reading
		// TODO:: ADD OPTION FOR NON-CONST R,G,B
		// TO PREVENT COPYING BELOW
		const vector<uint8_t>& src_r = image.get_r();
		const vector<uint8_t>& src_g = image.get_g();
		const vector<uint8_t>& src_b = image.get_b();

		if (src_r.size() != src_g.size() || src_g.size() != src_b.size()) {
			// r == g, g == b, so b == g!
			throw std::invalid_argument(
				"Blurrer: R,G,B channel dimensions do not match"
			);
		}

		// Refill
		std::fill(blurred.begin(), blurred.end(), false);

		out_r = src_r;
		out_g = src_g;
		out_b = src_b;

		// Go through all regions
		for (auto& region : blur_regions) {
			for (int region_row = region.first.first;
				region_row < region.first.second;
				region_row++
				) {
				for (int region_col = region.second.first;
					region_col < region.second.second;
					region_col++
					) {

					i = convert_yx_to_idx(region_row, region_col, width);

					// Already blurred - don't want to look at it again
					if (blurred[i]) {
						continue;
					}
					else {
						blurred[i] = true;
					}

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
									if (edge_mode == EdgeMode::IGNORE) {

										continue;
									}
									else if (edge_mode == EdgeMode::CONSTANT) {
										add_rgb_sums(
											sum_r,
											sum_g,
											sum_b,
											constant_edge_r,
											constant_edge_g,
											constant_edge_b);
									}
									// 'Normal' edge modes
									else {
										size_t mapped_idx = get_mapped_idx_naive(
											edge_mode,
											i,
											dx,
											dy,
											width,
											height);

										add_rgb_sums_at_idx(
											sum_r,
											sum_g,
											sum_b,
											src_r,
											src_g,
											src_b,
											mapped_idx);
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
										idx);
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
									idx);

								valid_count++;
							}
						}
					}

					int divisor = kernel_size;
					// IGNORE requires us to keep track of how many elements
					// were actually added
					if (edge_mode == EdgeMode::IGNORE) {
						divisor = valid_count;
					}

					// Average out
					set_output_pixel(
						out_r[i], out_g[i], out_b[i],
						sum_r, sum_g, sum_b,
						divisor
					);

					// Reset sums
					sum_r = 0;
					sum_g = 0;
					sum_b = 0;
				}
			}
		}

		// Done with blurring, assign the R/G/B channel storage to the image
		image.swap_rgb(out_r, out_g, out_b);
	}
}


void Blurrer::box_single_optimized(
	const BlurrerConfig& config,
	Image& image
) {
	// Box filter
	const BoxSettings& settings = std::get<0>(config.settings);

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
		height);

	EdgeMode edge_mode = config.options.edge_mode;

	int pixel_count = width * height;
	int kernel_size = 4 * radius_x * radius_y + 2 * radius_x + 2 * radius_y + 1;

	// For tracking rolling sums
	vector<uint64_t> sums_r(width), sums_g(width), sums_b(width);
	// For tracking outputs
	vector<uint8_t> out_r(pixel_count), out_g(pixel_count), out_b(pixel_count);

	for (int p = 0; p < passes; p++) {

		const vector<uint8_t>& src_r = image.get_r();
		const vector<uint8_t>& src_g = image.get_g();
		const vector<uint8_t>& src_b = image.get_b();

		if (src_r.size() != src_g.size() || src_g.size() != src_b.size()) {
			// r == g, g == b, so b == g!
			throw std::invalid_argument(
				"Blurrer: R,G,B channel dimensions do not match"
			);
		}

		// Go through each row
		for (int row = 0; row < height; row++) {
			// Get current sums
			calculate_sum_row(
				sums_r,
				sums_g,
				sums_b,
				row,
				radius_x,
				radius_y,
				edge_mode,
				src_r,
				src_g,
				src_b,
				height,
				width,
				constant_edge_r,
				constant_edge_g,
				constant_edge_b, 0, 0);

			// Fill the output for this row
			// Don't need to find divisor, as it's not IGNORE
			if (edge_mode != EdgeMode::IGNORE) {
				set_output_row(
					out_r,
					out_g,
					out_b,
					sums_r,
					sums_g,
					sums_b,
					kernel_size,
					row,
					width,
					0,
					width - 1);
			}
			// IGNORE, so need to find divisor per cell
			else {
				set_output_row_find_divisor(
					out_r,
					out_g,
					out_b,
					sums_r,
					sums_g,
					sums_b,
					row,
					radius_x,
					radius_y,
					height,
					width,
					0,
					width - 1);
			}
		}

		// Set the new RGB
		image.swap_rgb(out_r, out_g, out_b);
	}
}

void Blurrer::box_multi_naive(
	const BlurrerConfig& config,
	Image& image
) {
	// Box filter
	const BoxSettings& settings = std::get<0>(config.settings);

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
		height);

	EdgeMode edge_mode = config.options.edge_mode;

	int pixel_count = width * height;
	int kernel_size = 4 * radius_x * radius_y + 2 * radius_x + 2 * radius_y + 1;

	// For tracking outputs
	vector<uint8_t> out_r(pixel_count), out_g(pixel_count), out_b(pixel_count);

	uint32_t threads = config.options.num_threads;

	// Auto-define for best result
	if (threads == 0) {
		// Get the hardware suggestion for the number of threads available
		unsigned int h_thread_suggest = thread::hardware_concurrency();

		// Could not calculate, default to safe value
		if (h_thread_suggest == 0) {
			h_thread_suggest = 1;
		}

		threads = h_thread_suggest;
		//		unsigned int threads_max = std::min(h_thread_suggest, static_cast<unsigned int>(width));
		//		uint64_t workload_estimate = pixel_count * passes;
	}
	// TODO:: Figure out better rules for this^^ by benchmarking

	// Too many threads specified
	if (threads > width) {
		throw std::invalid_argument(
			"Blurrer: Max number of threads = width of the image"
		);
	}

	// Individual storages per-thread 
	// For tracking rolling sums
	vector<vector<uint64_t>> sums_r(threads), sums_g(threads), sums_b(threads);


	vector<jthread> thread_track;
	thread_track.reserve(threads);


	// Start  passes loop here
	for (int pass = 0; pass < passes; pass++) {
		// Get RGB source here from image
		const vector<uint8_t>& src_r = image.get_r();
		const vector<uint8_t>& src_g = image.get_g();
		const vector<uint8_t>& src_b = image.get_b();

		for (int thread_num = 0; thread_num < threads; thread_num++) {
			uint32_t start_col, end_col;

			// Get the work split
			get_col_range_thread(
				start_col,
				end_col,
				thread_num,
				threads,
				width);

			sums_r[thread_num].resize(end_col - start_col + 1);
			sums_g[thread_num].resize(end_col - start_col + 1);
			sums_b[thread_num].resize(end_col - start_col + 1);

			// Dispatch this thread
			thread_track.emplace_back(
				[this, thread_num,
				&src_r, &src_g, &src_b,
				&sums_r, &sums_g, &sums_b,
				&out_r, &out_g, &out_b,
				start_col, end_col, width,
				height, radius_x, radius_y, kernel_size, edge_mode,
				constant_edge_r, constant_edge_g, constant_edge_b]
				{
					box_multi_per_thread(
						src_r,
						src_g,
						src_b,
						start_col,
						end_col,
						height,
						width,
						radius_x,
						radius_y,
						kernel_size,
						edge_mode,
						sums_r[thread_num],
						sums_g[thread_num],
						sums_b[thread_num],
						out_r,
						out_g,
						out_b,
						constant_edge_r,
						constant_edge_g,
						constant_edge_b);
				}
					);
		}

		// Join threads
		for (jthread& thread : thread_track) {
			thread.join();
		}

		// Update image's RGB here
		image.swap_rgb(out_r, out_g, out_b);

		// Remove the threads to prepare for the next pass
		thread_track.clear();
	}

}

void Blurrer::gaussian_single_naive(
	const BlurrerConfig& config,
	Image& image
) {
	// Gaussian filter
	const GaussianSettings& settings = std::get<1>(config.settings);

	int radius_x = settings.radius_x;
	int radius_y = settings.radius_y;
	double sigma_x = settings.sigma_x;
	double sigma_y = settings.sigma_y;

	int width = image.get_width();
	int height = image.get_height();

	const uint8_t constant_edge_r = config.options.edge_constant.r;
	const uint8_t constant_edge_g = config.options.edge_constant.g;
	const uint8_t constant_edge_b = config.options.edge_constant.b;

	// Ensure the inputs are valid
	//TODO:: Remake so it becomes a proper verifier
	verify_blur_input(
		config.options.edge_mode,
		radius_x,
		radius_y,
		1,
		width,
		height
	);

	EdgeMode edge_mode = config.options.edge_mode;

	int pixel_count = width * height;
	int kernel_size = 4 * radius_x * radius_y + 2 * radius_x + 2 * radius_y + 1;

	// For tracking outputs
	vector<uint8_t> out_r(pixel_count), out_g(pixel_count), out_b(pixel_count);

	// Storages for x/y gaussian weights
	// -radius -> 0th
	vector<double> x_weights_store(radius_x * 2 + 1);
	vector<double> y_weights_store(radius_y * 2 + 1);

	// Calculate the weights and normalize them
	calculate_gaussian_weights_normalized(
		x_weights_store, y_weights_store,
		sigma_x, sigma_y,
		radius_x, radius_y
	);

	// Get the source channels
	const vector<uint8_t> src_r = image.get_r();
	const vector<uint8_t> src_g = image.get_g();
	const vector<uint8_t> src_b = image.get_b();

	size_t curr_idx;
	// Go through each pixel
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			curr_idx = convert_yx_to_idx(y, x, width);

			calculate_out_pixel_weighted(
				out_r[curr_idx], out_g[curr_idx], out_b[curr_idx],
				y, x,
				radius_x, radius_y,
				x_weights_store, y_weights_store,
				edge_mode,
				src_r, src_g, src_b,
				height, width,
				true,
				constant_edge_r, constant_edge_g, constant_edge_b
			);
		}
	}

	// Replace current RGB channels with blurred ones
	image.set_rgb(out_r, out_g, out_b);
}

void Blurrer::binomial_single_naive(
	const BlurrerConfig& config,
	Image& image
) {
	// Binomial filter
	const BinomialSettings& settings = std::get<2>(config.settings);

	int radius_x = settings.radius_x;
	int radius_y = settings.radius_y;

	int width = image.get_width();
	int height = image.get_height();

	const uint8_t constant_edge_r = config.options.edge_constant.r;
	const uint8_t constant_edge_g = config.options.edge_constant.g;
	const uint8_t constant_edge_b = config.options.edge_constant.b;

	// Ensure the inputs are valid
	//TODO:: Remake so it becomes a proper verifier
	verify_blur_input(
		config.options.edge_mode,
		radius_x,
		radius_y,
		1,
		width,
		height
	);

	EdgeMode edge_mode = config.options.edge_mode;

	int pixel_count = width * height;
	int kernel_size = 4 * radius_x * radius_y + 2 * radius_x + 2 * radius_y + 1;

	// For tracking outputs
	vector<uint8_t> out_r(pixel_count), out_g(pixel_count), out_b(pixel_count);

	// Storages for x/y gaussian weights
	// -radius -> 0th
	vector<double> x_weights_store(radius_x * 2 + 1);
	vector<double> y_weights_store(radius_y * 2 + 1);

	// Calculate the weights and normalize them
	calculate_binomial_normalized_weights(
		x_weights_store, y_weights_store,
		radius_x, radius_y
	);

	// Get the source channels
	const vector<uint8_t> src_r = image.get_r();
	const vector<uint8_t> src_g = image.get_g();
	const vector<uint8_t> src_b = image.get_b();

	size_t curr_idx;
	// Go through each pixel
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			curr_idx = convert_yx_to_idx(y, x, width);

			calculate_out_pixel_weighted(
				out_r[curr_idx], out_g[curr_idx], out_b[curr_idx],
				y, x,
				radius_x, radius_y,
				x_weights_store, y_weights_store,
				edge_mode,
				src_r, src_g, src_b,
				height, width,
				true,
				constant_edge_r, constant_edge_g, constant_edge_b
			);
		}
	}

	// Replace current RGB channels with blurred ones
	image.set_rgb(out_r, out_g, out_b);
}

void Blurrer::median_single_naive(
	const BlurrerConfig& config,
	Image& image
) {
	// Median filter
	const MedianSettings& settings = std::get<3>(config.settings);

	int radius_x = settings.radius_x;
	int radius_y = settings.radius_y;

	int width = image.get_width();
	int height = image.get_height();

	const uint8_t constant_edge_r = config.options.edge_constant.r;
	const uint8_t constant_edge_g = config.options.edge_constant.g;
	const uint8_t constant_edge_b = config.options.edge_constant.b;

	// Ensure the inputs are valid
	//TODO:: Remake so it becomes a proper verifier
	verify_blur_input(
		config.options.edge_mode,
		radius_x,
		radius_y,
		1,
		width,
		height
	);

	EdgeMode edge_mode = config.options.edge_mode;

	int pixel_count = width * height;
	int kernel_size = 4 * radius_x * radius_y + 2 * radius_x + 2 * radius_y + 1;

	// For tracking outputs
	vector<uint8_t> out_r(pixel_count), out_g(pixel_count), out_b(pixel_count);

	// Storages for median calculation
	vector<uint8_t> kernel_r(kernel_size), kernel_g(kernel_size), kernel_b(kernel_size);

	// Get the source channels
	const vector<uint8_t> src_r = image.get_r();
	const vector<uint8_t> src_g = image.get_g();
	const vector<uint8_t> src_b = image.get_b();

	size_t idx;
	uint32_t end = kernel_size - 1;

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			idx = convert_yx_to_idx(y, x, width);

			fill_kernel_temps(
				kernel_r, kernel_g, kernel_b,
				idx,
				radius_x, radius_y,
				height, width,
				src_r, src_g, src_b,
				edge_mode,
				constant_edge_r, constant_edge_g, constant_edge_b
			);

			if (edge_mode == EdgeMode::IGNORE) {
				end = get_divisor_ignore(
					y, x, radius_x, radius_y, height, width
				) - 1;
			}

			find_kernel_median_rgb(
				kernel_r, kernel_g, kernel_b,
				0, end,
				out_r[idx], out_g[idx], out_b[idx]
			);
		}
	}

	// Replace current RGB channels with blurred ones
	image.set_rgb(out_r, out_g, out_b);
}

void Blurrer::lens_single_naive(
	const BlurrerConfig& config,
	Image& image
) {
	// Median filter
	const LensSettings& settings = std::get<4>(config.settings);

	int radius = settings.radius;

	int width = image.get_width();
	int height = image.get_height();

	const uint8_t constant_edge_r = config.options.edge_constant.r;
	const uint8_t constant_edge_g = config.options.edge_constant.g;
	const uint8_t constant_edge_b = config.options.edge_constant.b;

	// Ensure the inputs are valid
	//TODO:: Remake so it becomes a proper verifier
	verify_blur_input(
		config.options.edge_mode,
		radius,
		radius,
		1,
		width,
		height
	);

	EdgeMode edge_mode = config.options.edge_mode;

	int pixel_count = width * height;
	int kernel_size = 4 * radius * radius + 4 * radius + 1;

	// For tracking outputs
	vector<uint8_t> out_r(pixel_count), out_g(pixel_count), out_b(pixel_count);

	// Get the source channels
	const vector<uint8_t> src_r = image.get_r();
	const vector<uint8_t> src_g = image.get_g();
	const vector<uint8_t> src_b = image.get_b();

	size_t idx;

	int total_weights = calculate_total_weights_lens(radius);

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			idx = convert_yx_to_idx(y, x, width);
			calculate_out_pixel_lens(
				out_r[idx], out_g[idx], out_b[idx],
				radius,
				idx, height, width,
				total_weights,
				src_r, src_g, src_b,
				edge_mode,
				constant_edge_r, constant_edge_g, constant_edge_b
			);
		}
	}

	// Replace current RGB channels with blurred ones
	image.set_rgb(out_r, out_g, out_b);
}


void Blurrer::verify_blur_input(
	EdgeMode mode,
	int radius_x, int radius_y,
	int passes,
	int width, int height
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
}

void Blurrer::add_rgb_sums(uint64_t& sum_r,
	uint64_t& sum_g,
	uint64_t& sum_b, uint8_t r_val, uint8_t g_val, uint8_t b_val) {
	sum_r += r_val;
	sum_g += g_val;
	sum_b += b_val;
}

void Blurrer::add_rgb_sums_at_idx(
	uint64_t& sum_r,
	uint64_t& sum_g,
	uint64_t& sum_b,
	const vector<uint8_t>& r,
	const vector<uint8_t>& g,
	const vector<uint8_t>& b,
	size_t idx
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

void Blurrer::calculate_kernel_row_sum_in_bounds(
	uint64_t& sum_r, uint64_t& sum_g, uint64_t& sum_b,
	int caller_idx,
	int dx_begin, int dx_end, int dy,
	int width,
	const vector<uint8_t>& src_r,
	const vector<uint8_t>& src_g,
	const vector<uint8_t>& src_b
) {
	sum_r = 0;
	sum_g = 0;
	sum_b = 0;

	for (int dx = dx_begin; dx <= dx_end; dx++) {
		size_t idx = static_cast<size_t>(
			caller_idx + width * dy + dx
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
}

void Blurrer::calculate_kernel_row_sum(
	uint64_t& sum_r, uint64_t& sum_g, uint64_t& sum_b,
	int caller_idx,
	int dx_begin, int dx_end, int dy,
	int height, int width,
	EdgeMode edge_mode,
	const vector<uint8_t>& src_r,
	const vector<uint8_t>& src_g,
	const vector<uint8_t>& src_b,
	uint8_t constant_edge_r, uint8_t constant_edge_g, uint8_t constant_edge_b
) {
	sum_r = 0;
	sum_g = 0;
	sum_b = 0;

	for (int dx = dx_begin; dx <= dx_end; dx++) {
		// The pixel is outside, must handle
		if (is_outside(
			caller_idx,
			dx,
			dy,
			height,
			width)) {
			// Specially handled edge modes
			if (edge_mode == EdgeMode::IGNORE) {
				continue;
			}
			else if (edge_mode == EdgeMode::CONSTANT) {
				add_rgb_sums(
					sum_r,
					sum_g,
					sum_b,
					constant_edge_r,
					constant_edge_g,
					constant_edge_b);
			}
			// 'Normal' edge modes
			else {
				size_t mapped_idx = get_mapped_idx_naive(
					edge_mode,
					caller_idx,
					dx,
					dy,
					width,
					height);

				add_rgb_sums_at_idx(
					sum_r,
					sum_g,
					sum_b,
					src_r,
					src_g,
					src_b,
					mapped_idx);
			}
		}
		// Inside. Use the normal values instead of edge-handled ones
		else {
			size_t idx = static_cast<size_t>(
				caller_idx + width * dy + dx
				);

			add_rgb_sums_at_idx(
				sum_r,
				sum_g,
				sum_b,
				src_r,
				src_g,
				src_b,
				idx);
		}
	}
}

void Blurrer::calculate_sum_pixel_using_above(
	uint64_t& sum_r, uint64_t& sum_g, uint64_t& sum_b,
	int row, int col,
	int radius_x, int radius_y,
	EdgeMode edge_mode,
	const vector<uint8_t>& src_r,
	const vector<uint8_t>& src_g,
	const vector<uint8_t>& src_b,
	int height, int width,
	bool row_col_already_verified,
	uint8_t constant_edge_r, uint8_t constant_edge_g, uint8_t constant_edge_b
) {

	// Ensure this isn't accidentally called in the top row
	if (!row_col_already_verified && row == 0) {
		throw std::invalid_argument(
			"Blurrer: Cannot use optimization on top row. No value above to reuse."
		);
	}

	// Ensure the pixel is inside the image
	if (!row_col_already_verified && (row < 0 || row >= height || col < 0 || col >= width)) {
		throw std::invalid_argument(
			"Blurrer: Cannot calculate sum for out-of-bounds pixels."
		);
	}

	// Convert to idx
	int curr_idx = convert_yx_to_idx(row, col, width);

	// Convert above pixel's yx to idx. Here, row > 0, and not OOB.
	int above_idx = convert_yx_to_idx(row - 1, col, width);

	// 1) Find the sums of the top row of the kernel of above pixel
	uint64_t top_sum_r = 0;
	uint64_t top_sum_g = 0;
	uint64_t top_sum_b = 0;
	calculate_kernel_row_sum(
		top_sum_r,
		top_sum_g,
		top_sum_b,
		above_idx,
		-radius_x,
		radius_x,
		-radius_y,
		height,
		width,
		edge_mode,
		src_r,
		src_g,
		src_b,
		constant_edge_r,
		constant_edge_g,
		constant_edge_b);

	// 2) Find the sums of the bottom row of the kernel of the current pixel
	uint64_t bottom_sum_r = 0;
	uint64_t bottom_sum_g = 0;
	uint64_t bottom_sum_b = 0;
	calculate_kernel_row_sum(
		bottom_sum_r,
		bottom_sum_g,
		bottom_sum_b,
		curr_idx,
		-radius_x,
		radius_x,
		radius_y,
		height,
		width,
		edge_mode,
		src_r,
		src_g,
		src_b,
		constant_edge_r,
		constant_edge_g,
		constant_edge_b);

	// 3) Replace the current buffer with the result
	// 3.1) Use current buffer (above element's sum) as basis
	// 3.2) Remove the top sums and add the bottom sums from the result

	// NOTE: Split to prevent underflow. += bottom - top can be < 0.
	sum_r -= top_sum_r;
	sum_r += bottom_sum_r;

	sum_g -= top_sum_g;
	sum_g += bottom_sum_g;

	sum_b -= top_sum_b;
	sum_b += bottom_sum_b;

	// Done
}

void Blurrer::calculate_sum_pixel(
	uint64_t& sum_r, uint64_t& sum_g, uint64_t& sum_b,
	int row, int col,
	int radius_x, int radius_y,
	EdgeMode edge_mode,
	const vector<uint8_t>& src_r,
	const vector<uint8_t>& src_g,
	const vector<uint8_t>& src_b,
	int height, int width,
	bool row_col_already_verified,
	uint8_t constant_edge_r, uint8_t constant_edge_g, uint8_t constant_edge_b
) {
	// Ensure the pixel is inside the image
	if (!row_col_already_verified && (row < 0 || row >= height || col < 0 || col >= width)) {
		throw std::invalid_argument(
			"Blurrer: Cannot calculate sum for out-of-bounds pixels."
		);
	}

	// Convert to idx
	int curr_idx = convert_yx_to_idx(row, col, width);

	// Set the sums to zero to honor the replacement instead of incrementing
	sum_r = 0;
	sum_g = 0;
	sum_b = 0;

	uint64_t buf_r = 0;
	uint64_t buf_g = 0;
	uint64_t buf_b = 0;

	bool kernel_all_inside = is_kernel_all_inside(
		curr_idx,
		radius_x,
		radius_y,
		height,
		width);

	if (kernel_all_inside) {
		// Go through all rows
		for (int dy = -radius_y; dy <= radius_y; dy++) {
			calculate_kernel_row_sum_in_bounds(
				buf_r,
				buf_g,
				buf_b,
				curr_idx,
				-radius_x,
				radius_x,
				dy,
				width,
				src_r,
				src_g,
				src_b);

			// Add buffered row sums to output
			sum_r += buf_r;
			sum_g += buf_g;
			sum_b += buf_b;
		}
	}
	else {
		// Go through all rows
		for (int dy = -radius_y; dy <= radius_y; dy++) {
			// Use row summer to get the total sum of the kernel one row at a time
			calculate_kernel_row_sum(
				buf_r,
				buf_g,
				buf_b,
				curr_idx,
				-radius_x,
				radius_x,
				dy,
				height,
				width,
				edge_mode,
				src_r,
				src_g,
				src_b,
				constant_edge_r,
				constant_edge_g,
				constant_edge_b);

			// Add buffered row sums to output
			sum_r += buf_r;
			sum_g += buf_g;
			sum_b += buf_b;
		}
	}

	// Done
}

void Blurrer::calculate_sum_row(
	vector<uint64_t>& sums_r, vector<uint64_t>& sums_g, vector<uint64_t>& sums_b,
	int curr_row,
	int radius_x, int radius_y,
	EdgeMode edge_mode,
	const vector<uint8_t>& src_r,
	const vector<uint8_t>& src_g,
	const vector<uint8_t>& src_b,
	int height, int width,
	int start_col, int end_col,
	uint8_t constant_edge_r, uint8_t constant_edge_g, uint8_t constant_edge_b
) {
	int local_c;
	// Go through each pixel in this row
	for (int c = start_col; c <= end_col; c++) {
		local_c = c - start_col;
		// Manually calculate for first row
		if (curr_row == 0) {
			calculate_sum_pixel(
				sums_r[local_c],
				sums_g[local_c],
				sums_b[local_c],
				curr_row,
				c,
				radius_x,
				radius_y,
				edge_mode,
				src_r,
				src_g,
				src_b,
				height,
				width,
				true,
				constant_edge_r,
				constant_edge_g,
				constant_edge_b);
		}

		// Re-use existing buffers otherwise
		else {
			calculate_sum_pixel_using_above(
				sums_r[local_c],
				sums_g[local_c],
				sums_b[local_c],
				curr_row,
				c,
				radius_x,
				radius_y,
				edge_mode,
				src_r,
				src_g,
				src_b,
				height,
				width,
				true,
				constant_edge_r,
				constant_edge_g,
				constant_edge_b);
		}
	}
}

void Blurrer::set_output_pixel(
	uint8_t& output_r, uint8_t& output_g, uint8_t& output_b,
	uint64_t sum_r, uint64_t sum_g, uint64_t sum_b,
	uint32_t divisor
) {
	output_r = static_cast<uint8_t>((sum_r + divisor / 2) / divisor);
	output_g = static_cast<uint8_t>((sum_g + divisor / 2) / divisor);
	output_b = static_cast<uint8_t>((sum_b + divisor / 2) / divisor);
}

uint32_t Blurrer::get_divisor_ignore(
	int row, int col,
	int radius_x, int radius_y,
	int height, int width
) {
	uint32_t horizontal = (std::min(width - 1, col + radius_x)
		- std::max(0, col - radius_x) + 1);
	uint32_t vertical = (std::min(height - 1, row + radius_y)
		- std::max(0, row - radius_y) + 1);
	return horizontal * vertical;
}

void Blurrer::set_output_pixel_find_divisor(
	uint8_t& output_r, uint8_t& output_g, uint8_t& output_b,
	uint64_t sum_r, uint64_t sum_g, uint64_t sum_b,
	int row, int col,
	int radius_x, int radius_y,
	int height, int width
) {
	uint32_t divisor = get_divisor_ignore(
		row,
		col,
		radius_x,
		radius_y,
		height,
		width);

	set_output_pixel(
		output_r,
		output_g,
		output_b,
		sum_r,
		sum_g,
		sum_b,
		divisor);
}

void Blurrer::set_output_row(
	vector<uint8_t>& output_r, vector<uint8_t>& output_g, vector<uint8_t>& output_b,

	const vector<uint64_t>& sum_r,
	const vector<uint64_t>& sum_g,
	const vector<uint64_t>& sum_b,
	uint32_t divisor,
	int row, int width,
	int start_col, int end_col
) {
	int local_c;
	for (int c = start_col; c <= end_col; c++) {
		local_c = c - start_col;

		set_output_pixel(
			output_r[row * width + c],
			output_g[row * width + c],
			output_b[row * width + c],

			sum_r[local_c],
			sum_g[local_c],
			sum_b[local_c],

			divisor);
	}
}

void Blurrer::set_output_row_find_divisor(
	vector<uint8_t>& output_r, vector<uint8_t>& output_g, vector<uint8_t>& output_b,

	const vector<uint64_t>& sum_r,
	const vector<uint64_t>& sum_g,
	const vector<uint64_t>& sum_b,
	int row,
	int radius_x, int radius_y,
	int height, int width,
	int start_col, int end_col
) {
	int local_c;

	for (int c = start_col; c <= end_col; c++) {
		local_c = c - start_col;

		set_output_pixel_find_divisor(
			output_r[row * width + c],
			output_g[row * width + c],
			output_b[row * width + c],

			sum_r[local_c],
			sum_g[local_c],
			sum_b[local_c],

			row,
			c,
			radius_x,
			radius_y,
			height,
			width);
	}
}

void Blurrer::get_col_range_thread(
	uint32_t& col_begin_buf, uint32_t& col_end_buf,
	uint32_t thread_num, uint32_t total_threads,
	int total_cols
) {
	// Base columns each thread gets is total cols / total threads 
	uint32_t base = total_cols / total_threads;
	// To distribute remainder fairly, if thread number is < total cols % total threads, add 1
	// remainder var is used to determine whether to add 1
	uint32_t remainder = total_cols % total_threads;

	col_begin_buf = base * thread_num + std::min(thread_num, remainder);
	// Add base and 1 for remainder as needed
	// Subtract 1 as end is inclusive
	col_end_buf = col_begin_buf + base + (thread_num < remainder ? 1 : 0) - 1;
}

void Blurrer::box_multi_per_thread(
	const vector<uint8_t>& src_r,
	const vector<uint8_t>& src_g,
	const vector<uint8_t>& src_b,
	uint32_t start_col, uint32_t end_col,
	int rows, int img_cols,
	int radius_x, int radius_y,
	int kernel_size,
	EdgeMode edge_mode,

	// Sum buffers
	vector<uint64_t>& sums_r,
	vector<uint64_t>& sums_g,
	vector<uint64_t>& sums_b,

	// Output writing
	vector<uint8_t>& out_r,
	vector<uint8_t>& out_g,
	vector<uint8_t>& out_b,
	uint8_t constant_edge_r, uint8_t constant_edge_g, uint8_t constant_edge_b
) {

	for (int row = 0; row < rows; row++) {
		calculate_sum_row(
			sums_r,
			sums_g,
			sums_b,
			row,
			radius_x,
			radius_y,
			edge_mode,
			src_r,
			src_g,
			src_b,
			rows,
			img_cols,
			start_col,
			end_col,
			constant_edge_r,
			constant_edge_g,
			constant_edge_b
		);

		// Fill the output for this row
		// Don't need to find divisor, as it's not IGNORE
		if (edge_mode != EdgeMode::IGNORE) {
			set_output_row(
				out_r,
				out_g,
				out_b,
				sums_r,
				sums_g,
				sums_b,
				kernel_size,
				row,
				img_cols,
				start_col,
				end_col
			);
		}
		// IGNORE, so need to find divisor per cell
		else {
			set_output_row_find_divisor(
				out_r,
				out_g,
				out_b,
				sums_r,
				sums_g,
				sums_b,
				row,
				radius_x,
				radius_y,
				rows,
				img_cols,
				start_col,
				end_col
			);
		}
	}
}

void Blurrer::calculate_out_pixel_weighted(
	uint8_t& out_r, uint8_t& out_g, uint8_t& out_b,
	int row, int col,
	int radius_x, int radius_y,
	const vector<double>& weights_x, const vector<double>& weights_y,
	EdgeMode edge_mode,
	const vector<uint8_t>& src_r,
	const vector<uint8_t>& src_g,
	const vector<uint8_t>& src_b,
	int height, int width,
	bool row_col_already_verified,
	uint8_t constant_edge_r,
	uint8_t constant_edge_g,
	uint8_t constant_edge_b
) {
	// Ensure the pixel is inside the image
	if (!row_col_already_verified && (row < 0 || row >= height || col < 0 || col >= width)) {
		throw std::invalid_argument(
			"Blurrer: Cannot calculate sum for out-of-bounds pixels."
		);
	}

	// Convert to idx
	int curr_idx = convert_yx_to_idx(row, col, width);
	// Storage for idx conversion 
	int kernel_curr_idx = 0;

	bool kernel_all_inside = is_kernel_all_inside(
		curr_idx,
		radius_x,
		radius_y,
		height,
		width
	);

	double kernel_row_sum_r = 0;
	double kernel_row_sum_g = 0;
	double kernel_row_sum_b = 0;

	// Accumulators for the final pixel sum for more accurate rounding
	double sum_r_accum = 0;
	double sum_g_accum = 0;
	double sum_b_accum = 0;

	// Reusable storage
	double weight_x;
	double weight_y;

	// Reusable storage for non-all-inside case
	double r_to_add, g_to_add, b_to_add;

	if (kernel_all_inside) {
		// Go through each pixel in the kernel
		for (int dy = -radius_y; dy <= radius_y; dy++) {
			for (int dx = -radius_x; dx <= radius_x; dx++) {
				weight_x = weights_x[dx + radius_x];

				// Get the 1D index for the current kernel pixel
				kernel_curr_idx = convert_yx_to_idx(row + dy, col + dx, width);

				// Add x-weighted value to the row sum
				kernel_row_sum_r += src_r[kernel_curr_idx] * weight_x;
				kernel_row_sum_g += src_g[kernel_curr_idx] * weight_x;
				kernel_row_sum_b += src_b[kernel_curr_idx] * weight_x;
			}

			weight_y = weights_y[dy + radius_y];

			// Apply the y weight and save
			sum_r_accum += kernel_row_sum_r * weight_y;
			sum_g_accum += kernel_row_sum_g * weight_y;
			sum_b_accum += kernel_row_sum_b * weight_y;

			// Reset row sums
			kernel_row_sum_r = 0;
			kernel_row_sum_g = 0;
			kernel_row_sum_b = 0;
		}
	}
	else {
		// Go through each pixel in the kernel
		for (int dy = -radius_y; dy <= radius_y; dy++) {
			for (int dx = -radius_x; dx <= radius_x; dx++) {
				weight_x = weights_x[dx + radius_x];

				if (is_outside(curr_idx, dx, dy, height, width)) {
					// Specially handled edge modes
					if (edge_mode == EdgeMode::IGNORE) {
						continue;
					}
					else if (edge_mode == EdgeMode::CONSTANT) {
						r_to_add = constant_edge_r;
						g_to_add = constant_edge_g;
						b_to_add = constant_edge_b;
					}
					// 'Normal' edge modes
					else {
						kernel_curr_idx = get_mapped_idx_naive(
							edge_mode,
							curr_idx,
							dx,
							dy,
							width,
							height
						);

						r_to_add = src_r[kernel_curr_idx];
						g_to_add = src_g[kernel_curr_idx];
						b_to_add = src_b[kernel_curr_idx];
					}
				}
				else {
					// Get the 1D index for the current kernel pixel
					kernel_curr_idx = convert_yx_to_idx(row + dy, col + dx, width);
					r_to_add = src_r[kernel_curr_idx];
					g_to_add = src_g[kernel_curr_idx];
					b_to_add = src_b[kernel_curr_idx];
				}

				// Add x-weighted value to the row sum
				kernel_row_sum_r += r_to_add * weight_x;
				kernel_row_sum_g += g_to_add * weight_x;
				kernel_row_sum_b += b_to_add * weight_x;
			}

			weight_y = weights_y[dy + radius_y];

			// Apply the y weight and save
			sum_r_accum += kernel_row_sum_r * weight_y;
			sum_g_accum += kernel_row_sum_g * weight_y;
			sum_b_accum += kernel_row_sum_b * weight_y;

			// Reset row sums
			kernel_row_sum_r = 0;
			kernel_row_sum_g = 0;
			kernel_row_sum_b = 0;
		}

		if (edge_mode == EdgeMode::IGNORE) {
			double participating_sum = get_sum_participating_weights_ignore(
				curr_idx,
				radius_x,
				radius_y,
				height,
				width,
				weights_x,
				weights_y
			);

			sum_r_accum /= participating_sum;
			sum_g_accum /= participating_sum;
			sum_b_accum /= participating_sum;
		}
	}


	// Convert back to output
	out_r = static_cast<uint8_t>(std::round(sum_r_accum));
	out_g = static_cast<uint8_t>(std::round(sum_g_accum));
	out_b = static_cast<uint8_t>(std::round(sum_b_accum));
}

double Blurrer::get_sum_participating_weights_ignore(
	int caller_idx,
	int radius_x, int radius_y,
	int height, int width,
	const vector<double>& weights_x, const vector<double>& weights_y
) {
	double participating_weight_sum = 0.0;

	for (int dy = -radius_y; dy <= radius_y; dy++) {
		for (int dx = -radius_x; dx <= radius_x; dx++) {
			if (!is_outside(caller_idx, dx, dy, height, width)) {
				participating_weight_sum +=
					weights_x[dx + radius_x] *
					weights_y[dy + radius_y];
			}
		}
	}

	return participating_weight_sum;
}

void Blurrer::calculate_gaussian_weights_normalized(
	// Storage
	vector<double>& x_weights_store, vector<double>& y_weights_store,
	double sigma_x, double sigma_y,
	int radius_x, int radius_y
) {
	// Get raw weights first
	calculate_gaussian_weights_raw(
		x_weights_store, y_weights_store,
		sigma_x, sigma_y,
		radius_x, radius_y
	);

	normalize_weights(
		x_weights_store, y_weights_store,
		x_weights_store.size(), y_weights_store.size()
	);

}

void Blurrer::normalize_weights(
	vector<double>& x_weights_store, vector<double>& y_weights_store,
	size_t size_x, size_t size_y
) {
	double x_weights_sum = std::accumulate(
		x_weights_store.begin(), x_weights_store.end(),
		0.0
	);

	double y_weights_sum = std::accumulate(
		y_weights_store.begin(), y_weights_store.end(),
		0.0
	);

	// Make sure we're not dividing by 0
	if (x_weights_sum == 0 && y_weights_sum == 0) {
		throw(std::runtime_error(
			"Blurrer: Gaussian weight normalization tried dividing by 0")
			);
	}

	std::transform(
		x_weights_store.begin(), x_weights_store.end(), x_weights_store.begin(),
		[x_weights_sum](double val) {
			return val / x_weights_sum;
		});

	std::transform(
		y_weights_store.begin(), y_weights_store.end(), y_weights_store.begin(),
		[y_weights_sum](double val) {
			return val / y_weights_sum;
		});
}

void Blurrer::calculate_gaussian_weights_raw(
	// Storage
	vector<double>& x_weights_store, vector<double>& y_weights_store,
	double sigma_x, double sigma_y,
	int radius_x, int radius_y
) {
	calculate_gaussian_weights_1d(
		x_weights_store,
		sigma_x,
		radius_x
	);

	calculate_gaussian_weights_1d(
		y_weights_store,
		sigma_y,
		radius_y
	);
}

void Blurrer::calculate_gaussian_weights_1d(
	vector<double>& i_weights_store,
	double sigma_i,
	int radius_i
) {
	i_weights_store[radius_i] = 1;
	double exp_str;
	for (int64_t di = 1; di <= radius_i; di++) {
		exp_str = (-(static_cast<double>(di * di) / (2.0 * sigma_i * sigma_i)));
		exp_str = exp(exp_str);
		i_weights_store[di + radius_i] = exp_str;
		i_weights_store[radius_i - di] = exp_str;
	}
}

void Blurrer::calculate_binomial_normalized_weights(
	vector<double>& x_weights_store, vector<double>& y_weights_store,
	int radius_x, int radius_y
) {
	calculate_binomial_normalized_weights_1d(x_weights_store, radius_x);
	calculate_binomial_normalized_weights_1d(y_weights_store, radius_y);
}

void Blurrer::calculate_binomial_normalized_weights_1d(
	vector<double>& i_weights_store,
	int radius_i
) {
	// w0 = 2^(-2r)
	// w(di + 1) = wdi * (2r - di) / (di + 1)

	int two_r = 2 * radius_i;

	// First weight (0th) 
	double curr_w = pow(2.0, -two_r);
	i_weights_store[0] = curr_w;
	// Due to symmetry, it's also the last 
	i_weights_store[radius_i * 2] = curr_w;

	/*
	 ex: radius_i = 3
	 so, 0 - 6th indices
	 0th & 6th already calculated above
	 in loop:
	 1st & 5th -> di = 0
	 2nd & 4th -> di = 1
	 3rd & 3rd -> di = 2 = radius_i - 1

	 (precalculated)
	 w0 = 2^(-2r) = 2^(-6) = 1/64
	 w1 = w5 = w0 * (6 - 0) / 1 = 6w0 = 6/64
	 w2 = w4 = w1 * (6 - 1) / 2 = 2.5w1 = 15/64
	 w3 = w3 = w2 * (6 - 2) / 3 = 4w2/3 = 15/64 * 4/3 = 20/64

	 [1/64, 6/64, 15/64, 20/64, 15/64, 6/64, 1/64]
	*/

	for (int di = 0; di < radius_i; di++) {
		// Cast to maintain double precision
		curr_w *= (two_r - di) / static_cast<double>(di + 1);
		// Assign the new values
		// Symmetrical, so assign to opposite index of the center
		i_weights_store[di + 1] = curr_w;
		i_weights_store[two_r - (di + 1)] = curr_w;
	}
}


void Blurrer::fill_kernel_temps(
	vector<uint8_t>& kernel_pixels_r,
	vector<uint8_t>& kernel_pixels_g,
	vector<uint8_t>& kernel_pixels_b,
	int caller_idx,
	int radius_x,
	int radius_y,
	int height,
	int width,
	const vector<uint8_t>& src_r,
	const vector<uint8_t>& src_g,
	const vector<uint8_t>& src_b,
	EdgeMode edge_mode,
	uint8_t constant_edge_r,
	uint8_t constant_edge_g,
	uint8_t constant_edge_b
) {
	int k_i = 0;
	int curr_src_idx;

	if (is_kernel_all_inside(caller_idx, radius_x, radius_y, height, width)) {
		// Go through all kernel pixels
		for (int dy = -radius_y; dy <= radius_y; dy++) {
			for (int dx = -radius_x; dx <= radius_x; dx++) {
				curr_src_idx = caller_idx + dy * width + dx;

				kernel_pixels_r[k_i] = src_r[curr_src_idx];
				kernel_pixels_g[k_i] = src_g[curr_src_idx];
				kernel_pixels_b[k_i] = src_b[curr_src_idx];

				// Increment kernel idx - ready to fill next one
				k_i++;
			}
		}
	}
	else {
		// Go through all kernel pixels
		for (int dy = -radius_y; dy <= radius_y; dy++) {
			for (int dx = -radius_x; dx <= radius_x; dx++) {
				curr_src_idx = caller_idx + dy * width + dx;

				if (is_outside(caller_idx, dx, dy, height, width)) {
					if (edge_mode == EdgeMode::IGNORE) {
						continue;
					}
					if (edge_mode == EdgeMode::CONSTANT) {
						kernel_pixels_r[k_i] = constant_edge_r;
						kernel_pixels_g[k_i] = constant_edge_g;
						kernel_pixels_b[k_i] = constant_edge_b;
						k_i++;
					}
					else {
						size_t mapped_idx = get_mapped_idx_naive(
							edge_mode, caller_idx, dx, dy, width, height
						);
						kernel_pixels_r[k_i] = src_r[mapped_idx];
						kernel_pixels_g[k_i] = src_g[mapped_idx];
						kernel_pixels_b[k_i] = src_b[mapped_idx];
						k_i++;

					}
				}
				else { // As normal
					kernel_pixels_r[k_i] = src_r[curr_src_idx];
					kernel_pixels_g[k_i] = src_g[curr_src_idx];
					kernel_pixels_b[k_i] = src_b[curr_src_idx];

					// Increment kernel idx - ready to fill next one
					k_i++;

				}
			}
		}
	}
}

void Blurrer::find_kernel_median_rgb(
	vector<uint8_t>& kernel_pixels_r,
	vector<uint8_t>& kernel_pixels_g,
	vector<uint8_t>& kernel_pixels_b,
	int start_idx, int end_idx,
	uint8_t& median_r, uint8_t& median_g, uint8_t& median_b
) {
	median_r = find_kernel_median_1d(kernel_pixels_r, start_idx, end_idx);
	median_g = find_kernel_median_1d(kernel_pixels_g, start_idx, end_idx);
	median_b = find_kernel_median_1d(kernel_pixels_b, start_idx, end_idx);
}

uint8_t Blurrer::find_kernel_median_1d(
	vector<uint8_t>& kernel_pixels,
	int start_idx, int end_idx
) {
	int midpoint = (end_idx - start_idx) / 2;
	int median_idx = start_idx + midpoint;

	std::nth_element(
		kernel_pixels.begin() + start_idx,
		kernel_pixels.begin() + median_idx,
		kernel_pixels.begin() + end_idx + 1
	);

	return kernel_pixels[median_idx];
}



void Blurrer::calculate_out_pixel_lens(
	uint8_t& out_r, uint8_t& out_g, uint8_t& out_b,
	int radius,
	int caller_idx,
	int height, int width,
	int total_weights,
	const vector<uint8_t>& src_r,
	const vector<uint8_t>& src_g,
	const vector<uint8_t>& src_b,
	EdgeMode edge_mode,
	uint8_t constant_edge_r,
	uint8_t constant_edge_g,
	uint8_t constant_edge_b
) {
	// Accumulators
	uint64_t sum_r = 0, sum_g = 0, sum_b = 0;
	int rsq = radius * radius;

	// The kernel is inside. No need to check for out-of-bounds
	if (is_kernel_all_inside(caller_idx, radius, radius, height, width)) {
		for (int dy = 0; dy <= radius; dy++) {
			// Circle 
			for (int dx = 0; dx <= sqrt(rsq - dy * dy); dx++) {
				// Using dx,dy, we can calculate for 
				// [dy, dx], [dy, -dx], [-dy, dx], [-dy, -dx].
				// (Ensures no duplication when dy and/or dx are 0
				add_from_src_all_dir(
					sum_r, sum_g, sum_b,
					caller_idx,
					dx, dy,
					width,
					src_r, src_g, src_b
				);
			}
		}

		// Save the output (divide accumulator by the weights)
		set_output_pixel(
			out_r, out_g, out_b,
			sum_r, sum_g, sum_b,
			total_weights
		);
	}
	else {
		int ignore_in_bounds = 0;

		for (int dy = 0; dy <= radius; dy++) {
			for (int dx = 0; dx <= sqrt(rsq - dy * dy); dx++) {
				ignore_in_bounds += add_from_src_all_dir_figure_oob(
					sum_r, sum_g, sum_b,
					caller_idx, dx, dy,
					edge_mode,
					height, width,
					src_r, src_g, src_b,
					constant_edge_r, constant_edge_g, constant_edge_b
				);
			}
		}

		if (edge_mode == EdgeMode::IGNORE) {
			total_weights = ignore_in_bounds;
		}
		set_output_pixel(
			out_r, out_g, out_b,
			sum_r, sum_g, sum_b,
			total_weights
		);

	}
}

int Blurrer::calculate_total_weights_lens(int radius) {
	int counter = 2 * radius + 1;
	int rsq = radius * radius;

	for (int dy = 1; dy <= radius; dy++) {
		counter += 4 * static_cast<int>(sqrt(rsq - dy * dy)) + 2;
	}

	return counter;
}

void Blurrer::add_from_src_all_dir(
	uint64_t& sum_r, uint64_t& sum_g, uint64_t& sum_b,
	int caller_idx,
	int dx, int dy,
	int width,
	const vector<uint8_t>& src_r,
	const vector<uint8_t>& src_g,
	const vector<uint8_t>& src_b
) {
	int curr_idx;

	// ++ -> always
	curr_idx = caller_idx + dy * width + dx;
	add_from_src(
		sum_r, sum_g, sum_b,
		src_r[curr_idx], src_g[curr_idx], src_b[curr_idx]
	);

	// +- -> equal to ++ if dx = 0. Must prevent duplication
	if (dx != 0) {
		curr_idx = caller_idx + dy * width - dx;
		add_from_src(
			sum_r, sum_g, sum_b,
			src_r[curr_idx], src_g[curr_idx], src_b[curr_idx]
		);
	}

	// -+ -> equal to ++ if dy = 0. Must prevent duplication
	if (dy != 0) {
		curr_idx = caller_idx - dy * width + dx;
		add_from_src(
			sum_r, sum_g, sum_b,
			src_r[curr_idx], src_g[curr_idx], src_b[curr_idx]
		);
	}

	// -- -> equal to ++ if dy = 0 and dx == 0. Must prevent duplication
	if (dx != 0 && dy != 0) {
		curr_idx = caller_idx - dy * width - dx;
		add_from_src(
			sum_r, sum_g, sum_b,
			src_r[curr_idx], src_g[curr_idx], src_b[curr_idx]
		);
	}
}

int Blurrer::add_from_src_all_dir_figure_oob(
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
) {
	int ignore_ret = 0;

	ignore_ret += add_from_src_figure_oob(
		sum_r, sum_g, sum_b,
		caller_idx, dx, dy,
		edge_mode,
		height, width,
		src_r, src_g, src_b,
		constant_edge_r, constant_edge_g, constant_edge_b
	);

	// dy, -dx -> equal to dy, dx if dx = 0. Must prevent duplication
	if (dx != 0) {
		ignore_ret += add_from_src_figure_oob(
			sum_r, sum_g, sum_b,
			caller_idx, -dx, dy,
			edge_mode,
			height, width,
			src_r, src_g, src_b,
			constant_edge_r, constant_edge_g, constant_edge_b
		);
	}

	// -dy, dx -> equal to dy, dx if dy = 0. Must prevent duplication
	if (dy != 0) {
		ignore_ret += add_from_src_figure_oob(
			sum_r, sum_g, sum_b,
			caller_idx, dx, -dy,
			edge_mode,
			height, width,
			src_r, src_g, src_b,
			constant_edge_r, constant_edge_g, constant_edge_b
		);
	}

	// -dy, -dx -> equal to dy, dx if dy = 0 and dx == 0. Must prevent duplication
	if (dx != 0 && dy != 0) {
		ignore_ret += add_from_src_figure_oob(
			sum_r, sum_g, sum_b,
			caller_idx, -dx, -dy,
			edge_mode,
			height, width,
			src_r, src_g, src_b,
			constant_edge_r, constant_edge_g, constant_edge_b
		);
	}

	if (edge_mode == EdgeMode::IGNORE) {
		return ignore_ret;
	}
	else {
		return 0;
	}
}

void Blurrer::add_from_src(
	uint64_t& r, uint64_t& g, uint64_t& b,
	uint8_t rta, uint8_t gta, uint8_t bta
) {
	r += rta;
	g += gta;
	b += bta;
}

int Blurrer::add_from_src_figure_oob(
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
) {
	int curr_idx;

	if (is_outside(caller_idx, dx, dy, height, width)) {
		// Ignore - don't add anything
		if (edge_mode == EdgeMode::IGNORE) {
			return 0;
		}

		// Constant - Add the set constants
		if (edge_mode == EdgeMode::CONSTANT) {
			sum_r += constant_edge_r;
			sum_g += constant_edge_g;
			sum_b += constant_edge_b;
			return 1;
		}

		// Remap curr_idx for other edge modes
		curr_idx = get_mapped_idx_naive(
			edge_mode, caller_idx, dx, dy, width, height
		);
	}
	else {
		curr_idx = caller_idx + dy * width + dx;
	}

	// Reading from src the same way regardless of inside or mapped
	add_from_src(
		sum_r, sum_g, sum_b,
		src_r[curr_idx], src_g[curr_idx], src_b[curr_idx]
	);

	return 1;
}

void Blurrer::calculate_sigma_from_radius_gaussian_xy(
	double& sigma_x, double& sigma_y,
	int radius_x, int radius_y
) {
	sigma_x = calculate_sigma_from_radius_gaussian_1d(radius_x);
	sigma_y = calculate_sigma_from_radius_gaussian_1d(radius_y);
}


double Blurrer::calculate_sigma_from_radius_gaussian_1d(int radius) {
	return 0.3 * radius + 0.5;
}
