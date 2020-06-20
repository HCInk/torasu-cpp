#ifndef STD_INCLUDE_TORASU_STD_SPOILSD_HPP_
#define STD_INCLUDE_TORASU_STD_SPOILSD_HPP_

//
// DataResource Spoils
//
// ~ This file spoils all DataResource-types from torasu::tstd
//

#ifdef TORASU_SPOILS_UNWRAP_ALL
	#define TORASU_SPOILS_UNWRAP_STD
#endif

namespace torasu::tstd {

class Dnum;
#ifdef TORASU_SPOILS_UNWRAP_STD
	#include <torasu/std/DPNum.hpp>
#endif

class Dstring;
#ifdef TORASU_SPOILS_UNWRAP_STD
	#include <torasu/std/DPString.hpp>
#endif

class Dbimg;
class Dbimg_FORMAT;
#ifdef TORASU_SPOILS_UNWRAP_STD
#include <torasu/std/DRBImg.hpp>
#endif

class Daudio_buffer;
class Daudio_buffer_FORMAT;
#ifdef TORASU_SPOILS_UNWRAP_STD
#include <torasu/std/Daudio_buffer.hpp>
#endif

class Dfile;
#ifdef TORASU_SPOILS_UNWRAP_STD
	#include <torasu/std/Dfile.hpp>
#endif

} // namespace torasu::tstd


#endif // STD_INCLUDE_TORASU_STD_SPOILSD_HPP_
