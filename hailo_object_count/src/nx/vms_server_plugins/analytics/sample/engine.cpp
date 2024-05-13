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
namespace hailo_object_count {

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
"eventTypes": [
        {
            "id": ")json" + kLineCrossingEvent + R"json(",
            "name": "Line Crossing Event"
        }
    ],
   
    "deviceAgentSettingsModel": {
        "type": "Settings",
        "items":
        [
			{
				"type": "GroupBox",
						"caption": "Line Crossing Settings",
						"items": [								
								
								{
									"type": "LineFigure",
									"name": ")json" + kLineCrossingDrawing + R"json(",
									"caption": "Person Counting: Line Crossing",
									"description": "Crossing Line",
									"minPoints": 2,
									"maxPoints": 2,
									"allowedDirections": "one"
								}										
						]

			},
			{
				"type": "GroupBox",
						"caption": "ROI Settings",
						"items": [								
								
								{
									"type": "PolygonFigure",
									"name": ")json" + kPolygonRegion + R"json(",
									"caption": "Person Counting: ROI",
									"description": "Crossing Line",
									"minPoints": 3,
									"maxPoints": 50
									
								}										
						]

			},
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
							"name": ")json" + kEnablePerson + R"json(",
							"caption": "Person",
							"description": "If true, selected",
							"defaultValue": true
						},
						{
							"type": "CheckBox",
							"name": ")json" + kEnableBicycle + R"json(",
							"caption": "Bicycle",
							"description": "If true, selected",
							"defaultValue": false
						},
						{
							"type": "CheckBox",
							"name": ")json" + kEnableCar + R"json(",
							"caption": "Car",
							"description": "If true, selected",
							"defaultValue": false
						},
						{
							"type": "CheckBox",
							"name": ")json" + kEnableMotorbike + R"json(",
							"caption": "Motorbike",
							"description": "If true, selected",
							"defaultValue": false
						},
						{
							"type": "CheckBox",
							"name": ")json" + kEnableAeroplane + R"json(",
							"caption": "Aeroplane",
							"description": "If true, selected",
							"defaultValue": false
						},
						{
							"type": "CheckBox",
							"name": ")json" + kEnableBus + R"json(",
							"caption": "Bus",
							"description": "If true, selected",
							"defaultValue": false
						},
						{
							"type": "CheckBox",
							"name": ")json" + kEnableTrain + R"json(",
							"caption": "Train",
							"description": "If true, selected",
							"defaultValue": false
						},
						{
							"type": "CheckBox",
							"name": ")json" + kEnableTruck + R"json(",
							"caption": "Truck",
							"description": "If true, selected",
							"defaultValue": false
						},
						{
							"type": "CheckBox",
							"name": ")json" + kEnableBoat + R"json(",
							"caption": "Boat",
							"description": "If true, selected",
							"defaultValue": false
						},
						{
							"type": "CheckBox",
							"name": ")json" + kEnableTrafficLight + R"json(",
							"caption": "Traffic Light",
							"description": "If true, selected",
							"defaultValue": false
						},
						{
							"type": "CheckBox",
							"name": ")json" + kEnableFireHydrant + R"json(",
							"caption": "Fire Hydrant",
							"description": "If true, selected",
							"defaultValue": false
						},
						{
							"type": "CheckBox",
							"name": ")json" + kEnableStopSign + R"json(",
							"caption": "Stop Sign",
							"description": "If true, selected",
							"defaultValue": false
						},
						{
							"type": "CheckBox",
							"name": ")json" + kEnableParkingMeter + R"json(",
							"caption": "Parking Meter",
							"description": "If true, selected",
							"defaultValue": false
						},
                        {
							"type": "CheckBox",
							"name": ")json" + kEnableBench + R"json(",
							"caption": "Bench",
							"description": "If true, selected",
							"defaultValue": false
						},
                        {
							"type": "CheckBox",
							"name": ")json" + kEnableBird + R"json(",
							"caption": "Bird",
							"description": "If true, selected",
							"defaultValue": false
						},
                        {
							"type": "CheckBox",
							"name": ")json" + kEnableCat + R"json(",
							"caption": "Cat",
							"description": "If true, selected",
							"defaultValue": false
						},
                        {
							"type": "CheckBox",
							"name": ")json" + kEnableDog + R"json(",
							"caption": "Dog",
							"description": "If true, selected",
							"defaultValue": false
						},
                        {
							"type": "CheckBox",
							"name": ")json" + kEnableHorse + R"json(",
							"caption": "Horse",
							"description": "If true, selected",
							"defaultValue": false
						},
                        {
							"type": "CheckBox",
							"name": ")json" + kEnableSheep + R"json(",
							"caption": "Sheep",
							"description": "If true, selected",
							"defaultValue": false
						},
                        {
							"type": "CheckBox",
							"name": ")json" + kEnableCow + R"json(",
							"caption": "Cow",
							"description": "If true, selected",
							"defaultValue": false
						},
                        {
							"type": "CheckBox",
							"name": ")json" + kEnableElephant + R"json(",
							"caption": "Elephant",
							"description": "If true, selected",
							"defaultValue": false
						},
                        {
							"type": "CheckBox",
							"name": ")json" + kEnableBear + R"json(",
							"caption": "Bear",
							"description": "If true, selected",
							"defaultValue": false
						},
                        {
							"type": "CheckBox",
							"name": ")json" + kEnableBZebra + R"json(",
							"caption": "Zebra",
							"description": "If true, selected",
							"defaultValue": false
						},
                        {
							"type": "CheckBox",
							"name": ")json" + kEnableBGiraffe + R"json(",
							"caption": "Giraffe",
							"description": "If true, selected",
							"defaultValue": false
						},
                        {
							"type": "CheckBox",
							"name": ")json" + kEnableBackpack + R"json(",
							"caption": "Backpack",
							"description": "If true, selected",
							"defaultValue": false
						},
                        {
							"type": "CheckBox",
							"name": ")json" + kEnableUmbrella + R"json(",
							"caption": "Umbrella",
							"description": "If true, selected",
							"defaultValue": false
						},
                        {
							"type": "CheckBox",
							"name": ")json" + kEnableHandbag + R"json(",
							"caption": "Handbag",
							"description": "If true, selected",
							"defaultValue": false
						},
                        {
							"type": "CheckBox",
							"name": ")json" + kEnableTie + R"json(",
							"caption": "Tie",
							"description": "If true, selected",
							"defaultValue": false
						},
                        {
							"type": "CheckBox",
							"name": ")json" + kEnableSuitcase + R"json(",
							"caption": "Suitcase",
							"description": "If true, selected",
							"defaultValue": false
						},
                        {
							"type": "CheckBox",
							"name": ")json" + kEnableFrisbee + R"json(",
							"caption": "Frisbee",
							"description": "If true, selected",
							"defaultValue": false
						},
                        {
							"type": "CheckBox",
							"name": ")json" + kEnableSkis + R"json(",
							"caption": "Skis",
							"description": "If true, selected",
							"defaultValue": false
						},
                        {
							"type": "CheckBox",
							"name": ")json" + kEnableSnowboard + R"json(",
							"caption": "Snowboard",
							"description": "If true, selected",
							"defaultValue": false
						},
                        {
							"type": "CheckBox",
							"name": ")json" + kEnableSportsBall + R"json(",
							"caption": "SportsBall",
							"description": "If true, selected",
							"defaultValue": false
						},
                        {
							"type": "CheckBox",
							"name": ")json" + kEnableKite + R"json(",
							"caption": "Kite",
							"description": "If true, selected",
							"defaultValue": false
						},
                        {
							"type": "CheckBox",
							"name": ")json" + kEnableBaseballBat + R"json(",
							"caption": "BaseballBat",
							"description": "If true, selected",
							"defaultValue": false
						},
                        {
							"type": "CheckBox",
							"name": ")json" + kEnableBaseballGlove + R"json(",
							"caption": "BaseballGlove",
							"description": "If true, selected",
							"defaultValue": false
						},
                        {
							"type": "CheckBox",
							"name": ")json" + kEnableSkateboard + R"json(",
							"caption": "Skateboard",
							"description": "If true, selected",
							"defaultValue": false
						},
                        {
							"type": "CheckBox",
							"name": ")json" + kEnableSurfboard + R"json(",
							"caption": "Surfboard",
							"description": "If true, selected",
							"defaultValue": false
						},
                        {
							"type": "CheckBox",
							"name": ")json" + kEnableTennisRacket + R"json(",
							"caption": "TennisRacket",
							"description": "If true, selected",
							"defaultValue": false
						},
                        {
							"type": "CheckBox",
							"name": ")json" + kEnableBottle + R"json(",
							"caption": "Bottle",
							"description": "If true, selected",
							"defaultValue": false
						},
                        {
							"type": "CheckBox",
							"name": ")json" + kEnableWineGlass + R"json(",
							"caption": "Wine Glass",
							"description": "If true, selected",
							"defaultValue": false
						},
                        {
							"type": "CheckBox",
							"name": ")json" + kEnableCup + R"json(",
							"caption": "Cup",
							"description": "If true, selected",
							"defaultValue": false
						},
                        {
							"type": "CheckBox",
							"name": ")json" + kEnableFork + R"json(",
							"caption": "Fork",
							"description": "If true, selected",
							"defaultValue": false
						},
                        {
							"type": "CheckBox",
							"name": ")json" + kEnableKnife + R"json(",
							"caption": "Knife",
							"description": "If true, selected",
							"defaultValue": false
						},
                        {
							"type": "CheckBox",
							"name": ")json" + kEnableSpoon + R"json(",
							"caption": "Spoon",
							"description": "If true, selected",
							"defaultValue": false
						},
                        {
							"type": "CheckBox",
							"name": ")json" + kEnableBowl + R"json(",
							"caption": "Bowl",
							"description": "If true, selected",
							"defaultValue": false
						},
                        {
							"type": "CheckBox",
							"name": ")json" + kEnableBanana + R"json(",
							"caption": "Banana",
							"description": "If true, selected",
							"defaultValue": false
						},
                        {
							"type": "CheckBox",
							"name": ")json" + kEnableApple + R"json(",
							"caption": "Apple",
							"description": "If true, selected",
							"defaultValue": false
						},
                        {
							"type": "CheckBox",
							"name": ")json" + kEnableSandwich + R"json(",
							"caption": "Sandwich",
							"description": "If true, selected",
							"defaultValue": false
						},
                        {
							"type": "CheckBox",
							"name": ")json" + kEnableOrange + R"json(",
							"caption": "Orange",
							"description": "If true, selected",
							"defaultValue": false
						},
                        {
							"type": "CheckBox",
							"name": ")json" + kEnableBroccoli + R"json(",
							"caption": "Broccoli",
							"description": "If true, selected",
							"defaultValue": false
						},
                        {
							"type": "CheckBox",
							"name": ")json" + kEnableCarrot + R"json(",
							"caption": "Carrot",
							"description": "If true, selected",
							"defaultValue": false
						},
                        {
							"type": "CheckBox",
							"name": ")json" + kEnableHotdog + R"json(",
							"caption": "Hotdog",
							"description": "If true, selected",
							"defaultValue": false
						},
                        {
							"type": "CheckBox",
							"name": ")json" + kEnablePizza + R"json(",
							"caption": "Piza",
							"description": "If true, selected",
							"defaultValue": false
						},
                        {
							"type": "CheckBox",
							"name": ")json" + kEnableDonut + R"json(",
							"caption": "Donut",
							"description": "If true, selected",
							"defaultValue": false
						},
                        {
							"type": "CheckBox",
							"name": ")json" + kEnableCake + R"json(",
							"caption": "Cake",
							"description": "If true, selected",
							"defaultValue": false
						},
                        {
							"type": "CheckBox",
							"name": ")json" + kEnableChair + R"json(",
							"caption": "Chair",
							"description": "If true, selected",
							"defaultValue": false
						},
                        {
							"type": "CheckBox",
							"name": ")json" + kEnableSofa + R"json(",
							"caption": "Sofa",
							"description": "If true, selected",
							"defaultValue": false
						},
                        {
							"type": "CheckBox",
							"name": ")json" + kEnablePottedplant + R"json(",
							"caption": "Potted plant",
							"description": "If true, selected",
							"defaultValue": false
						},
                        {
							"type": "CheckBox",
							"name": ")json" + kEnableBed + R"json(",
							"caption": "Bed",
							"description": "If true, selected",
							"defaultValue": false
						},
                        {
							"type": "CheckBox",
							"name": ")json" + kEnableDiningtable + R"json(",
							"caption": "Dining table",
							"description": "If true, selected",
							"defaultValue": false
						},
                        {
							"type": "CheckBox",
							"name": ")json" + kEnableToilet + R"json(",
							"caption": "Toilet",
							"description": "If true, selected",
							"defaultValue": false
						},
                        {
							"type": "CheckBox",
							"name": ")json" + kEnableTvmonitor + R"json(",
							"caption": "Tv monitor",
							"description": "If true, selected",
							"defaultValue": false
						},
                        {
							"type": "CheckBox",
							"name": ")json" + kEnableLaptop + R"json(",
							"caption": "Laptop",
							"description": "If true, selected",
							"defaultValue": false
						},
                        {
							"type": "CheckBox",
							"name": ")json" + kEnableMouse + R"json(",
							"caption": "Mouse",
							"description": "If true, selected",
							"defaultValue": false
						},
                        {
							"type": "CheckBox",
							"name": ")json" + kEnableRemote + R"json(",
							"caption": "Remote",
							"description": "If true, selected",
							"defaultValue": false
						},
                        {
							"type": "CheckBox",
							"name": ")json" + kEnableKeyboard + R"json(",
							"caption": "Keyboard",
							"description": "If true, selected",
							"defaultValue": false
						},
                        {
							"type": "CheckBox",
							"name": ")json" + kEnableCellPhone + R"json(",
							"caption": "Cell Phone",
							"description": "If true, selected",
							"defaultValue": false
						},
                        {
							"type": "CheckBox",
							"name": ")json" + kEnableMicrowave + R"json(",
							"caption": "Microwave",
							"description": "If true, selected",
							"defaultValue": false
						},
                        {
							"type": "CheckBox",
							"name": ")json" + kEnableOven + R"json(",
							"caption": "Oven",
							"description": "If true, selected",
							"defaultValue": false
						},
                        {
							"type": "CheckBox",
							"name": ")json" + kEnableToaster + R"json(",
							"caption": "Toaster",
							"description": "If true, selected",
							"defaultValue": false
						},
                        {
							"type": "CheckBox",
							"name": ")json" + kEnableSink + R"json(",
							"caption": "Sink",
							"description": "If true, selected",
							"defaultValue": false
						},
                        {
							"type": "CheckBox",
							"name": ")json" + kEnableRefrigerator + R"json(",
							"caption": "Refrigerator",
							"description": "If true, selected",
							"defaultValue": false
						},
                        {
							"type": "CheckBox",
							"name": ")json" + kEnableBook + R"json(",
							"caption": "Book",
							"description": "If true, selected",
							"defaultValue": false
						},
                        {
							"type": "CheckBox",
							"name": ")json" + kEnableClock + R"json(",
							"caption": "Clock",
							"description": "If true, selected",
							"defaultValue": false
						},
                        {
							"type": "CheckBox",
							"name": ")json" + kEnableVase + R"json(",
							"caption": "Vase",
							"description": "If true, selected",
							"defaultValue": false
						},
                        {
							"type": "CheckBox",
							"name": ")json" + kEnableScissors + R"json(",
							"caption": "Scissors",
							"description": "If true, selected",
							"defaultValue": false
						},
                        {
							"type": "CheckBox",
							"name": ")json" + kEnableTeddyBear + R"json(",
							"caption": "Teddy Bear",
							"description": "If true, selected",
							"defaultValue": false
						},
                        {
							"type": "CheckBox",
							"name": ")json" + kEnableHairDrier + R"json(",
							"caption": "Hair Drier",
							"description": "If true, selected",
							"defaultValue": false
						},
                        {
							"type": "CheckBox",
							"name": ")json" + kEnableToothbrush + R"json(",
							"caption": "Toothbrush",
							"description": "If true, selected",
							"defaultValue": false
						}
				]
			}
        ]
    }
}
)json";
}

} // namespace hailo_object_count
} // namespace analytics
} // namespace vms_server_plugins
} // namespace nx
