#include "../include/torasu/std/Rmultiply.hpp"

#include <memory>
#include <string>
#include <optional>
#include <chrono>

#include <torasu/torasu.hpp>
#include <torasu/render_tools.hpp>
#include <torasu/log_tools.hpp>

#include <torasu/std/Dbimg.hpp>

using namespace std;

namespace {
auto IDENT = "STD::RMULTIPLY";
} // namespace

namespace torasu::tstd {

Rmultiply::Rmultiply(NumSlot a, NumSlot b)
	: SimpleRenderable(false, true), a(a), b(b) {}

Rmultiply::~Rmultiply() {}

Identifier Rmultiply::getType() {
	return IDENT;
}

RenderResult* Rmultiply::render(RenderInstruction* ri) {

	torasu::tools::RenderHelper rh(ri);
	auto& lrib = rh.lrib;

	auto selPipleine = ri->getResultSettings()->getPipeline();

	if (selPipleine == TORASU_STD_PL_NUM) {
		torasu::ResultSettings resSetting(TORASU_STD_PL_NUM, torasu::tools::NO_FORMAT);
		auto rendA = rh.enqueueRender(a.get(), &resSetting);
		auto rendB = rh.enqueueRender(b.get(), &resSetting);

		std::unique_ptr<RenderResult> resA(rh.fetchRenderResult(rendA));
		std::unique_ptr<RenderResult> resB(rh.fetchRenderResult(rendB));

		auto a = rh.evalResult<Dnum>(resA.get());
		auto b = rh.evalResult<Dnum>(resB.get());

		if (a && b) {
			double mulRes = a.getResult()->getNum() * b.getResult()->getNum();
			return rh.buildResult(new Dnum(mulRes));
		} else {
			if (rh.mayLog(WARN)) {
				torasu::tools::LogInfoRefBuilder errorCauses(lrib.linstr);
				if (!a)
					errorCauses.logCause(WARN, "Operand A failed to render", a.takeInfoTag());
				if (!b)
					errorCauses.logCause(WARN, "Operand B failed to render", b.takeInfoTag());

				lrib.logCause(WARN, "Sub render failed to provide operands, returning 0", errorCauses);
			}

			return rh.buildResult(new Dnum(0), RenderResultStatus_OK_WARN);
		}

	} else if (selPipleine == TORASU_STD_PL_VIS) {
		Dbimg_FORMAT* fmt;
		if (!(fmt = rh.getFormat<Dbimg_FORMAT>())) {
			return new RenderResult(RenderResultStatus_INVALID_FORMAT);
		}

		torasu::tools::ResultSettingsSingleFmt resSetting(TORASU_STD_PL_VIS, fmt);

		// Sub-Renderings

		auto rendA = rh.enqueueRender(a.get(), &resSetting);
		auto rendB = rh.enqueueRender(b.get(), &resSetting);

		unique_ptr<RenderResult> resA(rh.fetchRenderResult(rendA));
		unique_ptr<RenderResult> resB(rh.fetchRenderResult(rendB));

		// Calculating Result from Results

		auto a = rh.evalResult<Dbimg>(resA.get());
		auto b = rh.evalResult<Dbimg>(resB.get());


		if (a && b) {
			Dbimg* result;
			if (a.canFreeResult()) result = a.ejectResult();
			else if (b.canFreeResult()) result = b.ejectResult();
			else result = new Dbimg(*fmt);

			const uint32_t height = result->getHeight();
			const uint32_t width = result->getWidth();
			const uint32_t channels = 4;
			const size_t dataSize = height*width*channels;


			uint8_t* srcA = a.getResult()->getImageData();
			uint8_t* srcB = b.getResult()->getImageData();
			uint8_t* dest = result->getImageData();

			auto li = ri->getLogInstruction();

			bool doBench = li.level <= LogLevel::DEBUG;
			std::chrono::time_point<std::chrono::steady_clock> bench;
			if (doBench) bench = std::chrono::steady_clock::now();

			for (size_t i = 0; i < dataSize; i++) {
				// dest[i] = (srcA[i]>>4)*(srcB[i]>>4);
				dest[i] = ((uint16_t) srcA[i]*srcB[i]) >> 8;
				// *dest++ = ((uint16_t) *srcA++ * *srcB++) >> 8;
			}

			if (doBench) li.logger->log(LogLevel::DEBUG,
											"Mul Time = " + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - bench).count()) + "[ms]");


			return rh.buildResult(result);
		} else {

			if (rh.mayLog(WARN)) {
				torasu::tools::LogInfoRefBuilder errorCauses(lrib.linstr);
				if (!a)
					errorCauses.logCause(WARN, "Operand A failed to render", a.takeInfoTag());
				if (!b)
					errorCauses.logCause(WARN, "Operand B failed to render", b.takeInfoTag());

				lrib.logCause(WARN, "Sub render failed to provide operands, returning empty image", errorCauses);

			}

			Dbimg* errRes = new Dbimg(*fmt);
			errRes->clear();
			return rh.buildResult(errRes, RenderResultStatus_OK_WARN);
		}

	} else {
		return new RenderResult(RenderResultStatus_INVALID_SEGMENT);
	}

}

torasu::ElementMap Rmultiply::getElements() {
	torasu::ElementMap elems;

	elems["a"] = a;
	elems["b"] = b;

	return elems;
}

const torasu::OptElementSlot Rmultiply::setElement(std::string key, const ElementSlot* elem) {
	if (key == "a") return NumSlot::trySetRenderableSlot(&a, elem, 1);
	if (key == "b") return NumSlot::trySetRenderableSlot(&b, elem, 1);
	return nullptr;
}

namespace {

static const ElementFactory::SlotDescriptor SLOT_INDEX[] = {
	{.id = "a", .label = {.name = "Operand A", .description = "Multiplier on the left side"}, .optional = false, .renderable = true},
	{.id = "b", .label = {.name = "Operand B", .description = "Multiplier on the right side"}, .optional = false, .renderable = true},
};

static class : public torasu::ElementFactory {
	torasu::Identifier getType() const override {
		return IDENT;
	}

	torasu::UserLabel getLabel() const override {
		return {
			.name = "Multiply",
			.description = "Multiplies two values"
		};
	}

	torasu::Element* create(torasu::DataResource** data, const torasu::ElementMap& elements) const override {
		std::unique_ptr<Rmultiply> elem(new Rmultiply(1, 1));
		for (auto element : elements) {
			elem->setElement(element.first, &element.second);
		}
		return elem.release();
	}

	SlotIndex getSlotIndex() const override {
		return {.slotIndex = SLOT_INDEX, .slotCount = sizeof(SLOT_INDEX)/sizeof(ElementFactory::SlotDescriptor)};
	}
} FACTORY_INSTANCE;

} // namespace

const torasu::ElementFactory* const Rmultiply::FACTORY = &FACTORY_INSTANCE;

} // namespace torasu::tstd