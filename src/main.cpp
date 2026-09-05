#include "kyrase/image.h"
#include "kyrase/tester.h"
#include "kyrase/reader.h"
#include "kyrase/writer.h"
#include "kyrase/pipeline_runner.h"
#include "kyrase/operation_request.h"
#include "kyrase/image_hash.h"

#include <chrono>

#include <filesystem>

constexpr int RUNS = 3;
constexpr int WARMUP_RUNS = 2;

void compare_images_mae(
	const std::string& path_a,
	const std::string& path_b
) {
	Reader reader;

	Image image_a = reader.read_ppm_p6(path_a);
	Image image_b = reader.read_ppm_p6(path_b);

	if (
		image_a.get_width() != image_b.get_width()
		|| image_a.get_height() != image_b.get_height()
		) {
		throw std::invalid_argument("Images must have the same dimensions");
	}

	const auto& a_r = image_a.get_r();
	const auto& a_g = image_a.get_g();
	const auto& a_b = image_a.get_b();

	const auto& b_r = image_b.get_r();
	const auto& b_g = image_b.get_g();
	const auto& b_b = image_b.get_b();

	uint64_t total_difference = 0;

	for (size_t i = 0; i < a_r.size(); i++) {
		total_difference += std::abs(static_cast<int>(a_r[i]) - static_cast<int>(b_r[i]));
		total_difference += std::abs(static_cast<int>(a_g[i]) - static_cast<int>(b_g[i]));
		total_difference += std::abs(static_cast<int>(a_b[i]) - static_cast<int>(b_b[i]));
	}

	double mae =
		static_cast<double>(total_difference)
		/ (a_r.size() * 3);

	std::cout << "MAE: " << mae << '\n';
}

void generate_kyrase_blur_outputs(
	const std::string& image_path,
	const std::string& test_results_path
) {
	Blurrer blurrer;
	Reader reader;
	Writer writer;

	constexpr uint8_t CONSTANT_R = 17;
	constexpr uint8_t CONSTANT_G = 83;
	constexpr uint8_t CONSTANT_B = 201;

	constexpr int BOX_RADIUS_X = 4;
	constexpr int BOX_RADIUS_Y = 2;
	constexpr int BOX_PASSES = 3;

	constexpr int GAUSSIAN_RADIUS_X = 5;
	constexpr int GAUSSIAN_RADIUS_Y = 3;
	constexpr double GAUSSIAN_SIGMA_X = 1.8;
	constexpr double GAUSSIAN_SIGMA_Y = 1.1;

	constexpr int BINOMIAL_RADIUS_X = 4;
	constexpr int BINOMIAL_RADIUS_Y = 2;

	constexpr int MEDIAN_RADIUS_X = 2;
	constexpr int MEDIAN_RADIUS_Y = 2;

	constexpr int LENS_RADIUS = 4;

	const std::filesystem::path output_dir =
		std::filesystem::path(test_results_path) / "kyrase";

	std::filesystem::create_directories(output_dir);

	const std::vector<std::pair<std::string, EdgeMode>> edge_modes = {
		{"reflect101", EdgeMode::REFLECT101},
		{"reflect", EdgeMode::REFLECT},
		{"clamp", EdgeMode::CLAMP},
		{"wrap", EdgeMode::WRAP},
		{"constant", EdgeMode::CONSTANT},
		{"ignore", EdgeMode::IGNORE}
	};

	Image original_image = reader.read_ppm_p6(image_path);

	const std::string image_name =
		std::filesystem::path(image_path).stem().string();

	for (const auto& [edge_name, edge_mode] : edge_modes) {
		Blurrer::BlurrerOptions options{
			edge_mode,
			{CONSTANT_R, CONSTANT_G, CONSTANT_B}
		};

		// Box
		{
			Blurrer::BlurrerConfig config{
				Blurrer::BoxSettings{
					BOX_RADIUS_X,
					BOX_RADIUS_Y,
					BOX_PASSES
				},
				options
			};

			Image output = blurrer.blur(
				config,
				original_image,
				Blurrer::SINGLE_NAIVE
			);

			writer.write_ppm_p6(
				output_dir.string(),
				"kyrase_" + image_name +
				"_output_box_" + edge_name + ".ppm",
				output
			);
		}

		// Gaussian
		{
			Blurrer::BlurrerConfig config{
				Blurrer::GaussianSettings{
					GAUSSIAN_RADIUS_X,
					GAUSSIAN_RADIUS_Y,
					GAUSSIAN_SIGMA_X,
					GAUSSIAN_SIGMA_Y
				},
				options
			};

			Image output = blurrer.blur(
				config,
				original_image,
				Blurrer::SINGLE_NAIVE
			);

			writer.write_ppm_p6(
				output_dir.string(),
				"kyrase_" + image_name +
				"_output_gaussian_" + edge_name + ".ppm",
				output
			);
		}

		// Binomial
		{
			Blurrer::BlurrerConfig config{
				Blurrer::BinomialSettings{
					BINOMIAL_RADIUS_X,
					BINOMIAL_RADIUS_Y
				},
				options
			};

			Image output = blurrer.blur(
				config,
				original_image,
				Blurrer::SINGLE_NAIVE
			);

			writer.write_ppm_p6(
				output_dir.string(),
				"kyrase_" + image_name +
				"_output_binomial_" + edge_name + ".ppm",
				output
			);
		}

		// Median
		{
			Blurrer::BlurrerConfig config{
				Blurrer::MedianSettings{
					MEDIAN_RADIUS_X,
					MEDIAN_RADIUS_Y
				},
				options
			};

			Image output = blurrer.blur(
				config,
				original_image,
				Blurrer::SINGLE_NAIVE
			);

			writer.write_ppm_p6(
				output_dir.string(),
				"kyrase_" + image_name +
				"_output_median_" + edge_name + ".ppm",
				output
			);
		}

		// Lens
		{
			Blurrer::BlurrerConfig config{
				Blurrer::LensSettings{
					LENS_RADIUS
				},
				options
			};

			Image output = blurrer.blur(
				config,
				original_image,
				Blurrer::SINGLE_NAIVE
			);

			writer.write_ppm_p6(
				output_dir.string(),
				"kyrase_" + image_name +
				"_output_lens_" + edge_name + ".ppm",
				output
			);
		}
	}
}


