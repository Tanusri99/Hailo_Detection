// https://cplusplus.com/forum/beginner/51572/
#pragma once
#include <string>

namespace nx {
namespace vms_server_plugins {
namespace analytics {
namespace hailo_object_detection {

	std::string base64_encode(unsigned char const*, unsigned int len);
	std::string base64_decode(std::string const& s);

} // namespace hailo_object_detection
} // namespace analytics
} // namespace vms_server_plugins
} // namespace nx
