// Copyright 2018-present Network Optix, Inc. Licensed under MPL 2.0: www.mozilla.org/MPL/2.0/

#pragma once

#include <nx/sdk/analytics/helpers/consuming_device_agent.h>
#include <nx/sdk/helpers/uuid_helper.h>
#include <nx/sdk/analytics/helpers/consuming_device_agent.h>
#include <nx/sdk/ptr.h>
#include <nx/sdk/analytics/helpers/object_metadata_packet.h>

#include "engine.h"

#include <thread>
#include <atomic>
#include <algorithm>
#include <vector>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace nx {
namespace vms_server_plugins {
namespace analytics {
namespace hailo_object_detection {

class DeviceAgent: public nx::sdk::analytics::ConsumingDeviceAgent
{
public:
    DeviceAgent(
        const nx::sdk::IDeviceInfo* deviceInfo,
        std::vector<std::string> enabledObjectList,        
        std::string hailoServerAddress
        );
    virtual ~DeviceAgent() override;

protected:
    virtual std::string manifestString() const override;

    virtual bool pushUncompressedVideoFrame(
        const nx::sdk::analytics::IUncompressedVideoFrame* videoFrame) override;

    virtual bool pullMetadataPackets(
        std::vector<nx::sdk::analytics::IMetadataPacket*>* metadataPackets) override;

    virtual nx::sdk::Result<const nx::sdk::ISettingsResponse*> settingsReceived() override;        
   

    virtual void doSetNeededMetadataTypes(
        nx::sdk::Result<void>* outValue,
        const nx::sdk::analytics::IMetadataTypes* neededMetadataTypes) override;

private:
    nx::sdk::Ptr<nx::sdk::analytics::IMetadataPacket> generateEventMetadataPacket();
    nx::sdk::Ptr<nx::sdk::analytics::IMetadataPacket> generateObjectMetadataPacket();


private:
    nx::sdk::Uuid m_trackId;

    /** Used for binding object and event metadata to the particular video frame. */
    int64_t m_lastVideoFrameTimestampUs = 0;
    std::string m_label;
    float m_confidence;
    float m_xMin;
    float m_yMin;
    float m_width;
    float m_height;
    std::string m_objectType;
    

private:
    void startServer();
    void createObject(json data);
    std::map<std::string, const std::string> map;
    void createHashMap();
    std::string getObjectType();
    std::shared_ptr<std::atomic<bool>> serverOnline = std::make_shared<std::atomic<bool>>(true /*or true*/);
    std::thread* t1;   
    void setEnabledObjects(std::vector<std::string> enabledObjectList);
    std::vector<std::string> m_enabledObjectList; // selected names   
    std::string postCURL(const std::string &jsonstr, std::string endpoint); // curl post    
    std::string m_hailoServerAddress;  // flask server address

    // device info
    std::string m_deviceID;
    std::string m_deviceLogin;
    std::string m_devicePassword;
    std::string m_deviceURL;
    std::string m_logicalId;

};

} // namespace hailo_object_detection
} // namespace analytics
} // namespace vms_server_plugins
} // namespace nx
