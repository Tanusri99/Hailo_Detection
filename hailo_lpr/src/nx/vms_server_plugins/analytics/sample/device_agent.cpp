// Copyright 2018-present Network Optix, Inc. Licensed under MPL 2.0: www.mozilla.org/MPL/2.0/

#include "device_agent.h"
#include "plugin_settings.h"
#include "engine.h"
#include "applogger.h"
#include <curl/curl.h>
#include <string>
#include <iostream>
#include <unistd.h>
#include <fstream>
#include <chrono>

#include "cppkafka/consumer.h"
#include "cppkafka/configuration.h"

#include <nx/sdk/analytics/helpers/event_metadata.h>
#include <nx/sdk/analytics/helpers/event_metadata_packet.h>
#include <nx/sdk/analytics/helpers/object_metadata.h>
#include <nx/sdk/analytics/helpers/object_metadata_packet.h>
#include <nx/sdk/helpers/string.h>
#include <nx/sdk/helpers/string_map.h>
#include <nx/sdk/helpers/settings_response.h>
#include <nx/sdk/helpers/error.h>
#include <nx/sdk/helpers/uuid_helper.h>


namespace nx {
namespace vms_server_plugins {
namespace analytics {
namespace hailo_lpr {

using namespace nx::sdk;
using namespace nx::sdk::analytics;

using cppkafka::Consumer;
using cppkafka::Configuration;
using cppkafka::Message;
using cppkafka::TopicPartitionList;

/**
 * @param deviceInfo Various information about the related device, such as its id, vendor, model,
 *     etc.
 */
DeviceAgent::DeviceAgent(
    const nx::sdk::IDeviceInfo* deviceInfo,
    std::vector<std::string> enabledObjectList,   
    std::string hailoServerAddress
    ):
    // Call the DeviceAgent helper class constructor telling it to verbosely report to stderr.
    ConsumingDeviceAgent(deviceInfo, /*enableOutput*/ true)
{
    // get device info
    this->m_deviceID = deviceInfo->id();
    this->m_deviceID = this->m_deviceID.substr(1, this->m_deviceID.size() - 2); // remove {}
    this->m_deviceLogin = deviceInfo->login();
    this->m_devicePassword = deviceInfo->password();
    this->m_deviceURL = deviceInfo->url();
    this->m_logicalId = deviceInfo->logicalId();
    this->m_hailoServerAddress = hailoServerAddress;

    AppLogger::debug("Device ID: " + m_deviceID);
    AppLogger::debug("Device login: " + m_deviceLogin);
    AppLogger::debug("Device password: " + m_devicePassword);
    AppLogger::debug("Device url: " + m_deviceURL);
    AppLogger::debug("m_logicalId: " + m_logicalId); 
    AppLogger::debug("hailoServerAddress: " + hailoServerAddress); 

    serverOnline->store(true);
    t1 = new std::thread([this]() {startServer();} );

    this->setEnabledObjects(enabledObjectList); // load selected objects from Engine 
}

DeviceAgent::~DeviceAgent()
{
    serverOnline->store(false);
    if (t1 != nullptr)
    {
        t1->join();
        delete t1;
    }
    std::string jsonstr = "{\"camera_id\":\"";
    jsonstr += this->m_deviceID;
    jsonstr += "\",\"analytic_type\":\"";
    jsonstr += "detection";
    jsonstr += "\"}";
    std::string endpoint = "remove_source";
    // scotta removed curl for pipeline testing
    // std::string res = postCURL(jsonstr, endpoint);
}

// load enabled objects into a list
void DeviceAgent::setEnabledObjects(std::vector<std::string> enabledObjectList)
{
	this->m_enabledObjectList.clear();
	AppLogger::debug("Load Enabled objects");
	std::vector<std::string>::iterator it;
	for (it= enabledObjectList.begin(); it!= enabledObjectList.end(); ++it)
	{
		this->m_enabledObjectList.push_back(*it);
		AppLogger::debug(*it);
	}
		
}

// post config data to external server
std::string DeviceAgent::postCURL(const std::string &jsonstr, std::string endpoint)
{
    std::string response;
    CURL *curl;
    struct curl_slist *slist1;

    // std::string host = "http://localhost";
    // std::string port = "47760";
    // std::string url = host + ":" + port + "/" + endpoint;
    
    // dynamic flask server address from plugin setting
    std::string url = this->m_hailoServerAddress + "/" + endpoint;
    AppLogger::debug("flask url : " + url);

    slist1 = NULL;
    slist1 = curl_slist_append(slist1, "Content-Type: application/json");

    // initialse curl instance
    curl = curl_easy_init();

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str()); 
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L); // 0=ignore
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonstr.c_str());
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "curl/7.38.0");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist1);
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 50L);
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
    curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);

    // get response
    response = std::to_string(curl_easy_perform(curl));

    // curl clean up
    curl_easy_cleanup(curl);
    curl = NULL;
    curl_slist_free_all(slist1);
    slist1 = NULL;

    return response;
}

