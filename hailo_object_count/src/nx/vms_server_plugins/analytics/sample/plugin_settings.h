#pragma once

#include <iostream>
#include <vector>
#include <set>
#include <algorithm>

namespace nx {
namespace vms_server_plugins {
namespace analytics {
namespace hailo_object_count {	

	//Object types
    const std::string kPersonObjectType = "nx.hailo_object_count.person";
    const std::string kBicycleObjectType = "nx.hailo_object_count.bicycle";
    const std::string kCarObjectType = "nx.hailo_object_count.car";
    const std::string kMotorbikeObjectType = "nx.hailo_object_count.motorbike";
    const std::string kAeroplaneObjectType = "nx.hailo_object_count.aeroplane";
    const std::string kBusObjectType = "nx.hailo_object_count.bus";
    const std::string kTrainObjectType = "nx.hailo_object_count.train";
    const std::string kTruckObjectType = "nx.hailo_object_count.truck";
    const std::string kBoatObjectType = "nx.hailo_object_count.boat";
    const std::string kTrafficLightObjectType = "nx.hailo_object_count.traffic_light";
    const std::string kFireHydrantObjectType = "nx.hailo_object_count.firehy_drant";
    const std::string kStopSignObjectType = "nx.hailo_object_count.stop_sign";
    const std::string kParkingMeterObjectType = "nx.hailo_object_count.parking_meter";
    const std::string kBenchObjectType = "nx.hailo_object_count.bench";
    const std::string kBirdObjectType = "nx.hailo_object_count.bird";
    const std::string kCatObjectType = "nx.hailo_object_count.cat";
    const std::string kDogObjectType = "nx.hailo_object_count.dog";
    const std::string kHorseObjectType = "nx.hailo_object_count.horse";
    const std::string kSheepObjectType = "nx.hailo_object_count.sheep";
    const std::string kCowObjectType = "nx.hailo_object_count.cow";
    const std::string kElephantObjectType = "nx.hailo_object_count.elephant";
    const std::string kBearObjectType = "nx.hailo_object_count.bear";
    const std::string kZebraObjectType = "nx.hailo_object_count.zebra";
    const std::string kGiraffeObjectType = "nx.hailo_object_count.giraffe";
    const std::string kBackpackObjectType = "nx.hailo_object_count.backpack";
    const std::string kUmbrellaObjectType = "nx.hailo_object_count.umbrella";
    const std::string kHandbagObjectType = "nx.hailo_object_count.handbag";
    const std::string kTieObjectType = "nx.hailo_object_count.tie";
    const std::string kSuitcaseObjectType = "nx.hailo_object_count.suitcase";
    const std::string kFrisbeeObjectType = "nx.hailo_object_count.frisbee";
    const std::string kSkisObjectType = "nx.hailo_object_count.skis";
    const std::string kSnowboardObjectType = "nx.hailo_object_count.snowbaord";
    const std::string kSportsBallObjectType = "nx.hailo_object_count.sportsball";
    const std::string kKiteObjectType = "nx.hailo_object_count.kite";
    const std::string kBaseballBatObjectType = "nx.hailo_object_count.baseball_bat";
    const std::string kBaseballGloveObjectType = "nx.hailo_object_count.baseball_glove";
    const std::string kSkateboardObjectType = "nx.hailo_object_count.skateboard";
    const std::string kSurfboardObjectType = "nx.hailo_object_count.surfboard";
    const std::string kTennisRacketObjectType = "nx.hailo_object_count.tennis_rackets";
    const std::string kBottleObjectType = "nx.hailo_object_count.bottle";
    const std::string kWineGlassObjectType = "nx.hailo_object_count.wineglass";
    const std::string kCupObjectType = "nx.hailo_object_count.cup";
    const std::string kForkObjectType = "nx.hailo_object_count.fork";
    const std::string kKnifeObjectType = "nx.hailo_object_count.knife";
    const std::string kSpoonObjectType = "nx.hailo_object_count.spoon";
    const std::string kBowlObjectType = "nx.hailo_object_count.bowl";
    const std::string kBananaObjectType = "nx.hailo_object_count.banana";
    const std::string kAppleObjectType = "nx.hailo_object_count.apple";
    const std::string kSandwichObjectType = "nx.hailo_object_count.sandwich";
    const std::string kOrangeObjectType = "nx.hailo_object_count.orange";
    const std::string kBroccoliObjectType = "nx.hailo_object_count.broccoli";
    const std::string kCarrotObjectType = "nx.hailo_object_count.carrot";
    const std::string kHotdogObjectType = "nx.hailo_object_count.hotdog";
    const std::string kPizzaObjectType = "nx.hailo_object_count.pizza";
    const std::string kDonutObjectType = "nx.hailo_object_count.donut";
    const std::string kCakeObjectType = "nx.hailo_object_count.cake";
    const std::string kChairObjectType = "nx.hailo_object_count.chair";
    const std::string kSofaObjectType = "nx.hailo_object_count.sofa";
    const std::string kPottedplantObjectType = "nx.hailo_object_count.potted_plant";
    const std::string kBedObjectType = "nx.hailo_object_count.bed";
    const std::string kDiningtableObjectType = "nx.hailo_object_count.dinning_table";
    const std::string kToiletObjectType = "nx.hailo_object_count.toilet";
    const std::string kTvmonitorObjectType = "nx.hailo_object_count.tv_monitor";
    const std::string kLaptopObjectType = "nx.hailo_object_count.laptop";
    const std::string kMouseObjectType = "nx.hailo_object_count.mouse";
    const std::string kRemoteObjectType = "nx.hailo_object_count.remote";
    const std::string kKeyboardObjectType = "nx.hailo_object_count.keyboard";
    const std::string kCellPhoneObjectType = "nx.hailo_object_count.cell_phone";
    const std::string kMicrowaveObjectType = "nx.hailo_object_count.microwave";
    const std::string kOvenObjectType = "nx.hailo_object_count.oven";
    const std::string kToasterObjectType = "nx.hailo_object_count.toaster";
    const std::string kSinkObjectType = "nx.hailo_object_count.sink";
    const std::string kRefrigeratorObjectType = "nx.hailo_object_count.refrigerator";
    const std::string kBookObjectType = "nx.hailo_object_count.book";
    const std::string kClockObjectType = "nx.hailo_object_count.clock";
    const std::string kVaseObjectType = "nx.hailo_object_count.vase";
    const std::string kScissorsObjectType = "nx.hailo_object_count.scissor";
    const std::string kTeddyBearObjectType = "nx.hailo_object_count.teddy_bear";
    const std::string kHairDrierObjectType = "nx.hailo_object_count.hair_drier";
    const std::string kToothbrushObjectType = "nx.hailo_object_count.toothbrush";