void bench_blur(
	const Blurrer::BlurrerConfig& config,
	const Image& original_image,
	Blurrer::OptimizationLevel optimization_level,
	const std::string& output_path,
	const std::string& result_name
) {
	Blurrer blurrer;
	Writer writer;

	double accumulator_ms_non_mod = 0;
	double accumulator_ms_mod = 0;

	Image image_non_mod = original_image;
	Image modifiable_image = original_image;

	for (int i = 0; i < RUNS + WARMUP_RUNS; i++) {
		modifiable_image = original_image;

		auto start_non_mod = std::chrono::steady_clock::now();

		image_non_mod = blurrer.blur(
			config,
			original_image,
			optimization_level
		);

		auto end_non_mod = std::chrono::steady_clock::now();

		auto start_mod = std::chrono::steady_clock::now();

		blurrer.blur_modifiable(
			config,
			modifiable_image,
			optimization_level
		);

		auto end_mod = std::chrono::steady_clock::now();

		if (i >= WARMUP_RUNS) {
			accumulator_ms_non_mod +=
				std::chrono::duration<double, std::milli>(
					end_non_mod - start_non_mod
				).count();

			accumulator_ms_mod +=
				std::chrono::duration<double, std::milli>(
					end_mod - start_mod
				).count();
		}
	}

	std::cout
		<< result_name << " non-mod average run (ms): "
		<< accumulator_ms_non_mod / RUNS << '\n'

		<< result_name << " mod average run (ms): "
		<< accumulator_ms_mod / RUNS << '\n'

		<< result_name << " non-mod hash: "
		<< hash_image_fnv1a(image_non_mod) << '\n'

		<< result_name << " mod hash: "
		<< hash_image_fnv1a(modifiable_image) << '\n';

	writer.write_ppm_p6(
		output_path,
		result_name + "_non_mod.ppm",
		image_non_mod
	);

	writer.write_ppm_p6(
		output_path,
		result_name + "_mod.ppm",
		modifiable_image
	);
}