void DeviceAgent::startServer()
{
    createHashMap();
    std::shared_ptr<std::atomic<bool>> online(this->serverOnline);

    // TODO: Change broker to pull from Settings page instead of hardcoded address
    std::string brokers = "localhost:9092";
    std::string kafka_topic = "lpr_" + this->m_deviceID;
    std::string group_id = "gID_lpr_" + this->m_deviceID;

    // Construct the configuration
    Configuration config = {
        { "metadata.broker.list", brokers },
        { "group.id", group_id },
        // Disable auto commit
        { "enable.auto.commit", false }
    };

    // json string format data to be sent to flask app (external) via curl 
    std::string jsonstr = "{\"kafka_topic\":\"";
    jsonstr += kafka_topic;
    jsonstr += "\",\"camera_id\":\"";
    jsonstr += this->m_deviceID;
    jsonstr += "\",\"analytic_type\":\"";
    jsonstr += "detection";
    jsonstr += "\"}";

    AppLogger::debug("Jsonstr data: " + jsonstr);

    // inital send
    std::string endpoint = "add_source";
    //scotta removed for pipeline testing
    std::string res = "1";
    // std::string res = postCURL(jsonstr, endpoint); // send topic + cam info via curl
    // AppLogger::debug("Response from flask server: " + res);

    // check if send successful and continue to retry if not
    while(res != "0" && online->load() && online.use_count() == 2)
    {
        sleep(30);
        //scotta removed for pipeline testing 
        // res = postCURL(jsonstr, endpoint); // send topic + cam info via curl
        // AppLogger::debug("Response from flask server: " + res);
    }

    // Create the consumer
    Consumer consumer(config);
    consumer.subscribe({ kafka_topic });

    time_t rawtime;
    struct tm * timeinfo;
    char buffer[3];
    std::string secs;
    bool sent = false;

    while (online->load() && online.use_count() == 2)
    {    
        try
        {
            time(&rawtime);
            timeinfo = localtime(&rawtime);
            strftime(buffer, 3, "%S", timeinfo);
            secs.assign(buffer, 2);

            if (secs == "00" && sent == false)
            {
                endpoint = "heartbeat_camera";
                //scotta removed for pipeline testing
                std::string res = "1";
                // res = postCURL(jsonstr, endpoint); // send topic + cam info via curl
                // AppLogger::debug("Response from flask server: " + res);

                while(res != "0" && online->load() && online.use_count() == 2)
                {
                    sleep(2);
                    //scotta removed for pipeline testing
                    // res = postCURL(jsonstr, endpoint); // send topic + cam info via curl
                    // AppLogger::debug("Response from flask server: " + res);
                }
                sent = true;
            }
            else if(secs != "00")
            {
                sent = false;
            }

            Message msg = consumer.poll();
           
            if(msg)
            {
                // If we managed to get a message
                if (msg.get_error()) {
                    // Ignore EOF notifications from rdkafka
                    if (!msg.is_eof()) {
                        // Applogger::debug("[Kafka] Received error notification: " + msg.get_error());
                    }
                }
                else {
                    // Print the key (if any)
                    // if (msg.get_key()) {
                    //     cout << msg.get_key() << " -> ";
                    // }
                    // Print the payload
                    std::string payload = msg.get_payload();
                    json data = json::parse(payload);
                    createObject(data);

                    // Now commit the message
                    consumer.commit(msg);
                }
            }
        }
        catch (const std::exception& e)
        {
            //respose code from curl post
            std::string err1 = e.what();
            AppLogger::debug("Error in consuming kafka data: " + err1);
        }
    }
    return;
}

