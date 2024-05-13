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
#include <map>

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
#include <nx/kit/json.h>

namespace nx {
namespace vms_server_plugins {
namespace analytics {
namespace hailo_object_count {

using namespace nx::sdk;
using namespace nx::sdk::analytics;
using NXPoint = nx::sdk::analytics::Point;

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
    jsonstr += "count";
    jsonstr += "\"}";
    std::string endpoint = "remove_source";
    std::string res = postCURL(jsonstr, endpoint);
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
    std::string kafka_topic = "count_" + this->m_deviceID;
    std::string group_id = "gID_count_" + this->m_deviceID;

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
    jsonstr += "count";
    jsonstr += "\"}";
  
    AppLogger::debug("Jsonstr data: " + jsonstr);

    // inital send
    std::string endpoint = "add_source";
    std::string res = postCURL(jsonstr, endpoint); // send topic + cam info via curl
    AppLogger::debug("Response from flask server: " + res);

    // check if send successful and continue to retry if not
    while(res != "0" && online->load() && online.use_count() == 2)
    {
        sleep(30);
        res = postCURL(jsonstr, endpoint); // send topic + cam info via curl
        AppLogger::debug("Response from flask server: " + res);
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
                AppLogger::debug("Jsonstr data from heartbeat_camera: " + jsonstr);
                res = postCURL(jsonstr, endpoint); // send topic + cam info via curl
                AppLogger::debug("Response from flask server: heart_beat route" + res);

                while(res != "0" && online->load() && online.use_count() == 2)
                {
                    sleep(2);
                    res = postCURL(jsonstr, endpoint); // send topic + cam info via curl
                    AppLogger::debug("Response from heart beat camera: " + res);
                    AppLogger::debug("Jsonstr data from heartbeat_camera: " + jsonstr);
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
            AppLogger::debug("Error in consuming kafka data");
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
    m_trackId = nx::sdk::UuidHelper::randomUuid();
    m_xMin = data["detection"]["bbox"]["xmin"];
    m_yMin = data["detection"]["bbox"]["ymin"];
    m_width = data["detection"]["bbox"]["width"];
    m_height = data["detection"]["bbox"]["height"];

    // people count
    this->m_people_count_type = data["people_count_meta"]["people_count_type"];    

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
    map.insert(std::make_pair("person", kPersonObjectType));
    map.insert(std::make_pair("bicycle", kBicycleObjectType));
    map.insert(std::make_pair("car", kCarObjectType));
    map.insert(std::make_pair("motorcycle", kMotorbikeObjectType));
    map.insert(std::make_pair("airplane", kAeroplaneObjectType));
    map.insert(std::make_pair("bus", kBusObjectType));
    map.insert(std::make_pair("train", kTrainObjectType));
    map.insert(std::make_pair("truck", kTruckObjectType));
    map.insert(std::make_pair("boat", kBoatObjectType));
    map.insert(std::make_pair("traffic light", kTrafficLightObjectType));
    map.insert(std::make_pair("fire hydrant", kFireHydrantObjectType));
    map.insert(std::make_pair("stop sign", kStopSignObjectType));
    map.insert(std::make_pair("parking meter", kParkingMeterObjectType));
    map.insert(std::make_pair("bench", kBenchObjectType));
    map.insert(std::make_pair("bird", kBirdObjectType));
    map.insert(std::make_pair("cat", kCatObjectType));
    map.insert(std::make_pair("dog", kDogObjectType));
    map.insert(std::make_pair("horse", kHorseObjectType));
    map.insert(std::make_pair("sheep", kSheepObjectType));
    map.insert(std::make_pair("cow", kCowObjectType));
    map.insert(std::make_pair("elephant", kElephantObjectType));
    map.insert(std::make_pair("bear", kBearObjectType));
    map.insert(std::make_pair("zebra", kZebraObjectType));
    map.insert(std::make_pair("giraffe", kGiraffeObjectType));
    map.insert(std::make_pair("backpack", kBackpackObjectType));
    map.insert(std::make_pair("umbrella", kUmbrellaObjectType));
    map.insert(std::make_pair("handbag", kHandbagObjectType));
    map.insert(std::make_pair("tie", kTieObjectType));
    map.insert(std::make_pair("suitcase", kSuitcaseObjectType));
    map.insert(std::make_pair("frisbee", kFrisbeeObjectType));
    map.insert(std::make_pair("skis", kSkisObjectType));
    map.insert(std::make_pair("snowboard", kSnowboardObjectType));
    map.insert(std::make_pair("sports ball", kSportsBallObjectType));
    map.insert(std::make_pair("kite", kKiteObjectType));
    map.insert(std::make_pair("baseball bat", kBaseballBatObjectType));
    map.insert(std::make_pair("baseball glove", kBaseballGloveObjectType));
    map.insert(std::make_pair("skateboard", kSkateboardObjectType));
    map.insert(std::make_pair("surfboard", kSurfboardObjectType));
    map.insert(std::make_pair("tennis racket", kTennisRacketObjectType));
    map.insert(std::make_pair("bottle", kBottleObjectType));
    map.insert(std::make_pair("wine glass", kWineGlassObjectType ));
    map.insert(std::make_pair("cup", kCupObjectType));
    map.insert(std::make_pair("fork", kForkObjectType));
    map.insert(std::make_pair("knife", kKnifeObjectType));
    map.insert(std::make_pair("spoon", kSpoonObjectType));
    map.insert(std::make_pair("bowl", kBowlObjectType));
    map.insert(std::make_pair("banana", kBananaObjectType));
    map.insert(std::make_pair("apple", kAppleObjectType));
    map.insert(std::make_pair("sandwich", kSandwichObjectType));
    map.insert(std::make_pair("orange", kOrangeObjectType));
    map.insert(std::make_pair("broccoli", kBroccoliObjectType));
    map.insert(std::make_pair("carrot", kCarrotObjectType));
    map.insert(std::make_pair("hot dog", kHotdogObjectType));
    map.insert(std::make_pair("pizza", kPizzaObjectType));
    map.insert(std::make_pair("donut", kDonutObjectType));
    map.insert(std::make_pair("cake", kCakeObjectType));
    map.insert(std::make_pair("chair", kChairObjectType));
    map.insert(std::make_pair("sofa", kSofaObjectType));
    map.insert(std::make_pair("potted plant", kPottedplantObjectType));
    map.insert(std::make_pair("bed", kBedObjectType));
    map.insert(std::make_pair("dining table", kDiningtableObjectType));
    map.insert(std::make_pair("toilet", kToiletObjectType));
    map.insert(std::make_pair("tv", kTvmonitorObjectType));
    map.insert(std::make_pair("laptop", kLaptopObjectType));
    map.insert(std::make_pair("mouse", kMouseObjectType));
    map.insert(std::make_pair("remote", kRemoteObjectType));
    map.insert(std::make_pair("keyboard", kKeyboardObjectType));
    map.insert(std::make_pair("cell phone", kCellPhoneObjectType));
    map.insert(std::make_pair("microwave", kMicrowaveObjectType));
    map.insert(std::make_pair("oven", kOvenObjectType));
    map.insert(std::make_pair("toaster", kToasterObjectType));
    map.insert(std::make_pair("sink", kSinkObjectType));
    map.insert(std::make_pair("refrigerator", kRefrigeratorObjectType));
    map.insert(std::make_pair("book", kBookObjectType));
    map.insert(std::make_pair("clock", kClockObjectType));
    map.insert(std::make_pair("vase", kVaseObjectType));
    map.insert(std::make_pair("scissors", kScissorsObjectType));
    map.insert(std::make_pair("teddy bear", kTeddyBearObjectType));
    map.insert(std::make_pair("hair drier", kHairDrierObjectType));
    map.insert(std::make_pair("toothbrush", kToothbrushObjectType));
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


Result<const ISettingsResponse*> DeviceAgent::settingsReceived()
{
    AppLogger::debug("Device Agent received settings");
	
	const auto settingsResponse = new sdk::SettingsResponse(); 

          
   
    // user objects selection
    this->m_enabledObjectList.clear();
    
    std::string person = settingValue(kEnablePerson);   
	
    if(person == "true")
    {
        AppLogger::debug("Person Added");
		this->m_enabledObjectList.push_back("person");
    } 

    std::string bicycle = settingValue(kEnableBicycle);   
   
    if(bicycle == "true")
    {
        AppLogger::debug("Bicycle Added");
		this->m_enabledObjectList.push_back("bicycle");
    } 

    std::string car = settingValue(kEnableCar);   
   
    if(car == "true")
    {
        AppLogger::debug("Car Added");
		this->m_enabledObjectList.push_back("car");
    } 

    std::string motorcycle = settingValue(kEnableMotorbike);   
   
    if(motorcycle == "true")
    {
        AppLogger::debug("Motorcycle Added");
		this->m_enabledObjectList.push_back("motorcycle");
    }

    std::string airplane = settingValue(kEnableAeroplane);   
   
    if(airplane == "true")
    {
        AppLogger::debug("airplane Added");
		this->m_enabledObjectList.push_back("airplane");
    }

    std::string bus = settingValue(kEnableBus);   
   
    if(bus == "true")
    {
        AppLogger::debug("bus Added");
		this->m_enabledObjectList.push_back("bus");
    }

    std::string train = settingValue(kEnableTrain);   
   
    if(train == "true")
    {
        AppLogger::debug("train Added");
		this->m_enabledObjectList.push_back("train");
    }

    std::string truck = settingValue(kEnableTruck);   
   
    if(truck == "true")
    {
        AppLogger::debug("truck Added");
		this->m_enabledObjectList.push_back("truck");
    }

    std::string boat = settingValue(kEnableBoat);   
   
    if(boat == "true")
    {
        AppLogger::debug("boat Added");
		this->m_enabledObjectList.push_back("boat");
    }

    std::string traffic_light = settingValue(kEnableTrafficLight);   
   
    if(traffic_light == "true")
    {
        AppLogger::debug("traffic light Added");
		this->m_enabledObjectList.push_back("traffic light");
    }

    std::string fire_hydrant = settingValue(kEnableFireHydrant);   
   
    if(fire_hydrant == "true")
    {
        AppLogger::debug("fire_hydrant Added");
		this->m_enabledObjectList.push_back("fire hydrant");
    }

    std::string stop_sign = settingValue(kEnableStopSign);   
   
    if(stop_sign == "true")
    {
        AppLogger::debug("stop_sign Added");
		this->m_enabledObjectList.push_back("stop sign");
    }

    std::string parking_meter = settingValue(kEnableParkingMeter);   
   
    if(parking_meter == "true")
    {
        AppLogger::debug("parking_meter Added");
		this->m_enabledObjectList.push_back("parking meter");
    }

    std::string bench = settingValue(kEnableBench);   
   
    if(bench == "true")
    {
        AppLogger::debug("bench Added");
		this->m_enabledObjectList.push_back("bench");
    }

    std::string bird = settingValue(kEnableBird);   
   
    if(bird == "true")
    {
        AppLogger::debug("bird Added");
		this->m_enabledObjectList.push_back("bird");
    }

    std::string cat = settingValue(kEnableCat);   
   
    if(cat == "true")
    {
        AppLogger::debug("cat Added");
		this->m_enabledObjectList.push_back("cat");
    }

    std::string dog = settingValue(kEnableDog);   
   
    if(dog == "true")
    {
        AppLogger::debug("dog Added");
		this->m_enabledObjectList.push_back("dog");
    }

    std::string horse = settingValue(kEnableHorse);   
   
    if(horse == "true")
    {
        AppLogger::debug("horse Added");
		this->m_enabledObjectList.push_back("horse");
    }

    std::string sheep = settingValue(kEnableSheep);   
   
    if(sheep == "true")
    {
        AppLogger::debug("sheep Added");
		this->m_enabledObjectList.push_back("sheep");
    }

    std::string cow = settingValue(kEnableCow);   
   
    if(cow == "true")
    {
        AppLogger::debug("cow Added");
		this->m_enabledObjectList.push_back("cow");
    }

    std::string elephant = settingValue(kEnableElephant);   
   
    if(elephant == "true")
    {
        AppLogger::debug("elephant Added");
		this->m_enabledObjectList.push_back("elephant");
    }

    std::string bear = settingValue(kEnableBear);   
   
    if(bear == "true")
    {
        AppLogger::debug("bear Added");
		this->m_enabledObjectList.push_back("bear");
    }

    std::string zebra = settingValue(kEnableBZebra);   
   
    if(zebra == "true")
    {
        AppLogger::debug("zebra Added");
		this->m_enabledObjectList.push_back("zebra");
    }

    std::string giraffe = settingValue(kEnableBGiraffe);   
   
    if(giraffe == "true")
    {
        AppLogger::debug("giraffe Added");
		this->m_enabledObjectList.push_back("giraffe");
    }

    std::string backpack = settingValue(kEnableBackpack);   
   
    if(backpack == "true")
    {
        AppLogger::debug("backpack Added");
		this->m_enabledObjectList.push_back("backpack");
    }

    std::string umbrella = settingValue(kEnableUmbrella);   
   
    if(umbrella == "true")
    {
        AppLogger::debug("umbrella Added");
		this->m_enabledObjectList.push_back("umbrella");
    }

    std::string handbag = settingValue(kEnableHandbag);   
   
    if(handbag == "true")
    {
        AppLogger::debug("handbag Added");
		this->m_enabledObjectList.push_back("handbag");
    }

    std::string tie = settingValue(kEnableTie);   
   
    if(tie == "true")
    {
        AppLogger::debug("tie Added");
		this->m_enabledObjectList.push_back("tie");
    }

    std::string suitcase = settingValue(kEnableSuitcase);   
   
    if(suitcase == "true")
    {
        AppLogger::debug("suitcase Added");
		this->m_enabledObjectList.push_back("suitcase");
    }

    std::string frisbee = settingValue(kEnableFrisbee);   
   
    if(frisbee == "true")
    {
        AppLogger::debug("frisbee Added");
		this->m_enabledObjectList.push_back("frisbee");
    }

    std::string skis = settingValue(kEnableSkis);   
   
    if(skis == "true")
    {
        AppLogger::debug("skis Added");
		this->m_enabledObjectList.push_back("skis");
    }

    std::string snowboard = settingValue(kEnableSnowboard);   
   
    if(snowboard == "true")
    {
        AppLogger::debug("snowboard Added");
		this->m_enabledObjectList.push_back("snowboard");
    }

    std::string kite = settingValue(kEnableKite);   
   
    if(kite == "true")
    {
        AppLogger::debug("kite Added");
		this->m_enabledObjectList.push_back("kite");
    }

    std::string baseball_bat = settingValue(kEnableBaseballBat);   
   
    if(baseball_bat == "true")
    {
        AppLogger::debug("baseball bat Added");
		this->m_enabledObjectList.push_back("baseball bat");
    }

    std::string baseball_glove = settingValue(kEnableBaseballGlove);   
   
    if(baseball_glove == "true")
    {
        AppLogger::debug("baseball glove Added");
		this->m_enabledObjectList.push_back("baseball glove");
    }

    std::string skateboard = settingValue(kEnableSkateboard);   
   
    if(skateboard == "true")
    {
        AppLogger::debug("skateboard Added");
		this->m_enabledObjectList.push_back("skateboard");
    }

    std::string surfboard = settingValue(kEnableSurfboard);   
   
    if(surfboard == "true")
    {
        AppLogger::debug("surfboard Added");
		this->m_enabledObjectList.push_back("surfboard");
    }

    std::string tennis_racket = settingValue(kEnableTennisRacket);   
   
    if(tennis_racket == "true")
    {
        AppLogger::debug("tennis racket Added");
		this->m_enabledObjectList.push_back("tennis racket");
    }

    std::string bottle = settingValue(kEnableBottle);   
   
    if(bottle == "true")
    {
        AppLogger::debug("bottle Added");
		this->m_enabledObjectList.push_back("bottle");
    }

    std::string wine_glass = settingValue(kEnableWineGlass);   
   
    if(wine_glass == "true")
    {
        AppLogger::debug("wine_glass Added");
		this->m_enabledObjectList.push_back("wine glass");
    }

    std::string cup = settingValue(kEnableCup);   
   
    if(cup == "true")
    {
        AppLogger::debug("cup Added");
		this->m_enabledObjectList.push_back("cup");
    }

    std::string fork = settingValue(kEnableFork);   
   
    if(fork == "true")
    {
        AppLogger::debug("fork Added");
		this->m_enabledObjectList.push_back("fork");
    }

    std::string knife = settingValue(kEnableKnife);   
   
    if(knife == "true")
    {
        AppLogger::debug("knife Added");
		this->m_enabledObjectList.push_back("knife");
    }

    std::string bowl = settingValue(kEnableBowl);   
   
    if(bowl == "true")
    {
        AppLogger::debug("bowl Added");
		this->m_enabledObjectList.push_back("bowl");
    }

    std::string banana = settingValue(kEnableBanana);   
   
    if(banana == "true")
    {
        AppLogger::debug("banana Added");
		this->m_enabledObjectList.push_back("banana");
    }

    std::string apple = settingValue(kEnableApple);   
   
    if(apple == "true")
    {
        AppLogger::debug("apple Added");
		this->m_enabledObjectList.push_back("apple");
    }

    std::string sandwich = settingValue(kEnableSandwich);   
   
    if(sandwich == "true")
    {
        AppLogger::debug("sandwich Added");
		this->m_enabledObjectList.push_back("sandwich");
    }

    std::string orange = settingValue(kEnableOrange);   
   
    if(orange == "true")
    {
        AppLogger::debug("orange Added");
		this->m_enabledObjectList.push_back("orange");
    }

    std::string broccoli = settingValue(kEnableBroccoli);   
   
    if(broccoli == "true")
    {
        AppLogger::debug("broccoli Added");
		this->m_enabledObjectList.push_back("broccoli");
    }

    std::string carrot = settingValue(kEnableCarrot);   
   
    if(carrot == "true")
    {
        AppLogger::debug("carrot Added");
		this->m_enabledObjectList.push_back("carrot");
    }

    std::string hot_dog = settingValue(kEnableHotdog);   
   
    if(hot_dog == "true")
    {
        AppLogger::debug("hot_dog Added");
		this->m_enabledObjectList.push_back("hot dog");
    }

    std::string pizza = settingValue(kEnablePizza);   
   
    if(pizza == "true")
    {
        AppLogger::debug("pizza Added");
		this->m_enabledObjectList.push_back("pizza");
    }

    std::string donut = settingValue(kEnableDonut);   
   
    if(donut == "true")
    {
        AppLogger::debug("donut Added");
		this->m_enabledObjectList.push_back("donut");
    }

    std::string cake = settingValue(kEnableCake);   
   
    if(cake == "true")
    {
        AppLogger::debug("cake Added");
		this->m_enabledObjectList.push_back("cake");
    }

    std::string chair = settingValue(kEnableChair);   
   
    if(chair == "true")
    {
        AppLogger::debug("chair Added");
		this->m_enabledObjectList.push_back("chair");
    }

    std::string sofa = settingValue(kEnableSofa);   
   
    if(sofa == "true")
    {
        AppLogger::debug("sofa Added");
		this->m_enabledObjectList.push_back("sofa");
    }

    std::string potted_plant = settingValue(kEnablePottedplant);   
   
    if(potted_plant == "true")
    {
        AppLogger::debug("potted plant Added");
		this->m_enabledObjectList.push_back("potted plant");
    }

    std::string bed = settingValue(kEnableBed);   
   
    if(bed == "true")
    {
        AppLogger::debug("bed Added");
		this->m_enabledObjectList.push_back("bed");
    }

    std::string dining_table = settingValue(kEnableDiningtable);   
   
    if(dining_table == "true")
    {
        AppLogger::debug("dining_table Added");
		this->m_enabledObjectList.push_back("dining table");
    }

    std::string toilet = settingValue(kEnableToilet);   
   
    if(toilet == "true")
    {
        AppLogger::debug("toilet Added");
		this->m_enabledObjectList.push_back("toilet");
    }

    std::string tv = settingValue(kEnableTvmonitor);
   
    if(tv == "true")
    {
        AppLogger::debug("tv Added");
		this->m_enabledObjectList.push_back("tv");
    }

    std::string laptop = settingValue(kEnableLaptop);
   
    if(laptop == "true")
    {
        AppLogger::debug("laptop Added");
		this->m_enabledObjectList.push_back("laptop");
    }

    std::string mouse = settingValue(kEnableMouse);
   
    if(mouse == "true")
    {
        AppLogger::debug("mouse Added");
		this->m_enabledObjectList.push_back("mouse");
    }

    std::string remote = settingValue(kEnableRemote);
   
    if(remote == "true")
    {
        AppLogger::debug("remote Added");
		this->m_enabledObjectList.push_back("remote");
    }

    std::string keyboard = settingValue(kEnableKeyboard);
   
    if(keyboard == "true")
    {
        AppLogger::debug("keyboard Added");
		this->m_enabledObjectList.push_back("keyboard");
    }

    std::string cell_phone = settingValue(kEnableCellPhone);
   
    if(cell_phone == "true")
    {
        AppLogger::debug("cell phone Added");
		this->m_enabledObjectList.push_back("cell phone");
    }

    std::string microwave = settingValue(kEnableMicrowave);
   
    if(microwave == "true")
    {
        AppLogger::debug("microwave Added");
		this->m_enabledObjectList.push_back("microwave");
    }

    std::string toaster = settingValue(kEnableToaster);
   
    if(toaster == "true")
    {
        AppLogger::debug("toaster Added");
		this->m_enabledObjectList.push_back("toaster");
    }

    std::string sink = settingValue(kEnableSink);
   
    if(sink == "true")
    {
        AppLogger::debug("sink Added");
		this->m_enabledObjectList.push_back("sink");
    }

    std::string refrigerator = settingValue(kEnableRefrigerator);
   
    if(refrigerator == "true")
    {
        AppLogger::debug("refrigerator Added");
		this->m_enabledObjectList.push_back("refrigerator");
    }

    std::string book = settingValue(kEnableBook);
   
    if(book == "true")
    {
        AppLogger::debug("book Added");
		this->m_enabledObjectList.push_back("book");
    }

    std::string clock = settingValue(kEnableClock);
   
    if(clock == "true")
    {
        AppLogger::debug("clock Added");
		this->m_enabledObjectList.push_back("clock");
    }

    std::string vase = settingValue(kEnableVase);
   
    if(vase == "true")
    {
        AppLogger::debug("vase Added");
		this->m_enabledObjectList.push_back("vase");
    }

    std::string scissors = settingValue(kEnableScissors);
   
    if(scissors == "true")
    {
        AppLogger::debug("scissors Added");
		this->m_enabledObjectList.push_back("scissors");
    }

    std::string teddy_bear = settingValue(kEnableTeddyBear);
   
    if(teddy_bear == "true")
    {
        AppLogger::debug("teddy_bear Added");
		this->m_enabledObjectList.push_back("teddy bear");
    }

    std::string hair_drier = settingValue(kEnableHairDrier);
   
    if(hair_drier == "true")
    {
        AppLogger::debug("hair drier Added");
		this->m_enabledObjectList.push_back("hair drier");
    }

    std::string toothbrush = settingValue(kEnableToothbrush);
   
    if(toothbrush == "true")
    {
        AppLogger::debug("toothbrush Added");
		this->m_enabledObjectList.push_back("toothbrush");
    }

    // Confidence Threshold Settings
    std::string confidenceThreshold = settingValue(kConfidenceThreshold);

    // line crossing settings
    
    std::string lineDrawing = settingValue(kLineCrossingDrawing);
    AppLogger::debug("Line drawing\n" + lineDrawing);

    std::string jsonErr;
    const auto jsonDrawing = nx::kit::Json::parse(lineDrawing.c_str(), jsonErr);

    nx::kit::Json jsonLabel = jsonDrawing["label"];
    std::string labelStr = jsonLabel.string_value();
    AppLogger::debug("Label: " + labelStr);

    nx::kit::Json jsonFigure = jsonDrawing["figure"];

    nx::kit::Json jsonDirection = jsonFigure["direction"];
    std::string dirStr = jsonDirection.string_value();
    this->m_direction = dirStr;
    AppLogger::debug("direction: " + dirStr);
    bool isLeft = (dirStr == "left");
    std::string leftStr = isLeft ? "Is left" : "Is right";   

    nx::kit::Json jsonColor = jsonFigure["color"];
    std::string colorStr = jsonColor.string_value();    

    nx::kit::Json jsonShowCam = jsonDrawing["showOnCamera"];
    bool showCam = jsonShowCam.bool_value();    

    nx::kit::Json pointArray = jsonFigure["points"];

    // only 2 points for line segment
    NXPoint point1(0, 0);
    NXPoint point2(0, 0);
    size_t itemIndex = 0;
    for (auto& item : pointArray.array_items())
    {
        
        if (item.is_array())
        {
            nx::kit::Json nextPoint = item.array_items();
            nx::kit::Json pX = nextPoint[0];
            nx::kit::Json pY = nextPoint[1];

            double xValue = pX.number_value();
            double yValue = pY.number_value();

            if (itemIndex == 0)
            {
                point1.x = (float)xValue;
                point1.y = (float)yValue;
                point1.y = 1 - point1.y; // translating for external python line coordiante

                this->m_point1_x = std::to_string(point1.x);
                this->m_point1_y = std::to_string(point1.y); 

            }
            else if (itemIndex == 1)
            {
                point2.x = (float)xValue;
                point2.y = (float)yValue;
                point2.y = 1 - point2.y; // translating for external python line coordiante

                this->m_point2_x = std::to_string(point2.x);
                this->m_point2_y = std::to_string(point2.y);
            }

            NXPoint point(xValue, yValue);
            //ptsVector.push_back(point);
           
        }
        else
        {
            AppLogger::debug("Not array");
        }

        itemIndex++;
    }

    // create an output string stream
    std::ostringstream oss_obj;
    // iterate over the vector and add each element to the output stream
    for (int i = 0; i < m_enabledObjectList.size(); ++i) {
        oss_obj << m_enabledObjectList[i] << " ";
    }
    // convert the output stream to a string
    std::string objectsVectorStr = oss_obj.str();
    
    // send line points to external python element
    std::string jsonstr = "{\"device_id\":\"";
    jsonstr += this->m_deviceID; 
    jsonstr += "\",\"point1_x\":\"";
    jsonstr += this->m_point1_x;
    jsonstr += "\",\"point1_y\":\"";
    jsonstr += this->m_point1_y;
    jsonstr += "\",\"point2_x\":\"";
    jsonstr += this->m_point2_x;
    jsonstr += "\",\"point2_y\":\"";
    jsonstr += this->m_point2_y;
    jsonstr += "\",\"direction\":\"";
    jsonstr += this->m_direction;
    jsonstr += "\",\"label\":\"";
    jsonstr += labelStr;
    jsonstr += "\",\"object_type\":\"";
    jsonstr += objectsVectorStr;
    jsonstr += "\"}";

    AppLogger::debug("line data sent over draw_line route " + jsonstr);

    std::string endpoint = "draw_line";
   
    std::string res = postCURL(jsonstr, endpoint); // send topic + cam info via curl
    AppLogger::debug("Response from flask server: draw_line route " + res);
 
    // polygon ROI Settings
    std::string polygonRegion = settingValue(kPolygonRegion);
    AppLogger::debug("Polygon drawing\n" + polygonRegion);

    std::string polyErr;
    const auto polygonDrawing = nx::kit::Json::parse(polygonRegion.c_str(), polyErr);

    nx::kit::Json polyLabel = polygonDrawing["label"];
    std::string polyLabelStr = polyLabel.string_value();
    AppLogger::debug("Label: " + polyLabelStr);

    nx::kit::Json polyFigure = polygonDrawing["figure"];

    // NO direction variable for Polygon, fill in but not used
    std::string polyDirection = "left";

    nx::kit::Json polyColor = polyFigure["color"];
    std::string colorPolyStr = polyColor.string_value();
    AppLogger::debug("Color: " + colorPolyStr);

    nx::kit::Json polyShowCam = polygonDrawing["showOnCamera"];
    bool showPolyCam = polyShowCam.bool_value();
    AppLogger::debug("Show Cam?  " + polyShowCam.dump());

    nx::kit::Json pointPolyArray = polyFigure["points"];

    std::vector<double> ptsPolyVector;
   
    for (auto& item : pointPolyArray.array_items())
    {
        AppLogger::debug("Point: " + item.dump());
        if (item.is_array())
        {
            nx::kit::Json nextPoint = item.array_items();
            nx::kit::Json pX = nextPoint[0];
            nx::kit::Json pY = nextPoint[1];

            double xVal = pX.number_value();
            ptsPolyVector.push_back(xVal);

            double yVal = pY.number_value();       
            ptsPolyVector.push_back(yVal);

            AppLogger::debug("Point: x=" + pX.dump() + ", y=" + pY.dump());
        }
        else
        {
            AppLogger::debug("Not array");
        }
    }

    // create an output string stream
    std::ostringstream oss;
    // iterate over the vector and add each element to the output stream
    for (int i = 0; i < ptsPolyVector.size(); ++i) {
        oss << ptsPolyVector[i] << " ";
    }
    // convert the output stream to a string
    std::string polyVectorStr = oss.str();    

    // send polygon points to external python element
    std::string jsonstr_poly = "{\"device_id\":\"";
    jsonstr_poly += this->m_deviceID; 
    jsonstr_poly += "\",\"poly_vector\":\"";
    jsonstr_poly += polyVectorStr;
    jsonstr_poly += "\",\"label\":\"";
    jsonstr_poly += polyLabelStr;
    jsonstr_poly += "\",\"object_type\":\"";
    jsonstr_poly += objectsVectorStr;
    jsonstr_poly += "\"}";

    AppLogger::debug("line data sent over polygon route " + jsonstr_poly);
    std::string endpoint_poly = "polygon";
    std::string res_poly = postCURL(jsonstr_poly, endpoint_poly); // send topic + cam info via curl
    AppLogger::debug("Response from flask server: polygon route " + res_poly);

    // selected objects route
    std::string jsonstr_objects = "{\"object_type\":\"";
    jsonstr_objects += "count";
    jsonstr_objects += "\",\"device_id\":\"";
    jsonstr_objects += this->m_deviceID;
    jsonstr_objects += "\"}";
    std::string endpoint_objects = "objects";
    std::string res_objects = postCURL(jsonstr_objects, endpoint_objects); // send topic + cam info via curl

	
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
            "id": ")json" + kLineCrossingEvent + R"json(",
            "name": "Line Crossing Event"
        }
    ],
    "objectTypes": [
        {
            "id": ")json" + kPersonObjectType + R"json(",
            "name": "Person"
        },
        {
            "id": ")json" + kBicycleObjectType + R"json(",
            "name": "Bicycle"
        },
        {
            "id": ")json" + kCarObjectType + R"json(",
            "name": "Car"
        },
        {
            "id": ")json" + kMotorbikeObjectType + R"json(",
            "name": "Motorbike"
        },
        {
            "id": ")json" + kAeroplaneObjectType + R"json(",
            "name": "Aeroplane"
        },
        {
            "id": ")json" + kBusObjectType + R"json(",
            "name": "Bus"
        },
        {
            "id": ")json" + kTrainObjectType + R"json(",
            "name": "Train"
        },
        {
            "id": ")json" + kTruckObjectType + R"json(",
            "name": "Truck"
        },
        {
            "id": ")json" + kBoatObjectType + R"json(",
            "name": "Boat"
        },
        {
            "id": ")json" + kTrafficLightObjectType + R"json(",
            "name": "Traffic Light"
        },
        {
            "id": ")json" + kFireHydrantObjectType + R"json(",
            "name": "Fire Hydrant"
        },
        {
            "id": ")json" + kStopSignObjectType + R"json(",
            "name": "Stop Sign"
        },
        {
            "id": ")json" + kParkingMeterObjectType + R"json(",
            "name": "Parking Meter"
        },
        {
            "id": ")json" + kBenchObjectType + R"json(",
            "name": "Bench"
        },
        {
            "id": ")json" + kBirdObjectType + R"json(",
            "name": "Bird"
        },
        {
            "id": ")json" + kCatObjectType + R"json(",
            "name": "Cat"
        },
        {
            "id": ")json" + kDogObjectType + R"json(",
            "name": "Dog"
        },
        {
            "id": ")json" + kHorseObjectType + R"json(",
            "name": "Horse"
        },
        {
            "id": ")json" + kSheepObjectType + R"json(",
            "name": "Sheep"
        },
        {
            "id": ")json" + kCowObjectType + R"json(",
            "name": "Cow"
        },
        {
            "id": ")json" + kElephantObjectType + R"json(",
            "name": "Elephant"
        },
        {
            "id": ")json" + kBearObjectType + R"json(",
            "name": "Bear"
        },
        {
            "id": ")json" + kZebraObjectType + R"json(",
            "name": "Zebra"
        },
        {
            "id": ")json" + kGiraffeObjectType + R"json(",
            "name": "Giraffe"
        },
        {
            "id": ")json" + kBackpackObjectType + R"json(",
            "name": "Backpack"
        },
        {
            "id": ")json" + kUmbrellaObjectType + R"json(",
            "name": "Umbrella"
        },
        {
            "id": ")json" + kHandbagObjectType + R"json(",
            "name": "Handbag"
        },
        {
            "id": ")json" + kTieObjectType + R"json(",
            "name": "Tie"
        },
        {
            "id": ")json" + kSuitcaseObjectType + R"json(",
            "name": "Suitcase"
        },
        {
            "id": ")json" + kFrisbeeObjectType + R"json(",
            "name": "Frisbee"
        },
        {
            "id": ")json" + kSkisObjectType + R"json(",
            "name": "Skis"
        },
        {
            "id": ")json" + kSnowboardObjectType + R"json(",
            "name": "Snowboard"
        },
        {
            "id": ")json" + kSportsBallObjectType + R"json(",
            "name": "Sports Ball"
        },
        {
            "id": ")json" + kKiteObjectType + R"json(",
            "name": "Kite"
        },
        {
            "id": ")json" + kBaseballBatObjectType + R"json(",
            "name": "Baseball Bat"
        },
        {
            "id": ")json" + kBaseballGloveObjectType + R"json(",
            "name": "Baseball Glove"
        },
        {
            "id": ")json" + kSkateboardObjectType + R"json(",
            "name": "Skateboard"
        },
        {
            "id": ")json" + kSurfboardObjectType + R"json(",
            "name": "Surfboard"
        },
        {
            "id": ")json" + kTennisRacketObjectType + R"json(",
            "name": "Tennis Racket"
        },
        {
            "id": ")json" + kBottleObjectType + R"json(",
            "name": "Bottle"
        },
        {
            "id": ")json" + kWineGlassObjectType + R"json(",
            "name": "Wine Glass"
        },
        {
            "id": ")json" + kCupObjectType + R"json(",
            "name": "Cup"
        },
        {
            "id": ")json" + kForkObjectType + R"json(",
            "name": "Fork"
        },
        {
            "id": ")json" + kKnifeObjectType + R"json(",
            "name": "Knife"
        },
        {
            "id": ")json" + kSpoonObjectType + R"json(",
            "name": "Spoon"
        },
        {
            "id": ")json" + kBowlObjectType + R"json(",
            "name": "Bowl"
        },
        {
            "id": ")json" + kBananaObjectType + R"json(",
            "name": "Banana"
        },
        {
            "id": ")json" + kAppleObjectType + R"json(",
            "name": "Apple"
        },
        {
            "id": ")json" + kSandwichObjectType + R"json(",
            "name": "Sandwich"
        },
        {
            "id": ")json" + kOrangeObjectType + R"json(",
            "name": "Orange"
        },
        {
            "id": ")json" + kBroccoliObjectType + R"json(",
            "name": "Broccoli"
        },
        {
            "id": ")json" + kCarrotObjectType + R"json(",
            "name": "Carrot"
        },
        {
            "id": ")json" + kHotdogObjectType + R"json(",
            "name": "Hotdog"
        },
        {
            "id": ")json" + kPizzaObjectType + R"json(",
            "name": "Pizza"
        },
        {
            "id": ")json" + kDonutObjectType + R"json(",
            "name": "Donut"
        },
        {
            "id": ")json" + kCakeObjectType + R"json(",
            "name": "Cake"
        },
        {
            "id": ")json" + kChairObjectType + R"json(",
            "name": "Chair"
        },
        {
            "id": ")json" + kSofaObjectType + R"json(",
            "name": "Sofa"
        },
        {
            "id": ")json" + kPottedplantObjectType + R"json(",
            "name": "Potted Plant"
        },
        {
            "id": ")json" + kBedObjectType + R"json(",
            "name": "Bed"
        },
        {
            "id": ")json" + kDiningtableObjectType + R"json(",
            "name": "Dining Table"
        },
        {
            "id": ")json" + kToiletObjectType + R"json(",
            "name": "Toilet"
        },
        {
            "id": ")json" + kTvmonitorObjectType + R"json(",
            "name": "Tv Monitor"
        },        
        {
            "id": ")json" + kLaptopObjectType + R"json(",
            "name": "Laptop"
        },
        {
            "id": ")json" + kMouseObjectType + R"json(",
            "name": "Mouse"
        },
        {
            "id": ")json" + kRemoteObjectType + R"json(",
            "name": "Remote"
        },
        {
            "id": ")json" + kKeyboardObjectType + R"json(",
            "name": "Keyboard"
        },
        {
            "id": ")json" + kCellPhoneObjectType + R"json(",
            "name": "Cell Phone"
        },
        {
            "id": ")json" + kMicrowaveObjectType + R"json(",
            "name": "Microwave"
        },
        {
            "id": ")json" + kOvenObjectType + R"json(",
            "name": "Oven"
        },
        {
            "id": ")json" + kToasterObjectType + R"json(",
            "name": "Toaster"
        },
        {
            "id": ")json" + kSinkObjectType + R"json(",
            "name": "Sink"
        },
        {
            "id": ")json" + kRefrigeratorObjectType + R"json(",
            "name": "Refrigerator"
        },
        {
            "id": ")json" + kBookObjectType + R"json(",
            "name": "Book"
        },
        {
            "id": ")json" + kClockObjectType + R"json(",
            "name": "Clock"
        },
        {
            "id": ")json" + kVaseObjectType + R"json(",
            "name": "Vase"
        },        
        {
            "id": ")json" + kScissorsObjectType + R"json(",
            "name": "Scissors"
        },
        {
            "id": ")json" + kTeddyBearObjectType + R"json(",
            "name": "Teddy Bear"
        },
        {
            "id": ")json" + kHairDrierObjectType + R"json(",
            "name": "Hair Drier"
        },
        {
            "id": ")json" + kToothbrushObjectType + R"json(",
            "name": "Toothbrush"
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
    // auto current_heartbeat = std::chrono::high_resolution_clock::now();
    // auto elapsed_sec = std::chrono::duration_cast<std::chrono::seconds>(current_heartbeat - this->m_heartbeat);

    // // push line data to external python app at every one minute
    // if (elapsed_sec.count() > 60)
    // {
    //     this->m_heartbeat = current_heartbeat;

    //     //send line data to external python element      
    //     std::string jsonstr = "{\"device_id\":\"";       
    //     jsonstr += this->m_deviceID; 
    //     jsonstr += "\",\"zmq_address\":\"";
    //     jsonstr += this->m_zmq_port;
    //     jsonstr += "\",\"point1_x\":\"";
    //     jsonstr += this->m_point1_x;
    //     jsonstr += "\",\"point1_y\":\"";
    //     jsonstr += this->m_point1_y;
    //     jsonstr += "\",\"point2_x\":\"";
    //     jsonstr += this->m_point2_x;
    //     jsonstr += "\",\"point2_y\":\"";
    //     jsonstr += this->m_point2_y;
    //     jsonstr += "\",\"direction\":\"";
    //     jsonstr += this->m_direction;
    //     jsonstr += "\"}";    
        
    //     std::string endpoint = "heartbeat_test";
    //     std::string res = postCURL(jsonstr, endpoint);
    // }

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
    eventMetadata->setTypeId(kLineCrossingEvent);
    eventMetadata->setIsActive(true);
    eventMetadata->setCaption("Line Crossed");
    eventMetadata->setDescription("Line Crossed In");

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
    //AppLogger::debug("m_objectType: " + m_objectType);
    //handle unselected objects
    if (m_objectType != "" )
    {
       objectMetadata->setTypeId(m_objectType);
    }

    objectMetadata->setTrackId(this->m_trackId);
  
    objectMetadata->setSubtype(this->m_people_count_type);    

    float width = m_width;
    float height = m_height;
    float x = m_xMin;
    float y = m_yMin;

    objectMetadata->setBoundingBox(Rect(x, y, width, height));
    objectMetadata->addAttribute(nx::sdk::makePtr<Attribute>(IAttribute::Type::string, "Count Type: ", this->m_people_count_type ));
    
    objectMetadataPacket->addItem(objectMetadata.get());

    

    return objectMetadataPacket;
}

} // namespace hailo_object_count
} // namespace analytics
} // namespace vms_server_plugins
} // namespace nx
