#include "kyrase/writer.h"

void Writer::write_ppm_p6(
	const string& file_path, 
	const string& file_name,
	const Image& image
) {

	if (file_path.empty() || file_name.empty()) {
		throw std::invalid_argument(
			"Writer:: PPM Writer expects non-empty file information"
		);
	}

	constexpr const char* MAGIC = "P6";

	std::filesystem::path full_path = std::filesystem::path(file_path) / file_name;

	// Add extension if it wasn't in the name
	if (full_path.extension().empty()) {
		full_path += ".ppm";
	}
	// If the wrong extension was in the name, that's an issue
	else if (full_path.extension() != ".ppm") {
		throw std::invalid_argument(
			"Writer: PPM Writer requires .ppm file extension"
		);
	}
	
	// Create output stream for the given file
	std::ofstream output_file(full_path, std::ios::binary);

	// Ensure that the path is valid
	if (!output_file) {
		throw std::runtime_error(
			"Writer: PPM Writer could not create output file"
		);
	}
	
	int width = image.get_width();
	int height = image.get_height();

	// Write header
	if (!(output_file
		<< MAGIC << '\n'
		<< width << " "
		<< height << '\n'
		<< image.get_max_color_val())) {
		throw std::runtime_error("Writer: PPM Writer failed to write header");
	}

	// Add whitespace
	output_file << '\n';
	
	const size_t pixel_count = static_cast<size_t>(width) 
		* static_cast<size_t>(height);

	// Get the channel arrays
	const vector<uint8_t>& r = image.get_r();
	const vector<uint8_t>& g = image.get_g();
	const vector<uint8_t>& b = image.get_b();

	// Unified pixels array
	vector<uint8_t> pixels(pixel_count * 3);

	for (size_t i = 0, j = 0; i < pixel_count; ++i, j += 3) {
		pixels[j + 0] = r[i];
		pixels[j + 1] = g[i];
		pixels[j + 2] = b[i];
	}

	// Write the pixel color values
	output_file.write(
		reinterpret_cast<const char*>(pixels.data()),
		static_cast<std::streamsize>(pixels.size())
	);

	// Couldn't write the pixel data
	if (!output_file) {
		throw std::runtime_error("Writer: PPM Writer failed to write pixel data");
	}
}
