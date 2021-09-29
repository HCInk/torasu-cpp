#ifndef STD_INCLUDE_TORASU_STD_TORASU_FULL_HPP_
#define STD_INCLUDE_TORASU_STD_TORASU_FULL_HPP_

/*

	 _____ ___  ____      _    ____  _   _
	|_   _/ _ \|  _ \    / \  / ___|| | | |
	  | || | | | |_) |  / _ \ \___ \| | | |
	  | || |_| |  _ <  / ___ \ ___) | |_| |
	  |_| \___/|_| \_\/_/   \_\____/ \___/

			TORASU FULL-HEADER

	-> This includes all public headers of TORASU

!! Should only be used in situation when many components are used at once
!! Otherwise use the individual headers to save compiletime
!! Never use this in public headers!

*/

// CORE HEADERS
#include <torasu/torasu.hpp>
#include <torasu/json.hpp>

#include <torasu/log_tools.hpp>
#include <torasu/render_tools.hpp>
#include <torasu/slot_tools.hpp>

#include <torasu/DataPackable.hpp>
#include <torasu/RenderableProperties.hpp>
#include <torasu/SimpleRenderable.hpp>

#include <torasu/Dlog_entry.hpp>

// STD NAMES
#include <torasu/std/context_names.hpp>
#include <torasu/std/pipeline_names.hpp>
#include <torasu/std/property_names.hpp>

// STD TOOLS
#include <torasu/std/simple_render.hpp>

// STD INTERFACES
#include <torasu/std/EIcore_runner.hpp>
#include <torasu/std/LIcore_logger.hpp>

// STD DATA-TYPES
#include <torasu/std/Dnum.hpp>
#include <torasu/std/Dstring.hpp>
#include <torasu/std/Dstring_map.hpp>
#include <torasu/std/Dstring_pair.hpp>
#include <torasu/std/Dfile.hpp>
#include <torasu/std/Dbimg.hpp>
#include <torasu/std/Dbimg_sequence.hpp>
#include <torasu/std/Daudio_buffer.hpp>

// STD RENDERABLES
#include <torasu/std/Rlog_message.hpp>
#include <torasu/std/Rerror.hpp>
#include <torasu/std/Rproperty.hpp>
#include <torasu/std/Rmix_pipelines.hpp>
#include <torasu/std/Rfallback.hpp>
#include <torasu/std/Rrctx_value.hpp>
#include <torasu/std/Rmod_rctx.hpp>

#include <torasu/std/Rnum.hpp>
#include <torasu/std/Radd.hpp>
#include <torasu/std/Rsubtract.hpp>
#include <torasu/std/Rmultiply.hpp>
#include <torasu/std/Rdivide.hpp>
#include <torasu/std/Rfloor_mod.hpp>
#include <torasu/std/Rsin.hpp>

#include <torasu/std/Rmatrix.hpp>

#include <torasu/std/Rstring.hpp>
#include <torasu/std/Rstring_concat.hpp>
#include <torasu/std/Rstring_map.hpp>
#include <torasu/std/Rstring_replace.hpp>
#include <torasu/std/Rnumber_string.hpp>

#include <torasu/std/Rlocal_file.hpp>
#include <torasu/std/Rnet_file.hpp>
#include <torasu/std/Rstring_file.hpp>
#include <torasu/std/Rjson_prop.hpp>

#endif // STD_INCLUDE_TORASU_STD_TORASU_FULL_HPP_
