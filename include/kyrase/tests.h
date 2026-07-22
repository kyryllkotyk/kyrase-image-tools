#ifndef TESTS_H_
#define TESTS_H_
#include "kyrase/tester.h"
#include "kyrase/blurrer.h"
#include "kyrase/image.h"
#include "kyrase/reader.h"
#include "kyrase/writer.h"
#include "kyrase/pipeline_runner.h"
#include "kyrase/operation_request.h"
#include "kyrase/tests.h"

#include <stdexcept>
#include <string>
#include <vector>

#include <fstream>
#include <filesystem>

using std::string;
using std::vector;
using BlurrerBoxSettings = Blurrer::BlurrerBoxSettings;

namespace fs = std::filesystem;

string edge_mode_to_string(const Blurrer::BlurrerEdgeMode mode);
void run_box_blur_tests();
bool run_image_tests();
bool run_reader_tests();
bool run_writer_tests();
bool run_pipeline_runner_tests();

#endif
