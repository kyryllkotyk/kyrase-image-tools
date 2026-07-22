#include "kyrase/tests.h"

bool run_image_tests() {
    Tester tester;

    std::vector<uint8_t> good_r(4);
    std::vector<uint8_t> good_g(4);
    std::vector<uint8_t> good_b(4);
    std::vector<uint8_t> good_a(4);
    std::vector<uint8_t> bad_size(3);

    tester.add_test("Invalid format", [] {
        Image image("", 2, 2, 3, 255);
        }, true);

    tester.add_test("Invalid width", [] {
        Image image("P6", 0, 2, 3, 255);
        }, true);

    tester.add_test("Invalid height", [] {
        Image image("P6", 2, -1, 3, 255);
        }, true);

    tester.add_test("Invalid channel count low", [] {
        Image image("P6", 2, 2, 0, 255);
        }, true);

    tester.add_test("Invalid channel count high", [] {
        Image image("P6", 2, 2, 5, 255);
        }, true);

    tester.add_test("Unsupported max color value", [] {
        Image image("P6", 2, 2, 3, 100);
        }, true);

    tester.add_test("Valid RGB allocation constructor", [] {
        Image image("P6", 2, 2, 3, 255);
        }, false);

    tester.add_test("Valid RGBA allocation constructor", [] {
        Image image("P6", 2, 2, 4, 255);
        }, false);

    tester.add_test("RGB vector constructor with wrong R size", [=] {
        Image image("P6", 2, 2, 255, bad_size, good_g, good_b);
        }, true);

    tester.add_test("RGBA vector constructor with wrong A size", [=] {
        Image image("P6", 2, 2, 255, good_r, good_g, good_b, bad_size);
        }, true);

    tester.add_test("Valid RGB vector constructor", [=] {
        Image image("P6", 2, 2, 255, good_r, good_g, good_b);
        }, false);

    tester.add_test("Valid RGBA vector constructor", [=] {
        Image image("P6", 2, 2, 255, good_r, good_g, good_b, good_a);
        }, false);

    return tester.run_tests();
}


bool run_reader_tests() {
    Tester tester;
    Reader reader;

    const std::filesystem::path assets_path = "../../../assets";

    auto asset = [&assets_path](const std::string& file_name) {
        return (assets_path / file_name).string();
        };

    tester.add_test(
        "Reader accepts small 5x5 PPM image",
        [&reader, &asset]() {
            Image image = reader.read_ppm_p6(asset("testimage.ppm"));
        },
        false
    );

    tester.add_test(
        "Reader accepts larger phone-converted PPM image",
        [&reader, &asset]() {
            Image image = reader.read_ppm_p6(asset("cat1.ppm"));
        },
        false
    );

    tester.add_test(
        "Reader rejects non-PPM extension",
        [&reader, &asset]() {
            Image image = reader.read_ppm_p6(asset("testimage.jpg"));
        },
        true
    );

    tester.add_test(
        "Reader rejects non-P6 magic number",
        [&reader, &asset]() {
            const std::string filename = asset("test_bad_magic.ppm");

            {
                std::ofstream file(filename, std::ios::binary);
                file << "P3\n";
                file << "1 1\n";
                file << "255\n";
                file << "0 0 0\n";
            }

            try {
                Image image = reader.read_ppm_p6(filename);
                std::filesystem::remove(filename);
            }
            catch (...) {
                std::filesystem::remove(filename);
                throw;
            }
        },
        true
    );

    tester.add_test(
        "Reader rejects malformed PPM header",
        [&reader, &asset]() {
            const std::string filename = asset("test_malformed_header.ppm");

            {
                std::ofstream file(filename, std::ios::binary);
                file << "P6\n";
                file << "5 5\n";
                // Missing max color value
            }

            try {
                Image image = reader.read_ppm_p6(filename);
                std::filesystem::remove(filename);
            }
            catch (...) {
                std::filesystem::remove(filename);
                throw;
            }
        },
        true
    );

    tester.add_test(
        "Reader rejects missing PPM file",
        [&reader, &asset]() {
            Image image = reader.read_ppm_p6(asset("does_not_exist.ppm"));
        },
        true
    );

    return tester.run_tests();
}

