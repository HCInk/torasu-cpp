#ifndef STD_INCLUDE_TORASU_STD_RMATRIX_HPP_
#define STD_INCLUDE_TORASU_STD_RMATRIX_HPP_

#include <string>
#include <initializer_list>

#include <torasu/torasu.hpp>
#include <torasu/SimpleRenderable.hpp>

#include <torasu/std/Dmatrix.hpp>

namespace torasu::tstd {

class Rmatrix : public tools::SimpleRenderable {
private:
	Dmatrix* valdr;

protected:
	virtual ResultSegment* renderSegment(ResultSegmentSettings* resSettings, RenderInstruction* ri);

public:
	/**
	 * @brief  Create Matrix-Renderable
	 * @param  numbers: Numbers to initialize
	 * @param  height: Height of matrix (0 to take size of numbers as height / create vertical vector)
	 */
	explicit Rmatrix(std::initializer_list<torasu::tstd::Dnum> numbers, size_t height = 0);
	/**
	 * @brief  Create Matrix-Renderable
	 * @param  val: Matrix-value
	 */
	explicit Rmatrix(Dmatrix val);
	virtual ~Rmatrix();

	virtual DataResource* getData();
	virtual void setData(DataResource* data);
};

} // namespace torasu::tstd

#endif // STD_INCLUDE_TORASU_STD_RMATRIX_HPP_
