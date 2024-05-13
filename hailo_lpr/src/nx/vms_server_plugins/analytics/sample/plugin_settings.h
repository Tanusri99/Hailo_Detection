#pragma once

#include <iostream>
#include <vector>
#include <set>
#include <algorithm>

namespace nx {
namespace vms_server_plugins {
namespace analytics {
namespace hailo_lpr {	

	//Object types
    const std::string kCarObjectType = "nx.hailo_lpr.car";

    //Object event type
    const std::string kNewObjectEventType = "nx.hailo_lpr.newObject";

    // Settings
    const std::string kConfidenceThreshold{ "nx.hailo_lpr.confidenceThreshold" };
    const std::string kDefaultConfidenceThreshold{ "0.8" };
	const std::string kHailoServerAddress{ "nx.hailo_lpr.hailoServerAddress" };
	const std::string kDefaultHailoServerAddress{ "http://127.0.0.1:47760" };

    // user selection	
	const std::string kEnableCar{"enableCar"};	

} // namespace hailo_lpr
} // namespace analytics
} // namespace vms_server_plugins
} // namespace nx