void DeviceAgent::createObject(json data)
{
//read from json to variables
    m_label = data["detection"]["label"];
    m_confidence = data["detection"]["confidence"];
    m_lastVideoFrameTimestampUs = data["meta"]["timestamp"];
    // m_trackId = nx::sdk::UuidHelper::randomUuid();
    m_trackId = nx::sdk::UuidHelper::fromStdString(data["detection"]["obj_uuid"]);
    m_xMin = data["detection"]["bbox"]["xmin"];
    m_yMin = data["detection"]["bbox"]["ymin"];
    m_width = data["detection"]["bbox"]["width"];
    m_height = data["detection"]["bbox"]["height"];

    auto eventMetadataPacket = generateEventMetadataPacket();
    auto objectMetadataPacket = generateObjectMetadataPacket();
    if (getObjectType() != "" && eventMetadataPacket)
    {
        // Send generated metadata packet to the Server.
        pushMetadataPacket(eventMetadataPacket.releasePtr());
    }
    if (getObjectType() != "" && objectMetadataPacket)
    {
        pushMetadataPacket(objectMetadataPacket.releasePtr());
    }
}

void DeviceAgent::createHashMap()
{
    map.insert(std::make_pair("car", kCarObjectType));
    return;
}

// print vector elements
void printVector(const std::vector<std::string>& vec) {
  for (const auto& str : vec) {
	  AppLogger::debug(str + ", ");
  }
}


std::string DeviceAgent::getObjectType()
{
    
    AppLogger::debug("m_deviceId" + m_deviceID);
    AppLogger::debug("m_enabledObjectList.size: " + std::to_string(m_enabledObjectList.size()));
    printVector(m_enabledObjectList);
    AppLogger::debug("m_label:  " + m_label);
    if(map.find(m_label) != map.end())
    {
        // if detected object is in the object selection list
        if (std::find(m_enabledObjectList.begin(),  m_enabledObjectList.end(), m_label) != m_enabledObjectList.end())
        {
            return map[m_label];
        }
        else
        {
            return "";
        }    
    }
        
    else
    {
        return "";
    }
}


Result<const ISettingsResponse*> DeviceAgent::settingsReceived() {
    AppLogger::debug("Device Agent received settings");
	
	const auto settingsResponse = new sdk::SettingsResponse(); 
   
    // user objects selection
    this->m_enabledObjectList.clear();

    std::string car = settingValue(kEnableCar);   
    if(car == "true")
    {
        AppLogger::debug("Car Added");
		this->m_enabledObjectList.push_back("car");
    } 

    // selected objects route
    std::string jsonstr_objects = "{\"object_type\":\"";
    jsonstr_objects += "detection";
    jsonstr_objects += "\",\"device_id\":\"";
    jsonstr_objects += this->m_deviceID;
    jsonstr_objects += "\"}";
    std::string endpoint_objects = "objects";
    //scotta removed for pipeline testing
    // std::string res_objects = postCURL(jsonstr_objects, endpoint_objects); // send kafka topic via curl

	return settingsResponse;
}


/**
 *  @return JSON with the particular structure. Note that it is possible to fill in the values
 * that are not known at compile time, but should not depend on the DeviceAgent settings.
 */