    //Object event type
    const std::string kNewObjectEventType = "nx.hailo_object_count.newObject";

    // Settings
    const std::string kConfidenceThreshold{ "nx.hailo_object_count.confidenceThreshold" };
    const std::string kDefaultConfidenceThreshold{ "0.8" };
	const std::string kHailoServerAddress{ "nx.hailo_object_count.hailoServerAddress" };
	const std::string kDefaultHailoServerAddress{ "http://127.0.0.1:47760" };
    const std::string kLineCrossingDrawing = "nx.hailo_object_count.lineCrossingDrawing";
    const std::string kPolygonRegion = "nx.hailo_object_count.polygonRegion";

    // line crossing variables
	const std::string kLineCrossingEvent = "nx.hailo_object_count.lineCrossingEvent";

    // user selection	
	const std::string kEnablePerson{"enablePerson"};
    const std::string kEnableBicycle{"enableBicycle"};
	const std::string kEnableCar{"enableCar"};	
	const std::string kEnableMotorbike{"enableMotorbike"};
	const std::string kEnableAeroplane{"enableAeroplane"};
	const std::string kEnableBus{"enableBus"};
	const std::string kEnableTrain{"enableTrain"};
	const std::string kEnableTruck{"enableTruck"};
	const std::string kEnableBoat{"enableBoat"};
    const std::string kEnableTrafficLight{"enableTrafficLight"};
    const std::string kEnableFireHydrant{"enableFireHydrant"};
    const std::string kEnableStopSign{"enableStopSign"};
    const std::string kEnableParkingMeter{"enableParkingMeter"};
    const std::string kEnableBench{"enableBench"};
    const std::string kEnableBird{"enableBird"};
    const std::string kEnableCat{"enableCat"};
	const std::string kEnableDog{"enableDog"};	
	const std::string kEnableHorse{"enableHorse"};
    const std::string kEnableSheep{"enableSheep"};
    const std::string kEnableCow{"enableCow"};
    const std::string kEnableElephant{"enableElephant"};
    const std::string kEnableBear{"enableBear"};
    const std::string kEnableBZebra{"enableZebra"};
    const std::string kEnableBGiraffe{"enableGiraffe"};    
	const std::string kEnableBackpack{"enableBackpack"};
    const std::string kEnableUmbrella{"enableUmbrella"};   
	const std::string kEnableHandbag{"enableHandbag"};
    const std::string kEnableTie{"enableTie"};
    const std::string kEnableSuitcase{"enableSuitcase"};   
	const std::string kEnableFrisbee{"enableFrisbee"};
    const std::string kEnableSkis{"enableSkis"};
    const std::string kEnableSnowboard{"enableSnowboard"};
    const std::string kEnableSportsBall{"enableSportsBall"};
    const std::string kEnableKite{"enableKite"};
    const std::string kEnableBaseballBat{"enableBaseballBat"};
    const std::string kEnableBaseballGlove{"enableBaseballGlove"};
    const std::string kEnableSkateboard{"enableSkateboard"};
    const std::string kEnableSurfboard{"enableSurfboard"};
    const std::string kEnableTennisRacket{"enableTennisRacket"};
    const std::string kEnableBottle{"enableBottle"};
    const std::string kEnableWineGlass{"enableWineGlass"};
    const std::string kEnableCup{"enableCup"};
    const std::string kEnableFork{"enableFork"};
    const std::string kEnableKnife{"enableKnife"};
    const std::string kEnableSpoon{"enableSpoon"};
    const std::string kEnableBowl{"enableBowl"};
    const std::string kEnableBanana{"enableBanana"};
	const std::string kEnableApple{"enableApple"};
	const std::string kEnableSandwich{"enableSandwich"};
    const std::string kEnableOrange{"enableOrange"};
	const std::string kEnableBroccoli{"enableBroccoli"};
    const std::string kEnableCarrot{"enableCarrot"};
    const std::string kEnableHotdog{"enableHotdog"};
    const std::string kEnablePizza{"enablePizza"};
	const std::string kEnableDonut{"enableDonut"};
    const std::string kEnableCake{"enableCake"};
    const std::string kEnableChair{"enableChair"};
    const std::string kEnableSofa{"enableSofa"};
    const std::string kEnablePottedplant{"enablePottedplant"};
    const std::string kEnableBed{"enableBed"};
    const std::string kEnableDiningtable{"enableDiningtable"};
    const std::string kEnableToilet{"enableToilet"};
    const std::string kEnableTvmonitor{"enableTvmonitor"};
    const std::string kEnableLaptop{"enableLaptop"};
    const std::string kEnableMouse{"enableMouse"};
    const std::string kEnableRemote{"enableRemote"};
    const std::string kEnableKeyboard{"enableKeyboard"};
    const std::string kEnableCellPhone{"enableCellPhone"};
    const std::string kEnableMicrowave{"enableMicrowave"};
    const std::string kEnableOven{"enableOven"};
    const std::string kEnableToaster{"enableToaster"};
    const std::string kEnableSink{"enableSink"};
    const std::string kEnableRefrigerator{"enableRefrigerator"};
    const std::string kEnableBook{"enableBook"};
    const std::string kEnableClock{"enableClock"};
    const std::string kEnableVase{"enableVase"};
    const std::string kEnableScissors{"enableScissors"};
    const std::string kEnableTeddyBear{"enableTeddyBear"};
    const std::string kEnableHairDrier{"enableHairDrier"};
    const std::string kEnableToothbrush{"enableToothbrush"};

} // namespace hailo_object_count
} // namespace analytics
} // namespace vms_server_plugins
} // namespace nx
