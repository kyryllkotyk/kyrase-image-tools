#ifndef PIPELINE_RUNNER_H_
#define PIPELINE_RUNNER_H_

#include <string>

#include "operation_request.h"
#include "image.h"
#include "reader.h"
#include "writer.h"

using std::string;

class PipelineRunner {
public:
	void run_operation(const OperationRequest& request);
	//TODO:: Implement
	void run_benchmark(const OperationRequest& request, int runs, int ops_per_run);
};

#endif
