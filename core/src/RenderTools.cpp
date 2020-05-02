#include "../include/torasu/tools.hpp"

using namespace std;

namespace torasu::tools {

RenderInstructionBuilder::RenderInstructionBuilder() {
	segments = new vector<ResultSegmentSettings*>();
}

RenderInstructionBuilder::~RenderInstructionBuilder() {
	for (ResultSegmentSettings* segment : *segments) {
		delete segment;
	}

	delete segments;
}

void RenderInstructionBuilder::buildResultSettings() {
	if (rs == NULL) {
		delete rs;
	}

	rs = new ResultSettings();

	for (ResultSegmentSettings* segment : *segments) {
		rs->push_back(segment);
	}

}

} // namespace torasu::tools