// Copyright 2018-present Network Optix, Inc. Licensed under MPL 2.0: www.mozilla.org/MPL/2.0/

#include "engine.h"
#include "device_agent.h"
#include "plugin_settings.h"
#include "applogger.h"


#include <nx/sdk/i_device_info.h>
#include <nx/sdk/helpers/uuid_helper.h>
#include <nx/sdk/helpers/string.h>
#include <nx/sdk/helpers/error.h>
#include <nx/sdk/helpers/settings_response.h>


namespace nx {
namespace vms_server_plugins {
namespace analytics {
namespace hailo_lpr {

using namespace nx::sdk;
using namespace nx::sdk::analytics;

Engine::Engine():
    // Call the DeviceAgent helper class constructor telling it to verbosely report to stderr.
    nx::sdk::analytics::Engine(/*enableOutput*/ true)
{
    AppLogger::clear();
}

Engine::~Engine()
{
}

/**
 * Called when the Server opens a video-connection to the camera if the plugin is enabled for this
 * camera.
 *
 * @param outResult The pointer to the structure which needs to be filled with the resulting value
 *     or the error information.
 * @param deviceInfo Contains various information about the related device such as its id, vendor,
 *     model, etc.
 */
void Engine::doObtainDeviceAgent(Result<IDeviceAgent*>* outResult, const IDeviceInfo* deviceInfo)
{
    DeviceAgent *pDevice = new DeviceAgent(
        deviceInfo,
        this->m_enabledObjectList,       
        this->m_hailoServerAddress
        );
    *outResult = pDevice;
}

Result<const ISettingsResponse*> Engine::settingsReceived()
{
    AppLogger::debug("Engine Settings received", true);
	
    auto settingsResponse = new SettingsResponse();

	this->m_hailoServerAddress = settingValue(kHailoServerAddress);
    AppLogger::debug("this->m_hailoServerAddress in Engine setting: " + this->m_hailoServerAddress);

    return settingsResponse;
}  //engine settings

/**
 * @return JSON with the particular structure. Note that it is possible to fill in the values
 *     that are not known at compile time, but should not depend on the Engine settings.
 */
std::string Engine::manifestString() const
{
    // Ask the Server to supply uncompressed video frames in YUV420 format (see
    // https://en.wikipedia.org/wiki/YUV).
    //
    // Note that this format is used internally by the Server, therefore requires minimum
    // resources for decoding, thus it is the recommended format.
    return /*suppress newline*/ 1 + (const char*) R"json(
{
    
    "deviceAgentSettingsModel": {
        "type": "Settings",
        "items":
        [
            {
				"type": "GroupBox",
						"caption": "Confidence Threshold Settings",
						"items": [
								
								{
									"type": "TextField",
									"name": ")json" + kConfidenceThreshold + R"json(",
									"caption": "Confidence Threshold (0.1 - 0.9)",
									"defaultValue": ")json" + kDefaultConfidenceThreshold + R"json("
								}
						]
			},
            {
				"type": "GroupBox",
                "caption": "Object Selection Settings:",
                "items":
                [
						{
							"type": "CheckBox",
							"name": ")json" + kEnableCar + R"json(",
							"caption": "Car",
							"description": "If true, selected",
							"defaultValue": true
						}
				]
			}
        ]
    }
}
)json";
}

} // namespace hailo_lpr
} // namespace analytics
} // namespace vms_server_plugins
} // namespace nx
