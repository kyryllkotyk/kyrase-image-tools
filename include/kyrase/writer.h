#ifndef WRITER_H_
#define WRITER_H_

#include <string>
#include <fstream>
#include <filesystem>

#include "../include/kyrase/image.h"

using std::string;

class Writer {
public:
	// Converts internal image representation back into a PPM file
	// Saves the PPM image in the given file path (if the path exists)
	void write_ppm_p6(const string& file_path, const string& file_name, const Image& image);
};


#endif