bool run_writer_tests() {
    Tester tester;
    Reader reader;
    Writer writer;

    const std::filesystem::path assets_path = "../../../assets";

    auto asset = [&assets_path](const std::string& file_name) {
        return (assets_path / file_name).string();
        };

    tester.add_test(
        "Writer writes small 5x5 PPM image",
        [&reader, &writer, &assets_path, &asset]() {
            Image image = reader.read_ppm_p6(asset("testimage.ppm"));

            writer.write_ppm_p6(
                assets_path.string(),
                "testimage_post.ppm",
                image
            );

            if (!std::filesystem::exists(asset("testimage_post.ppm"))) {
                throw std::runtime_error("Writer test: testimage_post.ppm was not created");
            }
        },
        false
    );

    tester.add_test(
        "Writer writes larger phone-converted PPM image",
        [&reader, &writer, &assets_path, &asset]() {
            Image image = reader.read_ppm_p6(asset("cat1.ppm"));

            writer.write_ppm_p6(
                assets_path.string(),
                "cat1_post.ppm",
                image
            );

            if (!std::filesystem::exists(asset("cat1_post.ppm"))) {
                throw std::runtime_error("Writer test: cat1_post.ppm was not created");
            }
        },
        false
    );

    tester.add_test(
        "Writer adds missing .ppm extension",
        [&reader, &writer, &assets_path, &asset]() {
            Image image = reader.read_ppm_p6(asset("testimage.ppm"));

            writer.write_ppm_p6(
                assets_path.string(),
                "testimage_no_extension_post",
                image
            );

            if (!std::filesystem::exists(asset("testimage_no_extension_post.ppm"))) {
                throw std::runtime_error("Writer test: missing-extension file was not created");
            }
        },
        false
    );

    tester.add_test(
        "Writer rejects empty file path",
        [&reader, &writer, &asset]() {
            Image image = reader.read_ppm_p6(asset("testimage.ppm"));

            writer.write_ppm_p6(
                "",
                "bad_output.ppm",
                image
            );
        },
        true
    );

    tester.add_test(
        "Writer rejects empty file name",
        [&reader, &writer, &assets_path, &asset]() {
            Image image = reader.read_ppm_p6(asset("testimage.ppm"));

            writer.write_ppm_p6(
                assets_path.string(),
                "",
                image
            );
        },
        true
    );

    tester.add_test(
        "Writer rejects wrong file extension",
        [&reader, &writer, &assets_path, &asset]() {
            Image image = reader.read_ppm_p6(asset("testimage.ppm"));

            writer.write_ppm_p6(
                assets_path.string(),
                "bad_output.jpg",
                image
            );
        },
        true
    );

    tester.add_test(
        "Writer rejects missing output directory",
        [&reader, &writer, &asset]() {
            Image image = reader.read_ppm_p6(asset("testimage.ppm"));

            writer.write_ppm_p6(
                "../../../assets/this_folder_should_not_exist",
                "bad_output.ppm",
                image
            );
        },
        true
    );

    return tester.run_tests();
}

bool run_pipeline_runner_tests() {
    Tester tester;

    const string input_file = "../../../assets/testimage.ppm";

    tester.add_test(
        "PipelineRunner: Run blur with matching config and no save",
        [=]() {
            PipelineRunner runner;

            OperationRequest request{
                OperationName::BLUR,
                input_file,
                OutputOptions{
                    false,
                    "",
                    ""
                },
                BlurrerConfig{
                    BlurrerBoxSettings{3, 3}
                }
            };

            runner.run_operation(request);
        },
        false
    );

    tester.add_test(
        "PipelineRunner: Run sharpen with matching config and no save",
        [=]() {
            PipelineRunner runner;

            OperationRequest request{
                OperationName::SHARPEN,
                input_file,
                OutputOptions{
                    false,
                    "",
                    ""
                },
                SharpenerConfig{}
            };

            runner.run_operation(request);
        },
        false
    );

    tester.add_test(
        "PipelineRunner: Blur rejects SharpenerConfig",
        [=]() {
            PipelineRunner runner;

            OperationRequest request{
                OperationName::BLUR,
                input_file,
                OutputOptions{
                    false,
                    "",
                    ""
                },
                SharpenerConfig{}
            };

            runner.run_operation(request);
        },
        true
    );

    tester.add_test(
        "PipelineRunner: Sharpen rejects BlurrerConfig",
        [=]() {
            PipelineRunner runner;

            OperationRequest request{
                OperationName::SHARPEN,
                input_file,
                OutputOptions{
                    false,
                    "",
                    ""
                },
                BlurrerConfig{
                    BlurrerBoxSettings{3, 3}
                }
            };

            runner.run_operation(request);
        },
        true
    );

    tester.add_test(
        "PipelineRunner: Save result writes output file",
        [=]() {
            PipelineRunner runner;

            const fs::path output_dir = fs::temp_directory_path();
            const string output_name = "kyrase_pipeline_runner_test_output.ppm";
            const fs::path output_file = output_dir / output_name;

            if (fs::exists(output_file)) {
                fs::remove(output_file);
            }

            OperationRequest request{
                OperationName::BLUR,
                input_file,
                OutputOptions{
                    true,
                    output_dir.string(),
                    output_name
                },
                BlurrerConfig{
                    BlurrerBoxSettings{3, 3}
                }
            };

            runner.run_operation(request);

            if (!fs::exists(output_file)) {
                throw std::runtime_error(
                    "PipelineRunner test expected output file to be created"
                );
            }

            fs::remove(output_file);
        },
        false
    );

    return tester.run_tests() ? 0 : 1;
}


