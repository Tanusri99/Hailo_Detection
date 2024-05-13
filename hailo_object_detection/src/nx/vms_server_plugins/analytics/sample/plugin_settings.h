#pragma once

#include <iostream>
#include <vector>
#include <set>
#include <algorithm>

namespace nx {
namespace vms_server_plugins {
namespace analytics {
namespace hailo_object_detection {	

	//Object types
    const std::string kPersonObjectType = "nx.hailo_object_detection.person";
    const std::string kBicycleObjectType = "nx.hailo_object_detection.bicycle";
    const std::string kCarObjectType = "nx.hailo_object_detection.car";
    const std::string kMotorbikeObjectType = "nx.hailo_object_detection.motorbike";
    const std::string kAeroplaneObjectType = "nx.hailo_object_detection.aeroplane";
    const std::string kBusObjectType = "nx.hailo_object_detection.bus";
    const std::string kTrainObjectType = "nx.hailo_object_detection.train";
    const std::string kTruckObjectType = "nx.hailo_object_detection.truck";
    const std::string kBoatObjectType = "nx.hailo_object_detection.boat";
    const std::string kTrafficLightObjectType = "nx.hailo_object_detection.traffic_light";
    const std::string kFireHydrantObjectType = "nx.hailo_object_detection.firehy_drant";
    const std::string kStopSignObjectType = "nx.hailo_object_detection.stop_sign";
    const std::string kParkingMeterObjectType = "nx.hailo_object_detection.parking_meter";
    const std::string kBenchObjectType = "nx.hailo_object_detection.bench";
    const std::string kBirdObjectType = "nx.hailo_object_detection.bird";
    const std::string kCatObjectType = "nx.hailo_object_detection.cat";
    const std::string kDogObjectType = "nx.hailo_object_detection.dog";
    const std::string kHorseObjectType = "nx.hailo_object_detection.horse";
    const std::string kSheepObjectType = "nx.hailo_object_detection.sheep";
    const std::string kCowObjectType = "nx.hailo_object_detection.cow";
    const std::string kElephantObjectType = "nx.hailo_object_detection.elephant";
    const std::string kBearObjectType = "nx.hailo_object_detection.bear";
    const std::string kZebraObjectType = "nx.hailo_object_detection.zebra";
    const std::string kGiraffeObjectType = "nx.hailo_object_detection.giraffe";
    const std::string kBackpackObjectType = "nx.hailo_object_detection.backpack";
    const std::string kUmbrellaObjectType = "nx.hailo_object_detection.umbrella";
    const std::string kHandbagObjectType = "nx.hailo_object_detection.handbag";
    const std::string kTieObjectType = "nx.hailo_object_detection.tie";
    const std::string kSuitcaseObjectType = "nx.hailo_object_detection.suitcase";
    const std::string kFrisbeeObjectType = "nx.hailo_object_detection.frisbee";
    const std::string kSkisObjectType = "nx.hailo_object_detection.skis";
    const std::string kSnowboardObjectType = "nx.hailo_object_detection.snowbaord";
    const std::string kSportsBallObjectType = "nx.hailo_object_detection.sportsball";
    const std::string kKiteObjectType = "nx.hailo_object_detection.kite";
    const std::string kBaseballBatObjectType = "nx.hailo_object_detection.baseball_bat";
    const std::string kBaseballGloveObjectType = "nx.hailo_object_detection.baseball_glove";
    const std::string kSkateboardObjectType = "nx.hailo_object_detection.skateboard";
    const std::string kSurfboardObjectType = "nx.hailo_object_detection.surfboard";
    const std::string kTennisRacketObjectType = "nx.hailo_object_detection.tennis_rackets";
    const std::string kBottleObjectType = "nx.hailo_object_detection.bottle";
    const std::string kWineGlassObjectType = "nx.hailo_object_detection.wineglass";
    const std::string kCupObjectType = "nx.hailo_object_detection.cup";
    const std::string kForkObjectType = "nx.hailo_object_detection.fork";
    const std::string kKnifeObjectType = "nx.hailo_object_detection.knife";
    const std::string kSpoonObjectType = "nx.hailo_object_detection.spoon";
    const std::string kBowlObjectType = "nx.hailo_object_detection.bowl";
    const std::string kBananaObjectType = "nx.hailo_object_detection.banana";
    const std::string kAppleObjectType = "nx.hailo_object_detection.apple";
    const std::string kSandwichObjectType = "nx.hailo_object_detection.sandwich";
    const std::string kOrangeObjectType = "nx.hailo_object_detection.orange";
    const std::string kBroccoliObjectType = "nx.hailo_object_detection.broccoli";
    const std::string kCarrotObjectType = "nx.hailo_object_detection.carrot";
    const std::string kHotdogObjectType = "nx.hailo_object_detection.hotdog";
    const std::string kPizzaObjectType = "nx.hailo_object_detection.pizza";
    const std::string kDonutObjectType = "nx.hailo_object_detection.donut";
    const std::string kCakeObjectType = "nx.hailo_object_detection.cake";
    const std::string kChairObjectType = "nx.hailo_object_detection.chair";
    const std::string kSofaObjectType = "nx.hailo_object_detection.sofa";
    const std::string kPottedplantObjectType = "nx.hailo_object_detection.potted_plant";
    const std::string kBedObjectType = "nx.hailo_object_detection.bed";
    const std::string kDiningtableObjectType = "nx.hailo_object_detection.dinning_table";
    const std::string kToiletObjectType = "nx.hailo_object_detection.toilet";
    const std::string kTvmonitorObjectType = "nx.hailo_object_detection.tv_monitor";
    const std::string kLaptopObjectType = "nx.hailo_object_detection.laptop";
    const std::string kMouseObjectType = "nx.hailo_object_detection.mouse";
    const std::string kRemoteObjectType = "nx.hailo_object_detection.remote";
    const std::string kKeyboardObjectType = "nx.hailo_object_detection.keyboard";
    const std::string kCellPhoneObjectType = "nx.hailo_object_detection.cell_phone";
    const std::string kMicrowaveObjectType = "nx.hailo_object_detection.microwave";
    const std::string kOvenObjectType = "nx.hailo_object_detection.oven";
    const std::string kToasterObjectType = "nx.hailo_object_detection.toaster";
    const std::string kSinkObjectType = "nx.hailo_object_detection.sink";
    const std::string kRefrigeratorObjectType = "nx.hailo_object_detection.refrigerator";
    const std::string kBookObjectType = "nx.hailo_object_detection.book";
    const std::string kClockObjectType = "nx.hailo_object_detection.clock";
    const std::string kVaseObjectType = "nx.hailo_object_detection.vase";
    const std::string kScissorsObjectType = "nx.hailo_object_detection.scissor";
    const std::string kTeddyBearObjectType = "nx.hailo_object_detection.teddy_bear";
    const std::string kHairDrierObjectType = "nx.hailo_object_detection.hair_drier";
    const std::string kToothbrushObjectType = "nx.hailo_object_detection.toothbrush";

    //Object event type
    const std::string kNewObjectEventType = "nx.hailo_object_detection.newObject";

    // Settings
    const std::string kConfidenceThreshold{ "nx.hailo_object_detection.confidenceThreshold" };
    const std::string kDefaultConfidenceThreshold{ "0.8" };
	const std::string kHailoServerAddress{ "nx.hailo_object_detection.hailoServerAddress" };
	const std::string kDefaultHailoServerAddress{ "http://127.0.0.1:47760" };

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

} // namespace hailo_object_detection
} // namespace analytics
} // namespace vms_server_plugins
} // namespace nx
