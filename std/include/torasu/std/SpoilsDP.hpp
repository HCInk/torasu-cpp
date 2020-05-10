#ifndef STD_INCLUDE_TORASU_STD_SPOILSDP_HPP_
#define STD_INCLUDE_TORASU_STD_SPOILSDP_HPP_

// 
// DataPackable Spoils
// 
// ~ This file spoils all DataPackable-types from torasu::tstd
//

#ifdef TORASU_SPOILS_UNWRAP_ALL
#define TORASU_SPOILS_UNWRAP_STD
#endif

namespace torasu::tstd {

class DPNum;
#ifdef TORASU_SPOILS_UNWRAP_STD
#include <torasu/std/DPNum.hpp>
#endif

} // namespace torasu::tstd


#endif // STD_INCLUDE_TORASU_STD_SPOILSDP_HPP_