void bench_sharpen(
	const Sharpener::SharpenerConfig& config,
	const Image& original_image,
	Sharpener::SharpenOptimizationLevel optimization_level,
	const std::string& output_path,
	const std::string& result_name
) {
	Sharpener sharpener;
	Writer writer;

	double accumulator_ms_non_mod = 0;
	double accumulator_ms_mod = 0;

	Image image_non_mod = original_image;
	Image modifiable_image = original_image;

	for (int i = 0; i < RUNS + WARMUP_RUNS; i++) {
		modifiable_image = original_image;

		auto start_non_mod = std::chrono::steady_clock::now();

		image_non_mod = sharpener.sharpen(
			config,
			original_image,
			optimization_level
		);

		auto end_non_mod = std::chrono::steady_clock::now();

		auto start_mod = std::chrono::steady_clock::now();

		sharpener.sharpen_modifiable(
			config,
			modifiable_image,
			optimization_level
		);

		auto end_mod = std::chrono::steady_clock::now();

		if (i >= WARMUP_RUNS) {
			accumulator_ms_non_mod +=
				std::chrono::duration<double, std::milli>(
					end_non_mod - start_non_mod
				).count();

			accumulator_ms_mod +=
				std::chrono::duration<double, std::milli>(
					end_mod - start_mod
				).count();
		}
	}

	std::cout
		<< result_name << " non-mod average run (ms): "
		<< accumulator_ms_non_mod / RUNS << '\n'

		<< result_name << " mod average run (ms): "
		<< accumulator_ms_mod / RUNS << '\n'

		<< result_name << " non-mod hash: "
		<< hash_image_fnv1a(image_non_mod) << '\n'

		<< result_name << " mod hash: "
		<< hash_image_fnv1a(modifiable_image) << '\n';

	writer.write_ppm_p6(
		output_path,
		result_name + "_non_mod.ppm",
		image_non_mod
	);

	writer.write_ppm_p6(
		output_path,
		result_name + "_mod.ppm",
		modifiable_image
	);
}

int main() {
	const std::string input_path =
		R"(C:\Users\Kyryll\Kyrase\kyrase-image-tools\assets\cat2.ppm)";

	const std::string output_path =
		R"(C:\Users\Kyryll\Kyrase\kyrase-image-tools\test_results)";

	Reader reader;

	Image img = reader.read_ppm_p6(input_path);


	// ========================================================================
	// 1) UNSHARP MASK
	// ========================================================================

	Blurrer::BlurrerConfig blurrer_config{
		Blurrer::GaussianSettings{
			3,      // radius x
			3,      // radius y
			1.4,    // sigma x
			1.4     // sigma y
		},
		Blurrer::BlurrerOptions{
			EdgeMode::CLAMP
		}
	};

	Sharpener::SharpenerConfig unsharp_config{
		Sharpener::UnsharpMaskSettings{
			blurrer_config,
			Blurrer::SINGLE_NAIVE,
			1.0,    // sharp amount
			0       // threshold
		}
	};

	bench_sharpen(
		unsharp_config,
		img,
		Sharpener::SINGLE_NAIVE,
		output_path,
		"unsharp_gaussian_single_naive"
	);


	// ========================================================================
	// 2) INDEPENDENT HIGH PASS
	// ========================================================================

	Blurrer blurrer;

	Image blurred_image = blurrer.blur(
		blurrer_config,
		img,
		Blurrer::SINGLE_NAIVE
	);

	Sharpener::SharpenerConfig high_pass_config{
		Sharpener::HighPassSettings{
			blurred_image,
			1.0,    // sharp amount
			0       // threshold
		}
	};

	bench_sharpen(
		high_pass_config,
		img,
		Sharpener::SINGLE_NAIVE,
		output_path,
		"high_pass_gaussian_single_naive"
	);


	// ========================================================================
	// 3) CONVOLUTIONAL SHARPENING
	// ========================================================================

	vector<double> sharpen_kernel{
		 0.0, -1.0,  0.0,
		-1.0,  5.0, -1.0,
		 0.0, -1.0,  0.0
	};

	Sharpener::SharpenerConfig convolutional_config{
		Sharpener::ConvolutionalSettings{
			sharpen_kernel,
			1,      // radius x
			1       // radius y
		},
		Sharpener::SharpenerOptions{
			EdgeMode::CLAMP
		}
	};

	bench_sharpen(
		convolutional_config,
		img,
		Sharpener::SINGLE_NAIVE,
		output_path,
		"convolutional_3x3_single_naive"
	);

	return 0;
}