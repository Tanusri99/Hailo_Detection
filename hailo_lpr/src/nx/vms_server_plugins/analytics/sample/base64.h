// https://cplusplus.com/forum/beginner/51572/
#pragma once
#include <string>

namespace nx {
namespace vms_server_plugins {
namespace analytics {
namespace hailo_lpr {

	std::string base64_encode(unsigned char const*, unsigned int len);
	std::string base64_decode(std::string const& s);

} // namespace hailo_lpr
} // namespace analytics
} // namespace vms_server_plugins
} // namespace nx