string edge_mode_to_string(const Blurrer::BlurrerEdgeMode mode) {
	switch (mode) {
	case Blurrer::BlurrerEdgeMode::REFLECT101:
		return "reflect101";
	case Blurrer::BlurrerEdgeMode::REFLECT:
		return "reflect";
	case Blurrer::BlurrerEdgeMode::CLAMP:
		return "clamp";
	case Blurrer::BlurrerEdgeMode::WRAP:
		return "wrap";
	case Blurrer::BlurrerEdgeMode::CONSTANT:
		return "constant";
	case Blurrer::BlurrerEdgeMode::REFLECT_CALLER:
		return "reflect_caller";
	default:
		return "unknown";
	}
}
void run_box_blur_tests() {
	Tester tester;

	tester.add_test("Blur test with saved PPM outputs", []() {
		Reader reader;
		Writer writer;
		Blurrer blurrer;

		const std::filesystem::path main_folder =
			R"(C:\Users\Kyryll\Kyrase\kyrase-image-tools)";

		const std::filesystem::path input_file_path =
			main_folder / "assets" / "blur_test_img.ppm";

		const std::filesystem::path output_path =
			main_folder / "test_results";

		std::filesystem::create_directories(output_path);

		const string input_file = input_file_path.string();
		const string output_path_string = output_path.string();

		const vector<Blurrer::BlurrerEdgeMode> edge_modes = {
			Blurrer::BlurrerEdgeMode::REFLECT101,
			Blurrer::BlurrerEdgeMode::REFLECT,
			Blurrer::BlurrerEdgeMode::CLAMP,
			Blurrer::BlurrerEdgeMode::WRAP,
			Blurrer::BlurrerEdgeMode::CONSTANT,
			Blurrer::BlurrerEdgeMode::REFLECT_CALLER
		};

		for (const Blurrer::BlurrerEdgeMode edge_mode : edge_modes) {
			Image image = reader.read_ppm_p6(input_file);

			Blurrer::BlurrerConfig config{
				Blurrer::BlurrerBoxSettings{
					2,
					2,
					1
				},
				Blurrer::BlurrerOptions{
					edge_mode,
					{ 0, 0, 0 }
				}
			};

			Image blurred = blurrer.blur_naive(
				config,
				image
			);

			const string output_name =
				"blur_test_result_" + edge_mode_to_string(edge_mode);

			writer.write_ppm_p6(
				output_path_string,
				output_name,
				blurred
			);

			const std::filesystem::path reread_path =
				output_path / (output_name + ".ppm");

			Image reread = reader.read_ppm_p6(
				reread_path.string()
			);

			if (reread.get_width() != image.get_width()
				|| reread.get_height() != image.get_height()
				|| reread.get_channels() != image.get_channels()
				|| reread.get_r().size() != image.get_r().size()
				|| reread.get_g().size() != image.get_g().size()
				|| reread.get_b().size() != image.get_b().size()) {
				throw std::runtime_error(
					"Blur output failed round-trip validation for "
					+ edge_mode_to_string(edge_mode)
				);
			}
		}
	},
	false
	);

	tester.run_tests();
}