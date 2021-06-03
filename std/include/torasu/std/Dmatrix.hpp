#ifndef STD_INCLUDE_TORASU_STD_DMATRIX_HPP_
#define STD_INCLUDE_TORASU_STD_DMATRIX_HPP_

#include <string>
#include <vector>
#include <initializer_list>

#include <torasu/torasu.hpp>
#include <torasu/DataPackable.hpp>
#include <torasu/slot_tools.hpp>
#include <torasu/std/Dnum.hpp>

namespace torasu::tstd {

class Dmatrix : public torasu::DataPackable {
private:
	size_t height = 1;
	size_t width = 1;
	std::vector<torasu::tstd::Dnum>* nums = nullptr;
	void initBuffer(size_t size, size_t height, std::initializer_list<torasu::tstd::Dnum>* init = nullptr);

protected:
	void load() override;
	torasu::json makeJson() override;

public:
	explicit Dmatrix(std::string jsonStripped);
	explicit Dmatrix(torasu::json jsonParsed);
	/**
	 * @brief  Create matrix
	 * @param  numbers: Numbers to initialize
	 * @param  height: Height of matrix (0 to take size of numbers as height / create vertical vector)
	 */
	explicit Dmatrix(std::initializer_list<torasu::tstd::Dnum> numbers, size_t height = 0);

	virtual ~Dmatrix();

	torasu::tstd::Dnum* getNums();
	size_t getWidth();
	size_t getHeight();

	std::string getIdent() const override;
	Dmatrix* clone() const override;
};

} // namespace torasu::tstd

#endif // STD_INCLUDE_TORASU_STD_DMATRIX_HPP_
