#ifndef STD_INCLUDE_TORASU_STD_DBIMG_SEQUENCE_HPP_
#define STD_INCLUDE_TORASU_STD_DBIMG_SEQUENCE_HPP_

#include <utility>
#include <map>
#include <functional>
#include <string>

#include <torasu/torasu.hpp>

#include <torasu/std/spoilsD.hpp>

namespace torasu::tstd {

class Dbimg_sequence : public torasu::DataResource {
private:
	double time_padding;
	std::multimap<double, torasu::tstd::Dbimg*, std::less<double>> frames;

public:
	Dbimg_sequence();
	~Dbimg_sequence();
	Dbimg_sequence(const Dbimg_sequence&);

	Dbimg* addFrame(double pts, Dbimg_FORMAT format);
	std::multimap<double, torasu::tstd::Dbimg*, std::less<double>>& getFrames();

	std::string getIdent() override;
	DataDump* dumpResource() override;
	Dbimg_sequence* clone() override;
};

} // namespace torasu::tstd


#endif // STD_INCLUDE_TORASU_STD_DBIMG_SEQUENCE_HPP_
