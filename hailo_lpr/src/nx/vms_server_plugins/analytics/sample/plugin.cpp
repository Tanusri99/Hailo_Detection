// Copyright 2018-present Network Optix, Inc. Licensed under MPL 2.0: www.mozilla.org/MPL/2.0/

#pragma once

#include <nx/sdk/analytics/helpers/object_metadata_packet.h>
#include <nx/sdk/analytics/helpers/consuming_device_agent.h>
#include <nx/sdk/helpers/uuid_helper.h>
#include <nx/sdk/ptr.h>

#include "plugin.h"
#include "engine.h"
#include "device_agent.h"
#include "plugin_settings.h"
#include "applogger.h"

namespace nx {
namespace vms_server_plugins {
namespace analytics {
namespace hailo_lpr {

using namespace nx::sdk;
using namespace nx::sdk::analytics;

Result<IEngine*> Plugin::doObtainEngine()
{
    AppLogger::clear();
    return new Engine();
}

/**
 * JSON with the particular structure. Note that it is possible to fill in the values that are not
 * known at compile time.
 *
 * - id: Unique identifier for a plugin with format "{vendor_id}.{plugin_id}", where
 *     {vendor_id} is the unique identifier of the plugin creator (person or company name) and
 *     {plugin_id} is the unique (for a specific vendor) identifier of the plugin.
 * - name: A human-readable short name of the plugin (displayed in the "Camera Settings" window
 *     of the Client).
 * - description: Description of the plugin in a few sentences.
 * - version: Version of the plugin.
 * - vendor: Plugin creator (person or company) name.
 */
std::string Plugin::manifestString() const
{
    return /*suppress newline*/ 1 + (const char*) R"json(
{
    "id": "nx.hailo_lpr",
    "name": "Hailo LPR",
    "description": "LPR plugin using Hailo's AI accelerators",
    "version": "1.0.1",
    "vendor": "Art of Logic",
    "engineSettingsModel": {
        "type": "Settings",
        "items": [
            {
                "type": "GroupBox",
                "caption": "Configuration settings",
                "items": [
                    
                    {
                        "type": "TextField",
						"name": ")json" + kHailoServerAddress + R"json(",
						"caption": "Hailo server address",
						"defaultValue": ")json" + kDefaultHailoServerAddress + R"json("
                    }

                ]
            }
            
			
        ]
    }
}
)json";
}

/**
 * Called by the Server to instantiate the Plugin object.
 *
 * The Server requires the function to have C linkage, which leads to no C++ name mangling in the
 * export table of the plugin dynamic library, so that makes it possible to write plugins in any
 * language and compiler.
 *
 * NX_PLUGIN_API is the macro defined by CMake scripts for exporting the function.
 */
extern "C" NX_PLUGIN_API nx::sdk::IPlugin* createNxPlugin()
{
    // The object will be freed when the Server calls releaseRef().
    return new Plugin();
}

} // namespace hailo_lpr
} // namespace analytics
} // namespace vms_server_plugins
} // namespace nx
