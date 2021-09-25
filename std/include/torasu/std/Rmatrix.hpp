#ifndef STD_INCLUDE_TORASU_STD_RMATRIX_HPP_
#define STD_INCLUDE_TORASU_STD_RMATRIX_HPP_

#include <string>
#include <map>
#include <initializer_list>

#include <torasu/torasu.hpp>
#include <torasu/slot_tools.hpp>
#include <torasu/SimpleRenderable.hpp>

#include <torasu/std/Dmatrix.hpp>
#include <torasu/std/Rnum.hpp>

namespace torasu::tstd {

class Rmatrix : public tools::SimpleRenderable {
private:
	Dnum height;
	std::map<size_t, torasu::tools::ManagedSlot<torasu::tstd::NumSlot>> vals;

public:
	/**
	 * @brief  Create Matrix-Renderable
	 * @param  numbers: Numbers to initialize
	 * @param  height: Height of matrix (0 to take size of numbers as height / create vertical vector)
	 */
	explicit Rmatrix(std::initializer_list<torasu::tstd::NumSlot> numbers, size_t height = 0);
	virtual ~Rmatrix();
	Identifier getType() override;

	RenderResult* render(RenderInstruction* ri) override;

	DataResource* getData() override;
	void setData(DataResource* data) override;
	torasu::ElementMap getElements() override;
	void setElement(std::string key, Element* elem) override;
};

} // namespace torasu::tstd

#endif // STD_INCLUDE_TORASU_STD_RMATRIX_HPP_
