#ifndef READER_H_
#define READER_H_

#include <string>
#include <fstream>
#include <filesystem>

#include "image.h"

using std::string;

class Reader {
public:
	// Reads the PPM image in the given file path (if the path exists)
	// Converts into internal image representation
	Image read_ppm_p6(const string& file_path);
};


#endif