std::string DeviceAgent::manifestString() const
{
    // Tell the Server that the plugin can generate the events and objects of certain types.
    // Id values are strings and should be unique. Format of ids:
    // `{vendor_id}.{plugin_id}.{event_type_id/object_type_id}`.
    //
    // See the plugin manifest for the explanation of vendor_id and plugin_id.
    return /*suppress newline*/ 1 + (const char*) R"json(
{
    "eventTypes": [
        {
            "id": ")json" + kNewObjectEventType + R"json(",
            "name": "New Object detected"
        }
    ],
    "objectTypes": [
        {
            "id": ")json" + kCarObjectType + R"json(",
            "name": "Car"
        }
    ]
}
)json";
}

/**setTrackId
 * Called when the Server sends a new uncompressed frame from a camera.
 */
bool DeviceAgent::pushUncompressedVideoFrame(const IUncompressedVideoFrame* videoFrame)
{
    return true; //< There were no errors while processing the video frame.
}

/**
 * Serves the similar purpose as pushMetadataPacket(). The differences are:
 * - pushMetadataPacket() is called by the plugin, while pullMetadataPackets() is called by Server.
 * - pushMetadataPacket() expects one metadata packet, while pullMetadataPacket expects the
 *     std::vector of them.
 *
 * There are no strict rules for deciding which method is "better". A rule of thumb is to use
 * pushMetadataPacket() when you generate one metadata packet and do not want to store it in the
 * class field, and use pullMetadataPackets otherwise.
 */
bool DeviceAgent::pullMetadataPackets(std::vector<IMetadataPacket*>* metadataPackets)
{
    return true; //< There were no errors while filling metadataPackets.
}

void DeviceAgent::doSetNeededMetadataTypes(
    nx::sdk::Result<void>* /*outValue*/,
    const nx::sdk::analytics::IMetadataTypes* /*neededMetadataTypes*/)
{
}

//-------------------------------------------------------------------------------------------------
// private

Ptr<IMetadataPacket> DeviceAgent::generateEventMetadataPacket()
{
    // EventMetadataPacket contains arbitrary number of EventMetadata.
    const auto eventMetadataPacket = makePtr<EventMetadataPacket>();
    // Bind event metadata packet to the last video frame using a timestamp.
    eventMetadataPacket->setTimestampUs(m_lastVideoFrameTimestampUs);
    // Zero duration means that the event is not sustained, but momental.
    eventMetadataPacket->setDurationUs(0);

    // EventMetadata contains an information about event.
    const auto eventMetadata = makePtr<EventMetadata>();
    // Set all required fields.
    eventMetadata->setTypeId(kNewObjectEventType);
    eventMetadata->setIsActive(true);
    eventMetadata->setCaption("New Object Detected");
    eventMetadata->setDescription("New Object Detected");

    eventMetadataPacket->addItem(eventMetadata.get());


    return eventMetadataPacket;
}

// generate object type detected from hailo
//TODO:

Ptr<IMetadataPacket> DeviceAgent::generateObjectMetadataPacket()
{
    // ObjectMetadataPacket contains arbitrary number of ObjectMetadata.
    const auto objectMetadataPacket = makePtr<ObjectMetadataPacket>();

    // Bind the object metadata to the last video frame using a timestamp.
    objectMetadataPacket->setTimestampUs(m_lastVideoFrameTimestampUs);
    objectMetadataPacket->setDurationUs(0);

    // ObjectMetadata contains information about an object on the frame.
    const auto objectMetadata = makePtr<ObjectMetadata>();
    // Set all required fields.
    m_objectType = getObjectType(); // get detected object type
    AppLogger::debug("m_objectType: " + m_objectType);
    // handle unselected objects
    if (m_objectType != "" )
    {
        objectMetadata->setTypeId(m_objectType);
    }
        
    // objectMetadata->setTypeId(kPersonObjectType);

    objectMetadata->setTrackId(m_trackId);

    float width = m_width;
    float height = m_height;
    float x = m_xMin;
    float y = m_yMin;
    objectMetadata->setBoundingBox(Rect(x, y, width, height));

    objectMetadataPacket->addItem(objectMetadata.get());

    return objectMetadataPacket;
}

} // namespace hailo_lpr
} // namespace analytics
} // namespace vms_server_plugins
} // namespace nx
