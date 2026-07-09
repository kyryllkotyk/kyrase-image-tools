#include "../include/kyrase/pipeline_runner.h"

void PipelineRunner::run_operation(const OperationRequest& request) {	
	Reader reader;
	
	// Read the image information to get the image
	Image image = reader.read_ppm_p6(request.input_file);


	// BLURRER

	if (request.name == OperationName::BLUR) {
		// Ensure the config matches the name
		if (!std::holds_alternative<BlurrerConfig>(request.operation_config)) {
			throw std::invalid_argument(
				"Pipeline Runner: Blur operation expects BlurrerConfig"
			);
		}

		// Get the config now that it's confirmed
		const BlurrerConfig& config = std::get<BlurrerConfig>
			(request.operation_config);

		// Fill out blur settings, call blurrer
		//TODO:: Add once blur is implemented
	}
	

	// SHARPENER

	else if (request.name == OperationName::SHARPEN) {
		// Ensure config matches the name
		if (!std::holds_alternative<SharpenerConfig>(request.operation_config)) {
			throw std::invalid_argument(
				"Pipeline Runner: Sharpen operation expects SharpenerConfig"
			);
		}

		// Get the config now that it's confirmed
		const SharpenerConfig& config = std::get<SharpenerConfig>(request.operation_config);
		
		// Fill out sharpen settings, call sharpener
		//TODO:: Add once sharpener is implemented
	}
	
	// If saving results was requested
	if (request.output_options.save_results) {
		Writer writer;
		
		// Ensure correct format
		if (image.get_image_format() == "P6" || image.get_image_format() == "PPM") {
			writer.write_ppm_p6(
				request.output_options.output_path,
				request.output_options.output_name,
				image
			);
		}
		// Unsupported format
		else {
			throw std::invalid_argument(
				"Pipeline Runner: Unsupported image format"
			);
		}
	}
}
