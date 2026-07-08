#include "../include/kyrase/reader.h"

Image Reader::read_ppm_p6(const string& file_name) {
	std::filesystem::path path = file_name;

	if (path.extension() != ".ppm") {
		throw std::invalid_argument(
			"Reader: PPM Reader requires .ppm file extension"
		);
	}

	std::ifstream input_file(file_name, std::ios::binary);
	
	if (!input_file) {
		throw std::runtime_error(
			"Reader: PPM Reader could not open file"
		);
	}

	std::string magic;

	// Read the magic number
	input_file >> magic;

	if (magic != "P6") {
		throw std::invalid_argument(
			"Reader: PPM Reader requires P6 magic number"
		);
	}

	int width = 0, height = 0, max_color_val = 0;

	// Read header information
	if (!(input_file >> width >> height >> max_color_val)) {
		throw std::runtime_error("Reader: Malformed PPM header");
	}

	// Consume whitespace
	input_file.get();

	// All pixels stored sequentially
	vector<uint8_t> pixels(width * height * 3);

	// Read all the pixels into the big array
	input_file.read(
		reinterpret_cast<char*>(pixels.data()),
		pixels.size()
	);

	vector<uint8_t> r(width * height);
	vector<uint8_t> g(width * height);
	vector<uint8_t> b(width * height);

	// Distribute the pixels by channel
	for (size_t i = 0; i < width * height; i++) {
		r[i] = pixels[i * 3 + 0];
		g[i] = pixels[i * 3 + 1];
		b[i] = pixels[i * 3 + 2];
	}

	// Construct the image from data from the file
	return Image("PPM", width, height, 255, r, g, b);
}
