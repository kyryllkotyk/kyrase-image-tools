#ifndef OPERATION_REQUEST_H_
#define OPERATION_REQUEST_H_

#include <string>
#include <variant>

#include "sharpener.h"
#include "blurrer.h"

using std::string;

using BlurrerConfig = Blurrer::BlurrerConfig;
using SharpenerConfig = Sharpener::SharpenerConfig;

enum class OperationName {
    BLUR,
    SHARPEN 
};

// Simple options for output, will be expanded as needed
struct OutputOptions {
    bool save_results = false;
    string output_path;
    string output_name;
};

// TODO:: Implement
//struct ResultInfoOptions {
//    string result_file;
//    bool save_op_name;
//    bool save_width;
//    bool save_height;
//    bool save_channels;
//    bool save_image_format;
//    bool save_max_color_val;
//    bool save_pixel_count;
//};

// Can either be a config for blurrer or for sharpener
// As more features get added, more configs will get added here as well
using OperationConfig = std::variant<
    BlurrerConfig,
    SharpenerConfig
>;

struct OperationRequest {
    OperationName name;
    string input_file;
    OutputOptions output_options;
    //ResultInfoOptions result_options;
    OperationConfig operation_config;
};

#endif